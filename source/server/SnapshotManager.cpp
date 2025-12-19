/* SnapshotManager.cpp
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

#include "SnapshotManager.h"

#include "../GameState.h"

#include <chrono>
#include <algorithm>

using namespace std;



// Snapshot implementation
Snapshot::Snapshot(uint64_t tick, shared_ptr<GameState> gameState)
	: gameTick(tick), state(gameState)
{
	// Record timestamp
	auto now = chrono::system_clock::now();
	timestamp = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();
}



// SnapshotManager implementation
SnapshotManager::SnapshotManager(size_t historySize)
	: historySize(historySize)
{
}



void SnapshotManager::CreateSnapshot(const GameState &currentState, uint64_t gameTick, bool forceKeyframe)
{
	// Check if we should create a keyframe
	bool isKeyframe = forceKeyframe || ShouldCreateKeyframe();

	// Create snapshot
	auto state = make_shared<GameState>(currentState);
	Snapshot snapshot(gameTick, state);
	snapshot.isKeyframe = isKeyframe;

	// Estimate sizes
	size_t uncompressedSize = EstimateSnapshotSize(currentState);
	size_t compressedSize = uncompressedSize;

	// Calculate delta compression if not a keyframe
	if(!isKeyframe && !snapshots.empty())
	{
		const auto &previousState = snapshots.back().state;
		if(previousState)
			compressedSize = CalculateDelta(*previousState, currentState);
	}

	snapshot.compressedSize = compressedSize;

	// Update statistics
	++totalSnapshots;
	if(isKeyframe)
	{
		++totalKeyframes;
		snapshotsSinceLastKeyframe = 0;
	}
	else
		++snapshotsSinceLastKeyframe;

	totalUncompressedBytes += uncompressedSize;
	totalCompressedBytes += compressedSize;

	// Add to history
	snapshots.push_back(snapshot);

	// Prune if exceeds history size
	while(snapshots.size() > historySize)
		snapshots.pop_front();
}



const Snapshot *SnapshotManager::GetLatestSnapshot() const
{
	if(snapshots.empty())
		return nullptr;
	return &snapshots.back();
}



const Snapshot *SnapshotManager::GetSnapshotAtTick(uint64_t gameTick) const
{
	// Binary search for tick
	auto it = lower_bound(snapshots.begin(), snapshots.end(), gameTick,
		[](const Snapshot &snap, uint64_t tick) {
			return snap.gameTick < tick;
		});

	if(it != snapshots.end() && it->gameTick == gameTick)
		return &(*it);

	return nullptr;
}



vector<const Snapshot *> SnapshotManager::GetSnapshotsSince(uint64_t gameTick) const
{
	vector<const Snapshot *> result;

	for(const auto &snap : snapshots)
	{
		if(snap.gameTick > gameTick)
			result.push_back(&snap);
	}

	return result;
}



size_t SnapshotManager::CalculateDelta(const GameState &previous, const GameState &current) const
{
	// Simplified delta calculation
	// In a full implementation, this would compare:
	// - Ship positions/velocities (send only if changed > threshold)
	// - Projectile lists (send spawns/destroys)
	// - Visual effects (send new effects only)
	// - Player states (send only changes)

	size_t deltaSize = 0;

	// Header overhead
	deltaSize += 32;  // Tick, timestamp, flags

	// Ships: Assume ~30% of ships changed position significantly
	size_t shipCount = current.GetShipCount();
	size_t changedShips = shipCount * 30 / 100;
	deltaSize += changedShips * 48;  // UUID + position + velocity + facing

	// Projectiles: Send all (short-lived, change frequently)
	deltaSize += current.GetProjectileCount() * 32;  // Position + velocity + type

	// Visuals: Send all new effects (temporary)
	deltaSize += current.GetVisuals().size() * 24;

	// Estimate compression: typical 60-80% reduction
	return deltaSize * 3 / 10;  // 70% compression
}



void SnapshotManager::PruneOlderThan(uint64_t gameTick)
{
	// Remove snapshots older than specified tick
	while(!snapshots.empty() && snapshots.front().gameTick < gameTick)
		snapshots.pop_front();
}



size_t SnapshotManager::GetMemoryUsage() const
{
	size_t total = 0;

	// Snapshot overhead
	total += snapshots.size() * sizeof(Snapshot);

	// GameState memory (rough estimate)
	// Each GameState contains:
	// - std::list<shared_ptr<Ship>> (assume 10 ships avg, 1KB each)
	// - std::list<Projectile> (assume 20 projectiles, 128 bytes each)
	// - std::list<Visual> (assume 10 visuals, 64 bytes each)
	// - Other metadata
	size_t avgStateSize = 10 * 1024 + 20 * 128 + 10 * 64 + 256;
	total += snapshots.size() * avgStateSize;

	return total;
}



double SnapshotManager::GetAverageCompressionRatio() const
{
	if(totalUncompressedBytes == 0)
		return 1.0;

	return static_cast<double>(totalCompressedBytes) / totalUncompressedBytes;
}



bool SnapshotManager::ShouldCreateKeyframe() const
{
	// Create keyframe every N snapshots
	return snapshotsSinceLastKeyframe >= keyframeInterval;
}



size_t SnapshotManager::EstimateSnapshotSize(const GameState &state) const
{
	size_t size = 0;

	// Header
	size += 32;

	// Ships (UUID + full state)
	size += state.GetShipCount() * 128;

	// Projectiles
	size += state.GetProjectileCount() * 32;

	// Visuals
	size += state.GetVisuals().size() * 24;

	// Flotsam
	size += state.GetFlotsam().size() * 64;

	// Metadata
	size += 64;

	return size;
}
