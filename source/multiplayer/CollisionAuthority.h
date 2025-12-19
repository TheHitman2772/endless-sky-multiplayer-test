// CollisionAuthority.h
// Phase 3.3: Server-authoritative collision detection for multiplayer
// Ensures all clients see consistent collision results

#ifndef COLLISION_AUTHORITY_H_
#define COLLISION_AUTHORITY_H_

#include "../Point.h"
#include "../EsUuid.h"

#include <vector>
#include <memory>
#include <map>
#include <cstdint>

class Projectile;
class Ship;
class Minable;
class GameState;
class ProjectileSync;



// CollisionAuthority handles server-side collision detection for projectiles.
// Only the server runs collision detection; clients receive impact events.
class CollisionAuthority {
public:
	// Collision result for a single projectile
	struct CollisionResult {
		uint32_t projectileNetworkID;    // Network ID of projectile that collided
		enum class Type {
			NONE,             // No collision
			SHIP,             // Hit a ship
			ASTEROID,         // Hit an asteroid/minable
			EXPIRED           // Lifetime expired (no impact)
		} type;

		EsUuid targetUUID;                // UUID of target (ship/asteroid)
		Point impactPosition;             // Where the collision occurred
		double intersection;              // Intersection distance for visual effects
		bool targetDestroyed;             // Did this hit destroy the target?

		CollisionResult() : projectileNetworkID(0), type(Type::NONE),
			intersection(0.0), targetDestroyed(false) {}
	};

	CollisionAuthority();

	// Set the ProjectileSync instance (for network ID lookups)
	void SetProjectileSync(ProjectileSync *sync);

	// Run collision detection for all projectiles
	// Returns list of collisions that occurred this frame
	std::vector<CollisionResult> DetectCollisions(GameState &gameState);

	// Check collision between a specific projectile and all potential targets
	CollisionResult CheckProjectileCollision(const Projectile &projectile,
		uint32_t networkID, GameState &gameState);

	// Check if projectile hits a specific ship
	bool CheckProjectileShipCollision(const Projectile &projectile, const Ship &ship,
		double &intersection);

	// Check if projectile hits a specific asteroid
	bool CheckProjectileAsteroidCollision(const Projectile &projectile,
		const std::shared_ptr<Minable> &asteroid, double &intersection);

	// Get collision detection statistics
	uint64_t GetTotalCollisionsDetected() const;
	uint64_t GetShipHits() const;
	uint64_t GetAsteroidHits() const;

	// Reset statistics
	void ResetStatistics();


private:
	ProjectileSync *projectileSync;

	// Statistics
	uint64_t totalCollisions;
	uint64_t shipHits;
	uint64_t asteroidHits;

	// Helper: Check if two circles collide (simple collision)
	bool CirclesCollide(const Point &pos1, double radius1,
		const Point &pos2, double radius2, double &distance) const;

	// Helper: Calculate intersection point for visual effects
	double CalculateIntersection(const Point &projectilePos, const Point &targetPos,
		double targetRadius) const;
};



#endif
