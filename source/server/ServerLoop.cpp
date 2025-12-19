/* ServerLoop.cpp
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

#include "ServerLoop.h"

#include <thread>

using namespace std;
using namespace chrono;



ServerLoop::ServerLoop(uint32_t simulationHz, uint32_t broadcastHz)
	: targetSimulationHz(simulationHz), targetBroadcastHz(broadcastHz)
{
	UpdateTimingConfiguration();
}



void ServerLoop::Run()
{
	running = true;
	gameTick = 0;

	// Initialize timing
	lastSimulationTime = high_resolution_clock::now();
	lastBroadcastTime = lastSimulationTime;
	lastStatsUpdate = lastSimulationTime;
	accumulatedTime = 0.0;

	while(running)
	{
		auto frameStart = high_resolution_clock::now();

		// Process network input (non-blocking)
		if(inputCallback)
			inputCallback();

		// Calculate delta time since last frame
		auto currentTime = high_resolution_clock::now();
		duration<double> deltaTime = currentTime - lastSimulationTime;
		lastSimulationTime = currentTime;

		// Accumulate time for fixed timestep
		accumulatedTime += deltaTime.count();

		// Run simulation ticks (may run multiple times if behind)
		while(accumulatedTime >= simulationTimestep.count())
		{
			ProcessSimulation();
			accumulatedTime -= simulationTimestep.count();
		}

		// Check if time to broadcast
		duration<double> timeSinceBroadcast = currentTime - lastBroadcastTime;
		if(timeSinceBroadcast >= broadcastInterval)
		{
			ProcessBroadcast();
			lastBroadcastTime = currentTime;
		}

		// Update statistics
		UpdateStatistics();

		// Sleep to maintain target framerate
		SleepUntilNextFrame();
	}
}



void ServerLoop::Stop()
{
	running = false;
}



void ServerLoop::SetSimulationHz(uint32_t hz)
{
	targetSimulationHz = hz;
	UpdateTimingConfiguration();
}



void ServerLoop::SetBroadcastHz(uint32_t hz)
{
	targetBroadcastHz = hz;
	UpdateTimingConfiguration();
}



void ServerLoop::UpdateTimingConfiguration()
{
	// Calculate timestep duration
	simulationTimestep = duration<double>(1.0 / targetSimulationHz);
	broadcastInterval = duration<double>(1.0 / targetBroadcastHz);
}



void ServerLoop::UpdateStatistics()
{
	auto now = high_resolution_clock::now();
	duration<double> timeSinceStats = now - lastStatsUpdate;

	// Update stats every second
	if(timeSinceStats.count() >= 1.0)
	{
		// Calculate actual rates
		actualSimulationHz = ticksSinceLastStats / timeSinceStats.count();
		actualBroadcastHz = broadcastsSinceLastStats / timeSinceStats.count();

		// Reset counters
		lastStatsUpdate = now;
		ticksSinceLastStats = 0;
		broadcastsSinceLastStats = 0;
	}
}



void ServerLoop::ProcessSimulation()
{
	auto tickStart = high_resolution_clock::now();

	// Run simulation callback
	if(simulationCallback)
		simulationCallback(gameTick);

	// Increment tick counter
	++gameTick;
	++totalSimulationTicks;
	++ticksSinceLastStats;

	// Measure tick time
	auto tickEnd = high_resolution_clock::now();
	duration<double, milli> tickDuration = tickEnd - tickStart;

	// Update average tick time (exponential moving average)
	const double alpha = 0.1;  // Smoothing factor
	averageTickTime = alpha * tickDuration.count() + (1.0 - alpha) * averageTickTime;
}



void ServerLoop::ProcessBroadcast()
{
	// Run broadcast callback
	if(broadcastCallback)
		broadcastCallback(gameTick);

	++totalBroadcasts;
	++broadcastsSinceLastStats;
}



void ServerLoop::SleepUntilNextFrame()
{
	// Calculate time until next simulation tick
	auto now = high_resolution_clock::now();
	auto nextTickTime = lastSimulationTime + simulationTimestep;

	if(nextTickTime > now)
	{
		// Sleep until next tick
		this_thread::sleep_for(nextTickTime - now);
	}
	// If we're behind, don't sleep (will catch up via accumulator)
}
