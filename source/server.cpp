#include <server/server.h>

using namespace server;

Server::Server() {}

void Server::run(char* argv[]) {
  auto const address = boost::asio::ip::make_address(argv[1]);
  auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
  auto const doc_root = std::make_shared<std::string>(argv[3]);
  auto const threads = std::max<int>(1, std::atoi(argv[4]));
  auto const mode = std::string(argv[5]);

  boost::asio::io_context ioc{threads};

  std::make_shared<listener>(ioc, boost::asio::ip::tcp::endpoint{address, port}, doc_root)->run();

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
