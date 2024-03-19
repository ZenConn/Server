#include <server/listener.h>

listener::listener(boost::asio::io_context &ioc, boost::asio::ip::tcp::endpoint endpoint,
                   const std::shared_ptr<const std::string> &doc_root,
                   const std::shared_ptr<state> &state)
    : ioc_(ioc), acceptor_(boost::asio::make_strand(ioc)), doc_root_(doc_root), state_(state) {
  boost::beast::error_code ec;

  acceptor_.open(endpoint.protocol(), ec);
  if (ec) failure::handle(ec, "open");
  if (ec) return;

  acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
  if (ec) failure::handle(ec, "set_option");
  if (ec) return;

  acceptor_.bind(endpoint, ec);
  if (ec) failure::handle(ec, "bind");
  if (ec) return;

  acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
  if (ec) failure::handle(ec, "listen");
  if (ec) return;
}

void listener::run() {
  boost::asio::dispatch(
      acceptor_.get_executor(),
      boost::beast::bind_front_handler(&listener::do_accept, this->shared_from_this()));
}

void listener::do_accept() {
  acceptor_.async_accept(
      boost::asio::make_strand(ioc_),
      boost::beast::bind_front_handler(&listener::on_accept, shared_from_this()));
}

void listener::on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket) {
  if (ec) failure::handle(ec, "accept");
  else {
    std::make_shared<http_session>(std::move(socket), doc_root_, state_)->run();
  }

  do_accept();
}
