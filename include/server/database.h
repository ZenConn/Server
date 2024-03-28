#pragma once

#include <boost/asio.hpp>
#include <boost/current_function.hpp>
#include <boost/json.hpp>
#include <boost/mysql.hpp>
#include <boost/mysql/string_view.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <stack>
#include <thread>
#include <unordered_set>

#include "session.h"

class database_connection {
  boost::asio::io_context ioc_;
  boost::asio::ssl::context ssl_ioc_;

public:
  boost::mysql::tcp_ssl_connection sock;
  explicit database_connection(boost::json::object& config)
      : ioc_(false),
        ssl_ioc_(boost::asio::ssl::context::tls_client),
        sock(ioc_.get_executor(), ssl_ioc_) {
    auto database_config = config.at("database").as_object();

    boost::asio::ip::tcp::resolver resolver(ioc_.get_executor());
    auto endpoints = resolver.resolve(database_config.at("host").as_string(),
                                      database_config.at("port").as_string());

    boost::mysql::handshake_params database_params(database_config.at("username").as_string(),
                                                   database_config.at("password").as_string(),
                                                   database_config.at("name").as_string());
    sock.connect(*endpoints.begin(), database_params);
  }
};

class database {
  boost::json::object config_;

public:
  explicit database(boost::json::object& config) : config_(config) {}

  void server_start(boost::uuids::uuid& uuid) {
    auto conn = std::make_shared<database_connection>(config_);
    boost::mysql::string_view query = "INSERT INTO servers (uuid) VALUES (?)";
    auto statement = conn->sock.prepare_statement(query);
    boost::mysql::results result;
    conn->sock.execute(statement.bind(boost::uuids::to_string(uuid)), result);
    conn->sock.close_statement(statement);
  }

  void server_shutdown(boost::uuids::uuid& uuid) {
    auto conn = std::make_shared<database_connection>(config_);
    boost::mysql::string_view query = "UPDATE servers SET shutdown_at = now() WHERE uuid = ?";
    auto statement = conn->sock.prepare_statement(query);
    boost::mysql::results result;
    conn->sock.execute(statement.bind(boost::uuids::to_string(uuid)), result);
    conn->sock.close_statement(statement);
  }

  void add_session(std::shared_ptr<session>& session) {
    auto conn = std::make_shared<database_connection>(config_);
    boost::mysql::string_view query = "INSERT INTO sessions (uuid) VALUES (?)";
    auto statement = conn->sock.prepare_statement(query);
    boost::mysql::results result;
    conn->sock.execute(statement.bind(session->get_uuid()), result);
    conn->sock.close_statement(statement);
  }

  void remove_session(std::shared_ptr<session>& session) {
    auto conn = std::make_shared<database_connection>(config_);
    boost::mysql::string_view query = "UPDATE sessions SET disconnected_at = now() WHERE uuid = ?";
    auto statement = conn->sock.prepare_statement(query);
    boost::mysql::results result;
    conn->sock.execute(statement.bind(session->get_uuid()), result);
    conn->sock.close_statement(statement);
  }
};