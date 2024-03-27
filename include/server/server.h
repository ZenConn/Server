#pragma once

#include <boost/beast.hpp>
#include <boost/json.hpp>
#include <boost/json/parser.hpp>
#include <boost/json/serialize.hpp>
#include <boost/json/value.hpp>
#include <iostream>
#include <fstream>
#include <string>

#include "listener.h"
#include "state.h"

class ConfigNotFoundException : std::exception {};
class ConfigValidationException : std::exception {};

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
    void static validates(boost::json::object& config) {
      if (!config.contains("host") || !config.at("host").is_string()) {
        std::cout << "'host' is required and must be an string." << '\n';
        throw ConfigValidationException();
      }

      if (!config.contains("port") || !config.at("port").is_string()) {
        std::cout << "'port' is required and must be an string." << '\n';
        throw ConfigValidationException();
      }

      if (!config.contains("threads") || !config.at("threads").is_number()) {
        std::cout << "'threads' is required and must be a number." << '\n';
        throw ConfigValidationException();
      }

      if (!config.contains("mode") || !config.at("mode").is_string()) {
        std::cout << "'mode' is required and must be an string." << '\n';
        throw ConfigValidationException();
      }

      if (!config.contains("database") || !config.at("database").is_object()) {
        std::cout << "'database' is required and must be an object." << '\n';
        throw ConfigValidationException();
      }

      if (!config.at("database").as_object().contains("name")
          || !config.at("database").as_object().at("name").is_string()) {
        std::cout << "'database.name' is required and must be an string." << '\n';
        throw ConfigValidationException();
      }

      if (!config.at("database").as_object().contains("username")
          || !config.at("database").as_object().at("username").is_string()) {
        std::cout << "'database.username' is required and must be an string." << '\n';
        throw ConfigValidationException();
      }

      if (!config.at("database").as_object().contains("password")
          || !config.at("database").as_object().at("password").is_string()) {
        std::cout << "'database.password' is required and must be an string." << '\n';
        throw ConfigValidationException();
      }

      if (!config.at("database").as_object().contains("host")
          || !config.at("database").as_object().at("host").is_string()) {
        std::cout << "'database.host' is required and must be an string." << '\n';
        throw ConfigValidationException();
      }

      if (!config.at("database").as_object().contains("port")
          || !config.at("database").as_object().at("port").is_string()) {
        std::cout << "'database.port' is required and must be an string." << '\n';
        throw ConfigValidationException();
      }
    };
  };

}  // namespace server
