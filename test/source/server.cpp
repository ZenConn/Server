#include <doctest/doctest.h>
#include <server/server.h>
#include <server/version.h>

#include <string>

TEST_CASE("Server") {
  using namespace server;
  Server server;
}

TEST_CASE("Server version") {
  static_assert(std::string_view(SERVER_VERSION) == std::string_view("1.0"));
  CHECK(std::string(SERVER_VERSION) == std::string("1.0"));
}
