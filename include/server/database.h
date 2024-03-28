#pragma once

#include <boost/asio.hpp>
#include <boost/current_function.hpp>
#include <boost/json.hpp>
#include <boost/mysql.hpp>
#include <boost/mysql/string_view.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <iostream>
#include <fstream>
#include <stack>
#include <thread>
#include <unordered_set>

#include "session.h"

class error {
public:
  void static print(const boost::mysql::error_with_diagnostics& error) {
    std::string name = boost::uuids::to_string(boost::uuids::random_generator()()) + ".error_log";
    std::ofstream outfile(name.data());
    outfile << BOOST_CURRENT_FUNCTION << ": " << error.what() << '\n'
              << "Server diagnostics: " << error.get_diagnostics().server_message() << std::endl;
    outfile.close();
  }
};

class database_connection {
  boost::asio::io_context ioc_;

public:
  boost::mysql::unix_connection sock;
  explicit database_connection(boost::json::object& config) : ioc_(false), sock(ioc_) {
    auto database_config = config.at("database").as_object();
    boost::asio::local::stream_protocol::endpoint ep(database_config.at("sock").as_string());
    boost::mysql::handshake_params database_params(database_config.at("username").as_string(),
                                                   database_config.at("password").as_string(),
                                                   database_config.at("name").as_string());
    try {
      sock.connect(ep, database_params);
    } catch (const boost::mysql::error_with_diagnostics& err) {
      error::print(err);
    }
  }
};

class database {
  boost::json::object config_;

public:
  explicit database(boost::json::object& config) : config_(config) {}

  void server_start(boost::uuids::uuid& uuid) {
    auto conn = std::make_shared<database_connection>(config_);
    try {
      boost::mysql::string_view query = "INSERT INTO servers (uuid) VALUES (?)";
      auto statement = conn->sock.prepare_statement(query);
      boost::mysql::results result;
      conn->sock.execute(statement.bind(boost::uuids::to_string(uuid)), result);
      conn->sock.close_statement(statement);
    } catch (const boost::mysql::error_with_diagnostics& err) {
      error::print(err);
    }
  }

  void server_shutdown(boost::uuids::uuid& uuid) {
    auto conn = std::make_shared<database_connection>(config_);
    try {
      boost::mysql::string_view query = "UPDATE servers SET shutdown_at = now() WHERE uuid = ?";
      auto statement = conn->sock.prepare_statement(query);
      boost::mysql::results result;
      conn->sock.execute(statement.bind(boost::uuids::to_string(uuid)), result);
      conn->sock.close_statement(statement);
    } catch (const boost::mysql::error_with_diagnostics& err) {
      error::print(err);
    }
  }

  void add_session(std::shared_ptr<session>& session) {
    auto conn = std::make_shared<database_connection>(config_);
    try {
      boost::mysql::string_view query = "INSERT INTO sessions (uuid) VALUES (?)";
      auto statement = conn->sock.prepare_statement(query);
      boost::mysql::results result;
      conn->sock.execute(statement.bind(session->get_uuid()), result);
      conn->sock.close_statement(statement);
    } catch (const boost::mysql::error_with_diagnostics& err) {
      error::print(err);
    }
  }

  void remove_session(std::shared_ptr<session>& session) {
    auto conn = std::make_shared<database_connection>(config_);
    try {
      boost::mysql::string_view query
          = "UPDATE sessions SET disconnected_at = now() WHERE uuid = ?";
      auto statement = conn->sock.prepare_statement(query);
      boost::mysql::results result;
      conn->sock.execute(statement.bind(session->get_uuid()), result);
      conn->sock.close_statement(statement);
    } catch (const boost::mysql::error_with_diagnostics& err) {
      error::print(err);
    }
  }
};