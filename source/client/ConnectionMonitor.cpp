/* ConnectionMonitor.cpp
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

#include "ConnectionMonitor.h"

#include <algorithm>
#include <cmath>
#include <numeric>

using namespace std;
using namespace chrono;



ConnectionMonitor::ConnectionMonitor()
{
	lastPacketTime = steady_clock::now();
}



void ConnectionMonitor::RecordPing(uint32_t pingMs)
{
	currentPing = pingMs;

	// Add to history
	pingHistory.push_back(pingMs);
	if(pingHistory.size() > maxPingHistory)
		pingHistory.pop_front();

	// Update jitter
	UpdateJitter();

	// Update quality assessment
	UpdateQuality();

	// Record packet received
	lastPacketTime = steady_clock::now();
}



void ConnectionMonitor::RecordPacketSent(uint64_t packetId)
{
	++totalPacketsSent;

	// Track packet ID for loss detection
	sentPacketIds.push_back(packetId);
	if(sentPacketIds.size() > maxTrackedPackets)
		sentPacketIds.pop_front();
}



void ConnectionMonitor::RecordPacketReceived(uint64_t packetId)
{
	++totalPacketsReceived;
	lastPacketTime = steady_clock::now();

	// Remove from sent queue if present
	auto it = find(sentPacketIds.begin(), sentPacketIds.end(), packetId);
	if(it != sentPacketIds.end())
		sentPacketIds.erase(it);
}



void ConnectionMonitor::Update()
{
	// Check for timed out packets
	CheckForLostPackets();

	// Update quality based on current metrics
	UpdateQuality();
}



uint32_t ConnectionMonitor::GetAveragePing() const
{
	if(pingHistory.empty())
		return currentPing;

	uint64_t sum = accumulate(pingHistory.begin(), pingHistory.end(), 0ULL);
	return static_cast<uint32_t>(sum / pingHistory.size());
}



double ConnectionMonitor::GetPacketLoss() const
{
	return CalculatePacketLoss();
}



bool ConnectionMonitor::IsStable() const
{
	// Connection is stable if:
	// - Quality is good or better
	// - Jitter is low (< 50ms)
	// - Packet loss is minimal (< 5%)

	if(quality == Quality::DISCONNECTED)
		return false;

	if(quality == Quality::POOR || quality == Quality::TERRIBLE)
		return false;

	if(jitter > 50)
		return false;

	if(GetPacketLoss() > 5.0)
		return false;

	return true;
}



bool ConnectionMonitor::IsTimedOut() const
{
	auto now = steady_clock::now();
	auto elapsed = duration_cast<milliseconds>(now - lastPacketTime).count();
	return elapsed > timeoutMs;
}



uint64_t ConnectionMonitor::GetTimeSinceLastPacket() const
{
	auto now = steady_clock::now();
	return duration_cast<milliseconds>(now - lastPacketTime).count();
}



void ConnectionMonitor::UpdateQuality()
{
	// Determine quality based on ping and packet loss
	uint32_t avgPing = GetAveragePing();
	double packetLoss = GetPacketLoss();

	// Check for timeout first
	if(IsTimedOut())
	{
		quality = Quality::DISCONNECTED;
		return;
	}

	// Classify based on ping and packet loss
	if(avgPing < 50 && packetLoss < 1.0)
		quality = Quality::EXCELLENT;
	else if(avgPing < 100 && packetLoss < 3.0)
		quality = Quality::GOOD;
	else if(avgPing < 200 && packetLoss < 10.0)
		quality = Quality::FAIR;
	else if(avgPing < 500 && packetLoss < 25.0)
		quality = Quality::POOR;
	else
		quality = Quality::TERRIBLE;
}



void ConnectionMonitor::UpdateJitter()
{
	if(pingHistory.size() < 2)
	{
		jitter = 0;
		return;
	}

	// Calculate standard deviation of ping times
	double mean = GetAveragePing();
	double variance = 0.0;

	for(uint32_t ping : pingHistory)
	{
		double diff = ping - mean;
		variance += diff * diff;
	}

	variance /= pingHistory.size();
	jitter = static_cast<uint32_t>(sqrt(variance));
}



void ConnectionMonitor::CheckForLostPackets()
{
	// Packets that have been sent but not received for a while are considered lost
	// This is a simplified approach - in a real implementation, we'd need
	// sequence numbers and more sophisticated loss detection

	// For now, just estimate based on send/receive ratio
	if(totalPacketsSent > 0)
	{
		uint64_t expectedReceived = totalPacketsSent;
		if(totalPacketsReceived < expectedReceived)
			totalPacketsLost = expectedReceived - totalPacketsReceived;
	}
}



double ConnectionMonitor::CalculatePacketLoss() const
{
	if(totalPacketsSent == 0)
		return 0.0;

	double lossRate = static_cast<double>(totalPacketsLost) / totalPacketsSent;
	return lossRate * 100.0;  // Convert to percentage
}
