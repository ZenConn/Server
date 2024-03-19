#include <server/state.h>
#include <server/websocket_session.h>

void websocket_session::on_accept(boost::beast::error_code ec) {
  if (ec) return failure::handle(ec, "accept");

  session_ = std::make_shared<session>();
  state_->connected(session_);

  do_read();
}

void websocket_session::do_read() {
  ws_.async_read(buffer_,
                 boost::beast::bind_front_handler(&websocket_session::on_read, shared_from_this()));
}

void websocket_session::on_read(boost::beast::error_code ec, std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  if (ec == boost::beast::websocket::error::closed) {
    state_->disconnected(session_);
    return;
  }

  if (ec) failure::handle(ec, "read");

  ws_.text(ws_.got_text());
  ws_.async_write(buffer_.data(), boost::beast::bind_front_handler(&websocket_session::on_write,
                                                                   shared_from_this()));
}

void websocket_session::on_write(boost::beast::error_code ec, std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  if (ec) {
    state_->disconnected(session_);
    return failure::handle(ec, "write");
  }

  buffer_.consume(buffer_.size());

  do_read();
}