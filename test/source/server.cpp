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
  char arg_3[] = "1";
  char arg_4[] = "check";
  char* argv[] = {nullptr, arg_1, arg_2, arg_3, arg_4};
  server.run(argv);
  CHECK_EQ(server.status, ServerStatus::SHUTDOWN);
  CHECK_NE(server.status, ServerStatus::BOOT);
  CHECK_NE(server.status, ServerStatus::RUNNING);
}

TEST_CASE("Server version") {
  static_assert(std::string_view(SERVER_VERSION) == std::string_view("1.0"));
  CHECK(std::string(SERVER_VERSION) == std::string("1.0"));
}

TEST_CASE("Server can handle http requests") {
  using namespace server;
  Server server;
  char arg_1[] = "0.0.0.0";
  char arg_2[] = "9000";
  char arg_3[] = "1";
  char arg_4[] = "run";
  char* argv[] = {nullptr, arg_1, arg_2, arg_3, arg_4};

  auto thread = std::thread([&]() { server.run(argv); });
  thread.detach();

  // Wait for booting
  while (server.status != ServerStatus::RUNNING) {
  };

  boost::asio::io_context ioc;
  boost::asio::ip::tcp::resolver resolver(ioc);
  boost::beast::tcp_stream stream(ioc);

  auto const results = resolver.resolve("localhost", arg_2);

  stream.connect(results);

  boost::beast::http::request<boost::beast::http::string_body> request_get{
      boost::beast::http::verb::get, "/not-found", 10};
  request_get.set(boost::beast::http::field::host, "localhost");
  request_get.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  request_get.keep_alive(true);

  boost::beast::http::request<boost::beast::http::string_body> request_put{
      boost::beast::http::verb::put, "/not-found", 10};
  request_put.set(boost::beast::http::field::host, "localhost");
  request_put.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  request_put.keep_alive(true);

  boost::beast::http::request<boost::beast::http::string_body> request_wrong{
      boost::beast::http::verb::get, ".not-found", 10};
  request_wrong.set(boost::beast::http::field::host, "localhost");
  request_wrong.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  request_wrong.keep_alive(true);

  boost::beast::http::request<boost::beast::http::string_body> request_head{
      boost::beast::http::verb::head, "/app.json", 10};
  request_head.set(boost::beast::http::field::host, "localhost");
  request_head.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  request_head.keep_alive(true);

  boost::beast::http::request<boost::beast::http::string_body> request_file{
      boost::beast::http::verb::get, "/app.json", 10};
  request_file.set(boost::beast::http::field::host, "localhost");
  request_file.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  request_file.keep_alive(true);

  boost::beast::http::request<boost::beast::http::string_body> request_exception{
      boost::beast::http::verb::get, "/exception", 10};
  request_exception.set(boost::beast::http::field::host, "localhost");
  request_exception.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  request_exception.keep_alive(false);

  boost::beast::flat_buffer buffer;
  boost::beast::flat_buffer buffer_2;
  boost::beast::flat_buffer buffer_3;
  boost::beast::flat_buffer buffer_4;
  boost::beast::flat_buffer buffer_5;
  boost::beast::flat_buffer buffer_6;

  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  boost::beast::http::response<boost::beast::http::dynamic_body> res_2;
  boost::beast::http::response<boost::beast::http::dynamic_body> res_3;
  boost::beast::http::response<boost::beast::http::dynamic_body> res_4;
  boost::beast::http::response<boost::beast::http::dynamic_body> res_5;
  boost::beast::http::response<boost::beast::http::dynamic_body> res_6;

  boost::beast::http::write(stream, request_get);
  boost::beast::http::read(stream, buffer, res);
  auto output = boost::beast::buffers_to_string(res.body().data());
  CHECK_EQ(output, std::string("The resource '/not-found' was not found."));

  boost::beast::http::write(stream, request_put);
  boost::beast::http::read(stream, buffer_2, res_2);
  output = boost::beast::buffers_to_string(res_2.body().data());
  CHECK_EQ(output, std::string("Unknown HTTP-method"));

  boost::beast::http::write(stream, request_wrong);
  boost::beast::http::read(stream, buffer_3, res_3);
  output = boost::beast::buffers_to_string(res_3.body().data());
  CHECK_EQ(output, std::string("Illegal request-target"));

  boost::beast::http::write(stream, request_head);
  boost::beast::http::read(stream, buffer_4, res_4);
  output = boost::beast::buffers_to_string(res_4.body().data());
  CHECK_EQ(output, std::string(""));

  boost::beast::http::write(stream, request_file);
  boost::beast::http::read(stream, buffer_5, res_5);
  output = boost::beast::buffers_to_string(res_5.body().data());
  CHECK_EQ(output, std::string(""));

  boost::beast::http::write(stream, request_exception);
  boost::beast::http::read(stream, buffer_6, res_6);
  output = boost::beast::buffers_to_string(res_6.body().data());
  CHECK_EQ(output, std::string("An error occurred: 'User-triggered error'"));

  boost::beast::error_code ec;
  stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

  if (ec && ec != boost::beast::errc::not_connected) throw boost::beast::system_error{ec};
}

TEST_CASE("Server can handle websocket sessions") {
  using namespace server;
  Server server;
  char arg_1[] = "0.0.0.0";
  char arg_2[] = "9000";
  char arg_3[] = "1";
  char arg_4[] = "run";
  char* argv[] = {nullptr, arg_1, arg_2, arg_3, arg_4};

  auto thread = std::thread([&]() { server.run(argv); });
  thread.detach();

  // Wait for booting
  while (server.status != ServerStatus::RUNNING) {
  };

  boost::asio::io_context ioc;

  boost::asio::ip::tcp::resolver resolver{ioc};
  boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws{ioc};

  auto const results = resolver.resolve("localhost", "9000");

  boost::asio::connect(ws.next_layer(), results);

  ws.set_option(boost::beast::websocket::stream_base::decorator(
      [](boost::beast::websocket::request_type& req) {
        req.set(boost::beast::http::field::user_agent, std::string(BOOST_BEAST_VERSION_STRING));
      }));

  ws.handshake("localhost:9000", "/");

  ws.write(boost::asio::buffer(std::string("hello")));

  boost::beast::flat_buffer buffer;

  ws.read(buffer);
  ws.close(boost::beast::websocket::close_code::normal);

  auto output = boost::beast::buffers_to_string(buffer.data());
  CHECK_EQ(output, std::string("hello"));
}
