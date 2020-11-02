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

#include "nlohmann/json.hpp"

namespace aurora {
namespace gateway {

Session::Session(net::io_context &io, net::ssl::context &ssl)
    : resolver_(net::make_strand(io)),
      websocket_(net::make_strand(io), ssl),
      io_(io) {}

Session::~Session() {
  try {
    Disconnect(websocket::close_code::normal);
  } catch (...) {
  };
}

void Session::Connect(std::string &hostname, const std::string &port) {
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

void Session::OnMessage(beast::error_code error, std::size_t bytes_read) {
  AURORA_UNUSED(bytes_read);

  // TODO: Handle errors accordingly.
  if (error) {
  }

  // TODO: Validate and uncompress zlib data.

  // TODO: Unpack Erlang Term Format message.
  nlohmann::json payload = [this]() {
    std::ostringstream stream;
    stream << beast::make_printable(buffer_.data());

    return nlohmann::json::parse(stream.str());
  }();

  std::cout << payload << std::endl;

  // TODO: Handle the received opcode accordingly.
  //       Maybe do this using a map!
  if (payload["op"] == 10)
    OnHello(payload["d"]["heartbeat_interval"]);
  else if (payload["op"] == 11)
    did_ack_heartbeat_ = true;

  // Clean the buffer and continue listening for more messages.
  buffer_.clear();
  // websocket_.async_read(buffer_,
  // beast::bind_front_handler(&Session::OnMessage,
  //                                                         shared_from_this()));
}

void Session::OnError(beast::error_code error) {
  if (!error) return;
  // TODO: Reconnect
  std::cerr << "Closing connection due to internal error.";

  Disconnect(websocket::close_code(4000));
}

void Session::OnHello(int heartbeat_interval) {
  heartbeat_interval_ = heartbeat_interval;
  HeartbeatTask();
}

void Session::SendHeartbeat() {
  if (!did_ack_heartbeat_) {
    std::cerr << "Closing zombied connection";
    Disconnect(websocket::close_code(4000));
    return;
  }
  nlohmann::json payload;
  payload["op"] = 1;
  payload["d"] = 251;
  websocket_.async_write(
      net::buffer(payload.dump()),
      [this](beast::error_code const &ec, std::size_t bytes_transferred) {
        did_ack_heartbeat_ = false;
        OnError(ec);
      });
}
void Session::HeartbeatTask() {
  boost::asio::steady_timer t(
      io_, boost::asio::chrono::seconds(heartbeat_interval_));
  while (websocket_.is_open()) {
    t.async_wait([this](const boost::system::error_code &error) {
      if (error) {
        OnError(error);
      } else {
        SendHeartbeat();
      }
    });
  }
}

}  // namespace gateway
}  // namespace aurora
