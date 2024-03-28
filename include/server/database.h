#pragma once

#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <boost/mysql.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <iostream>
#include <stack>
#include <thread>
#include <unordered_set>

#include "session.h"

class database_connection {
  std::shared_ptr<boost::mysql::tcp_ssl_connection> connection_;
  boost::asio::io_context ioc_;
  boost::asio::ssl::context ssl_ioc_;

public:
  std::shared_ptr<std::mutex> ac_guard_;
  std::shared_ptr<std::mutex> op_guard_;
  bool locked = false;
  explicit database_connection(boost::json::object& config)
      : ioc_(true),
        ssl_ioc_(boost::asio::ssl::context::tls_client),
        ac_guard_(std::make_shared<std::mutex>()),
        op_guard_(std::make_shared<std::mutex>()) {
    connection_ = std::make_shared<boost::mysql::tcp_ssl_connection>(ioc_.get_executor(), ssl_ioc_);
    boost::asio::ip::tcp::resolver database_resolver(ioc_.get_executor());
    boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> database_endpoint
        = database_resolver.resolve(config.at("database").as_object().at("host").as_string(),
                                    config.at("database").as_object().at("port").as_string());

    boost::mysql::handshake_params database_params(
        config.at("database").as_object().at("username").as_string(),
        config.at("database").as_object().at("password").as_string(),
        config.at("database").as_object().at("name").as_string());

    try {
      connection_->connect(*database_endpoint.begin(), database_params);
    } catch (const boost::mysql::error_with_diagnostics& err) {
      std::cout << "Error: " << err.what() << '\n'
                << "Server diagnostics: " << err.get_diagnostics().server_message() << std::endl;
    }
  }

  void release() { this->locked = false; }
  void lock() { this->locked = true; }
  std::shared_ptr<boost::mysql::tcp_ssl_connection> get() { return connection_; }
};

class database {
  boost::json::object config_;
  std::stack<std::shared_ptr<database_connection>> connections_;
  std::unordered_set<std::shared_ptr<database_connection>> instances_;
  std::mutex guard_;

public:
  explicit database(boost::json::object& config)
      : config_(config), connections_(), guard_(std::mutex()) {}

  std::shared_ptr<database_connection> create_connection() {
    return std::make_shared<database_connection>(config_);
  }

  void prepare_connections(uint64_t connections) {
    for (std::size_t position = 0; position <= connections; position++) {
      instances_.insert(create_connection());
    }
  }

  database_connection* get_connection() {
    for (auto& conn : instances_) {
      std::scoped_lock<std::mutex> guard(*conn->ac_guard_);
      if (!conn->locked) {
        conn->lock();
        return conn.get();
      }
    }
    auto conn = create_connection();
    instances_.insert(conn);
    return conn.get();
  }

  void server_start(boost::uuids::uuid& uuid) {
    auto conn = this->get_connection();
    std::scoped_lock<std::mutex> guard(*conn->op_guard_);
    std::string query = "INSERT INTO servers (uuid) VALUES (?)";
    auto statement = conn->get()->prepare_statement(query);
    boost::mysql::results result;
    conn->get()->execute(statement.bind(boost::uuids::to_string(uuid)), result);
    conn->get()->close_statement(statement);
    conn->release();
  }

  void server_shutdown(boost::uuids::uuid& uuid) {
    auto conn = this->get_connection();
    std::scoped_lock<std::mutex> guard(*conn->op_guard_);
    std::string query = "UPDATE servers SET shutdown_at = now() WHERE uuid = ?";
    auto statement = conn->get()->prepare_statement(query);
    boost::mysql::results result;
    conn->get()->execute(statement.bind(boost::uuids::to_string(uuid)), result);
    conn->get()->close_statement(statement);
    conn->release();
  }

  void add_session(std::shared_ptr<session>& session) {
    auto conn = this->get_connection();
    std::scoped_lock<std::mutex> guard(*conn->op_guard_);
    std::string query = "INSERT INTO sessions (uuid) VALUES (?)";
    auto statement = conn->get()->prepare_statement(query);
    boost::mysql::results result;
    conn->get()->execute(statement.bind(session->get_uuid()), result);
    conn->get()->close_statement(statement);
    conn->release();
  }

  void remove_session(std::shared_ptr<session>& session) {
    auto conn = this->get_connection();
    std::scoped_lock<std::mutex> guard(*conn->op_guard_);
    std::string query = "UPDATE sessions SET disconnected_at = now() WHERE uuid = ?";
    auto statement = conn->get()->prepare_statement(query);
    boost::mysql::results result;
    conn->get()->execute(statement.bind(session->get_uuid()), result);
    conn->get()->close_statement(statement);
    conn->release();
  }
};