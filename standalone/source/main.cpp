#include <server/server.h>
#include <server/version.h>

auto main() -> int {
  server::Server server;
  server.run();
  return 0;
}
