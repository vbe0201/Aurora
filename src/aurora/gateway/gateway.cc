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

#define ZLIB_SUFFIX "\x00\x00\xff\xff"

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
    websocket_.close(code);
  }
}

void Session::SubscribeTo(uint16_t intents) { intents_ |= intents; }

void Session::UnsubscribeFrom(uint16_t intents) { intents_ &= ~intents; }

void Session::OnMessage(beast::error_code error, std::size_t bytes_read) {
  AURORA_UNUSED(bytes_read);

  // TODO: Handle errors accordingly.
  if (error) {
    OnError(error);
  }

  if (compress_) {
    if (buffer_.size() >= 4) {
      uint8_t *last_four_bytes = (uint8_t *)buffer_.data().data();
      last_four_bytes += buffer_.size() - 5;
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

  // TODO: Handle the received opcode accordingly.
  //       Maybe do this using a map!
  if (payload["op"] == 0) {
    last_sequence_ = payload["s"];
  } else if (payload["op"] == 10) {
    OnHello(payload["d"]["heartbeat_interval"]);
  } else if (payload["op"] == 11) {
    did_ack_heartbeat_ = true;
  }

  // Clean the buffer and continue listening for more messages.
  buffer_.clear();
  websocket_.async_read(buffer_, beast::bind_front_handler(&Session::OnMessage,
                                                           shared_from_this()));
}

template <class WriteHandler>
void Session::SendMessage(const nlohmann::json &payload, Opcode opcode,
                          WriteHandler &&handler) {
  nlohmann::json message;
  message["op"] = (int)opcode;
  message["d"] = payload;
  websocket_.async_write(net::buffer(message.dump()), handler);
}

void Session::SendMessage(nlohmann::json payload, Opcode opcode) {
  SendMessage(payload, opcode,
              [this](beast::error_code const &ec,
                     std::size_t bytes_transferred) { OnError(ec); });
}

void Session::OnError(beast::error_code error, std::size_t bytes_transferred) {
  AURORA_UNUSED(bytes_transferred);
  if (!error) return;
  // TODO: Reconnect
  std::cerr << "Closing connection due to internal error.\n";
  std::cerr << "Error message: " << error.message() << "\n";
  std::cerr << "Error code: " << error.value() << "\n";

  Disconnect(websocket::close_code(4000));
}

void Session::OnHello(int heartbeat_interval) {
  heartbeat_interval_ = heartbeat_interval;
  HeartbeatTask();
  Identify();
}

void Session::HeartbeatTask() {
  heartbeat_timer_.expires_from_now(
      std::chrono::milliseconds(heartbeat_interval_));
  heartbeat_timer_.async_wait([this](const boost::system::error_code &error) {
    if (error) {
      OnError(error);
    } else if (websocket_.is_open()) {
      SendHeartbeat();
    }
  });
}

void Session::SendHeartbeat() {
  if (!did_ack_heartbeat_) {
    std::cerr << "Closing zombied connection";
    Disconnect(websocket::close_code(4000));
    return;
  }
  SendMessage(
      last_sequence_, Opcode::kHeartbeat,
      [this](beast::error_code const &ec, std::size_t bytes_transferred) {
        did_ack_heartbeat_ = false;
        if (ec) {
          OnError(ec);
        } else if (websocket_.is_open()) {
          HeartbeatTask();
        }
      });
}

void Session::Identify() {
  nlohmann::json payload = {
      {"token", token_},
      {"intents", intents_},
      {"properties",
       {{"$os", "linux"}, {"$browser", "Aurora"}, {"$device", "Aurora"}}
      },
      {"compress", compress_}
  };
  SendMessage(payload, Opcode::kIdentify,
              [this](beast::error_code const &ec,
                     std::size_t bytes_transferred) { OnError(ec); });
}

}  // namespace gateway
}  // namespace aurora
