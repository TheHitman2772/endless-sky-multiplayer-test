/* ServerConfig.cpp
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

#include "ServerConfig.h"

#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;



ServerConfig::ServerConfig()
{
	// Use defaults defined in header
}



bool ServerConfig::LoadFromFile(const string &filename)
{
	ifstream file(filename);
	if(!file.is_open())
		return false;  // File not found, use defaults

	string line;
	while(getline(file, line))
	{
		// Skip empty lines and comments
		if(line.empty() || line[0] == '#')
			continue;

		// Parse "key = value" format
		size_t pos = line.find('=');
		if(pos == string::npos)
			continue;

		string key = line.substr(0, pos);
		string value = line.substr(pos + 1);

		// Trim whitespace
		key.erase(0, key.find_first_not_of(" \t"));
		key.erase(key.find_last_not_of(" \t") + 1);
		value.erase(0, value.find_first_not_of(" \t"));
		value.erase(value.find_last_not_of(" \t") + 1);

		// Parse configuration values
		if(key == "port")
			port = static_cast<uint16_t>(stoul(value));
		else if(key == "max_players")
			maxPlayers = stoul(value);
		else if(key == "max_connections_per_ip")
			maxConnectionsPerIP = stoul(value);
		else if(key == "simulation_hz")
			simulationHz = stoul(value);
		else if(key == "broadcast_hz")
			broadcastHz = stoul(value);
		else if(key == "server_name")
			serverName = value;
		else if(key == "motd")
			motd = value;
		else if(key == "password")
			password = value;
		else if(key == "starting_credits")
			startingCredits = stoll(value);
		else if(key == "starting_system")
			startingSystem = value;
		else if(key == "starting_planet")
			startingPlanet = value;
		else if(key == "enable_pvp")
			enablePvP = (value == "true" || value == "1");
		else if(key == "snapshot_history_size")
			snapshotHistorySize = stoul(value);
		else if(key == "command_buffer_size")
			commandBufferSize = stoul(value);
		else if(key == "verbose_logging")
			verboseLogging = (value == "true" || value == "1");
		else if(key == "enable_console")
			enableConsole = (value == "true" || value == "1");
	}

	return true;
}



bool ServerConfig::SaveToFile(const string &filename) const
{
	ofstream file(filename);
	if(!file.is_open())
		return false;

	file << "# Endless Sky Dedicated Server Configuration\n\n";

	file << "# Network Settings\n";
	file << "port = " << port << "\n";
	file << "max_players = " << maxPlayers << "\n";
	file << "max_connections_per_ip = " << maxConnectionsPerIP << "\n\n";

	file << "# Simulation Timing\n";
	file << "simulation_hz = " << simulationHz << "\n";
	file << "broadcast_hz = " << broadcastHz << "\n\n";

	file << "# Server Identity\n";
	file << "server_name = " << serverName << "\n";
	file << "motd = " << motd << "\n";
	file << "password = " << password << "\n\n";

	file << "# Gameplay Settings\n";
	file << "starting_credits = " << startingCredits << "\n";
	file << "starting_system = " << startingSystem << "\n";
	file << "starting_planet = " << startingPlanet << "\n";
	file << "enable_pvp = " << (enablePvP ? "true" : "false") << "\n\n";

	file << "# Performance Tuning\n";
	file << "snapshot_history_size = " << snapshotHistorySize << "\n";
	file << "command_buffer_size = " << commandBufferSize << "\n\n";

	file << "# Logging and Debugging\n";
	file << "verbose_logging = " << (verboseLogging ? "true" : "false") << "\n";
	file << "enable_console = " << (enableConsole ? "true" : "false") << "\n";

	return true;
}



bool ServerConfig::IsValid() const
{
	// Validate port range
	if(port == 0)
		return false;

	// Validate player limits
	if(maxPlayers == 0 || maxPlayers > 1000)
		return false;

	// Validate simulation rate (must be reasonable)
	if(simulationHz < 10 || simulationHz > 120)
		return false;

	// Validate broadcast rate (must be <= simulation rate)
	if(broadcastHz == 0 || broadcastHz > simulationHz)
		return false;

	// Validate snapshot history
	if(snapshotHistorySize == 0 || snapshotHistorySize > 1000)
		return false;

	// Validate command buffer
	if(commandBufferSize < 100)
		return false;

	// Validate starting credits (can be negative for challenge mode)
	// No validation needed

	// Validate system/planet names
	if(startingSystem.empty() || startingPlanet.empty())
		return false;

	return true;
}
