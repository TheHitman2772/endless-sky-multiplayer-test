/* CommandValidator.cpp
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

#include "CommandValidator.h"

#include <chrono>

using namespace std;



CommandValidator::CommandValidator()
{
}



CommandValidator::Result CommandValidator::ValidateCommand(const PlayerCommand &command, uint64_t currentTick)
{
	++totalCommands;

	// Check if command structure is valid
	if(!command.IsValid())
	{
		++rejectedCommands;
		return Result::MALFORMED;
	}

	// Check if player UUID is valid
	if(command.playerUUID.ToString().empty())
	{
		++rejectedCommands;
		return Result::INVALID_PLAYER;
	}

	// Check if tick is within acceptable range
	if(command.gameTick < currentTick - maxPastTicks)
	{
		++rejectedCommands;
		return Result::TOO_OLD;
	}

	if(command.gameTick > currentTick + maxFutureTicks)
	{
		++rejectedCommands;
		return Result::TOO_FUTURE;
	}

	// Get current time for rate limiting
	auto now = chrono::steady_clock::now();
	uint64_t currentTime = chrono::duration_cast<chrono::milliseconds>(
		now.time_since_epoch()
	).count();

	// Check rate limit
	if(!CheckRateLimit(command.playerUUID, currentTime))
	{
		++rejectedCommands;
		return Result::RATE_LIMITED;
	}

	return Result::VALID;
}



string CommandValidator::ResultToString(Result result)
{
	switch(result)
	{
		case Result::VALID: return "Valid";
		case Result::INVALID_PLAYER: return "Invalid player UUID";
		case Result::INVALID_TICK: return "Invalid game tick";
		case Result::TOO_OLD: return "Command too old";
		case Result::TOO_FUTURE: return "Command too far in future";
		case Result::RATE_LIMITED: return "Rate limited";
		case Result::MALFORMED: return "Malformed command";
		default: return "Unknown";
	}
}



double CommandValidator::GetRejectionRate() const
{
	if(totalCommands == 0)
		return 0.0;
	return static_cast<double>(rejectedCommands) / static_cast<double>(totalCommands);
}



double CommandValidator::GetPlayerCommandRate(const EsUuid &playerUUID) const
{
	auto it = playerRateLimits.find(playerUUID);
	if(it == playerRateLimits.end())
		return 0.0;

	const auto &data = it->second;
	if(data.commandsInWindow == 0)
		return 0.0;

	// Calculate commands per second
	return static_cast<double>(data.commandsInWindow) * 1000.0 / static_cast<double>(RATE_LIMIT_WINDOW_MS);
}



void CommandValidator::ResetStatistics()
{
	totalCommands = 0;
	rejectedCommands = 0;
}



void CommandValidator::ClearPlayerTracking(const EsUuid &playerUUID)
{
	playerRateLimits.erase(playerUUID);
}



bool CommandValidator::CheckRateLimit(const EsUuid &playerUUID, uint64_t currentTime)
{
	auto &data = playerRateLimits[playerUUID];

	// Check if we need to start a new window
	if(currentTime - data.windowStartTime >= RATE_LIMIT_WINDOW_MS)
	{
		data.windowStartTime = currentTime;
		data.commandsInWindow = 0;
	}

	// Increment command count
	++data.commandsInWindow;
	data.lastCommandTime = currentTime;

	// Check if rate limit exceeded
	double commandsPerSecond = static_cast<double>(data.commandsInWindow) * 1000.0 /
	                           static_cast<double>(RATE_LIMIT_WINDOW_MS);

	if(commandsPerSecond > static_cast<double>(maxCommandsPerSecond))
		return false;

	return true;
}
