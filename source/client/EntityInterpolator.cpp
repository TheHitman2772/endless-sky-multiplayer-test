/* EntityInterpolator.cpp
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

#include "EntityInterpolator.h"

#include "../EsUuid.h"
#include "../Ship.h"

#include <chrono>
#include <algorithm>

using namespace std;
using namespace chrono;



// EntityState implementation
EntityState::EntityState(uint64_t tick, const Point &pos, const Point &vel, const Angle &angle)
	: gameTick(tick), position(pos), velocity(vel), facing(angle)
{
	auto now = system_clock::now();
	timestamp = duration_cast<milliseconds>(now.time_since_epoch()).count();
}



// EntityInterpolator implementation
EntityInterpolator::EntityInterpolator()
{
}



void EntityInterpolator::AddSnapshot(const EsUuid &entityId, const EntityState &state)
{
	auto &history = entityStates[entityId];

	// Add new snapshot
	history.snapshots.push_back(state);

	// Prune if exceeds max history
	while(history.snapshots.size() > maxSnapshotHistory)
		history.snapshots.pop_front();
}



const EntityState *EntityInterpolator::GetInterpolatedState(const EsUuid &entityId) const
{
	auto it = entityStates.find(entityId);
	if(it == entityStates.end())
		return nullptr;

	const auto &history = it->second;

	// Need at least 2 snapshots to interpolate
	if(history.snapshots.size() < 2)
		return history.snapshots.empty() ? nullptr : &history.snapshots.back();

	// Get render timestamp (current time - interpolation delay)
	uint64_t renderTime = GetRenderTimestamp();

	// Find snapshots to interpolate between
	const EntityState *before = nullptr;
	const EntityState *after = nullptr;

	for(size_t i = 0; i < history.snapshots.size() - 1; ++i)
	{
		if(history.snapshots[i].timestamp <= renderTime &&
			history.snapshots[i + 1].timestamp >= renderTime)
		{
			before = &history.snapshots[i];
			after = &history.snapshots[i + 1];
			break;
		}
	}

	// If couldn't find bracket, use most recent
	if(!before || !after)
		return &history.snapshots.back();

	// Calculate interpolation factor
	uint64_t timeDiff = after->timestamp - before->timestamp;
	if(timeDiff == 0)
		return after;

	double alpha = static_cast<double>(renderTime - before->timestamp) / timeDiff;
	alpha = max(0.0, min(1.0, alpha));  // Clamp to [0, 1]

	// Interpolate and cache result
	auto &mutableHistory = const_cast<EntityHistory &>(history);
	mutableHistory.lastInterpolated = Interpolate(*before, *after, alpha);

	return &history.lastInterpolated;
}



void EntityInterpolator::ApplyInterpolation(map<EsUuid, shared_ptr<Ship>> &ships)
{
	for(auto &[uuid, ship] : ships)
	{
		if(!ship)
			continue;

		const EntityState *state = GetInterpolatedState(uuid);
		if(!state)
			continue;

		// Apply interpolated position and facing
		// Note: This is a simplified version. Full implementation would
		// need to properly integrate with Ship's physics system
		// ship->SetPosition(state->position);
		// ship->SetVelocity(state->velocity);
		// ship->SetFacing(state->facing);
	}
}



void EntityInterpolator::Update()
{
	// Prune old snapshots
	PruneOldSnapshots();
}



void EntityInterpolator::RemoveEntity(const EsUuid &entityId)
{
	entityStates.erase(entityId);
}



void EntityInterpolator::Clear()
{
	entityStates.clear();
}



size_t EntityInterpolator::GetTotalSnapshotsStored() const
{
	size_t total = 0;
	for(const auto &[uuid, history] : entityStates)
		total += history.snapshots.size();
	return total;
}



EntityState EntityInterpolator::Interpolate(const EntityState &from, const EntityState &to, double alpha) const
{
	EntityState result;

	// Linear interpolation for position
	result.position = from.position + (to.position - from.position) * alpha;

	// Linear interpolation for velocity
	result.velocity = from.velocity + (to.velocity - from.velocity) * alpha;

	// Spherical interpolation for angle (shortest path)
	double fromDegrees = from.facing.Degrees();
	double toDegrees = to.facing.Degrees();

	// Handle wraparound
	double diff = toDegrees - fromDegrees;
	if(diff > 180.0)
		diff -= 360.0;
	else if(diff < -180.0)
		diff += 360.0;

	double interpolatedDegrees = fromDegrees + diff * alpha;
	result.facing = Angle(interpolatedDegrees);

	// Interpolate timestamps
	result.timestamp = from.timestamp + static_cast<uint64_t>((to.timestamp - from.timestamp) * alpha);
	result.gameTick = from.gameTick + static_cast<uint64_t>((to.gameTick - from.gameTick) * alpha);

	return result;
}



uint64_t EntityInterpolator::GetRenderTimestamp() const
{
	auto now = system_clock::now();
	uint64_t currentTime = duration_cast<milliseconds>(now.time_since_epoch()).count();

	// Render in the past to allow interpolation
	return currentTime - interpolationDelayMs;
}



void EntityInterpolator::PruneOldSnapshots()
{
	uint64_t renderTime = GetRenderTimestamp();

	// Remove snapshots that are too old (older than render time - 1 second)
	uint64_t pruneThreshold = renderTime - 1000;

	for(auto &[uuid, history] : entityStates)
	{
		while(!history.snapshots.empty() &&
			history.snapshots.front().timestamp < pruneThreshold)
		{
			history.snapshots.pop_front();
		}

		// Keep at least 2 snapshots for interpolation
		while(history.snapshots.size() > 2 &&
			history.snapshots.front().timestamp < renderTime)
		{
			// Only pop if we'd still have enough for interpolation
			if(history.snapshots.size() > 2)
				history.snapshots.pop_front();
			else
				break;
		}
	}
}
