/* PlayerCommand.h
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

#include "../Command.h"
#include "../EsUuid.h"
#include "../Point.h"

#include <cstdint>


// PlayerCommand: Represents a single player input command
//
// This structure captures a player's input at a specific game tick.
// Commands are timestamped and can be validated, buffered, and predicted.
//
// Design Goals:
// - Small size for network efficiency
// - Timestamped for deterministic replay
// - Contains all player input state for one frame
// - Supports client-side prediction and server reconciliation
struct PlayerCommand {
	// Player who issued this command
	EsUuid playerUUID;

	// Game tick when this command should be executed
	uint64_t gameTick = 0;

	// Ship movement and firing commands
	Command command;

	// Target point for autopilot/targeting (optional)
	Point targetPoint;
	bool hasTargetPoint = false;

	// Sequence number (for detecting duplicates and ordering)
	uint32_t sequenceNumber = 0;

	// Constructors
	PlayerCommand() = default;
	PlayerCommand(const EsUuid &uuid, uint64_t tick)
		: playerUUID(uuid), gameTick(tick) {}
	PlayerCommand(const EsUuid &uuid, uint64_t tick, const Command &cmd)
		: playerUUID(uuid), gameTick(tick), command(cmd) {}

	// Comparison operators (for ordering in buffer)
	bool operator<(const PlayerCommand &other) const
	{
		if(gameTick != other.gameTick)
			return gameTick < other.gameTick;
		return sequenceNumber < other.sequenceNumber;
	}

	bool operator==(const PlayerCommand &other) const
	{
		return playerUUID == other.playerUUID &&
		       gameTick == other.gameTick &&
		       sequenceNumber == other.sequenceNumber;
	}

	// Validation
	bool IsValid() const
	{
		// Must have valid player UUID
		if(playerUUID.ToString().empty())
			return false;

		// Game tick must be reasonable
		if(gameTick > 1000000000)  // Sanity check
			return false;

		return true;
	}
};
