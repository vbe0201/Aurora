#include "gateway/gateway.h"

int main(int argc, char** argv) {
  boost::asio::io_context ctx;
  boost::asio::ssl::context ssl{boost::asio::ssl::context::tlsv13_client};

  aurora::gateway::Session session(ctx, ssl);

  return 0;
}
