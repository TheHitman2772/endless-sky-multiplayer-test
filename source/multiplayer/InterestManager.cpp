// InterestManager.cpp

#include "InterestManager.h"

#include "../Ship.h"
#include "../Projectile.h"
#include "../Visual.h"

#include <cmath>
#include <algorithm>

using namespace std;



InterestManager::InterestManager()
	: config()
{
}



InterestManager::InterestManager(const Config &config)
	: config(config)
{
}



void InterestManager::SetConfig(const Config &config)
{
	this->config = config;
}



const InterestManager::Config &InterestManager::GetConfig() const
{
	return config;
}



void InterestManager::SetPlayerInterestCenter(const EsUuid &playerUUID, const Point &position)
{
	playerCenters[playerUUID] = position;
}



void InterestManager::RemovePlayer(const EsUuid &playerUUID)
{
	playerCenters.erase(playerUUID);
}



InterestManager::InterestLevel InterestManager::GetShipInterest(const EsUuid &playerUUID, const Ship &ship) const
{
	// Check if this ship belongs to the player (always CRITICAL)
	if(ship.GetOwnerPlayerUUID() == playerUUID)
		return InterestLevel::CRITICAL;

	double distance = GetDistanceToPlayer(playerUUID, ship.Position());
	return GetInterestLevelByDistance(distance);
}



InterestManager::InterestLevel InterestManager::GetProjectileInterest(const EsUuid &playerUUID,
	const Projectile &projectile) const
{
	// Projectiles use position from Body base class
	double distance = GetDistanceToPlayer(playerUUID, projectile.Position());

	// Projectiles are generally more critical if close (they can hit you)
	// Use slightly tighter ranges
	if(distance < config.criticalRange * 0.5)
		return InterestLevel::CRITICAL;
	else if(distance < config.highRange * 0.75)
		return InterestLevel::HIGH;
	else if(distance < config.mediumRange)
		return InterestLevel::MEDIUM;
	else if(distance < config.lowRange)
		return InterestLevel::LOW;
	else
		return InterestLevel::NONE;
}



InterestManager::InterestLevel InterestManager::GetVisualInterest(const EsUuid &playerUUID,
	const Visual &visual) const
{
	double distance = GetDistanceToPlayer(playerUUID, visual.Position());

	// Visuals (effects) are less critical, use standard ranges
	return GetInterestLevelByDistance(distance);
}



vector<const Ship *> InterestManager::GetInterestedShips(const EsUuid &playerUUID,
	const vector<shared_ptr<Ship>> &allShips) const
{
	vector<const Ship *> interestedShips;

	for(const auto &ship : allShips)
	{
		if(!ship)
			continue;

		InterestLevel level = GetShipInterest(playerUUID, *ship);
		if(level != InterestLevel::NONE)
			interestedShips.push_back(ship.get());
	}

	return interestedShips;
}



vector<const Projectile *> InterestManager::GetInterestedProjectiles(const EsUuid &playerUUID,
	const vector<Projectile> &allProjectiles) const
{
	vector<const Projectile *> interestedProjectiles;

	for(const auto &projectile : allProjectiles)
	{
		InterestLevel level = GetProjectileInterest(playerUUID, projectile);
		if(level != InterestLevel::NONE)
			interestedProjectiles.push_back(&projectile);
	}

	return interestedProjectiles;
}



bool InterestManager::ShouldUpdateThisTick(InterestLevel level, uint64_t currentTick) const
{
	switch(level)
	{
		case InterestLevel::CRITICAL:
			return currentTick % config.criticalFrequency == 0;
		case InterestLevel::HIGH:
			return currentTick % config.highFrequency == 0;
		case InterestLevel::MEDIUM:
			return currentTick % config.mediumFrequency == 0;
		case InterestLevel::LOW:
			return currentTick % config.lowFrequency == 0;
		case InterestLevel::NONE:
		default:
			return false;
	}
}



InterestManager::InterestLevel InterestManager::GetInterestLevelByDistance(double distance) const
{
	if(distance < config.criticalRange)
		return InterestLevel::CRITICAL;
	else if(distance < config.highRange)
		return InterestLevel::HIGH;
	else if(distance < config.mediumRange)
		return InterestLevel::MEDIUM;
	else if(distance < config.lowRange)
		return InterestLevel::LOW;
	else
		return InterestLevel::NONE;
}



size_t InterestManager::GetPlayerCount() const
{
	return playerCenters.size();
}



void InterestManager::Clear()
{
	playerCenters.clear();
}



double InterestManager::GetDistanceToPlayer(const EsUuid &playerUUID, const Point &position) const
{
	auto it = playerCenters.find(playerUUID);
	if(it == playerCenters.end())
		return numeric_limits<double>::max(); // Player not found, infinite distance

	const Point &playerCenter = it->second;
	Point delta = position - playerCenter;
	return sqrt(delta.X() * delta.X() + delta.Y() * delta.Y());
}
