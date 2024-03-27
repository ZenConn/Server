#pragma once

#include <memory>

#include "database.h"
#include "session.h"
#include "websocket_session.h"

class state : public std::enable_shared_from_this<state> {
  boost::uuids::uuid uuid_;
  boost::json::object config_;
  database database_;
  std::vector<std::shared_ptr<session>> sessions_;

public:
  state(boost::json::object& config, boost::asio::io_context& database_ioc,
        boost::asio::ssl::context& database_ssl_ioc,
        boost::mysql::handshake_params& database_params,
        boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp>& database_endpoints)
      : uuid_(boost::uuids::random_generator()()),
        config_(config),
        database_(database_ioc, database_ssl_ioc, database_params, database_endpoints) {
    this->database_.prepare_connections(
        config_.at("database").as_object().at("connections").as_int64());
    this->database_.server_start(uuid_);
  }
  std::string get_uuid() { return boost::uuids::to_string(uuid_); }

  void connected(std::shared_ptr<session>& session) { this->sessions_.push_back(session); }

  void disconnected(std::shared_ptr<session>& session) {
    std::remove(this->sessions_.begin(), this->sessions_.end(), session);
  }

  void shutdown() { this->database_.server_shutdown(uuid_); }
};