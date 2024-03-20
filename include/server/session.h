#pragma once

#include <boost/uuid/uuid_generators.hpp>
#include <memory>

class session : public std::enable_shared_from_this<session> {
public:
  boost::uuids::uuid uuid_;
  session() : uuid_(boost::uuids::random_generator()()) {}
};
