// ProjectileSync.h
// Phase 3.3: Projectile synchronization for multiplayer
// Coordinates projectile spawning, updates, and destruction across network

#ifndef PROJECTILE_SYNC_H_
#define PROJECTILE_SYNC_H_

#include "../Point.h"
#include "../Angle.h"
#include "../EsUuid.h"

#include <map>
#include <vector>
#include <memory>
#include <cstdint>
#include <string>

class Projectile;
class Weapon;
class Ship;
class PacketWriter;
class PacketReader;
class GameState;



// ProjectileSync manages synchronization of projectiles across the network.
// Server is authoritative for spawning and collisions, clients simulate movement.
class ProjectileSync {
public:
	// Projectile spawn event (server → client)
	struct ProjectileSpawn {
		uint32_t projectileID;        // Network ID for this projectile
		std::string weaponName;        // Name of the weapon that fired it
		EsUuid firingShipUUID;         // Ship that fired this projectile
		EsUuid targetShipUUID;         // Target ship (if homing/guided)
		Point position;                // Initial position
		Point velocity;                // Initial velocity
		Angle angle;                   // Facing angle
		uint64_t spawnTick;            // Game tick when spawned

		ProjectileSpawn() : projectileID(0), spawnTick(0) {}
	};

	// Projectile impact event (server → client)
	struct ProjectileImpact {
		uint32_t projectileID;         // Which projectile hit
		EsUuid targetUUID;             // What it hit (ship/asteroid UUID)
		Point impactPosition;          // Where the impact occurred
		double intersection;           // Intersection distance (for visual effects)
		uint64_t impactTick;           // Game tick when impact occurred

		ProjectileImpact() : projectileID(0), intersection(0.0), impactTick(0) {}
	};

	// Projectile destruction event (server → client, no hit)
	struct ProjectileDeath {
		uint32_t projectileID;         // Which projectile died
		Point deathPosition;           // Final position
		uint64_t deathTick;            // Game tick when died

		ProjectileDeath() : projectileID(0), deathTick(0) {}
	};

	ProjectileSync();

	// Set current game tick (call each frame)
	void SetCurrentTick(uint64_t tick);
	uint64_t GetCurrentTick() const;

	// Server-side: Register a newly spawned projectile
	uint32_t RegisterProjectileSpawn(const Projectile &projectile, const Ship *firingShip,
		const std::string &weaponName);

	// Server-side: Get all projectile spawns since last call
	std::vector<ProjectileSpawn> GetPendingSpawns();

	// Server-side: Register a projectile impact
	void RegisterImpact(uint32_t projectileID, const Ship *target, const Point &impactPos,
		double intersection);

	// Server-side: Register a projectile death (lifetime expired, anti-missile, etc.)
	void RegisterDeath(uint32_t projectileID, const Point &deathPos);

	// Server-side: Get all impacts since last call
	std::vector<ProjectileImpact> GetPendingImpacts();

	// Server-side: Get all deaths since last call
	std::vector<ProjectileDeath> GetPendingDeaths();

	// Client-side: Apply a projectile spawn from server
	void ApplySpawn(const ProjectileSpawn &spawn, GameState &gameState);

	// Client-side: Apply a projectile impact from server
	void ApplyImpact(const ProjectileImpact &impact, GameState &gameState);

	// Client-side: Apply a projectile death from server
	void ApplyDeath(const ProjectileDeath &death, GameState &gameState);

	// Find a projectile by network ID
	Projectile *FindProjectileByNetworkID(uint32_t networkID, GameState &gameState);

	// Get network ID for a projectile (if tracked)
	uint32_t GetNetworkID(const Projectile *projectile) const;

	// Check if a projectile is tracked
	bool IsTracked(const Projectile *projectile) const;

	// Serialize spawn to packet
	void WriteSpawn(PacketWriter &writer, const ProjectileSpawn &spawn);
	ProjectileSpawn ReadSpawn(PacketReader &reader);

	// Serialize impact to packet
	void WriteImpact(PacketWriter &writer, const ProjectileImpact &impact);
	ProjectileImpact ReadImpact(PacketReader &reader);

	// Serialize death to packet
	void WriteDeath(PacketWriter &writer, const ProjectileDeath &death);
	ProjectileDeath ReadDeath(PacketReader &reader);

	// Clear all tracking data
	void Clear();

	// Get statistics
	size_t GetTrackedProjectileCount() const;
	uint32_t GetNextProjectileID() const;


private:
	uint64_t currentTick;
	uint32_t nextProjectileID;

	// Pending events (cleared after GetPending* calls)
	std::vector<ProjectileSpawn> pendingSpawns;
	std::vector<ProjectileImpact> pendingImpacts;
	std::vector<ProjectileDeath> pendingDeaths;

	// Mapping of network ID to projectile pointer (for server tracking)
	std::map<uint32_t, const Projectile *> networkIDToProjectile;
	std::map<const Projectile *, uint32_t> projectileToNetworkID;
};



#endif
