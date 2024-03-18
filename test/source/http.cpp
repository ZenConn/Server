#include <doctest/doctest.h>
#include <server/http.h>

TEST_CASE("Http::mime_type") {
  boost::beast::string_view output = http::mime_type("hello");
  CHECK_EQ(std::string(output), std::string("application/text"));
}

TEST_CASE("Http::path_cat") {
  std::string output = http::path_cat("/server", "/manifest.json");
  CHECK_EQ(output, "/server/manifest.json");
}


