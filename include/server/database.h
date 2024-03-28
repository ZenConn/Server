#pragma once

#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <boost/mysql.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/current_function.hpp>
#include <chrono>
#include <iostream>
#include <stack>
#include <thread>
#include <unordered_set>

#include "session.h"

class database_connection {
  std::shared_ptr<boost::mysql::unix_connection> connection_;
  boost::asio::io_context ioc_;

public:
  explicit database_connection(boost::json::object& config) : ioc_(true) {
    connection_ = std::make_shared<boost::mysql::unix_connection>(ioc_);
    boost::asio::local::stream_protocol::endpoint ep(config.at("database").as_object().at("sock").as_string());
    boost::mysql::handshake_params database_params(
        config.at("database").as_object().at("username").as_string(),
        config.at("database").as_object().at("password").as_string(),
        config.at("database").as_object().at("name").as_string());

    try {
      connection_->connect(ep, database_params);
    } catch (const boost::mysql::error_with_diagnostics& err) {
      std::cout << "Error: " << err.what() << '\n'
                << "Server diagnostics: " << err.get_diagnostics().server_message() << std::endl;
    }
  }

  std::shared_ptr<boost::mysql::unix_connection> get() { return connection_; }
};

class database {
  boost::json::object config_;
  std::stack<std::shared_ptr<database_connection>> connections_;
  std::unordered_set<std::shared_ptr<database_connection>> instances_;

public:
  explicit database(boost::json::object& config) : config_(config), connections_() {}

  std::shared_ptr<database_connection> get_connection() {
    return std::make_shared<database_connection>(config_);
  }

  void server_start(boost::uuids::uuid& uuid) {
    auto conn = this->get_connection();
    try {
      std::string query = "INSERT INTO servers (uuid) VALUES (?)";
      auto statement = conn->get()->prepare_statement(query);
      boost::mysql::results result;
      conn->get()->execute(statement.bind(boost::uuids::to_string(uuid)), result);
      conn->get()->close_statement(statement);
    } catch (const boost::mysql::error_with_diagnostics& err) {
      std::cerr << BOOST_CURRENT_FUNCTION << ": " << err.what() << '\n'
                << "Server diagnostics: " << err.get_diagnostics().server_message() << std::endl;
    }
  }

  void server_shutdown(boost::uuids::uuid& uuid) {
    auto conn = this->get_connection();
    try {
      std::string query = "UPDATE servers SET shutdown_at = now() WHERE uuid = ?";
      auto statement = conn->get()->prepare_statement(query);
      boost::mysql::results result;
      conn->get()->execute(statement.bind(boost::uuids::to_string(uuid)), result);
      conn->get()->close_statement(statement);
    } catch (const boost::mysql::error_with_diagnostics& err) {
      std::cerr << BOOST_CURRENT_FUNCTION << ": " << err.what() << '\n'
                << "Server diagnostics: " << err.get_diagnostics().server_message() << std::endl;
    }
  }

  void add_session(std::shared_ptr<session>& session) {
    auto conn = this->get_connection();
    try {
      std::string query = "INSERT INTO sessions (uuid) VALUES (?)";
      auto statement = conn->get()->prepare_statement(query);
      boost::mysql::results result;
      conn->get()->execute(statement.bind(session->get_uuid()), result);
      conn->get()->close_statement(statement);
    } catch (const boost::mysql::error_with_diagnostics& err) {
      std::cerr << BOOST_CURRENT_FUNCTION << ": " << err.what() << '\n'
                << "Server diagnostics: " << err.get_diagnostics().server_message() << std::endl;
    }
  }

  void remove_session(std::shared_ptr<session>& session) {
    auto conn = this->get_connection();
    try {
      std::string query = "UPDATE sessions SET disconnected_at = now() WHERE uuid = ?";
      auto statement = conn->get()->prepare_statement(query);
      boost::mysql::results result;
      conn->get()->execute(statement.bind(session->get_uuid()), result);
      conn->get()->close_statement(statement);
    } catch (const boost::mysql::error_with_diagnostics& err) {
      std::cerr << BOOST_CURRENT_FUNCTION << ": " << err.what() << '\n'
                << "Server diagnostics: " << err.get_diagnostics().server_message() << std::endl;
    }
  }
};