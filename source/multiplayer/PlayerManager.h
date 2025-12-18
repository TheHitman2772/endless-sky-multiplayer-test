/* PlayerManager.h
 * Copyright (c) 2025 by Endless Sky Development Team
 *
 * Endless Sky is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "PlayerRegistry.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

class NetworkConnection;
class NetworkPlayer;
class Ship;


// PlayerManager: Central management for multiplayer players
//
// This class is the main interface for managing players in a multiplayer session.
// It handles player lifecycle (join/leave), ship ownership, and player events.
//
// Design Goals:
// - Single source of truth for all players
// - Event notifications (join, leave, ship ownership changes)
// - Manages PlayerRegistry internally
// - Server and client compatible (different usage patterns)
//
// Usage:
// Server:
//   PlayerManager manager;
//   auto player = manager.AddPlayer(uuid, "PlayerName");
//   player->SetConnection(connection);
//   manager.AssignShipToPlayer(ship, player);
//
// Client:
//   PlayerManager manager;  // Receives updates from server
//   auto player = manager.GetPlayer(uuid);
class PlayerManager {
public:
	// Event callback types
	using PlayerJoinCallback = std::function<void(std::shared_ptr<NetworkPlayer>)>;
	using PlayerLeaveCallback = std::function<void(std::shared_ptr<NetworkPlayer>)>;
	using ShipOwnershipCallback = std::function<void(std::shared_ptr<Ship>, std::shared_ptr<NetworkPlayer>)>;


public:
	PlayerManager();
	~PlayerManager() = default;

	// Player lifecycle
	std::shared_ptr<NetworkPlayer> AddPlayer(const EsUuid &uuid, const std::string &name);
	std::shared_ptr<NetworkPlayer> AddPlayer(std::shared_ptr<NetworkPlayer> player);
	void RemovePlayer(const EsUuid &uuid);
	void RemovePlayer(std::shared_ptr<NetworkPlayer> player);

	// Player lookup
	std::shared_ptr<NetworkPlayer> GetPlayer(const EsUuid &uuid) const;
	std::shared_ptr<NetworkPlayer> GetPlayer(size_t index) const;
	std::shared_ptr<NetworkPlayer> GetPlayerByName(const std::string &name) const;
	std::shared_ptr<NetworkPlayer> GetPlayerByConnection(NetworkConnection *conn) const;

	// Player queries
	bool HasPlayer(const EsUuid &uuid) const;
	size_t GetPlayerCount() const;
	std::vector<std::shared_ptr<NetworkPlayer>> GetAllPlayers() const;
	std::vector<std::shared_ptr<NetworkPlayer>> GetConnectedPlayers() const;

	// Ship ownership
	void AssignShipToPlayer(std::shared_ptr<Ship> ship, std::shared_ptr<NetworkPlayer> player);
	void UnassignShipFromPlayer(std::shared_ptr<Ship> ship);
	std::shared_ptr<NetworkPlayer> GetShipOwner(std::shared_ptr<Ship> ship) const;
	bool IsShipOwnedByPlayer(std::shared_ptr<Ship> ship, std::shared_ptr<NetworkPlayer> player) const;

	// Event callbacks
	void SetPlayerJoinCallback(PlayerJoinCallback callback) { onPlayerJoin = callback; }
	void SetPlayerLeaveCallback(PlayerLeaveCallback callback) { onPlayerLeave = callback; }
	void SetShipOwnershipCallback(ShipOwnershipCallback callback) { onShipOwnership = callback; }

	// Utilities
	void UpdatePlayerActivity(const EsUuid &uuid, uint64_t timestamp);
	void DisconnectInactivePlayers(uint64_t currentTime, uint64_t timeoutMs);

	// Cleanup
	void Clear();

	// Validation
	bool IsValid() const;


private:
	// Player registry (manages UUID/index mapping)
	PlayerRegistry registry;

	// Ship ownership tracking (Ship -> Player UUID)
	std::unordered_map<std::shared_ptr<Ship>, EsUuid> shipOwnership;

	// Event callbacks
	PlayerJoinCallback onPlayerJoin;
	PlayerLeaveCallback onPlayerLeave;
	ShipOwnershipCallback onShipOwnership;
};
