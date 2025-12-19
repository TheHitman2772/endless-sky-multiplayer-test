// StateSync.h
// Phase 3.2: State synchronization for multiplayer ships
// Coordinates ship state updates with priority-based scheduling and interest management

#ifndef STATE_SYNC_H_
#define STATE_SYNC_H_

#include "InterestManager.h"
#include "DeadReckoning.h"
#include "../EsUuid.h"
#include "../Point.h"
#include "../Angle.h"

#include <map>
#include <vector>
#include <memory>
#include <cstdint>

class Ship;
class GameState;
class PacketWriter;



// StateSync manages synchronization of ship states across the network.
// It uses interest management to optimize bandwidth and dead reckoning
// for smooth client-side prediction.
class StateSync {
public:
	// Priority levels for ship state updates (aligned with InterestManager)
	enum class UpdatePriority {
		CRITICAL,  // Player's own ship, very close entities
		HIGH,      // Close entities, always visible
		MEDIUM,    // Medium distance
		LOW,       // Far but still in range
		NONE       // Out of range, not synced
	};

	// What data to include in a ship update
	enum class UpdateScope {
		FULL,      // All ship data (position, velocity, angle, shields, hull, etc.)
		POSITION,  // Only position, velocity, angle (motion data)
		VITAL,     // Only shields, hull, energy (combat data)
		MINIMAL    // Absolute minimum (position only)
	};

	// A single ship state update
	struct ShipUpdate {
		EsUuid shipUUID;
		Point position;
		Point velocity;
		Angle angle;
		float shields;         // 0.0 - 1.0
		float hull;            // 0.0 - 1.0
		float energy;          // 0.0 - 1.0
		float fuel;            // 0.0 - 1.0
		uint16_t flags;        // Status flags (thrusting, firing, etc.)
		UpdateScope scope;     // What data is valid
		uint64_t timestamp;    // Game tick when captured

		ShipUpdate() : shields(1.0f), hull(1.0f), energy(1.0f), fuel(1.0f),
			flags(0), scope(UpdateScope::FULL), timestamp(0) {}
	};

	StateSync();

	// Set the interest manager (must be called before use)
	void SetInterestManager(InterestManager *manager);

	// Set current game tick (call each frame)
	void SetCurrentTick(uint64_t tick);
	uint64_t GetCurrentTick() const;

	// Capture current ship state for synchronization
	ShipUpdate CaptureShipState(const Ship &ship, UpdateScope scope = UpdateScope::FULL);

	// Get all ships that need to be updated for a specific player this tick
	std::vector<ShipUpdate> GetUpdatesForPlayer(const EsUuid &playerUUID,
		const std::vector<std::shared_ptr<Ship>> &allShips);

	// Apply a ship update to local state (client-side)
	void ApplyShipUpdate(Ship &ship, const ShipUpdate &update);

	// Track dead reckoning for a specific ship
	void UpdateDeadReckoning(const EsUuid &shipUUID, const DeadReckoning::State &state);

	// Get predicted state for a ship
	DeadReckoning::State PredictShipState(const EsUuid &shipUUID, uint64_t targetTick);

	// Serialize ship update to packet
	void WriteShipUpdate(PacketWriter &writer, const ShipUpdate &update);

	// Deserialize ship update from packet (would use PacketReader, simplified here)
	// ShipUpdate ReadShipUpdate(PacketReader &reader);

	// Get update priority for a ship relative to a player
	UpdatePriority GetUpdatePriority(const EsUuid &playerUUID, const Ship &ship);

	// Determine update scope based on priority and bandwidth
	UpdateScope DetermineUpdateScope(UpdatePriority priority);

	// Clear all dead reckoning data
	void ClearDeadReckoning();

	// Get statistics
	size_t GetTrackedShipCount() const;


private:
	InterestManager *interestManager;
	uint64_t currentTick;

	// Dead reckoning state for each ship
	std::map<EsUuid, DeadReckoning> shipDeadReckoning;

	// Convert InterestLevel to UpdatePriority
	UpdatePriority InterestToPriority(InterestManager::InterestLevel level);
};



#endif
