#include <server/server.h>

using namespace server;

Server::Server() {}

void Server::run(char* argv[]) {
  std::ifstream file(argv[1]);

  if (!file.is_open()) throw ConfigNotFoundException();

  std::string config_contents;
  std::string line;
  while (std::getline(file, line)) config_contents += line;

  boost::json::value config = boost::json::parse(config_contents);

  Server::validates(config.as_object());

  auto const address = boost::asio::ip::make_address(config.at("host").as_string());
  auto const port = static_cast<unsigned short>(std::atoi(config.at("port").as_string().c_str()));
  auto const doc_root = std::make_shared<std::string>("public");
  auto const threads = std::max<int>(1, config.at("threads").as_int64());
  auto const mode = std::string(config.at("mode").as_string());

  state_ = std::make_shared<state>(config.as_object());

  std::make_shared<listener>(ioc_, boost::asio::ip::tcp::endpoint{address, port}, doc_root, state_)
      ->run();

  boost::asio::signal_set signals(ioc_, SIGINT, SIGTERM);
  signals.async_wait([&](boost::beast::error_code const&, int) { ioc_.stop(); });

  std::vector<std::thread> v;
  v.reserve(threads - 1);
  for (auto i = threads - 1; i > 0; --i) v.emplace_back([&] { ioc_.run(); });

  this->status = ServerStatus::RUNNING;
  ioc_.run();
  for (auto& t : v) t.join();
}
