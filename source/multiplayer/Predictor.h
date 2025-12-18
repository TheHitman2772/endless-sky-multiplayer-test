/* Predictor.h
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

#include "PlayerCommand.h"

#include <cstdint>
#include <memory>
#include <vector>

class GameState;


// Predictor: Client-side prediction and reconciliation
//
// This class handles client-side prediction of game state, allowing the client
// to show immediate response to player input while waiting for server confirmation.
//
// How it works:
// 1. Client sends command to server
// 2. Client immediately predicts result (runs GameState::Step locally)
// 3. Server sends authoritative state update
// 4. Client reconciles: if mismatch, re-simulate from last confirmed state
//
// Design Goals:
// - Responsive local gameplay (no input lag)
// - Smooth reconciliation when prediction wrong
// - Minimal memory overhead
// - Works with GameState (Phase 2.1)
//
// Usage:
// Client:
//   Predictor predictor;
//   predictor.RecordCommand(localCmd);
//   predictor.PredictNextState(currentState, localCmd);
//   // Later, when server update arrives:
//   predictor.ReconcileWithServer(serverState, serverTick);
class Predictor {
public:
	Predictor();
	~Predictor() = default;

	// Record a command that was sent to server
	void RecordCommand(const PlayerCommand &command);

	// Predict next game state based on command
	// Returns predicted state (caller should use for rendering)
	std::shared_ptr<GameState> PredictNextState(
		const GameState &currentState,
		const PlayerCommand &command
	);

	// Reconcile with authoritative server state
	// Returns reconciled state (may differ from predicted)
	std::shared_ptr<GameState> ReconcileWithServer(
		const GameState &serverState,
		uint64_t serverTick
	);

	// Get last confirmed server tick
	uint64_t GetLastConfirmedTick() const { return lastConfirmedTick; }

	// Get number of unconfirmed commands
	size_t GetUnconfirmedCommandCount() const { return unconfirmedCommands.size(); }

	// Check if currently predicting ahead of server
	bool IsPredicting() const { return !unconfirmedCommands.empty(); }

	// Get prediction error count (for debugging)
	uint64_t GetPredictionErrorCount() const { return predictionErrors; }

	// Clear all prediction state
	void Clear();

	// Configuration
	void SetMaxUnconfirmedCommands(size_t max) { maxUnconfirmedCommands = max; }
	size_t GetMaxUnconfirmedCommands() const { return maxUnconfirmedCommands; }


private:
	// Commands sent to server but not yet confirmed
	std::vector<PlayerCommand> unconfirmedCommands;

	// Last confirmed tick from server
	uint64_t lastConfirmedTick = 0;

	// Configuration
	size_t maxUnconfirmedCommands = 60;  // Max 1 second of prediction at 60 Hz

	// Statistics
	uint64_t predictionErrors = 0;

	// Apply command to game state (prediction)
	void ApplyCommand(GameState &state, const PlayerCommand &command);

	// Check if prediction matches server state
	bool PredictionMatches(const GameState &predicted, const GameState &server) const;
};
