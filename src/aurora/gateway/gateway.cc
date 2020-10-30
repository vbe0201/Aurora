#include "gateway.h"

namespace aurora {
namespace gateway {

Session::Session(boost::asio::io_context &io, boost::asio::ssl::context &ssl)
    : resolver_(io), websocket_(io, ssl) {}

}  // namespace gateway
}  // namespace aurora
