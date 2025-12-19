/* ConnectionMonitor.h
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
#include <deque>


// ConnectionMonitor: Track network connection quality
//
// Monitors:
// - Round-trip time (RTT/ping)
// - Packet loss
// - Jitter (RTT variance)
// - Connection stability
//
// Uses:
// - Display network quality indicators to player
// - Adjust client-side prediction parameters
// - Warn about poor connection
// - Trigger reconnection on timeout
class ConnectionMonitor {
public:
	ConnectionMonitor();

	// Connection quality levels
	enum class Quality {
		EXCELLENT,    // < 50ms, no packet loss
		GOOD,         // 50-100ms, minimal packet loss
		FAIR,         // 100-200ms, some packet loss
		POOR,         // 200-500ms, significant packet loss
		TERRIBLE,     // > 500ms or high packet loss
		DISCONNECTED  // No connection
	};

	// Record ping measurement (called when pong received)
	void RecordPing(uint32_t pingMs);

	// Record packet sent (for loss tracking)
	void RecordPacketSent(uint64_t packetId);

	// Record packet received (for loss tracking)
	void RecordPacketReceived(uint64_t packetId);

	// Update (called every frame)
	void Update();

	// Get current ping (milliseconds)
	uint32_t GetPing() const { return currentPing; }

	// Get average ping over last N measurements
	uint32_t GetAveragePing() const;

	// Get ping jitter (standard deviation)
	uint32_t GetJitter() const { return jitter; }

	// Get packet loss percentage (0-100)
	double GetPacketLoss() const;

	// Get connection quality
	Quality GetQuality() const { return quality; }

	// Check if connection is stable
	bool IsStable() const;

	// Check if connection timed out
	bool IsTimedOut() const;

	// Get time since last packet (milliseconds)
	uint64_t GetTimeSinceLastPacket() const;

	// Statistics
	uint64_t GetTotalPacketsSent() const { return totalPacketsSent; }
	uint64_t GetTotalPacketsReceived() const { return totalPacketsReceived; }
	uint64_t GetTotalPacketsLost() const { return totalPacketsLost; }

	// Configuration
	void SetTimeout(uint32_t milliseconds) { timeoutMs = milliseconds; }
	uint32_t GetTimeout() const { return timeoutMs; }


private:
	// Ping tracking
	uint32_t currentPing = 0;
	std::deque<uint32_t> pingHistory;           // Last N ping measurements
	size_t maxPingHistory = 30;                 // Keep last 30 pings
	uint32_t jitter = 0;                        // Ping variance

	// Packet tracking
	uint64_t totalPacketsSent = 0;
	uint64_t totalPacketsReceived = 0;
	uint64_t totalPacketsLost = 0;
	std::deque<uint64_t> sentPacketIds;         // Recent sent packets
	size_t maxTrackedPackets = 100;             // Track last 100 packets

	// Connection state
	Quality quality = Quality::DISCONNECTED;
	std::chrono::time_point<std::chrono::steady_clock> lastPacketTime;
	uint32_t timeoutMs = 10000;                 // 10 second timeout

	// Helper methods
	void UpdateQuality();
	void UpdateJitter();
	void CheckForLostPackets();
	double CalculatePacketLoss() const;
};
