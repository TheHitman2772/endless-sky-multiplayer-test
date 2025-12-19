// CollisionAuthority.cpp

#include "CollisionAuthority.h"

#include "ProjectileSync.h"
#include "../Projectile.h"
#include "../Ship.h"
#include "../Minable.h"
#include "../GameState.h"
#include "../Body.h"

#include <cmath>
#include <algorithm>

using namespace std;



CollisionAuthority::CollisionAuthority()
	: projectileSync(nullptr), totalCollisions(0), shipHits(0), asteroidHits(0)
{
}



void CollisionAuthority::SetProjectileSync(ProjectileSync *sync)
{
	projectileSync = sync;
}



vector<CollisionAuthority::CollisionResult> CollisionAuthority::DetectCollisions(
	GameState &gameState)
{
	vector<CollisionResult> results;

	// Note: In full implementation, we'd iterate through gameState.projectiles
	// For now, this is a placeholder showing the structure

	// Placeholder implementation
	// for(auto &projectile : gameState.GetProjectiles())
	// {
	//     if(projectile.IsDead())
	//         continue;
	//
	//     uint32_t networkID = projectileSync ? projectileSync->GetNetworkID(&projectile) : 0;
	//     if(networkID == 0)
	//         continue;
	//
	//     CollisionResult result = CheckProjectileCollision(projectile, networkID, gameState);
	//     if(result.type != CollisionResult::Type::NONE)
	//         results.push_back(result);
	// }

	return results;
}



CollisionAuthority::CollisionResult CollisionAuthority::CheckProjectileCollision(
	const Projectile &projectile, uint32_t networkID, GameState &gameState)
{
	CollisionResult result;
	result.projectileNetworkID = networkID;
	result.type = CollisionResult::Type::NONE;

	// Check if projectile lifetime expired
	if(projectile.IsDead())
	{
		result.type = CollisionResult::Type::EXPIRED;
		result.impactPosition = projectile.Position();
		return result;
	}

	// Check collision with ships
	// In full implementation:
	// for(auto &ship : gameState.GetShips())
	// {
	//     if(!ship || ship->IsDestroyed())
	//         continue;
	//
	//     // Skip if same government (friendly fire check)
	//     if(ship->GetGovernment() == projectile.GetGovernment())
	//         continue;
	//
	//     double intersection;
	//     if(CheckProjectileShipCollision(projectile, *ship, intersection))
	//     {
	//         result.type = CollisionResult::Type::SHIP;
	//         result.targetUUID = ship->UUID();
	//         result.impactPosition = projectile.Position();
	//         result.intersection = intersection;
	//         result.targetDestroyed = (ship->Hull() <= 0); // After applying damage
	//         shipHits++;
	//         totalCollisions++;
	//         return result;
	//     }
	// }

	// Check collision with asteroids
	// Similar implementation for minables

	return result;
}



bool CollisionAuthority::CheckProjectileShipCollision(const Projectile &projectile,
	const Ship &ship, double &intersection)
{
	// Simple circle-circle collision (actual implementation would use proper masks)
	double projectileRadius = 5.0;  // Placeholder
	double shipRadius = ship.Radius();

	double distance;
	if(CirclesCollide(projectile.Position(), projectileRadius,
		ship.Position(), shipRadius, distance))
	{
		intersection = CalculateIntersection(projectile.Position(), ship.Position(), shipRadius);
		return true;
	}

	return false;
}



bool CollisionAuthority::CheckProjectileAsteroidCollision(const Projectile &projectile,
	const shared_ptr<Minable> &asteroid, double &intersection)
{
	if(!asteroid)
		return false;

	// Simple collision check
	double projectileRadius = 5.0;  // Placeholder
	double asteroidRadius = asteroid->Radius();

	double distance;
	if(CirclesCollide(projectile.Position(), projectileRadius,
		asteroid->Position(), asteroidRadius, distance))
	{
		intersection = CalculateIntersection(projectile.Position(), asteroid->Position(),
			asteroidRadius);
		asteroidHits++;
		totalCollisions++;
		return true;
	}

	return false;
}



uint64_t CollisionAuthority::GetTotalCollisionsDetected() const
{
	return totalCollisions;
}



uint64_t CollisionAuthority::GetShipHits() const
{
	return shipHits;
}



uint64_t CollisionAuthority::GetAsteroidHits() const
{
	return asteroidHits;
}



void CollisionAuthority::ResetStatistics()
{
	totalCollisions = 0;
	shipHits = 0;
	asteroidHits = 0;
}



bool CollisionAuthority::CirclesCollide(const Point &pos1, double radius1,
	const Point &pos2, double radius2, double &distance) const
{
	Point delta = pos2 - pos1;
	distance = sqrt(delta.X() * delta.X() + delta.Y() * delta.Y());

	return distance < (radius1 + radius2);
}



double CollisionAuthority::CalculateIntersection(const Point &projectilePos,
	const Point &targetPos, double targetRadius) const
{
	Point delta = targetPos - projectilePos;
	double distance = sqrt(delta.X() * delta.X() + delta.Y() * delta.Y());

	// Intersection is how far into the target the projectile penetrated
	// (used for visual effects)
	return max(0.0, targetRadius - distance);
}
