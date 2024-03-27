#include <server/server.h>

using namespace server;

Server::Server() {}

void Server::run(char* argv[]) {
  std::ifstream file(argv[1]);

  if (!file.is_open()) throw ConfigNotFoundException();

  std::string config_contents;
  std::string line;
  while (std::getline(file, line)) {
    config_contents += line;
  }

  boost::json::value config = boost::json::parse(config_contents);

  Server::validates(config.as_object());

  auto const address = boost::asio::ip::make_address(config.at("host").as_string());
  auto const port = static_cast<unsigned short>(std::atoi(config.at("port").as_string().c_str()));
  auto const doc_root = std::make_shared<std::string>("public");
  auto const threads = std::max<int>(1, config.at("threads").as_int64());
  auto const mode = std::string(config.at("mode").as_string());

  boost::asio::io_context ioc{threads};

  std::string database_username{config.at("database").as_object().at("username").as_string()},
      database_password{config.at("database").as_object().at("password").as_string()},
      database_name{config.at("database").as_object().at("name").as_string()},
      database_host{config.at("database").as_object().at("host").as_string()},
      database_port{config.at("database").as_object().at("port").as_string()};

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
    boost::asio::steady_timer timer(ioc, boost::asio::chrono::nanoseconds(1));
    timer.async_wait([&](boost::system::error_code) {
      this->status = ServerStatus::SHUTDOWN;
      ioc.stop();
    });
  }

  this->status = ServerStatus::RUNNING;
  ioc.run();

  for (auto& t : v) t.join();
}
