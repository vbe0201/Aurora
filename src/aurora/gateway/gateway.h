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

#include "common/defines.h"

namespace aurora {
namespace gateway {

// Abbreviate the boost namespaces.
namespace beast = boost::beast;
namespace http = beast::http;
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
  explicit Session(net::io_context& io, net::ssl::context& ssl);

  ~Session();

  AURORA_DISALLOW_COPY_AND_ASSIGN(Session);
  AURORA_DISALLOW_MOVE(Session);

  /**
   * @brief Establishes a WebSocket connection to the given host.
   * @param hostname The hostname to resolve.
   * @param port The port to use. If unsure, leave the default value.
   */
  void Connect(std::string& hostname, const std::string& port = "443");

  /**
   * @brief Closes an active WebSocket connection.
   * @param code The close code to send to the server.
   */
  void Disconnect(const websocket::close_code& code);

 private:
  net::ip::tcp::resolver resolver_;
  websocket::stream<beast::ssl_stream<net::ip::tcp::socket>> websocket_;
  beast::flat_buffer buffer_;
  boost::asio::io_context &io_;
  int heartbeat_interval_;

  /**
   * @brief A callback that is dispatched when a message has been received from
   * the host.
   * @param error Potential errors that may have occurred during the read.
   * @param bytes_read The total amount of bytes that have been read.
   *
   * @warning Never call this function manually!
   */
  void OnMessage(beast::error_code error, std::size_t bytes_read);

  /**
   * @brief Generic error handler.
   */
  void OnError(beast::error_code error);

  /**
   * @brief A callback that is dispatched once the gateway sends the initial
   * hello payload
   * @param heartbeat_interval Time in ms to wait between sending heartbeat
   * payloads
   *
   * @warning Never call this function manually!
   */
  void OnHello(int heartbeat_interval);

  /**
   * @brief Send a generic heartbeat payload
   */
  void SendHeartbeat();

  /**
   * @brief Automatically sends a heartbeat in regular intervals
   */
  void HeartbeatTask() AURORA_NORETURN;
};

}  // namespace gateway
}  // namespace aurora

#endif  // AURORA_GATEWAY_GATEWAY_H_
