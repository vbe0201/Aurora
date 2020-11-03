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
 * @file protocol.h
 * @brief Constants used by the Discord gateway protocol.
 * @author Valentin B.
 */

#ifndef AURORA_GATEWAY_PROTOCOL_H_
#define AURORA_GATEWAY_PROTOCOL_H_

/**
 * Opcodes used for communication with the Discord WebSocket gateway.
 */
enum class Opcode {
  /// Tells the client about a dispatched event from the server.
  kDispatch = 0,
  /// Requests a heartbeat from the client or tells the server that the
  /// connection is intact.
  kHeartbeat = 1,
  /// Starts a new session throughout the initial handshake.
  kIdentify = 2,
  /// Updates the presence of the client.
  kPresenceUpdate = 3,
  /// Updates the state of a voice channel.
  kVoiceStateUpdate = 4,
  /// Requests a ping from a voice server.
  kVoiceServerPing = 5,
  /// Tells Discord to resume a previous session that was cancelled.
  kResume = 6,
  /// Tells the client to reconnect to a new gateway instance.
  kReconnect = 7,
  /// Requests information about offline members in large guilds.
  kRequestGuildMembers = 8,
  /// Tells the client to close the connection and identify again.
  kInvalidSession = 9,
  /// A greeting from the gateway server, informing the client about the
  /// heartbeat interval.
  kHello = 10,
  /// Confirms the reception of a Heartbeat from the client.
  kHeartbeatAck = 11,
  /// Requests a guild synchronization.
  kGuildSync = 12,
  /// Connects to an ongoing voice call.
  kCallConnect = 13,
  /// Updates the guild subscriptions to only subscribe to relevant events.
  kGuildSubscriptions = 14,
  /// Connects to a lobby.
  kLobbyConnect = 15,
  /// Disconnects from a lobby.
  kLobbyDisconnect = 16,
  /// Updates the voice states of a lobby.
  kLobbyVoiceStatesUpdate = 17,
  /// Creates a stream within a guild voice channel.
  kStreamCreate = 18,
  /// Deletes a stream within a guild voice channel.
  kStreamDelete = 19,
  /// Subscribes to an ongoing stream within a guild and start watching it.
  kStreamWatch = 20,
  /// Requests a ping from a stream server.
  kStreamPing = 21,
  /// Sets the active stream to paused.
  kStreamSetPaused = 22,
};

/**
 * Intents acting like subscriptions to the Discord WebSocket gateway
 */
enum Intent : std::uint16_t {
    GUILDS = (1u << 0u),
    GUILD_MEMBERS = (1u << 1u),
    GUILD_BANS = (1u << 2u),
    GUILD_EMOJIS = (1u << 3u),
    GUILD_INTEGRATIONS = (1u << 4u),
    GUILD_WEBHOOKS = (1u << 5u),
    GUILD_INVITES = (1u << 6u),
    GUILD_VOICE_STATES = (1u << 7u),
    GUILD_PRESENCES = (1u << 8u),
    GUILD_MESSAGES = (1u << 9u),
    GUILD_MESSAGE_REACTIONS = (1u << 10u),
    GUILD_MESSAGE_TYPING = (1u << 11u),
    DIRECT_MESSAGES = (1u << 12u),
    DIRECT_MESSAGE_REACTIONS = (1u << 13u),
    DIRECT_MESSAGE_TYPING = (1u << 14u),
};

#endif  // AURORA_GATEWAY_PROTOCOL_H_
