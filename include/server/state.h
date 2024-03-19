#pragma once

#include <memory>

#include "websocket_session.h"
#include "session.h"

class state : public std::enable_shared_from_this<state> {
  std::vector<std::shared_ptr<session>> sessions_;

public:
  state() {}

  void connected(std::shared_ptr<session> & session) {
    this->sessions_.push_back(session);
  }

  void disconnected(std::shared_ptr<session> & session) {
    std::remove(this->sessions_.begin(), this->sessions_.end(), session);
  }
};