#include <server/state.h>
#include <server/websocket_session.h>

void websocket_session::on_accept(boost::beast::error_code ec) {
  if (ec) return on_error(ec, "accept", true);

  session_ = std::make_shared<session>();
  state_->connected(session_);

  boost::json::object welcome{{"status", 202}, {"uuid", session_->get_uuid()}};
  std::string message = boost::json::serialize(welcome);

  ws_.async_write(
      boost::asio::buffer(message),
      boost::beast::bind_front_handler(&websocket_session::on_write, shared_from_this()));
}

void websocket_session::do_read() {
  ws_.async_read(buffer_,
                 boost::beast::bind_front_handler(&websocket_session::on_read, shared_from_this()));
}

void websocket_session::on_read(boost::beast::error_code ec, std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  if (ec == boost::beast::websocket::error::closed) return on_error(ec, "closed", true);

  if (ec) on_error(ec, "read", false);

  ws_.text(ws_.got_text());
  ws_.async_write(buffer_.data(), boost::beast::bind_front_handler(&websocket_session::on_write,
                                                                   shared_from_this()));
}

void websocket_session::on_write(boost::beast::error_code ec, std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  if (ec) return on_error(ec, "write", true);

  buffer_.consume(buffer_.size());

  do_read();
}

void websocket_session::on_error(boost::system::error_code ec, char const* what,
                                 bool disconnected) {
  if (disconnected) state_->disconnected(session_);
  failure::handle(ec, what);
}