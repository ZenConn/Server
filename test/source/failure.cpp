#include <doctest/doctest.h>
#include <server/failure.h>

TEST_CASE("Failure") {
  boost::system::error_code ec;
  failure::handle(ec, "Some reason");
}
