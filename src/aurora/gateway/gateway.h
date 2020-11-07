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

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>

#include "common/defines.h"
#include "protocol.h"

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
  explicit Session(net::io_context& io, net::ssl::context& ssl,
                   bool compress = false);

  ~Session();

  AURORA_DISALLOW_COPY_AND_ASSIGN(Session);
  AURORA_DISALLOW_MOVE(Session);

  /**
   * @brief Establishes a WebSocket connection to the given host.
   * @param hostname The hostname to resolve.
   * @param port The port to use. If unsure, leave the default value.
   */
  void Connect(std::string token, std::string& hostname,
               const std::string& port = "443");

  /**
   * @brief Closes an active WebSocket connection.
   * @param code The close code to send to the server.
   */
  void Disconnect(const websocket::close_code& code);

  /**
   * @brief Start receiving certain gateway events
   * @param intents Intent(s) category to subscribe to
   */
  void SubscribeTo(uint16_t intent);

  /**
   * @brief Stop receiving certain gateway events
   * @param intents Intent(s) category to unsubscribe from
   */
  void UnsubscribeFrom(uint16_t intent);

 private:
  // User specific member variables
  std::uint16_t intents_ = 0;
  std::string token_;

  // Internal variables
  net::ip::tcp::resolver resolver_;
  websocket::stream<beast::ssl_stream<net::ip::tcp::socket>> websocket_;
  beast::flat_buffer buffer_;
  boost::asio::steady_timer heartbeat_timer_;
  int heartbeat_interval_ = 0, last_sequence_ = 0;
  bool did_ack_heartbeat_ = true, compress_;
  std::string session_id_, hostname_, port_;

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
   * @brief Sends a message to the gateway using a user specified
   * handler callback
   * @param payload The messages payload
   * @param opcode The message opcode
   * @param handler The handler to be called after the message was sent
   */
  template <class WriteHandler>
  void SendMessage(const nlohmann::json& payload, Opcode opcode,
                   WriteHandler&& handler);

  /**
   * @brief Sends a message to the gateway using a generic handler callback
   * @param payload The messages payload
   * @param opcode The message opcode
   */
  void SendMessage(const nlohmann::json& payload, Opcode opcode);

  /**
   * @brief Generic error handler.
   */
  void OnError(beast::error_code error, std::size_t bytes_transferred = 0);

  /**
   * @brief A callback that is dispatched once the gateway sends a dispatch
   * event
   * @param data Data sent with the dispatch payload
   * payloads
   *
   * @warning Never call this function manually!
   */
  void OnDispatch(nlohmann::json data);

  /**
   * @brief A callback that is dispatched if the gateway sends a reconnect
   * event
   * @param data Data sent with the reconnect payload
   * payloads
   *
   * @warning Never call this function manually!
   */
  void OnReconnect(const nlohmann::json& data);

  /**
   * @brief A callback that is dispatched if the gateway sends a invalid
   * session event
   * @param data Data sent with the invalid session payload
   * payloads
   *
   * @warning Never call this function manually!
   */
  void OnInvalidSession(const nlohmann::json& data);

  /**
   * @brief A callback that is dispatched once the gateway sends the initial
   * hello payload
   * @param data Data sent with the hello payload
   * payloads
   *
   * @warning Never call this function manually!
   */
  void OnHello(nlohmann::json data);

  /**
   * @brief A callback that is dispatched once the gateway sends a heartbeat
   * acknowledge event
   * @param data Data sent with the heartbeat ack payload
   * payloads
   *
   * @warning Never call this function manually!
   */
  void OnHeartbeatAck(const nlohmann::json& data);

  /**
   * @brief Automatically sends a heartbeat in regular intervals
   */
  void HeartbeatTask();

  /**
   * @brief Sends a generic heartbeat payload
   */
  void SendHeartbeat();

  /**
   * @brief Sends a generic identify payload
   */
  void Identify();

  /**
   * @brief Resumes a previously disconnected session
   */
  void Resume();
};

}  // namespace gateway
}  // namespace aurora

#endif  // AURORA_GATEWAY_GATEWAY_H_
