/* Server.h
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

#include "ServerConfig.h"

#include <memory>
#include <string>
#include <vector>

class GameState;
class NetworkManager;
class PlayerManager;
class CommandBuffer;
class CommandValidator;
class SnapshotManager;
class ServerLoop;
class EsUuid;


// Server: Main dedicated server class
//
// Responsibilities:
// - Initialize and manage all server subsystems
// - Accept and manage client connections
// - Run authoritative game simulation
// - Process player commands
// - Broadcast state updates to clients
// - Handle player join/leave events
//
// Architecture:
//   Server
//   ├── ServerConfig (configuration)
//   ├── NetworkManager (ENet networking)
//   ├── PlayerManager (player tracking)
//   ├── CommandBuffer (input queue)
//   ├── CommandValidator (validation + rate limiting)
//   ├── SnapshotManager (state history)
//   ├── ServerLoop (game timing)
//   └── GameState (authoritative state)
//
// Lifecycle:
//   1. Initialize(config)  - Set up all subsystems
//   2. Start()             - Begin accepting connections and simulation
//   3. Run()               - Main loop (blocks until shutdown)
//   4. Shutdown()          - Graceful cleanup
//
// Thread Safety:
// - Main simulation runs on single thread (deterministic)
// - Network I/O may use separate threads (ENet)
// - Commands are queued and processed on simulation thread
class Server {
public:
	Server();
	~Server();

	// Initialize server with configuration
	// Returns true on success, false on error
	bool Initialize(const ServerConfig &config);

	// Start server (begin accepting connections)
	bool Start();

	// Run server loop (blocks until Stop() called)
	void Run();

	// Stop server (graceful shutdown)
	void Stop();

	// Check if server is running
	bool IsRunning() const;

	// Get configuration
	const ServerConfig &GetConfig() const { return config; }

	// Get current game tick
	uint64_t GetGameTick() const;

	// Get connected player count
	size_t GetPlayerCount() const;

	// Statistics
	struct Statistics {
		uint64_t totalTicks = 0;
		uint64_t totalBroadcasts = 0;
		uint64_t totalCommandsProcessed = 0;
		uint64_t totalCommandsRejected = 0;
		size_t connectedPlayers = 0;
		double actualSimulationHz = 0.0;
		double actualBroadcastHz = 0.0;
		double averageTickTime = 0.0;
		size_t snapshotCount = 0;
		size_t snapshotMemoryUsage = 0;
	};

	Statistics GetStatistics() const;

	// Console commands (for server administration)
	void ExecuteConsoleCommand(const std::string &command);

	// Kick player
	void KickPlayer(const EsUuid &playerUuid, const std::string &reason);

	// Broadcast message to all players
	void BroadcastMessage(const std::string &message);


private:
	// Configuration
	ServerConfig config;

	// Core subsystems
	std::unique_ptr<GameState> gameState;
	std::unique_ptr<NetworkManager> networkManager;
	std::unique_ptr<PlayerManager> playerManager;
	std::unique_ptr<CommandBuffer> commandBuffer;
	std::unique_ptr<CommandValidator> commandValidator;
	std::unique_ptr<SnapshotManager> snapshotManager;
	std::unique_ptr<ServerLoop> serverLoop;

	// State
	bool initialized = false;
	bool running = false;

	// Statistics
	uint64_t totalCommandsProcessed = 0;
	uint64_t totalCommandsRejected = 0;

	// Initialization helpers
	bool InitializeNetwork();
	bool InitializeGameState();
	bool InitializeSubsystems();

	// Server loop callbacks
	void OnSimulationTick(uint64_t gameTick);
	void OnBroadcastTick(uint64_t gameTick);
	void OnProcessInput();

	// Network event handlers
	void OnClientConnected(size_t clientId);
	void OnClientDisconnected(size_t clientId);
	void OnClientCommand(size_t clientId, const std::vector<uint8_t> &data);

	// Game logic
	void ProcessCommands(uint64_t gameTick);
	void SimulateGameTick();
	void BroadcastGameState();

	// Console command handlers
	void HandleCommand_Status();
	void HandleCommand_Players();
	void HandleCommand_Kick(const std::string &playerName);
	void HandleCommand_Say(const std::string &message);
	void HandleCommand_Help();
	void HandleCommand_Shutdown();
};
