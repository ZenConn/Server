#include <doctest/doctest.h>
#include <server/http.h>

TEST_CASE("Http::mime_type") {
  boost::beast::string_view output = http::mime_type("hello");
  CHECK_EQ(std::string(output), std::string("application/text"));
}

TEST_CASE("Http::mime_type with dot") {
  boost::beast::string_view output = http::mime_type("app.json");
  CHECK_EQ(std::string(output), std::string("application/json"));
}

TEST_CASE("Http::path_cat") {
  std::string output = http::path_cat("/server", "/manifest.json");

#ifdef BOOST_MSVC
  CHECK_EQ(output, "\\server\\manifest.json");
#else
  CHECK_EQ(output, "/server/manifest.json");
#endif
}
