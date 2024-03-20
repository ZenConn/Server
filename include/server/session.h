#pragma once

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <memory>

class session : public std::enable_shared_from_this<session> {
  boost::uuids::uuid uuid_;

public:
  session() : uuid_(boost::uuids::random_generator()()) {}
  std::string get_uuid() { return boost::uuids::to_string(uuid_); }
};
