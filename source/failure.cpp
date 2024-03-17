#include <server/failure.h>

void failure::handle(boost::beast::error_code ec, char const* what) {
  std::cerr << what << ": " << ec.message() << std::endl;
}