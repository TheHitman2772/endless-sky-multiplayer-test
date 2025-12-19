/* ServerConfig.h
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
#include <string>


// ServerConfig: Configuration settings for dedicated server
//
// Manages all server configuration parameters including:
// - Network settings (ports, connections)
// - Simulation timing (tick rate, broadcast rate)
// - Server identity (name, MOTD)
// - Gameplay settings (max players, starting credits)
// - Performance tuning (snapshot intervals, buffer sizes)
//
// Configuration can be loaded from file or use sensible defaults.
class ServerConfig {
public:
	ServerConfig();

	// Load configuration from file
	// Returns true on success, false if file not found (uses defaults)
	bool LoadFromFile(const std::string &filename);

	// Save current configuration to file
	bool SaveToFile(const std::string &filename) const;

	// Validate configuration (checks for invalid values)
	bool IsValid() const;

	// Network settings
	uint16_t GetPort() const { return port; }
	void SetPort(uint16_t value) { port = value; }

	uint32_t GetMaxPlayers() const { return maxPlayers; }
	void SetMaxPlayers(uint32_t value) { maxPlayers = value; }

	uint32_t GetMaxConnectionsPerIP() const { return maxConnectionsPerIP; }
	void SetMaxConnectionsPerIP(uint32_t value) { maxConnectionsPerIP = value; }

	// Simulation timing
	uint32_t GetSimulationHz() const { return simulationHz; }
	void SetSimulationHz(uint32_t value) { simulationHz = value; }

	uint32_t GetBroadcastHz() const { return broadcastHz; }
	void SetBroadcastHz(uint32_t value) { broadcastHz = value; }

	// Server identity
	const std::string &GetServerName() const { return serverName; }
	void SetServerName(const std::string &value) { serverName = value; }

	const std::string &GetMessageOfTheDay() const { return motd; }
	void SetMessageOfTheDay(const std::string &value) { motd = value; }

	const std::string &GetPassword() const { return password; }
	void SetPassword(const std::string &value) { password = value; }
	bool RequiresPassword() const { return !password.empty(); }

	// Gameplay settings
	int64_t GetStartingCredits() const { return startingCredits; }
	void SetStartingCredits(int64_t value) { startingCredits = value; }

	const std::string &GetStartingSystem() const { return startingSystem; }
	void SetStartingSystem(const std::string &value) { startingSystem = value; }

	const std::string &GetStartingPlanet() const { return startingPlanet; }
	void SetStartingPlanet(const std::string &value) { startingPlanet = value; }

	bool IsPvPEnabled() const { return enablePvP; }
	void SetPvPEnabled(bool value) { enablePvP = value; }

	// Performance tuning
	uint32_t GetSnapshotHistorySize() const { return snapshotHistorySize; }
	void SetSnapshotHistorySize(uint32_t value) { snapshotHistorySize = value; }

	uint32_t GetCommandBufferSize() const { return commandBufferSize; }
	void SetCommandBufferSize(uint32_t value) { commandBufferSize = value; }

	// Logging and debugging
	bool IsVerboseLogging() const { return verboseLogging; }
	void SetVerboseLogging(bool value) { verboseLogging = value; }

	bool IsConsoleEnabled() const { return enableConsole; }
	void SetConsoleEnabled(bool value) { enableConsole = value; }


private:
	// Network settings
	uint16_t port = 31337;                      // Default port
	uint32_t maxPlayers = 32;                   // Maximum concurrent players
	uint32_t maxConnectionsPerIP = 3;           // Prevent IP flooding

	// Simulation timing
	uint32_t simulationHz = 60;                 // Server tick rate (60 FPS)
	uint32_t broadcastHz = 20;                  // Network update rate (20 Hz)

	// Server identity
	std::string serverName = "Endless Sky Server";
	std::string motd = "Welcome to Endless Sky Multiplayer!";
	std::string password;                       // Empty = no password

	// Gameplay settings
	int64_t startingCredits = 100000;           // New player credits
	std::string startingSystem = "Sol";         // Spawn system
	std::string startingPlanet = "Earth";       // Spawn planet
	bool enablePvP = false;                     // Player vs Player combat

	// Performance tuning
	uint32_t snapshotHistorySize = 120;         // 2 seconds at 60 Hz
	uint32_t commandBufferSize = 10000;         // Max buffered commands

	// Logging and debugging
	bool verboseLogging = false;                // Detailed logs
	bool enableConsole = true;                  // Console interface
};
