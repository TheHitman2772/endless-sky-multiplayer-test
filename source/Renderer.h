/* Renderer.h
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

class ClientState;
class GameState;


// Renderer: Decoupled rendering system
//
// This class handles all rendering operations, taking GameState (simulation)
// and ClientState (camera, UI) as input and producing visual output.
//
// Design Goals:
// - Pure rendering logic (no simulation or game logic)
// - Stateless (all state comes from GameState and ClientState)
// - Can be run in separate thread from simulation
// - Supports both single-player and multiplayer
//
// Architecture:
// - GameState provides WHAT to render (ships, projectiles, effects)
// - ClientState provides WHERE to render (camera position, zoom)
// - Renderer handles HOW to render (OpenGL calls, sprites, etc.)
//
// Usage:
//   Renderer renderer;
//   renderer.Render(gameState, clientState);
class Renderer {
public:
	Renderer();
	~Renderer() = default;

	// Render the game world
	// This is the main rendering entry point
	void Render(const GameState &gameState, const ClientState &clientState);

	// Render individual components (called by Render)
	void RenderBackground(const GameState &gameState, const ClientState &clientState);
	void RenderAsteroids(const GameState &gameState, const ClientState &clientState);
	void RenderShips(const GameState &gameState, const ClientState &clientState);
	void RenderProjectiles(const GameState &gameState, const ClientState &clientState);
	void RenderFlotsam(const GameState &gameState, const ClientState &clientState);
	void RenderVisuals(const GameState &gameState, const ClientState &clientState);
	void RenderUI(const GameState &gameState, const ClientState &clientState);

	// Performance metrics
	double GetFrameTime() const { return frameTime; }
	double GetFPS() const { return fps; }

	// Rendering options
	void SetShowDebug(bool show) { showDebug = show; }
	bool IsShowDebug() const { return showDebug; }


private:
	// Update performance metrics
	void UpdateMetrics();

	// Performance tracking
	double frameTime = 0.0;
	double fps = 60.0;
	uint64_t lastFrameTime = 0;

	// Rendering options
	bool showDebug = false;
};
