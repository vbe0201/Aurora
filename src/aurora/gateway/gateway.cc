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

#include "gateway.h"

#include <iostream>
#include <string>
#include <utility>

#include "common/assert.h"

namespace aurora {
namespace gateway {

Session::Session(net::io_context &io, net::ssl::context &ssl, bool compress)
    : resolver_(net::make_strand(io)),
      websocket_(net::make_strand(io), ssl),
      heartbeat_timer_(io),
      compress_(compress) {}

Session::~Session() {
  try {
    Disconnect(websocket::close_code::normal);
  } catch (...) {
  }
}

void Session::Connect(std::string token, std::string &hostname,
                      const std::string &port) {
  token_ = std::move(token);
  hostname_ = hostname;
  port_ = port;
  // Resolve the supplied host and connect to it.
  const auto results = resolver_.resolve(hostname, port);
  auto endpoint = net::connect(beast::get_lowest_layer(websocket_), results);

  // Set the SNI hostname accordingly.
  if (!SSL_set_tlsext_host_name(websocket_.next_layer().native_handle(),
                                hostname.c_str()))
    throw beast::system_error(
        beast::error_code(static_cast<int>(::ERR_get_error()),
                          net::error::get_ssl_category()),
        "Failed to set SNI Hostname");

  // Update the host string. This will provide the value of the Host HTTP header
  // during the initial WebSocket handshake.
  // See https://tools.ietf.org/html/rfc7230#section-5.4
  hostname += ':' + std::to_string(endpoint.port());

  // Perform the SSL handshake.
  websocket_.next_layer().handshake(net::ssl::stream_base::client);

  // Remove the User-Agent header to obfuscate the fact that we're Boost.Beast.
  websocket_.set_option(
      websocket::stream_base::decorator([](websocket::request_type &req) {
        req.erase(http::field::user_agent);
      }));

  // Perform the WebSocket handshake.
  // TODO: Use a proper target for the handshake.
  websocket_.handshake(hostname, "/");

  // Start listening for messages from the server.
  websocket_.async_read(buffer_, beast::bind_front_handler(&Session::OnMessage,
                                                           shared_from_this()));
}

void Session::Disconnect(const websocket::close_code &code) {
  if (websocket_.is_open()) {
    std::cerr << "Closing...\n";
    websocket_.close(code);
  }
}

void Session::SubscribeTo(uint16_t intents) { intents_ |= intents; }

void Session::UnsubscribeFrom(uint16_t intents) { intents_ &= ~intents; }

void Session::OnMessage(beast::error_code error, std::size_t bytes_read) {
#define ZLIB_SUFFIX "\x00\x00\xff\xff"
  AURORA_UNUSED(bytes_read);

  OnError(error);

  if (compress_) {
    // zlib compressed messages end with a four byte suffix
    if (buffer_.size() >= 4) {
      // Get the raw buffer data and increase it to get the last four bytes
      uint8_t *last_four_bytes = (uint8_t *)buffer_.data().data();
      last_four_bytes += buffer_.size() - 5;
      // Check if the zlib suffix is present
      if (memcmp(last_four_bytes, ZLIB_SUFFIX, 4) != 0) {
        // TODO: actually do the decompression part
      }
    }
  }

  // TODO: Unpack Erlang Term Format message.
  nlohmann::json payload = [this]() {
    std::ostringstream stream;
    stream << beast::make_printable(buffer_.data());

    return nlohmann::json::parse(stream.str());
  }();

  std::cout << payload << std::endl;

  Opcode op = payload["op"];
  nlohmann::json data = payload["d"];

  // Dispatch stores the event name in the t field
  // And the sequence in the s field
  if (op == Opcode::kDispatch) {
    last_sequence_ = payload["s"];
    data.emplace("t", payload["t"]);
  }

  // Handle message
  switch (op) {
    case Opcode::kDispatch:
      OnDispatch(data);
      break;
    case Opcode::kReconnect:
      OnReconnect(data);
      break;
    case Opcode::kInvalidSession:
      OnInvalidSession(data);
      break;
    case Opcode::kHello:
      OnHello(data);
      break;
    case Opcode::kHeartbeatAck:
      OnHeartbeatAck(data);
      break;
    default:
      AURORA_UNREACHABLE()
  }

  // Clean the buffer and continue listening for more messages.
  buffer_.clear();
  if (websocket_.is_open())
    websocket_.async_read(
        buffer_,
        beast::bind_front_handler(&Session::OnMessage, shared_from_this()));
#undef ZLIB_SUFFIX
}

template <class WriteHandler>
void Session::SendMessage(const nlohmann::json &payload, Opcode opcode,
                          WriteHandler &&handler) {
  // Fill message properly and send
  nlohmann::json message;
  message["op"] = (int)opcode;
  message["d"] = payload;
  websocket_.async_write(net::buffer(message.dump()), handler);
}

void Session::SendMessage(const nlohmann::json &payload, Opcode opcode) {
  SendMessage(payload, opcode,
              [this](beast::error_code const &ec,
                     std::size_t bytes_transferred) { OnError(ec); });
}

void Session::OnError(beast::error_code error, std::size_t bytes_transferred) {
  AURORA_UNUSED(bytes_transferred);
  if (!error) return;

  // Print error info and reconnect
  std::cerr << "Reconnection due to internal error.\n";
  std::cerr << "Error message: " << error.message() << "\n";
  std::cerr << "Error code: " << error.value() << "\n";

  Resume();
}

void Session::OnDispatch(nlohmann::json data) {
  std::string event_id = data["t"];
  if (event_id == "READY") {
    // Session id required for reconnecting
    session_id_ = data["session_id"];
  }
  // TODO: Dispatch event to handlers
}

void Session::OnReconnect(const nlohmann::json &data) { Resume(); }

void Session::OnInvalidSession(const nlohmann::json &data) {
  // data is just a boolean that tells whether a reconnect is possible
  if (data.is_boolean() and data == true) {
    Resume();
  } else {
    std::cerr << "The gateway closed the session without the option to reconnect. ";
    Disconnect(websocket::close_code(4000));
  }
}

void Session::OnHello(nlohmann::json data) {
  // Save interval time
  heartbeat_interval_ = data["heartbeat_interval"];
  // Start heartbeating
  HeartbeatTask();
  // Identify the client to the gateway
  Identify();
}

void Session::OnHeartbeatAck(const nlohmann::json &data) {
  did_ack_heartbeat_ = true;
}

void Session::HeartbeatTask() {
  // Set timer to wait however long the gateway wants us to
  heartbeat_timer_.expires_from_now(
      std::chrono::milliseconds(heartbeat_interval_));
  // Wait and send heartbeat after
  heartbeat_timer_.async_wait([this](const boost::system::error_code &error) {
    OnError(error);
    if (websocket_.is_open()) {
      SendHeartbeat();
    }
  });
}

void Session::SendHeartbeat() {
  if (!did_ack_heartbeat_) {
    std::cerr << "Gateway didn't acknowledge heartbeat (zombied). ";
    // TODO: Reconnect or re-identify here
    Disconnect(websocket::close_code(4000));
    return;
  }
  // Send heartbeat and go back to heartbeat task after
  SendMessage(
      last_sequence_, Opcode::kHeartbeat,
      [this](beast::error_code const &ec, std::size_t bytes_transferred) {
        did_ack_heartbeat_ = false;
        OnError(ec);
        if (websocket_.is_open()) {
          HeartbeatTask();
        }
      });
}

void Session::Identify() {
  // TODO: Send proper os information
  nlohmann::json payload = {
      {"token", token_},
      {"intents", intents_},
      {"properties",
       {{"$os", "linux"}, {"$browser", "Aurora"}, {"$device", "Aurora"}}},
      {"compress", compress_}};
  SendMessage(payload, Opcode::kIdentify,
              [this](beast::error_code const &ec,
                     std::size_t bytes_transferred) { OnError(ec); });
}

void Session::Resume() {
  // TODO: check if this works
  if (websocket_.is_open())
    // TODO: Reconnect or re-identify here
    // Discords invalidates sessions closed with exit code 1000/10001
    websocket_.close(websocket::close_code(2000));
  if (session_id_.empty()) {
    std::cerr << "Attempted resume before knowing session id\n";
    return;
  }
  // Fill payload, connect and send resume payload
  nlohmann::json payload{
      {"token", token_}, {"session_id", session_id_}, {"seq", last_sequence_}};
  Connect(token_, hostname_, port_);
  SendMessage(payload, Opcode::kResume);
}

}  // namespace gateway
}  // namespace aurora
