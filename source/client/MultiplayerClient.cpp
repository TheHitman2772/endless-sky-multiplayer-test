/* MultiplayerClient.cpp
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

#include "MultiplayerClient.h"

#include "../GameState.h"
#include "../network/NetworkManager.h"
#include "../multiplayer/Predictor.h"
#include "../multiplayer/PlayerCommand.h"
#include "../EsUuid.h"
#include "../Command.h"

#include <iostream>

using namespace std;



MultiplayerClient::MultiplayerClient()
{
	InitializeSubsystems();
}



MultiplayerClient::~MultiplayerClient()
{
	if(state != State::DISCONNECTED)
		Disconnect();

	CleanupSubsystems();
}



bool MultiplayerClient::Connect(const string &hostname, uint16_t port)
{
	if(state != State::DISCONNECTED)
	{
		cerr << "Already connected or connecting!" << endl;
		return false;
	}

	cout << "Connecting to " << hostname << ":" << port << "..." << endl;
	state = State::CONNECTING;

	// Connect via NetworkManager
	if(!networkManager->StartClient(hostname, port))
	{
		cerr << "Failed to connect to server!" << endl;
		state = State::DISCONNECTED;
		return false;
	}

	// Connection will complete asynchronously
	// OnConnected() will be called when successful
	return true;
}



void MultiplayerClient::Disconnect()
{
	if(state == State::DISCONNECTED)
		return;

	cout << "Disconnecting from server..." << endl;
	state = State::DISCONNECTING;

	// Shutdown network
	if(networkManager)
		networkManager->Shutdown();

	state = State::DISCONNECTED;
	cout << "Disconnected" << endl;
}



void MultiplayerClient::Update()
{
	if(state != State::CONNECTED)
		return;

	// Process incoming network packets
	ProcessNetworkInput();

	// Update interpolator
	interpolator.Update();

	// Update reconciliation
	reconciliation.Update();

	// Update connection monitor
	connectionMonitor.Update();

	// Check for timeout
	if(connectionMonitor.IsTimedOut())
	{
		cout << "Connection timed out!" << endl;
		Disconnect();
	}
}



void MultiplayerClient::SendCommand(const Command &command)
{
	if(state != State::CONNECTED)
		return;

	// Create PlayerCommand
	PlayerCommand playerCmd(*playerUUID, gameState->GetGameTick());
	playerCmd.command = command;
	playerCmd.sequenceNumber = static_cast<uint32_t>(commandsSent);

	// Send to server
	SendCommandToServer(playerCmd);

	// Apply prediction locally
	ApplyPrediction(playerCmd);

	// Track for reconciliation
	if(predictor)
		predictor->RecordCommand(playerCmd);

	++commandsSent;
}



const EsUuid &MultiplayerClient::GetPlayerUUID() const
{
	static EsUuid empty;
	return playerUUID ? *playerUUID : empty;
}



MultiplayerClient::Statistics MultiplayerClient::GetStatistics() const
{
	Statistics stats;

	stats.connectionState = state;
	stats.ping = connectionMonitor.GetPing();
	stats.packetLoss = connectionMonitor.GetPacketLoss();
	stats.jitter = connectionMonitor.GetJitter();
	stats.commandsSent = commandsSent;
	stats.stateUpdatesReceived = stateUpdatesReceived;
	stats.predictionErrors = predictionErrors;
	stats.interpolatedEntities = interpolator.GetTrackedEntityCount();

	return stats;
}



void MultiplayerClient::OnConnected()
{
	state = State::CONNECTED;
	cout << "Connected to server!" << endl;

	// Wait for SERVER_WELCOME packet with player UUID
}



void MultiplayerClient::OnDisconnected()
{
	cout << "Server disconnected" << endl;
	state = State::DISCONNECTED;
}



void MultiplayerClient::OnServerWelcome(const vector<uint8_t> &data)
{
	// Deserialize SERVER_WELCOME packet
	// TODO: Use PacketReader to deserialize
	// - Player UUID
	// - Initial game state
	// - Server configuration

	cout << "Received SERVER_WELCOME" << endl;

	// Generate player UUID for now
	playerUUID = make_unique<EsUuid>(EsUuid::MakeUuid());

	cout << "Player UUID: " << playerUUID->ToString() << endl;
}



void MultiplayerClient::OnStateUpdate(const vector<uint8_t> &data)
{
	// Deserialize state update from server
	// TODO: Use PacketReader to deserialize
	// - Game tick
	// - Snapshot data (ships, projectiles, etc.)

	++stateUpdatesReceived;

	// Create temporary server state
	// (In real implementation, deserialize from packet)
	GameState serverState;

	// Reconcile prediction with server state
	ReconcileWithServer(serverState);

	// Update connection monitor
	connectionMonitor.RecordPacketReceived(stateUpdatesReceived);
}



void MultiplayerClient::OnPlayerJoined(const vector<uint8_t> &data)
{
	// TODO: Deserialize player info and add to game state
	cout << "Player joined" << endl;
}



void MultiplayerClient::OnPlayerLeft(const vector<uint8_t> &data)
{
	// TODO: Deserialize player UUID and remove from game state
	cout << "Player left" << endl;
}



void MultiplayerClient::SendCommandToServer(const PlayerCommand &command)
{
	// Serialize command to packet
	// TODO: Use PacketWriter to serialize
	// - Player UUID
	// - Game tick
	// - Command state
	// - Sequence number

	// Send via network manager
	// TODO: networkManager->Send(packetData);

	// Track for connection monitoring
	connectionMonitor.RecordPacketSent(commandsSent);
}



void MultiplayerClient::ApplyPrediction(const PlayerCommand &command)
{
	if(!predictor || !gameState)
		return;

	// Predict next state
	auto predictedState = predictor->PredictNextState(*gameState, command);

	// Apply predicted state (copy relevant parts)
	if(predictedState)
	{
		// In a full implementation, we'd copy the predicted state
		// For now, just step the game forward
		gameState->Step();
	}
}



void MultiplayerClient::ReconcileWithServer(const GameState &serverState)
{
	if(!predictor || !gameState)
		return;

	// Get reconciled state from predictor
	uint64_t serverTick = serverState.GetGameTick();
	auto reconciledState = predictor->ReconcileWithServer(serverState, serverTick);

	if(reconciledState)
	{
		// Check for prediction error
		// (Simplified: just check if state differs)
		if(reconciledState->GetGameTick() != gameState->GetGameTick())
			++predictionErrors;

		// Apply reconciliation smoothly
		// TODO: Use ClientReconciliation to smooth position adjustments

		// Update game state
		*gameState = *reconciledState;
	}
}



void MultiplayerClient::ProcessNetworkInput()
{
	if(!networkManager)
		return;

	// Poll for network events
	// TODO: NetworkManager needs to provide event polling
	// This would call OnConnected, OnDisconnected, OnStateUpdate, etc.

	// Placeholder: In real implementation, NetworkManager would have:
	// while(auto event = networkManager->PollEvent()) {
	//     switch(event.type) {
	//         case CONNECTED: OnConnected(); break;
	//         case DISCONNECTED: OnDisconnected(); break;
	//         case PACKET_RECEIVED:
	//             switch(packet.type) {
	//                 case SERVER_WELCOME: OnServerWelcome(packet.data); break;
	//                 case SERVER_WORLD_STATE: OnStateUpdate(packet.data); break;
	//                 // etc.
	//             }
	//     }
	// }
}



void MultiplayerClient::InitializeSubsystems()
{
	// Create game state
	gameState = make_unique<GameState>();

	// Create network manager
	networkManager = make_unique<NetworkManager>();

	// Create predictor
	predictor = make_unique<Predictor>();

	cout << "Client subsystems initialized" << endl;
}



void MultiplayerClient::CleanupSubsystems()
{
	predictor.reset();
	networkManager.reset();
	gameState.reset();
	playerUUID.reset();
}
