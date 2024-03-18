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

TEST_CASE("Server respond http request") {
  using namespace server;
  Server server;
  char arg_1[] = "0.0.0.0";
  char arg_2[] = "9000";
  char arg_3[] = ".";
  char arg_4[] = "1";
  char arg_5[] = "run";
  char* argv[] = {nullptr, arg_1, arg_2, arg_3, arg_4, arg_5};

  auto thread = std::thread([&]() { server.run(argv); });
  thread.detach();

  boost::asio::io_context ioc;

  // These objects perform our I/O
  boost::asio::ip::tcp::resolver resolver(ioc);
  boost::beast::tcp_stream stream(ioc);

  // Look up the domain name
  auto const results = resolver.resolve("localhost", arg_2);

  // Make the connection on the IP address we get from a lookup
  stream.connect(results);

  // Set up an HTTP GET request message
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,
                                                                   "/not-found", 10};
  req.set(boost::beast::http::field::host, "localhost");
  req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  // Send the HTTP request to the remote host
  boost::beast::http::write(stream, req);

  // This buffer is used for reading and must be persisted
  boost::beast::flat_buffer buffer;

  // Declare a container to hold the response
  boost::beast::http::response<boost::beast::http::dynamic_body> res;

  // Receive the HTTP response
  boost::beast::http::read(stream, buffer, res);

  res.prepare_payload();

  auto output = boost::beast::buffers_to_string(res.body().data());

  CHECK_EQ(output, std::string("The resource '/not-found' was not found."));

  boost::beast::error_code ec;
  stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

  // not_connected happens sometimes
  // so don't bother reporting it.
  //
  if (ec && ec != boost::beast::errc::not_connected) throw boost::beast::system_error{ec};
}
