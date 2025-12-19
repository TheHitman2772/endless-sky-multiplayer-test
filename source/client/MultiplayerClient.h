/* MultiplayerClient.h
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

#include "ConnectionMonitor.h"
#include "EntityInterpolator.h"
#include "ClientReconciliation.h"

#include <memory>
#include <string>
#include <vector>

class GameState;
class NetworkManager;
class PlayerCommand;
class Predictor;
class EsUuid;
class Command;


// MultiplayerClient: Main multiplayer client
//
// Responsibilities:
// - Connect to dedicated server
// - Send player input at 60 Hz
// - Receive server state updates at 20-30 Hz
// - Apply client-side prediction
// - Reconcile with server corrections
// - Interpolate remote entities
// - Monitor connection quality
//
// Architecture:
//   MultiplayerClient
//   ├── NetworkManager (ENet client)
//   ├── Predictor (client-side prediction)
//   ├── ClientReconciliation (error correction)
//   ├── EntityInterpolator (smooth remote entities)
//   ├── ConnectionMonitor (ping/loss tracking)
//   └── GameState (predicted client state)
//
// Workflow:
//   1. Player Input → Create PlayerCommand
//   2. Send command to server (60 Hz)
//   3. Predict local state with Predictor
//   4. Receive server update (20 Hz)
//   5. Reconcile prediction error
//   6. Interpolate remote entities
//   7. Render smooth 60 FPS visuals
class MultiplayerClient {
public:
	MultiplayerClient();
	~MultiplayerClient();

	// Connection state
	enum class State {
		DISCONNECTED,
		CONNECTING,
		CONNECTED,
		DISCONNECTING
	};

	// Connect to server
	bool Connect(const std::string &hostname, uint16_t port);

	// Disconnect from server
	void Disconnect();

	// Get connection state
	State GetState() const { return state; }
	bool IsConnected() const { return state == State::CONNECTED; }

	// Update (call every frame at 60 Hz)
	void Update();

	// Send player command
	void SendCommand(const Command &command);

	// Get current game state (predicted + interpolated)
	GameState *GetGameState() { return gameState.get(); }
	const GameState *GetGameState() const { return gameState.get(); }

	// Get player UUID
	const EsUuid &GetPlayerUUID() const;

	// Connection quality
	const ConnectionMonitor &GetConnectionMonitor() const { return connectionMonitor; }
	uint32_t GetPing() const { return connectionMonitor.GetPing(); }
	ConnectionMonitor::Quality GetConnectionQuality() const { return connectionMonitor.GetQuality(); }

	// Statistics
	struct Statistics {
		State connectionState;
		uint32_t ping;
		double packetLoss;
		uint32_t jitter;
		uint64_t commandsSent;
		uint64_t stateUpdatesReceived;
		uint64_t predictionErrors;
		size_t interpolatedEntities;
	};

	Statistics GetStatistics() const;


private:
	// Connection state
	State state = State::DISCONNECTED;

	// Core subsystems
	std::unique_ptr<NetworkManager> networkManager;
	std::unique_ptr<GameState> gameState;          // Predicted client state
	std::unique_ptr<Predictor> predictor;
	ClientReconciliation reconciliation;
	EntityInterpolator interpolator;
	ConnectionMonitor connectionMonitor;

	// Player identity
	std::unique_ptr<EsUuid> playerUUID;

	// Statistics
	uint64_t commandsSent = 0;
	uint64_t stateUpdatesReceived = 0;
	uint64_t predictionErrors = 0;
	uint64_t lastSentCommandTick = 0;

	// Network event handlers
	void OnConnected();
	void OnDisconnected();
	void OnServerWelcome(const std::vector<uint8_t> &data);
	void OnStateUpdate(const std::vector<uint8_t> &data);
	void OnPlayerJoined(const std::vector<uint8_t> &data);
	void OnPlayerLeft(const std::vector<uint8_t> &data);

	// Command processing
	void SendCommandToServer(const PlayerCommand &command);
	void ApplyPrediction(const PlayerCommand &command);

	// State reconciliation
	void ReconcileWithServer(const GameState &serverState);

	// Input processing
	void ProcessNetworkInput();

	// Helpers
	void InitializeSubsystems();
	void CleanupSubsystems();
};
