/* SnapshotManager.h
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

#include <cstdint>
#include <deque>
#include <memory>
#include <vector>

class GameState;


// Snapshot: Single game state snapshot with metadata
struct Snapshot {
	uint64_t gameTick = 0;
	uint64_t timestamp = 0;                     // System time (milliseconds)
	std::shared_ptr<GameState> state;
	size_t compressedSize = 0;                  // Size after delta compression
	bool isKeyframe = false;                    // Full state (no delta)

	Snapshot() = default;
	Snapshot(uint64_t tick, std::shared_ptr<GameState> gameState);
};


// SnapshotManager: Manages game state snapshots with delta compression
//
// Responsibilities:
// - Create snapshots of current game state
// - Maintain circular buffer of recent snapshots
// - Delta compression (only send changes from previous snapshot)
// - Keyframe generation (periodic full snapshots)
// - Client synchronization support
//
// Delta Compression Strategy:
// - Every Nth snapshot is a keyframe (full state)
// - Other snapshots store only differences from previous
// - Reduces network bandwidth by 80-90% for typical gameplay
//
// Snapshot History:
// - Keep last N snapshots (configurable, default 120 = 2 sec at 60 Hz)
// - Enables client catchup and lag compensation
// - Old snapshots automatically pruned
class SnapshotManager {
public:
	explicit SnapshotManager(size_t historySize = 120);

	// Create snapshot of current game state
	// isKeyframe: Force full snapshot (no delta compression)
	void CreateSnapshot(const GameState &currentState, uint64_t gameTick, bool forceKeyframe = false);

	// Get most recent snapshot
	const Snapshot *GetLatestSnapshot() const;

	// Get snapshot at specific tick (for client synchronization)
	// Returns nullptr if not found (too old or doesn't exist)
	const Snapshot *GetSnapshotAtTick(uint64_t gameTick) const;

	// Get all snapshots since a specific tick (for client catchup)
	std::vector<const Snapshot *> GetSnapshotsSince(uint64_t gameTick) const;

	// Calculate delta between two snapshots
	// Returns estimated size of delta (for statistics)
	size_t CalculateDelta(const GameState &previous, const GameState &current) const;

	// Prune snapshots older than specified tick
	void PruneOlderThan(uint64_t gameTick);

	// Get snapshot count
	size_t GetSnapshotCount() const { return snapshots.size(); }

	// Get total memory usage (estimated)
	size_t GetMemoryUsage() const;

	// Statistics
	uint64_t GetTotalSnapshots() const { return totalSnapshots; }
	uint64_t GetTotalKeyframes() const { return totalKeyframes; }
	uint64_t GetTotalDeltaSnapshots() const { return totalSnapshots - totalKeyframes; }
	double GetAverageCompressionRatio() const;

	// Configuration
	void SetHistorySize(size_t size) { historySize = size; }
	size_t GetHistorySize() const { return historySize; }

	void SetKeyframeInterval(uint32_t interval) { keyframeInterval = interval; }
	uint32_t GetKeyframeInterval() const { return keyframeInterval; }


private:
	// Snapshot storage (circular buffer via deque)
	std::deque<Snapshot> snapshots;
	size_t historySize;                         // Max snapshots to keep

	// Keyframe configuration
	uint32_t keyframeInterval = 30;             // Generate keyframe every N snapshots
	uint32_t snapshotsSinceLastKeyframe = 0;

	// Statistics
	uint64_t totalSnapshots = 0;
	uint64_t totalKeyframes = 0;
	uint64_t totalUncompressedBytes = 0;
	uint64_t totalCompressedBytes = 0;

	// Helper: Check if next snapshot should be keyframe
	bool ShouldCreateKeyframe() const;

	// Helper: Estimate snapshot size
	size_t EstimateSnapshotSize(const GameState &state) const;
};
