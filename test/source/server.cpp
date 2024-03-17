#include <doctest/doctest.h>
#include <server/server.h>
#include <server/version.h>

#include <string>

TEST_CASE("Server") {
  using namespace server;
  Server server;
}

TEST_CASE("Server check") {
  using namespace server;
  Server server;
  char arg_1[] = "0.0.0.0";
  char arg_2[] = "9000";
  char arg_3[] = ".";
  char arg_4[] = "1";
  char arg_5[] = "check";
  char* argv[] = {nullptr, arg_1, arg_2, arg_3, arg_4, arg_5};
  server.run(argv);
  CHECK_EQ(server.status, ServerStatus::SHUTDOWN);
  CHECK_NE(server.status, ServerStatus::BOOT);
  CHECK_NE(server.status, ServerStatus::RUNNING);
}

TEST_CASE("Server version") {
  static_assert(std::string_view(SERVER_VERSION) == std::string_view("1.0"));
  CHECK(std::string(SERVER_VERSION) == std::string("1.0"));
}
