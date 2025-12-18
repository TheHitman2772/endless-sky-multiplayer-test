/* NetworkPlayer.h
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

#include "../Account.h"
#include "../CargoHold.h"
#include "../EsUuid.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Mission;
class NetworkConnection;
class Ship;


// NetworkPlayer: Multiplayer player representation
//
// This class represents a player in a multiplayer game session. Unlike PlayerInfo
// (which is single-player focused and contains UI state, save game data, etc.),
// NetworkPlayer is designed for network synchronization and multi-player management.
//
// Design Goals:
// - Lightweight and network-friendly
// - Separates local UI state from synced game state
// - Supports multiple players in same game world
// - Compatible with existing single-player code (PlayerInfo)
//
// Relationship to PlayerInfo:
// - Server: NetworkPlayer -> PlayerInfo (for each connected player)
// - Client: NetworkPlayer represents remote players, PlayerInfo for local player
class NetworkPlayer {
public:
	// Player roles/permissions
	enum class Role {
		PLAYER,      // Standard player
		MODERATOR,   // Can kick/mute players
		ADMIN        // Full server control
	};

	// Player connection status
	enum class Status {
		CONNECTING,  // Initial handshake in progress
		CONNECTED,   // Fully connected and playing
		DISCONNECTED // Disconnected (gracefully or timeout)
	};


public:
	NetworkPlayer();
	explicit NetworkPlayer(const EsUuid &uuid);
	NetworkPlayer(const EsUuid &uuid, const std::string &name);
	~NetworkPlayer() = default;

	// Copy and move
	NetworkPlayer(const NetworkPlayer &) = default;
	NetworkPlayer &operator=(const NetworkPlayer &) = default;
	NetworkPlayer(NetworkPlayer &&) = default;
	NetworkPlayer &operator=(NetworkPlayer &&) = default;

	// Player identification
	const EsUuid &GetUUID() const { return uuid; }
	void SetUUID(const EsUuid &newUuid) { uuid = newUuid; }

	const std::string &GetName() const { return name; }
	void SetName(const std::string &newName) { name = newName; }

	// Connection management
	NetworkConnection *GetConnection() const { return connection; }
	void SetConnection(NetworkConnection *conn) { connection = conn; }

	Status GetStatus() const { return status; }
	void SetStatus(Status newStatus) { status = newStatus; }

	// Ship management
	std::shared_ptr<Ship> GetFlagship() const { return flagship.lock(); }
	void SetFlagship(std::shared_ptr<Ship> ship) { flagship = ship; }

	const std::vector<std::weak_ptr<Ship>> &GetShips() const { return ships; }
	void AddShip(std::shared_ptr<Ship> ship);
	void RemoveShip(std::shared_ptr<Ship> ship);
	void ClearShips();
	size_t GetShipCount() const;

	// Account and cargo
	const Account &GetAccount() const { return account; }
	Account &GetAccount() { return account; }

	const CargoHold &GetCargo() const { return cargo; }
	CargoHold &GetCargo() { return cargo; }

	// Missions (using shared_ptr since Mission is not copyable)
	const std::vector<std::shared_ptr<Mission>> &GetMissions() const { return missions; }
	void AddMission(std::shared_ptr<Mission> mission);
	void RemoveMission(size_t index);
	void ClearMissions();

	// Permissions
	Role GetRole() const { return role; }
	void SetRole(Role newRole) { role = newRole; }

	bool IsAdmin() const { return role == Role::ADMIN; }
	bool IsModerator() const { return role == Role::MODERATOR || role == Role::ADMIN; }

	// Statistics
	uint64_t GetJoinTime() const { return joinTime; }
	void SetJoinTime(uint64_t time) { joinTime = time; }

	uint64_t GetLastActivityTime() const { return lastActivityTime; }
	void UpdateActivity(uint64_t time);

	// Validation
	bool IsValid() const;


private:
	// Player identification
	EsUuid uuid;
	std::string name;

	// Connection
	NetworkConnection *connection = nullptr;
	Status status = Status::CONNECTING;

	// Ships (weak_ptr to avoid circular ownership)
	std::weak_ptr<Ship> flagship;
	std::vector<std::weak_ptr<Ship>> ships;

	// Resources
	Account account;
	CargoHold cargo;
	std::vector<std::shared_ptr<Mission>> missions;

	// Permissions
	Role role = Role::PLAYER;

	// Statistics
	uint64_t joinTime = 0;
	uint64_t lastActivityTime = 0;
};
