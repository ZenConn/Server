#pragma once

#include <server/version.h>

#include <boost/beast.hpp>

#include "failure.h"
#include "session.h"

class state;

class websocket_session : public std::enable_shared_from_this<websocket_session> {
  boost::beast::websocket::stream<boost::beast::tcp_stream> ws_;
  boost::beast::flat_buffer buffer_;
  std::shared_ptr<state> state_;
  std::shared_ptr<session> session_;

public:
  explicit websocket_session(boost::asio::ip::tcp::socket&& socket,
                             std::shared_ptr<state> const& state)
      : ws_(std::move(socket)), state_(state) {}
  template <class Body, class Allocator> void do_accept(
      boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req) {
    ws_.set_option(
        boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::server));

    ws_.set_option(boost::beast::websocket::stream_base::decorator(
        [](boost::beast::websocket::response_type& res) {
          res.set(boost::beast::http::field::server, "ZenConn " + std::string(SERVER_VERSION));
        }));

    ws_.async_accept(
        req, boost::beast::bind_front_handler(&websocket_session::on_accept, shared_from_this()));
  }

private:
  void on_accept(boost::beast::error_code ec);
  void do_read();
  void on_read(boost::beast::error_code ec, std::size_t bytes_transferred);
  void on_write(boost::beast::error_code ec, std::size_t bytes_transferred);
  void on_error(boost::system::error_code ec, char const* what, bool disconnected);
};
