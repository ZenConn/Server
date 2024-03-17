#include <server/server.h>
#include <server/version.h>

#include <iostream>

auto main(int argc, char* argv[]) -> int {
  if (argc != 6) {
    std::cerr << "Usage: advanced-server <address> <port> <doc_root> <threads> <run|check>\n"
              << "Example:\n"
              << "   Server 0.0.0.0 8080 . 1 check\n";
    return EXIT_FAILURE;
  }

  server::Server server;
  server.run(argv);
  return EXIT_SUCCESS;
}
