/* Predictor.cpp
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

#include "Predictor.h"

#include "../GameState.h"
#include "../Projectile.h"
#include "../Visual.h"
#include "../AsteroidField.h"

#include <algorithm>

using namespace std;



Predictor::Predictor()
{
}



void Predictor::RecordCommand(const PlayerCommand &command)
{
	// Add to unconfirmed commands
	unconfirmedCommands.push_back(command);

	// Limit buffer size
	if(unconfirmedCommands.size() > maxUnconfirmedCommands)
		unconfirmedCommands.erase(unconfirmedCommands.begin());
}



shared_ptr<GameState> Predictor::PredictNextState(
	const GameState &currentState,
	const PlayerCommand &command)
{
	// Create a copy of current state for prediction
	auto predictedState = make_shared<GameState>(currentState);

	// Apply command to predicted state
	ApplyCommand(*predictedState, command);

	// Step simulation forward
	predictedState->Step();

	return predictedState;
}



shared_ptr<GameState> Predictor::ReconcileWithServer(
	const GameState &serverState,
	uint64_t serverTick)
{
	// Update last confirmed tick
	lastConfirmedTick = serverTick;

	// Remove confirmed commands
	unconfirmedCommands.erase(
		remove_if(unconfirmedCommands.begin(), unconfirmedCommands.end(),
			[serverTick](const PlayerCommand &cmd) {
				return cmd.gameTick <= serverTick;
			}),
		unconfirmedCommands.end()
	);

	// If no unconfirmed commands, just use server state
	if(unconfirmedCommands.empty())
		return make_shared<GameState>(serverState);

	// Re-simulate unconfirmed commands on top of server state
	auto reconciledState = make_shared<GameState>(serverState);

	for(const auto &cmd : unconfirmedCommands)
	{
		ApplyCommand(*reconciledState, cmd);
		reconciledState->Step();
	}

	// Check if prediction was accurate (for statistics)
	// Note: This is a simplified check - full implementation would compare states
	if(!PredictionMatches(*reconciledState, serverState))
		++predictionErrors;

	return reconciledState;
}



void Predictor::Clear()
{
	unconfirmedCommands.clear();
	lastConfirmedTick = 0;
	predictionErrors = 0;
}



void Predictor::ApplyCommand(GameState &state, const PlayerCommand &command)
{
	// In a full implementation, this would:
	// 1. Find the player's ship in the GameState
	// 2. Apply the command to that ship
	// 3. Handle targeting, autopilot, etc.

	// For now, this is a placeholder
	// The actual implementation would need to:
	// - Look up ship by player UUID (requires ship ownership from Phase 2.2)
	// - Apply command.command to ship's controls
	// - Set target point if hasTargetPoint is true

	// Placeholder: Just increment game tick
	// Real implementation will integrate with Ship class
}



bool Predictor::PredictionMatches(const GameState &predicted, const GameState &server) const
{
	// In a full implementation, this would compare:
	// - Ship positions
	// - Ship velocities
	// - Projectile counts
	// - Game tick

	// For now, simple tick comparison
	return predicted.GetGameTick() == server.GetGameTick();
}
