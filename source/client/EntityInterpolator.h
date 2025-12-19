/* EntityInterpolator.h
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

#pragma once

#include "../Point.h"
#include "../Angle.h"

#include <cstdint>
#include <deque>
#include <map>
#include <memory>

class EsUuid;
class Ship;


// EntityState: Snapshot of entity state at a specific time
struct EntityState {
	uint64_t gameTick = 0;
	Point position;
	Point velocity;
	Angle facing;
	uint64_t timestamp = 0;  // Milliseconds since epoch

	EntityState() = default;
	EntityState(uint64_t tick, const Point &pos, const Point &vel, const Angle &angle);
};


// EntityInterpolator: Smooth movement for remote entities
//
// Problem: Server sends updates at 20-30 Hz, but client renders at 60 FPS
// Solution: Interpolate between server snapshots for smooth visuals
//
// Technique: Render slightly in the past (interpolation delay)
// - Buffer last 3-5 server snapshots
// - Render at timestamp T - interpolationDelay
// - Interpolate position/rotation between surrounding snapshots
//
// Benefits:
// - Smooth 60 FPS visuals from 20 Hz updates
// - Tolerates jitter and packet loss
// - No visual lag compared to extrapolation
//
// Trade-off:
// - Remote entities are ~100ms behind true position
// - Acceptable for most multiplayer games
class EntityInterpolator {
public:
	EntityInterpolator();

	// Add server snapshot for an entity
	void AddSnapshot(const EsUuid &entityId, const EntityState &state);

	// Get interpolated state at current time
	// Returns nullptr if entity not found or insufficient data
	const EntityState *GetInterpolatedState(const EsUuid &entityId) const;

	// Apply interpolated states to ships
	void ApplyInterpolation(std::map<EsUuid, std::shared_ptr<Ship>> &ships);

	// Update (called every frame)
	void Update();

	// Remove entity from tracking
	void RemoveEntity(const EsUuid &entityId);

	// Clear all entities
	void Clear();

	// Configuration
	void SetInterpolationDelay(uint32_t milliseconds) { interpolationDelayMs = milliseconds; }
	uint32_t GetInterpolationDelay() const { return interpolationDelayMs; }

	void SetMaxSnapshotHistory(size_t count) { maxSnapshotHistory = count; }
	size_t GetMaxSnapshotHistory() const { return maxSnapshotHistory; }

	// Statistics
	size_t GetTrackedEntityCount() const { return entityStates.size(); }
	size_t GetTotalSnapshotsStored() const;


private:
	// Per-entity state history
	struct EntityHistory {
		std::deque<EntityState> snapshots;
		EntityState lastInterpolated;  // Cached result
	};

	std::map<EsUuid, EntityHistory> entityStates;

	// Configuration
	uint32_t interpolationDelayMs = 100;  // Render 100ms in past
	size_t maxSnapshotHistory = 5;        // Keep last 5 snapshots

	// Helper methods
	EntityState Interpolate(const EntityState &from, const EntityState &to, double alpha) const;
	uint64_t GetRenderTimestamp() const;
	void PruneOldSnapshots();
};
