#pragma once

#include <boost/beast.hpp>
#include <memory>

#include "http.h"
#include "websocket_session.h"

class http_session : public std::enable_shared_from_this<http_session> {
  boost::beast::tcp_stream stream_;
  boost::beast::flat_buffer buffer_;
  std::shared_ptr<std::string const> doc_root_;
  static constexpr std::size_t queue_limit = 8;  // max responses
  std::vector<boost::beast::http::message_generator> response_queue_;
  boost::optional<boost::beast::http::request_parser<boost::beast::http::string_body>> parser_;

public:
  http_session(boost::asio::ip::tcp::socket&& socket,
               std::shared_ptr<std::string const> const& doc_root);
  void run();

private:
  void do_read();
  void on_read(boost::beast::error_code ec, std::size_t bytes_transferred);
  void queue_write(boost::beast::http::message_generator response);
  bool do_write();
  void on_write(bool keep_alive, boost::beast::error_code ec, std::size_t bytes_transferred);
  void do_close();
};