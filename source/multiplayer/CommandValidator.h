/* CommandValidator.h
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
#include <string>


// CommandValidator: Server-side validation of player commands
//
// This class validates player commands to prevent:
// - Impossible commands (e.g., commands from the future)
// - Invalid player UUIDs
// - Commands too far in the past
// - Rapid-fire spam (rate limiting)
// - Malicious input
//
// Design Goals:
// - Reject invalid commands before processing
// - Rate limit to prevent spam
// - Track player command rates
// - Configurable validation rules
//
// Usage:
// Server:
//   CommandValidator validator;
//   if(validator.ValidateCommand(cmd, currentTick))
//       buffer.AddCommand(cmd);
//   else
//       LogInvalidCommand(cmd);
class CommandValidator {
public:
	// Validation result
	enum class Result {
		VALID,              // Command is valid
		INVALID_PLAYER,     // Player UUID invalid
		INVALID_TICK,       // Game tick out of range
		TOO_OLD,            // Command too far in the past
		TOO_FUTURE,         // Command too far in the future
		RATE_LIMITED,       // Player exceeding rate limit
		MALFORMED          // Command structure invalid
	};


public:
	CommandValidator();
	~CommandValidator() = default;

	// Validate a command
	Result ValidateCommand(const PlayerCommand &command, uint64_t currentTick);

	// Get human-readable validation result
	static std::string ResultToString(Result result);

	// Configuration
	void SetMaxPastTicks(uint64_t ticks) { maxPastTicks = ticks; }
	void SetMaxFutureTicks(uint64_t ticks) { maxFutureTicks = ticks; }
	void SetMaxCommandsPerSecond(uint32_t rate) { maxCommandsPerSecond = rate; }

	uint64_t GetMaxPastTicks() const { return maxPastTicks; }
	uint64_t GetMaxFutureTicks() const { return maxFutureTicks; }
	uint32_t GetMaxCommandsPerSecond() const { return maxCommandsPerSecond; }

	// Statistics
	uint64_t GetTotalCommandsValidated() const { return totalCommands; }
	uint64_t GetTotalCommandsRejected() const { return rejectedCommands; }
	double GetRejectionRate() const;

	// Get player's current command rate
	double GetPlayerCommandRate(const EsUuid &playerUUID) const;

	// Reset statistics
	void ResetStatistics();

	// Clear rate limit tracking (e.g., after player leaves)
	void ClearPlayerTracking(const EsUuid &playerUUID);


private:
	// Rate limiting tracking
	struct RateLimitData {
		uint64_t lastCommandTime = 0;
		uint32_t commandsInWindow = 0;
		uint64_t windowStartTime = 0;
	};

	// Per-player rate limit tracking
	std::map<EsUuid, RateLimitData> playerRateLimits;

	// Configuration
	uint64_t maxPastTicks = 60;      // Max 1 second in past (at 60 Hz)
	uint64_t maxFutureTicks = 60;    // Max 1 second in future
	uint32_t maxCommandsPerSecond = 120;  // Max 2x simulation rate

	// Statistics
	uint64_t totalCommands = 0;
	uint64_t rejectedCommands = 0;

	// Rate limit window (milliseconds)
	static constexpr uint64_t RATE_LIMIT_WINDOW_MS = 1000;

	// Update rate limit tracking
	bool CheckRateLimit(const EsUuid &playerUUID, uint64_t currentTime);
};
