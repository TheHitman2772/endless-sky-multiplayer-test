/* PlayerManager.cpp
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

#include "PlayerManager.h"

#include "NetworkPlayer.h"
#include "../Ship.h"

using namespace std;



PlayerManager::PlayerManager()
{
}



shared_ptr<NetworkPlayer> PlayerManager::AddPlayer(const EsUuid &uuid, const string &name)
{
	// Check if already exists
	if(registry.HasUUID(uuid))
		return registry.GetByUUID(uuid);

	// Create new player
	auto player = make_shared<NetworkPlayer>(uuid, name);

	// Register
	registry.Register(player);

	// Notify
	if(onPlayerJoin)
		onPlayerJoin(player);

	return player;
}



shared_ptr<NetworkPlayer> PlayerManager::AddPlayer(shared_ptr<NetworkPlayer> player)
{
	if(!player)
		return nullptr;

	// Check if already exists
	if(registry.HasUUID(player->GetUUID()))
		return registry.GetByUUID(player->GetUUID());

	// Register
	registry.Register(player);

	// Notify
	if(onPlayerJoin)
		onPlayerJoin(player);

	return player;
}



void PlayerManager::RemovePlayer(const EsUuid &uuid)
{
	auto player = registry.GetByUUID(uuid);
	if(!player)
		return;

	// Remove all ship ownership
	auto ships = player->GetShips();
	for(const auto &weakShip : ships)
	{
		if(auto ship = weakShip.lock())
			UnassignShipFromPlayer(ship);
	}

	// Notify before removal
	if(onPlayerLeave)
		onPlayerLeave(player);

	// Unregister
	registry.Unregister(uuid);
}



void PlayerManager::RemovePlayer(shared_ptr<NetworkPlayer> player)
{
	if(!player)
		return;

	RemovePlayer(player->GetUUID());
}



shared_ptr<NetworkPlayer> PlayerManager::GetPlayer(const EsUuid &uuid) const
{
	return registry.GetByUUID(uuid);
}



shared_ptr<NetworkPlayer> PlayerManager::GetPlayer(size_t index) const
{
	return registry.GetByIndex(index);
}



shared_ptr<NetworkPlayer> PlayerManager::GetPlayerByName(const string &name) const
{
	return registry.GetByName(name);
}



shared_ptr<NetworkPlayer> PlayerManager::GetPlayerByConnection(NetworkConnection *conn) const
{
	if(!conn)
		return nullptr;

	// Linear search (could be optimized with connection->UUID map)
	for(const auto &player : registry.GetAllPlayers())
	{
		if(player && player->GetConnection() == conn)
			return player;
	}

	return nullptr;
}



bool PlayerManager::HasPlayer(const EsUuid &uuid) const
{
	return registry.HasUUID(uuid);
}



size_t PlayerManager::GetPlayerCount() const
{
	return registry.GetPlayerCount();
}



vector<shared_ptr<NetworkPlayer>> PlayerManager::GetAllPlayers() const
{
	return registry.GetAllPlayers();
}



vector<shared_ptr<NetworkPlayer>> PlayerManager::GetConnectedPlayers() const
{
	vector<shared_ptr<NetworkPlayer>> connected;

	for(const auto &player : registry.GetAllPlayers())
	{
		if(player && player->GetStatus() == NetworkPlayer::Status::CONNECTED)
			connected.push_back(player);
	}

	return connected;
}



void PlayerManager::AssignShipToPlayer(shared_ptr<Ship> ship, shared_ptr<NetworkPlayer> player)
{
	if(!ship || !player)
		return;

	// Track ownership
	shipOwnership[ship] = player->GetUUID();

	// Add ship to player
	player->AddShip(ship);

	// Notify
	if(onShipOwnership)
		onShipOwnership(ship, player);
}



void PlayerManager::UnassignShipFromPlayer(shared_ptr<Ship> ship)
{
	if(!ship)
		return;

	auto it = shipOwnership.find(ship);
	if(it == shipOwnership.end())
		return;

	// Get owner
	auto player = registry.GetByUUID(it->second);
	if(player)
		player->RemoveShip(ship);

	// Remove ownership
	shipOwnership.erase(it);

	// Notify (with nullptr player to indicate removal)
	if(onShipOwnership)
		onShipOwnership(ship, nullptr);
}



shared_ptr<NetworkPlayer> PlayerManager::GetShipOwner(shared_ptr<Ship> ship) const
{
	if(!ship)
		return nullptr;

	auto it = shipOwnership.find(ship);
	if(it == shipOwnership.end())
		return nullptr;

	return registry.GetByUUID(it->second);
}



bool PlayerManager::IsShipOwnedByPlayer(shared_ptr<Ship> ship, shared_ptr<NetworkPlayer> player) const
{
	if(!ship || !player)
		return false;

	auto owner = GetShipOwner(ship);
	return owner && owner->GetUUID() == player->GetUUID();
}



void PlayerManager::UpdatePlayerActivity(const EsUuid &uuid, uint64_t timestamp)
{
	auto player = registry.GetByUUID(uuid);
	if(player)
		player->UpdateActivity(timestamp);
}



void PlayerManager::DisconnectInactivePlayers(uint64_t currentTime, uint64_t timeoutMs)
{
	vector<EsUuid> toRemove;

	for(const auto &player : registry.GetAllPlayers())
	{
		if(!player)
			continue;

		// Check if timeout exceeded
		if(player->GetStatus() == NetworkPlayer::Status::CONNECTED)
		{
			uint64_t inactiveTime = currentTime - player->GetLastActivityTime();
			if(inactiveTime > timeoutMs)
			{
				player->SetStatus(NetworkPlayer::Status::DISCONNECTED);
				toRemove.push_back(player->GetUUID());
			}
		}
	}

	// Remove disconnected players
	for(const auto &uuid : toRemove)
		RemovePlayer(uuid);
}



void PlayerManager::Clear()
{
	// Clear all ship ownership
	shipOwnership.clear();

	// Clear registry
	registry.Clear();

	// Clear callbacks
	onPlayerJoin = nullptr;
	onPlayerLeave = nullptr;
	onShipOwnership = nullptr;
}



bool PlayerManager::IsValid() const
{
	// Registry must be valid
	if(!registry.IsValid())
		return false;

	// All ship owners must exist in registry
	for(const auto &pair : shipOwnership)
	{
		if(!registry.HasUUID(pair.second))
			return false;
	}

	return true;
}
