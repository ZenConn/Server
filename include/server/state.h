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
  explicit state(boost::json::object& config)
      : uuid_(boost::uuids::random_generator()()), config_(config), database_(config) {
    this->database_.server_start(uuid_);
  }
  std::string get_uuid() { return boost::uuids::to_string(uuid_); }

  void connected(std::shared_ptr<session>& session) {
    this->sessions_.push_back(session);
    this->database_.add_session(session);
  }

  void disconnected(std::shared_ptr<session>& session) {
    std::remove(this->sessions_.begin(), this->sessions_.end(), session);
    this->database_.remove_session(session);
  }

  void shutdown() { this->database_.server_shutdown(uuid_); }
};