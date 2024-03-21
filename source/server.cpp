#include <server/dotenv.h>
#include <server/server.h>

using namespace server;

Server::Server() {}

void Server::run(char* argv[]) {
  auto const address = boost::asio::ip::make_address(argv[1]);
  auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
  auto const doc_root = std::make_shared<std::string>("public");
  auto const threads = std::max<int>(1, std::atoi(argv[3]));
  auto const mode = std::string(argv[4]);

  boost::asio::io_context ioc{threads};

  std::string database_username = dotenv::getenv("DATABASE_USERNAME", "user"),
              database_password = dotenv::getenv("DATABASE_PASSWORD", ""),
              database_name = dotenv::getenv("DATABASE_NAME", "app"),
              database_host = dotenv::getenv("DATABASE_HOST", "localhost"),
              database_port = dotenv::getenv("DATABASE_PORT", boost::mysql::default_port_string);

  boost::asio::io_context database_ioc;
  boost::asio::ssl::context database_ssl_ioc(boost::asio::ssl::context::tls_client);
  boost::asio::ip::tcp::resolver database_resolver(database_ioc.get_executor());
  boost::mysql::handshake_params database_params(database_username, database_password,
                                                 database_name);

  boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> database_endpoints
      = database_resolver.resolve(database_host, database_port);

  auto shared_state = std::make_shared<state>(database_ioc, database_ssl_ioc, database_params,
                                              database_endpoints);

  std::make_shared<listener>(ioc, boost::asio::ip::tcp::endpoint{address, port}, doc_root,
                             shared_state)
      ->run();

  boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
  signals.async_wait([&](boost::beast::error_code const&, int) { ioc.stop(); });

  std::vector<std::thread> v;
  v.reserve(threads - 1);
  for (auto i = threads - 1; i > 0; --i) v.emplace_back([&ioc] { ioc.run(); });

  if (mode == "check") {
    boost::asio::steady_timer timer(ioc, boost::asio::chrono::seconds(10));
    timer.async_wait([&](boost::system::error_code) {
      this->status = ServerStatus::SHUTDOWN;
      ioc.stop();
    });
  }

  this->status = ServerStatus::RUNNING;
  ioc.run();

  for (auto& t : v) t.join();
}
