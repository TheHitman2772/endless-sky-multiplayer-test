/* CommandBuffer.cpp
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

#include "CommandBuffer.h"

#include <algorithm>

using namespace std;



CommandBuffer::CommandBuffer()
{
}



bool CommandBuffer::AddCommand(const PlayerCommand &command)
{
	// Validate command
	if(!command.IsValid())
		return false;

	// Check buffer size limit
	if(commandQueue.size() >= maxBufferSize)
		return false;

	// Check for duplicates
	if(IsDuplicate(command))
		return false;

	// Add to queue (multimap automatically sorts by key = gameTick)
	commandQueue.emplace(command.gameTick, command);

	// Track per-player
	playerCommands[command.playerUUID].push_back(command);

	return true;
}



vector<PlayerCommand> CommandBuffer::GetCommandsForTick(uint64_t gameTick)
{
	vector<PlayerCommand> result;

	// Get all commands with matching tick
	auto range = commandQueue.equal_range(gameTick);
	for(auto it = range.first; it != range.second; ++it)
		result.push_back(it->second);

	return result;
}



vector<PlayerCommand> CommandBuffer::GetCommandsUpToTick(uint64_t gameTick)
{
	vector<PlayerCommand> result;

	// Get all commands from start up to and including gameTick
	for(auto it = commandQueue.begin(); it != commandQueue.end() && it->first <= gameTick; ++it)
		result.push_back(it->second);

	return result;
}



void CommandBuffer::PruneOlderThan(uint64_t gameTick)
{
	// Remove all commands older than specified tick
	auto it = commandQueue.begin();
	while(it != commandQueue.end() && it->first < gameTick)
		it = commandQueue.erase(it);

	// Also clean up player commands
	for(auto &pair : playerCommands)
	{
		auto &cmds = pair.second;
		cmds.erase(
			remove_if(cmds.begin(), cmds.end(),
				[gameTick](const PlayerCommand &cmd) {
					return cmd.gameTick < gameTick;
				}),
			cmds.end()
		);
	}

	// Remove empty player entries
	for(auto it = playerCommands.begin(); it != playerCommands.end();)
	{
		if(it->second.empty())
			it = playerCommands.erase(it);
		else
			++it;
	}
}



vector<PlayerCommand> CommandBuffer::GetPlayerCommands(const EsUuid &playerUUID) const
{
	auto it = playerCommands.find(playerUUID);
	if(it != playerCommands.end())
		return it->second;
	return {};
}



uint64_t CommandBuffer::GetOldestTick() const
{
	if(commandQueue.empty())
		return 0;
	return commandQueue.begin()->first;
}



uint64_t CommandBuffer::GetNewestTick() const
{
	if(commandQueue.empty())
		return 0;
	return commandQueue.rbegin()->first;
}



bool CommandBuffer::HasCommandsForTick(uint64_t gameTick) const
{
	return commandQueue.find(gameTick) != commandQueue.end();
}



size_t CommandBuffer::GetCommandCount() const
{
	return commandQueue.size();
}



size_t CommandBuffer::GetPlayerCount() const
{
	return playerCommands.size();
}



void CommandBuffer::Clear()
{
	commandQueue.clear();
	playerCommands.clear();
}



bool CommandBuffer::IsValid() const
{
	// All commands in queue should be in player commands
	size_t totalPlayerCommands = 0;
	for(const auto &pair : playerCommands)
		totalPlayerCommands += pair.second.size();

	return totalPlayerCommands == commandQueue.size();
}



bool CommandBuffer::IsDuplicate(const PlayerCommand &command) const
{
	auto it = playerCommands.find(command.playerUUID);
	if(it == playerCommands.end())
		return false;

	// Check if this exact command already exists
	for(const auto &existing : it->second)
	{
		if(existing == command)
			return true;
	}

	return false;
}
