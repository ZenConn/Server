#pragma once

#include <boost/beast.hpp>
#include <boost/json.hpp>
#include <boost/json/parser.hpp>
#include <boost/json/serialize.hpp>
#include <boost/json/value.hpp>
#include <fstream>
#include <iostream>
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
      std::vector<std::tuple<bool, std::string>> errors;

      errors.emplace_back(!config.contains("host") || !config.at("host").is_string(),
                          "'host' is required and must be an string.");

      errors.emplace_back(!config.contains("port") || !config.at("port").is_string(),
                          "'port' is required and must be an string.");

      errors.emplace_back(!config.contains("threads") || !config.at("threads").is_number(),
                          "'threads' is required and must be a number.");

      errors.emplace_back(!config.contains("mode") || !config.at("mode").is_string(),
                          "'mode' is required and must be an string.");

      errors.emplace_back(!config.contains("database") || !config.at("database").is_object(),
                          "'database' is required and must be an object.");

      errors.emplace_back(!config.at("database").as_object().contains("name")
                              || !config.at("database").as_object().at("name").is_string(),
                          "'database.name' is required and must be an string.");

      errors.emplace_back(!config.at("database").as_object().contains("username")
                              || !config.at("database").as_object().at("username").is_string(),
                          "'database.username' is required and must be an string.");

      errors.emplace_back(!config.at("database").as_object().contains("password")
                              || !config.at("database").as_object().at("password").is_string(),
                          "'database.password' is required and must be an string.");

      errors.emplace_back(!config.at("database").as_object().contains("host")
                              || !config.at("database").as_object().at("host").is_string(),
                          "'database.host' is required and must be an string.");

      errors.emplace_back(!config.at("database").as_object().contains("port")
                              || !config.at("database").as_object().at("port").is_string(),
                          "'database.port' is required and must be an string.");

      errors.emplace_back(!config.at("database").as_object().contains("connections")
                              || !config.at("database").as_object().at("connections").is_number(),
                          "'database.connections' is required and must be a number.");

      for (auto item : errors) {
        if (std::get<0>(item)) throw ConfigValidationException();
      }
    };
  };

}  // namespace server
