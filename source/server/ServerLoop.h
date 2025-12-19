/* ServerLoop.h
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

#include <chrono>
#include <cstdint>
#include <functional>


// ServerLoop: Fixed-timestep game loop for dedicated server
//
// Implements a fixed-timestep simulation loop with separate rates for:
// - Simulation (60 Hz) - Game logic and physics
// - Network broadcast (20-30 Hz) - State synchronization
//
// Features:
// - Frame timing with sleep to maintain target FPS
// - Simulation accumulator for fixed timestep
// - Separate network broadcast interval
// - Performance statistics (actual FPS, tick time)
// - Graceful shutdown support
//
// Architecture:
// The loop runs continuously, calling callbacks at appropriate intervals:
// 1. Process network input (every loop iteration)
// 2. Simulate game tick (60 Hz)
// 3. Broadcast state (20-30 Hz)
// 4. Sleep to maintain target framerate
//
// Example Usage:
//   ServerLoop loop(60, 20);  // 60 Hz sim, 20 Hz broadcast
//   loop.SetSimulationCallback([](uint64_t tick) { /* simulate */ });
//   loop.SetBroadcastCallback([](uint64_t tick) { /* broadcast */ });
//   loop.Run();  // Runs until Stop() called
class ServerLoop {
public:
	// Construct with simulation Hz and broadcast Hz
	ServerLoop(uint32_t simulationHz = 60, uint32_t broadcastHz = 20);

	// Callbacks for simulation and networking
	using SimulationCallback = std::function<void(uint64_t gameTick)>;
	using BroadcastCallback = std::function<void(uint64_t gameTick)>;
	using InputCallback = std::function<void()>;

	void SetSimulationCallback(SimulationCallback callback) { simulationCallback = callback; }
	void SetBroadcastCallback(BroadcastCallback callback) { broadcastCallback = callback; }
	void SetInputCallback(InputCallback callback) { inputCallback = callback; }

	// Start the server loop (blocks until Stop() called)
	void Run();

	// Request stop (called from another thread or signal handler)
	void Stop();

	// Check if running
	bool IsRunning() const { return running; }

	// Get current game tick
	uint64_t GetGameTick() const { return gameTick; }

	// Performance statistics
	double GetActualSimulationHz() const { return actualSimulationHz; }
	double GetActualBroadcastHz() const { return actualBroadcastHz; }
	double GetAverageTickTime() const { return averageTickTime; }  // Milliseconds
	uint64_t GetTotalSimulationTicks() const { return totalSimulationTicks; }
	uint64_t GetTotalBroadcasts() const { return totalBroadcasts; }

	// Configuration
	void SetSimulationHz(uint32_t hz);
	void SetBroadcastHz(uint32_t hz);
	uint32_t GetTargetSimulationHz() const { return targetSimulationHz; }
	uint32_t GetTargetBroadcastHz() const { return targetBroadcastHz; }


private:
	// Target rates
	uint32_t targetSimulationHz;
	uint32_t targetBroadcastHz;

	// Timing
	std::chrono::duration<double> simulationTimestep;  // Duration per sim tick
	std::chrono::duration<double> broadcastInterval;   // Duration between broadcasts
	std::chrono::time_point<std::chrono::high_resolution_clock> lastSimulationTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> lastBroadcastTime;
	double accumulatedTime = 0.0;                      // For fixed timestep

	// State
	bool running = false;
	uint64_t gameTick = 0;

	// Callbacks
	SimulationCallback simulationCallback;
	BroadcastCallback broadcastCallback;
	InputCallback inputCallback;

	// Statistics
	uint64_t totalSimulationTicks = 0;
	uint64_t totalBroadcasts = 0;
	double actualSimulationHz = 0.0;
	double actualBroadcastHz = 0.0;
	double averageTickTime = 0.0;

	// FPS measurement
	std::chrono::time_point<std::chrono::high_resolution_clock> lastStatsUpdate;
	uint64_t ticksSinceLastStats = 0;
	uint64_t broadcastsSinceLastStats = 0;

	// Helper methods
	void UpdateTimingConfiguration();
	void UpdateStatistics();
	void ProcessSimulation();
	void ProcessBroadcast();
	void SleepUntilNextFrame();
};
