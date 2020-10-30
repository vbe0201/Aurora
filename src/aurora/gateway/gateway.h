// Copyright (C) 2020  Valentin B.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

/**
 * @file gateway.h
 * @brief Implementation of the Discord gateway protocol over WebSockets.
 * @author Valentin B.
 * @see https://discord.com/developers/docs/topics/gateway
 */

#ifndef AURORA_GATEWAY_GATEWAY_H_
#define AURORA_GATEWAY_GATEWAY_H_

#include <memory>

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

namespace aurora {
namespace gateway {

// Abbreviate the boost namespaces.
namespace beast = boost::beast;
namespace net = boost::asio;
namespace websocket = beast::websocket;

/**
 * Implementation of Discord's gateway sessions on top of the WebSocket
 * protocol.
 *
 * The gateway is responsible for exchanging stateful information and
 * dispatching events to the client in real-time.
 *
 * A gateway session runs on a single shard and utilizes zlib and ETF for
 * exchanging data over the underlying WebSocket. It concurrently processes the
 * events of up to 100 guilds and talks to other components to inform them about
 * state changes.
 */
class Session : public std::enable_shared_from_this<Session> {
 public:
  explicit Session(boost::asio::io_context& io, boost::asio::ssl::context& ssl);

 private:
  boost::asio::ip::tcp::resolver resolver_;
  websocket::stream<beast::ssl_stream<net::ip::tcp::socket>> websocket_;
  boost::beast::flat_buffer buffer_;
};

}  // namespace gateway
}  // namespace aurora

#endif  // AURORA_GATEWAY_GATEWAY_H_
