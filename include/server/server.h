#pragma once

#include <boost/beast.hpp>
#include <string>

#include "listener.h"

enum ServerStatus {
  BOOT,
  RUNNING,
  SHUTDOWN,
};

namespace server {
  class Server {
  public:
    ServerStatus status = ServerStatus::BOOT;
    Server();

    void run(char* argv[]);
  };

}  // namespace server
