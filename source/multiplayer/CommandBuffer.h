/* CommandBuffer.h
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

#include "PlayerCommand.h"
#include "../EsUuid.h"

#include <cstdint>
#include <map>
#include <queue>
#include <vector>


// CommandBuffer: Timestamp-ordered buffer for player commands
//
// This class manages a priority queue of player commands, ordered by game tick.
// It supports both server (all players) and client (local player) use cases.
//
// Design Goals:
// - Commands processed in timestamp order (deterministic)
// - Efficient insertion and retrieval
// - Per-player command tracking
// - Duplicate detection
// - Buffer size limits (prevent memory exhaustion)
//
// Usage:
// Server:
//   CommandBuffer buffer;
//   buffer.AddCommand(playerCmd);  // From network
//   auto cmds = buffer.GetCommandsForTick(currentTick);
//   buffer.ProcessCommand(cmd);
//
// Client:
//   CommandBuffer buffer;
//   buffer.AddCommand(localCmd);  // From input
//   // Keep for prediction/reconciliation
class CommandBuffer {
public:
	CommandBuffer();
	~CommandBuffer() = default;

	// Add a command to the buffer
	bool AddCommand(const PlayerCommand &command);

	// Get all commands for a specific tick
	std::vector<PlayerCommand> GetCommandsForTick(uint64_t gameTick);

	// Get all commands up to and including a specific tick
	std::vector<PlayerCommand> GetCommandsUpToTick(uint64_t gameTick);

	// Remove processed commands older than specified tick
	void PruneOlderThan(uint64_t gameTick);

	// Get commands for a specific player
	std::vector<PlayerCommand> GetPlayerCommands(const EsUuid &playerUUID) const;

	// Get the oldest queued tick
	uint64_t GetOldestTick() const;

	// Get the newest queued tick
	uint64_t GetNewestTick() const;

	// Check if buffer contains commands for a tick
	bool HasCommandsForTick(uint64_t gameTick) const;

	// Get buffer statistics
	size_t GetCommandCount() const;
	size_t GetPlayerCount() const;

	// Clear all commands
	void Clear();

	// Configuration
	void SetMaxBufferSize(size_t maxSize) { maxBufferSize = maxSize; }
	size_t GetMaxBufferSize() const { return maxBufferSize; }

	// Validation
	bool IsValid() const;


private:
	// Priority queue ordered by game tick
	std::multimap<uint64_t, PlayerCommand> commandQueue;

	// Track commands per player (for duplicate detection)
	std::map<EsUuid, std::vector<PlayerCommand>> playerCommands;

	// Configuration
	size_t maxBufferSize = 10000;  // Maximum commands in buffer

	// Check if command is duplicate
	bool IsDuplicate(const PlayerCommand &command) const;
};
