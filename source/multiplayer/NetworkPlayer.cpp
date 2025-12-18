/* NetworkPlayer.cpp
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

#include "NetworkPlayer.h"

#include "../Mission.h"
#include "../Ship.h"

#include <algorithm>

using namespace std;



NetworkPlayer::NetworkPlayer()
{
	// UUID starts empty - will be set explicitly or generated when needed
}



NetworkPlayer::NetworkPlayer(const EsUuid &uuid)
	: uuid(uuid)
{
}



NetworkPlayer::NetworkPlayer(const EsUuid &uuid, const string &name)
	: uuid(uuid), name(name)
{
}



void NetworkPlayer::AddShip(shared_ptr<Ship> ship)
{
	if(!ship)
		return;

	// Check if ship already in list
	for(const auto &weakShip : ships)
		if(weakShip.lock() == ship)
			return;

	ships.emplace_back(ship);

	// If no flagship, set this as flagship
	if(!flagship.lock())
		flagship = ship;
}



void NetworkPlayer::RemoveShip(shared_ptr<Ship> ship)
{
	if(!ship)
		return;

	// Remove from ships list
	ships.erase(
		remove_if(ships.begin(), ships.end(),
			[&ship](const weak_ptr<Ship> &weakShip) {
				return weakShip.lock() == ship;
			}),
		ships.end()
	);

	// Clear flagship if it was this ship
	if(flagship.lock() == ship)
	{
		flagship.reset();
		// Set new flagship to first remaining ship
		for(auto &weakShip : ships)
		{
			if(auto remainingShip = weakShip.lock())
			{
				flagship = remainingShip;
				break;
			}
		}
	}
}



void NetworkPlayer::ClearShips()
{
	ships.clear();
	flagship.reset();
}



size_t NetworkPlayer::GetShipCount() const
{
	// Count valid ships (not expired weak_ptr)
	size_t count = 0;
	for(const auto &weakShip : ships)
		if(weakShip.lock())
			++count;
	return count;
}



void NetworkPlayer::AddMission(shared_ptr<Mission> mission)
{
	if(mission)
		missions.push_back(mission);
}



void NetworkPlayer::RemoveMission(size_t index)
{
	if(index < missions.size())
		missions.erase(missions.begin() + index);
}



void NetworkPlayer::ClearMissions()
{
	missions.clear();
}



void NetworkPlayer::UpdateActivity(uint64_t time)
{
	lastActivityTime = time;
}



bool NetworkPlayer::IsValid() const
{
	// Must have valid UUID
	if(uuid.ToString().empty())
		return false;

	// Must have name
	if(name.empty())
		return false;

	// If connected, must have connection
	if(status == Status::CONNECTED && !connection)
		return false;

	return true;
}
