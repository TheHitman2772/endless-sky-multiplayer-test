// InterestManager.h
// Phase 3.2: Interest management for network state synchronization
// Determines which entities are relevant to which clients based on proximity

#ifndef INTEREST_MANAGER_H_
#define INTEREST_MANAGER_H_

#include "../Point.h"
#include "../EsUuid.h"

#include <map>
#include <set>
#include <vector>
#include <memory>

class Ship;
class Projectile;
class Visual;



// InterestManager optimizes network bandwidth by only synchronizing entities
// that are within a client's area of interest (typically based on view distance).
class InterestManager {
public:
	// Interest level determines update priority
	enum class InterestLevel {
		NONE,       // Not in interest range, don't sync
		LOW,        // Far away, low priority updates (e.g., every 5th tick)
		MEDIUM,     // Medium distance, medium priority (e.g., every 2nd tick)
		HIGH,       // Close range, high priority (every tick)
		CRITICAL    // Very close or player's own ship (every tick, highest priority)
	};

	// Configuration for interest ranges (in game units)
	struct Config {
		double criticalRange = 1000.0;   // Player's own ship + very close entities
		double highRange = 3000.0;        // Close entities (visible on screen)
		double mediumRange = 6000.0;      // Medium distance
		double lowRange = 10000.0;        // Far but still relevant
		// Beyond lowRange = NONE (not synced)

		// Update frequencies (ticks between updates)
		int criticalFrequency = 1;  // Every tick
		int highFrequency = 1;      // Every tick
		int mediumFrequency = 2;    // Every 2nd tick
		int lowFrequency = 5;       // Every 5th tick
	};

	InterestManager();
	explicit InterestManager(const Config &config);

	// Set the configuration
	void SetConfig(const Config &config);
	const Config &GetConfig() const;

	// Update the center of interest for a player (typically their ship's position)
	void SetPlayerInterestCenter(const EsUuid &playerUUID, const Point &position);
	void RemovePlayer(const EsUuid &playerUUID);

	// Get the interest level for a specific entity relative to a player
	InterestLevel GetShipInterest(const EsUuid &playerUUID, const Ship &ship) const;
	InterestLevel GetProjectileInterest(const EsUuid &playerUUID, const Projectile &projectile) const;
	InterestLevel GetVisualInterest(const EsUuid &playerUUID, const Visual &visual) const;

	// Get all ships that are of interest to a specific player
	std::vector<const Ship *> GetInterestedShips(const EsUuid &playerUUID,
		const std::vector<std::shared_ptr<Ship>> &allShips) const;

	// Get all projectiles that are of interest to a specific player
	std::vector<const Projectile *> GetInterestedProjectiles(const EsUuid &playerUUID,
		const std::vector<Projectile> &allProjectiles) const;

	// Check if an entity should be updated this tick based on interest level and frequency
	bool ShouldUpdateThisTick(InterestLevel level, uint64_t currentTick) const;

	// Get distance-based interest level (helper method)
	InterestLevel GetInterestLevelByDistance(double distance) const;

	// Get the number of players being tracked
	size_t GetPlayerCount() const;

	// Clear all player interest data
	void Clear();


private:
	Config config;

	// Map of player UUID to their current interest center (usually ship position)
	std::map<EsUuid, Point> playerCenters;

	// Calculate distance from player's interest center to a point
	double GetDistanceToPlayer(const EsUuid &playerUUID, const Point &position) const;
};



#endif

