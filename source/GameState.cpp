/* GameState.cpp
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

#include "GameState.h"

#include "AsteroidField.h"
#include "Flotsam.h"
#include "Projectile.h"
#include "Ship.h"
#include "System.h"
#include "Visual.h"

#include <algorithm>

using namespace std;



GameState::GameState()
{
}



// Copy constructor - deep copy for prediction/rollback
GameState::GameState(const GameState &other)
	: currentSystem(other.currentSystem), gameTick(other.gameTick)
{
	// Deep copy ships
	for(const auto &ship : other.ships)
		if(ship)
			ships.emplace_back(make_shared<Ship>(*ship));

	// Copy projectiles (value type)
	projectiles = other.projectiles;

	// Deep copy flotsam
	for(const auto &item : other.flotsam)
		if(item)
			flotsam.emplace_back(make_shared<Flotsam>(*item));

	// Copy visuals (value type)
	visuals = other.visuals;

	// Deep copy asteroid field if present
	if(other.asteroids)
		asteroids = make_unique<AsteroidField>(*other.asteroids);
}



// Copy assignment
GameState &GameState::operator=(const GameState &other)
{
	if(this != &other)
	{
		currentSystem = other.currentSystem;
		gameTick = other.gameTick;

		// Clear existing data
		ships.clear();
		projectiles.clear();
		flotsam.clear();
		visuals.clear();
		asteroids.reset();

		// Deep copy ships
		for(const auto &ship : other.ships)
			if(ship)
				ships.emplace_back(make_shared<Ship>(*ship));

		// Copy projectiles
		projectiles = other.projectiles;

		// Deep copy flotsam
		for(const auto &item : other.flotsam)
			if(item)
				flotsam.emplace_back(make_shared<Flotsam>(*item));

		// Copy visuals
		visuals = other.visuals;

		// Deep copy asteroid field
		if(other.asteroids)
			asteroids = make_unique<AsteroidField>(*other.asteroids);
	}
	return *this;
}



// Simulation step (advance game by one tick at 60 Hz)
void GameState::Step()
{
	// Increment game tick
	++gameTick;

	// Move all ships (this would normally involve AI, physics, etc.)
	// Ships generate visuals and flotsam as they move
	// Convert std::list<Visual> to std::vector<Visual> temporarily
	vector<Visual> visualsVec(visuals.begin(), visuals.end());

	for(auto &ship : ships)
		if(ship)
			ship->Move(visualsVec, flotsam);

	// Convert back to list
	visuals.clear();
	visuals.insert(visuals.end(), visualsVec.begin(), visualsVec.end());

	// Move projectiles and handle expiration
	// Projectiles create visuals and submunitions when they expire
	// For simplicity, we're not fully implementing this yet
	auto projIt = projectiles.begin();
	while(projIt != projectiles.end())
	{
		// Projectile::Move() would normally be called here
		// For now, just check if dead
		if(projIt->IsDead())
			projIt = projectiles.erase(projIt);
		else
			++projIt;
	}

	// Move visual effects and remove expired ones
	auto visualIt = visuals.begin();
	while(visualIt != visuals.end())
	{
		visualIt->Move();
		// Visuals expire after their lifetime ends
		// For now, we'll keep them (would need lifetime check)
		++visualIt;
	}

	// Update asteroid field if present
	// Asteroid field generates visuals and flotsam when asteroids are destroyed
	if(asteroids)
	{
		vector<Visual> visualsVec(visuals.begin(), visuals.end());
		asteroids->Step(visualsVec, flotsam, gameTick);
		visuals.clear();
		visuals.insert(visuals.end(), visualsVec.begin(), visualsVec.end());
	}
}



void GameState::SetSystem(const System *system)
{
	currentSystem = system;
}



void GameState::AddShip(shared_ptr<Ship> ship)
{
	if(ship)
		ships.push_back(ship);
}



void GameState::RemoveShip(shared_ptr<Ship> ship)
{
	if(ship)
		ships.remove(ship);
}



void GameState::AddProjectile(const Projectile &projectile)
{
	projectiles.push_back(projectile);
}



void GameState::RemoveProjectile(const Projectile &projectile)
{
	// Find and remove by comparing addresses
	// This is tricky with value types - we need a better comparison
	// For now, remove by position match
	projectiles.remove_if([&](const Projectile &p) {
		return &p == &projectile;
	});
}



void GameState::AddFlotsam(shared_ptr<Flotsam> item)
{
	if(item)
		flotsam.push_back(item);
}



void GameState::RemoveFlotsam(shared_ptr<Flotsam> item)
{
	if(item)
		flotsam.remove(item);
}



void GameState::AddVisual(const Visual &visual)
{
	visuals.push_back(visual);
}



void GameState::SetAsteroids(unique_ptr<AsteroidField> field)
{
	asteroids = move(field);
}



void GameState::Clear()
{
	currentSystem = nullptr;
	ships.clear();
	projectiles.clear();
	flotsam.clear();
	visuals.clear();
	asteroids.reset();
	gameTick = 0;
}



bool GameState::IsValid() const
{
	// Basic validation checks
	if(!currentSystem)
		return false;

	// Check that all ships are valid
	for(const auto &ship : ships)
		if(!ship)
			return false;

	// Check that all flotsam are valid
	for(const auto &item : flotsam)
		if(!item)
			return false;

	return true;
}
