/* PlayerRegistry.h
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

#include "../EsUuid.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class NetworkPlayer;


// PlayerRegistry: Fast player ID mapping and lookup
//
// This class provides O(1) lookup of players by:
// - UUID (primary identifier)
// - Player index (0-based, for iteration)
// - Player name (for chat/UI)
//
// Design Goals:
// - Fast lookups (critical for server performance)
// - Thread-safe operations (future: add mutex)
// - Stable player indices (for UI lists)
// - Handle player join/leave gracefully
class PlayerRegistry {
public:
	PlayerRegistry();
	~PlayerRegistry() = default;

	// Register a player (returns assigned index)
	size_t Register(std::shared_ptr<NetworkPlayer> player);

	// Unregister a player
	void Unregister(const EsUuid &uuid);
	void Unregister(size_t index);

	// Lookup by UUID
	std::shared_ptr<NetworkPlayer> GetByUUID(const EsUuid &uuid) const;
	bool HasUUID(const EsUuid &uuid) const;

	// Lookup by index
	std::shared_ptr<NetworkPlayer> GetByIndex(size_t index) const;
	bool HasIndex(size_t index) const;

	// Lookup by name (returns first match)
	std::shared_ptr<NetworkPlayer> GetByName(const std::string &name) const;

	// Get all players
	std::vector<std::shared_ptr<NetworkPlayer>> GetAllPlayers() const;
	size_t GetPlayerCount() const;

	// Get indices
	std::vector<size_t> GetActiveIndices() const;
	size_t GetNextAvailableIndex() const;

	// Clear all players
	void Clear();

	// Validation
	bool IsValid() const;


private:
	// UUID -> Player mapping (primary) - using map since EsUuid has operator< but no hash
	std::map<EsUuid, std::shared_ptr<NetworkPlayer>> playersByUUID;

	// Index -> Player mapping (for iteration)
	std::unordered_map<size_t, std::shared_ptr<NetworkPlayer>> playersByIndex;

	// Index -> UUID reverse mapping
	std::unordered_map<size_t, EsUuid> indexToUUID;

	// Next available player index
	size_t nextIndex = 0;

	// Free indices (from player removals)
	std::vector<size_t> freeIndices;
};
