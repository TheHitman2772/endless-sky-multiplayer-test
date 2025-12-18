/* PlayerRegistry.cpp
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

#include "PlayerRegistry.h"

#include "NetworkPlayer.h"

#include <algorithm>

using namespace std;



PlayerRegistry::PlayerRegistry()
{
}



size_t PlayerRegistry::Register(shared_ptr<NetworkPlayer> player)
{
	if(!player)
		return static_cast<size_t>(-1);

	const EsUuid &uuid = player->GetUUID();

	// Check if already registered
	if(playersByUUID.find(uuid) != playersByUUID.end())
		return static_cast<size_t>(-1);

	// Allocate index (reuse free index or get next)
	size_t index;
	if(!freeIndices.empty())
	{
		index = freeIndices.back();
		freeIndices.pop_back();
	}
	else
	{
		index = nextIndex++;
	}

	// Register in all maps
	playersByUUID[uuid] = player;
	playersByIndex[index] = player;
	indexToUUID[index] = uuid;

	return index;
}



void PlayerRegistry::Unregister(const EsUuid &uuid)
{
	auto it = playersByUUID.find(uuid);
	if(it == playersByUUID.end())
		return;

	// Find index
	size_t index = static_cast<size_t>(-1);
	for(const auto &pair : indexToUUID)
	{
		if(pair.second == uuid)
		{
			index = pair.first;
			break;
		}
	}

	// Remove from all maps
	playersByUUID.erase(uuid);
	if(index != static_cast<size_t>(-1))
	{
		playersByIndex.erase(index);
		indexToUUID.erase(index);
		freeIndices.push_back(index);
	}
}



void PlayerRegistry::Unregister(size_t index)
{
	auto it = indexToUUID.find(index);
	if(it == indexToUUID.end())
		return;

	const EsUuid &uuid = it->second;

	// Remove from all maps
	playersByUUID.erase(uuid);
	playersByIndex.erase(index);
	indexToUUID.erase(index);
	freeIndices.push_back(index);
}



shared_ptr<NetworkPlayer> PlayerRegistry::GetByUUID(const EsUuid &uuid) const
{
	auto it = playersByUUID.find(uuid);
	return (it != playersByUUID.end()) ? it->second : nullptr;
}



bool PlayerRegistry::HasUUID(const EsUuid &uuid) const
{
	return playersByUUID.find(uuid) != playersByUUID.end();
}



shared_ptr<NetworkPlayer> PlayerRegistry::GetByIndex(size_t index) const
{
	auto it = playersByIndex.find(index);
	return (it != playersByIndex.end()) ? it->second : nullptr;
}



bool PlayerRegistry::HasIndex(size_t index) const
{
	return playersByIndex.find(index) != playersByIndex.end();
}



shared_ptr<NetworkPlayer> PlayerRegistry::GetByName(const string &name) const
{
	// Linear search (could be optimized with name->UUID map if needed)
	for(const auto &pair : playersByUUID)
	{
		if(pair.second && pair.second->GetName() == name)
			return pair.second;
	}
	return nullptr;
}



vector<shared_ptr<NetworkPlayer>> PlayerRegistry::GetAllPlayers() const
{
	vector<shared_ptr<NetworkPlayer>> players;
	players.reserve(playersByUUID.size());

	for(const auto &pair : playersByUUID)
		if(pair.second)
			players.push_back(pair.second);

	return players;
}



size_t PlayerRegistry::GetPlayerCount() const
{
	return playersByUUID.size();
}



vector<size_t> PlayerRegistry::GetActiveIndices() const
{
	vector<size_t> indices;
	indices.reserve(playersByIndex.size());

	for(const auto &pair : playersByIndex)
		indices.push_back(pair.first);

	// Sort for consistent ordering
	sort(indices.begin(), indices.end());

	return indices;
}



size_t PlayerRegistry::GetNextAvailableIndex() const
{
	return freeIndices.empty() ? nextIndex : freeIndices.back();
}



void PlayerRegistry::Clear()
{
	playersByUUID.clear();
	playersByIndex.clear();
	indexToUUID.clear();
	freeIndices.clear();
	nextIndex = 0;
}



bool PlayerRegistry::IsValid() const
{
	// All maps should have same size
	if(playersByUUID.size() != playersByIndex.size())
		return false;

	if(playersByIndex.size() != indexToUUID.size())
		return false;

	// All players should be valid
	for(const auto &pair : playersByUUID)
		if(!pair.second || !pair.second->IsValid())
			return false;

	return true;
}
