#pragma once

#include <boost/asio.hpp>
#include <boost/mysql.hpp>

class database {
  boost::asio::io_context& ioc_;
  boost::asio::ssl::context& ssl_ioc_;
  boost::mysql::handshake_params& params_;
  boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> endpoints_;

public:
  database(boost::asio::io_context& ioc, boost::asio::ssl::context& ssl_ioc,
           boost::mysql::handshake_params& params,
           boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp>& endpoints)
      : ioc_(ioc), ssl_ioc_(ssl_ioc), params_(params), endpoints_(endpoints) {}

  std::shared_ptr<boost::mysql::tcp_ssl_connection> get_connection() {
    auto conn = std::make_shared<boost::mysql::tcp_ssl_connection>(ioc_.get_executor(), ssl_ioc_);
    conn->connect(*endpoints_.begin(), params_);
    return conn;
  }
};