/* Server.cpp
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

#include "Server.h"

#include "ServerLoop.h"
#include "SnapshotManager.h"
#include "../GameState.h"
#include "../network/NetworkManager.h"
#include "../multiplayer/PlayerManager.h"
#include "../multiplayer/CommandBuffer.h"
#include "../multiplayer/CommandValidator.h"
#include "../EsUuid.h"

#include <iostream>
#include <sstream>

using namespace std;



Server::Server()
{
}



Server::~Server()
{
	if(running)
		Stop();
}



bool Server::Initialize(const ServerConfig &cfg)
{
	if(initialized)
		return false;

	// Validate configuration
	if(!cfg.IsValid())
	{
		cerr << "Invalid server configuration!" << endl;
		return false;
	}

	config = cfg;

	// Initialize subsystems
	if(!InitializeGameState())
		return false;

	if(!InitializeNetwork())
		return false;

	if(!InitializeSubsystems())
		return false;

	initialized = true;
	cout << "Server initialized successfully" << endl;
	cout << "  Port: " << config.GetPort() << endl;
	cout << "  Max players: " << config.GetMaxPlayers() << endl;
	cout << "  Simulation: " << config.GetSimulationHz() << " Hz" << endl;
	cout << "  Broadcast: " << config.GetBroadcastHz() << " Hz" << endl;

	return true;
}



bool Server::Start()
{
	if(!initialized)
	{
		cerr << "Server not initialized!" << endl;
		return false;
	}

	if(running)
		return false;

	// Start network manager
	if(!networkManager->StartServer(config.GetPort()))
	{
		cerr << "Failed to start network server!" << endl;
		return false;
	}

	running = true;
	cout << "Server started on port " << config.GetPort() << endl;
	cout << "Server name: " << config.GetServerName() << endl;
	cout << "MOTD: " << config.GetMessageOfTheDay() << endl;

	return true;
}



void Server::Run()
{
	if(!running)
	{
		cerr << "Server not started!" << endl;
		return;
	}

	cout << "Server loop starting..." << endl;

	// Set up server loop callbacks
	serverLoop->SetSimulationCallback([this](uint64_t tick) { OnSimulationTick(tick); });
	serverLoop->SetBroadcastCallback([this](uint64_t tick) { OnBroadcastTick(tick); });
	serverLoop->SetInputCallback([this]() { OnProcessInput(); });

	// Run the server loop (blocks until Stop() called)
	serverLoop->Run();

	cout << "Server loop ended" << endl;
}



void Server::Stop()
{
	if(!running)
		return;

	cout << "Stopping server..." << endl;

	// Stop server loop
	if(serverLoop)
		serverLoop->Stop();

	// Disconnect all clients
	if(networkManager)
		networkManager->Shutdown();

	running = false;
	cout << "Server stopped" << endl;
}



bool Server::IsRunning() const
{
	return running;
}



uint64_t Server::GetGameTick() const
{
	if(serverLoop)
		return serverLoop->GetGameTick();
	return 0;
}



size_t Server::GetPlayerCount() const
{
	if(playerManager)
		return playerManager->GetConnectedPlayerCount();
	return 0;
}



Server::Statistics Server::GetStatistics() const
{
	Statistics stats;

	if(serverLoop)
	{
		stats.totalTicks = serverLoop->GetTotalSimulationTicks();
		stats.totalBroadcasts = serverLoop->GetTotalBroadcasts();
		stats.actualSimulationHz = serverLoop->GetActualSimulationHz();
		stats.actualBroadcastHz = serverLoop->GetActualBroadcastHz();
		stats.averageTickTime = serverLoop->GetAverageTickTime();
	}

	stats.totalCommandsProcessed = totalCommandsProcessed;
	stats.totalCommandsRejected = totalCommandsRejected;

	if(playerManager)
		stats.connectedPlayers = playerManager->GetConnectedPlayerCount();

	if(snapshotManager)
	{
		stats.snapshotCount = snapshotManager->GetSnapshotCount();
		stats.snapshotMemoryUsage = snapshotManager->GetMemoryUsage();
	}

	return stats;
}



void Server::ExecuteConsoleCommand(const string &command)
{
	istringstream iss(command);
	string cmd;
	iss >> cmd;

	if(cmd == "status")
		HandleCommand_Status();
	else if(cmd == "players")
		HandleCommand_Players();
	else if(cmd == "kick")
	{
		string playerName;
		getline(iss, playerName);
		if(!playerName.empty() && playerName[0] == ' ')
			playerName = playerName.substr(1);
		HandleCommand_Kick(playerName);
	}
	else if(cmd == "say")
	{
		string message;
		getline(iss, message);
		if(!message.empty() && message[0] == ' ')
			message = message.substr(1);
		HandleCommand_Say(message);
	}
	else if(cmd == "help")
		HandleCommand_Help();
	else if(cmd == "shutdown" || cmd == "stop")
		HandleCommand_Shutdown();
	else
		cout << "Unknown command: " << cmd << " (type 'help' for list)" << endl;
}



void Server::KickPlayer(const EsUuid &playerUuid, const string &reason)
{
	// Implementation would disconnect the player's network connection
	cout << "Kicking player " << playerUuid.ToString() << ": " << reason << endl;
}



void Server::BroadcastMessage(const string &message)
{
	// Implementation would send chat message to all connected clients
	cout << "[BROADCAST] " << message << endl;
}



bool Server::InitializeNetwork()
{
	networkManager = make_unique<NetworkManager>();

	// Register connection callbacks
	// TODO: Set up callbacks when NetworkManager supports them

	return true;
}



bool Server::InitializeGameState()
{
	gameState = make_unique<GameState>();

	// TODO: Initialize game world
	// - Load starting system
	// - Set up initial state

	return true;
}



bool Server::InitializeSubsystems()
{
	// Create player manager
	playerManager = make_unique<PlayerManager>();

	// Create command processing
	commandBuffer = make_unique<CommandBuffer>();
	commandValidator = make_unique<CommandValidator>();

	// Create snapshot manager
	snapshotManager = make_unique<SnapshotManager>(config.GetSnapshotHistorySize());

	// Create server loop
	serverLoop = make_unique<ServerLoop>(config.GetSimulationHz(), config.GetBroadcastHz());

	return true;
}



void Server::OnSimulationTick(uint64_t gameTick)
{
	// Process player commands for this tick
	ProcessCommands(gameTick);

	// Simulate game world
	SimulateGameTick();

	// Create snapshot for history
	snapshotManager->CreateSnapshot(*gameState, gameTick);
}



void Server::OnBroadcastTick(uint64_t gameTick)
{
	// Broadcast current game state to all clients
	BroadcastGameState();
}



void Server::OnProcessInput()
{
	// Process network input (non-blocking)
	if(networkManager)
	{
		// Poll for new connections, disconnections, and packets
		// This would call OnClientConnected, OnClientDisconnected, OnClientCommand
		// TODO: Implement when NetworkManager supports event polling
	}
}



void Server::OnClientConnected(size_t clientId)
{
	cout << "Client connected: " << clientId << endl;

	// Create new player
	// TODO: Create NetworkPlayer and add to PlayerManager
	// TODO: Send SERVER_WELCOME packet with game state
}



void Server::OnClientDisconnected(size_t clientId)
{
	cout << "Client disconnected: " << clientId << endl;

	// Remove player
	// TODO: Remove from PlayerManager
	// TODO: Broadcast SERVER_PLAYER_LEAVE to other clients
}



void Server::OnClientCommand(size_t clientId, const vector<uint8_t> &data)
{
	// Deserialize command from packet data
	// TODO: Use PacketReader to deserialize PlayerCommand
	// TODO: Validate with CommandValidator
	// TODO: Add to CommandBuffer if valid
}



void Server::ProcessCommands(uint64_t gameTick)
{
	// Get all commands for this tick
	auto commands = commandBuffer->GetCommandsForTick(gameTick);

	for(const auto &cmd : commands)
	{
		// Validate command
		auto result = commandValidator->ValidateCommand(cmd, gameTick);

		if(result == CommandValidator::Result::VALID)
		{
			// Apply command to game state
			// TODO: Apply command to player's ship
			++totalCommandsProcessed;
		}
		else
		{
			// Reject invalid command
			++totalCommandsRejected;

			if(config.IsVerboseLogging())
			{
				cout << "Rejected command from player " << cmd.playerUUID.ToString()
					<< " at tick " << cmd.gameTick << endl;
			}
		}
	}

	// Prune old commands (keep last 2 seconds)
	if(gameTick > 120)
		commandBuffer->PruneOlderThan(gameTick - 120);
}



void Server::SimulateGameTick()
{
	// Step game state forward one tick
	gameState->Step();
}



void Server::BroadcastGameState()
{
	// Get latest snapshot
	const auto *snapshot = snapshotManager->GetLatestSnapshot();
	if(!snapshot)
		return;

	// TODO: Serialize snapshot to packet
	// TODO: Broadcast to all connected clients via NetworkManager
	// TODO: Use delta compression for bandwidth efficiency

	if(config.IsVerboseLogging())
	{
		cout << "Broadcasting state at tick " << snapshot->gameTick
			<< " (" << snapshot->compressedSize << " bytes)" << endl;
	}
}



void Server::HandleCommand_Status()
{
	auto stats = GetStatistics();

	cout << "\n=== Server Status ===" << endl;
	cout << "Running: " << (running ? "Yes" : "No") << endl;
	cout << "Players: " << stats.connectedPlayers << " / " << config.GetMaxPlayers() << endl;
	cout << "Game Tick: " << GetGameTick() << endl;
	cout << "Simulation: " << stats.actualSimulationHz << " Hz (target: "
		<< config.GetSimulationHz() << " Hz)" << endl;
	cout << "Broadcast: " << stats.actualBroadcastHz << " Hz (target: "
		<< config.GetBroadcastHz() << " Hz)" << endl;
	cout << "Avg Tick Time: " << stats.averageTickTime << " ms" << endl;
	cout << "Total Ticks: " << stats.totalTicks << endl;
	cout << "Total Broadcasts: " << stats.totalBroadcasts << endl;
	cout << "Commands Processed: " << stats.totalCommandsProcessed << endl;
	cout << "Commands Rejected: " << stats.totalCommandsRejected << endl;
	cout << "Snapshots: " << stats.snapshotCount << " ("
		<< (stats.snapshotMemoryUsage / 1024) << " KB)" << endl;
	cout << endl;
}



void Server::HandleCommand_Players()
{
	cout << "\n=== Connected Players ===" << endl;
	cout << "Total: " << GetPlayerCount() << endl;
	// TODO: List all players with details
	cout << endl;
}



void Server::HandleCommand_Kick(const string &playerName)
{
	if(playerName.empty())
	{
		cout << "Usage: kick <player_name>" << endl;
		return;
	}

	cout << "Kicking player: " << playerName << endl;
	// TODO: Find player by name and kick
}



void Server::HandleCommand_Say(const string &message)
{
	if(message.empty())
	{
		cout << "Usage: say <message>" << endl;
		return;
	}

	BroadcastMessage(message);
}



void Server::HandleCommand_Help()
{
	cout << "\n=== Server Console Commands ===" << endl;
	cout << "  status    - Show server statistics" << endl;
	cout << "  players   - List connected players" << endl;
	cout << "  kick <player> - Kick a player" << endl;
	cout << "  say <msg> - Broadcast a message" << endl;
	cout << "  shutdown  - Stop the server" << endl;
	cout << "  help      - Show this help" << endl;
	cout << endl;
}



void Server::HandleCommand_Shutdown()
{
	cout << "Shutting down server..." << endl;
	Stop();
}
