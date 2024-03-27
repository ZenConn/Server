#pragma once

#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <boost/mysql.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

class database_connection {
  std::shared_ptr<boost::mysql::tcp_ssl_connection> connection_;

public:
  std::shared_ptr<std::mutex> ac_guard_;
  std::shared_ptr<std::mutex> op_guard_;
  bool locked = false;
  explicit database_connection(std::shared_ptr<boost::mysql::tcp_ssl_connection> connection)
      : connection_(std::move(connection)),
        ac_guard_(std::make_shared<std::mutex>()),
        op_guard_(std::make_shared<std::mutex>()) {}

  void release() { this->locked = false; }
  void lock() { this->locked = true; }
  std::shared_ptr<boost::mysql::tcp_ssl_connection> get() { return connection_; }
};

class database {
  boost::asio::io_context& ioc_;
  boost::asio::ssl::context& ssl_ioc_;
  boost::mysql::handshake_params& params_;
  boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> endpoints_;
  std::vector<std::shared_ptr<database_connection>> connections_;

public:
  database(boost::asio::io_context& ioc, boost::asio::ssl::context& ssl_ioc,
           boost::mysql::handshake_params& params,
           boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp>& endpoints)
      : ioc_(ioc), ssl_ioc_(ssl_ioc), params_(params), endpoints_(endpoints) {}

  std::shared_ptr<database_connection> create_connection() {
    auto conn = std::make_shared<boost::mysql::tcp_ssl_connection>(ioc_.get_executor(), ssl_ioc_);
    conn->connect(*endpoints_.begin(), params_);
    return std::make_shared<database_connection>(conn);
  }

  void prepare_connections(uint64_t connections) {
    for (std::size_t position = 0; position < connections; position++) {
      connections_.emplace_back(create_connection());
    }
  }

  std::shared_ptr<database_connection> get_connection() {
    for (auto& conn : connections_) {
      std::scoped_lock<std::mutex> guard(*conn->ac_guard_);
      if (!conn->locked) {
        conn->lock();
        return conn;
      }
    }

    auto conn = create_connection();
    conn->lock();
    connections_.emplace_back(conn);
    return conn;
  }

  void insert_server(boost::uuids::uuid& uuid) {
    auto conn = get_connection();
    std::scoped_lock<std::mutex> guard(*conn->op_guard_);
    std::string query = "INSERT INTO servers (uuid) VALUES (?)";
    auto statement = conn->get()->prepare_statement(query);
    boost::mysql::results result;
    conn->get()->execute(statement.bind(boost::uuids::to_string(uuid)), result);
    conn->release();
  }
};