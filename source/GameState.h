/* GameState.h
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
#include <list>
#include <memory>
#include <vector>

class AsteroidField;
class Flotsam;
class Projectile;
class Ship;
class System;
class Visual;


// GameState: Server-authoritative simulation state
//
// This class represents the complete state of the game world for simulation purposes.
// It is decoupled from rendering and UI, making it suitable for:
// - Server-side authoritative simulation
// - Client-side prediction
// - State synchronization over network
// - Deterministic replay
//
// Design Goals:
// - Pure data container (no rendering logic)
// - Serializable for network transmission
// - Copyable for prediction/rollback
// - Independent of PlayerInfo (supports multiple players)
class GameState {
public:
	GameState();
	~GameState() = default;

	// Copy constructor and assignment (for prediction/rollback)
	GameState(const GameState &other);
	GameState &operator=(const GameState &other);

	// Move semantics
	GameState(GameState &&other) = default;
	GameState &operator=(GameState &&other) = default;

	// Simulation step (advance game by one tick at 60 Hz)
	void Step();

	// Current system (where simulation is happening)
	const System *GetSystem() const { return currentSystem; }
	void SetSystem(const System *system);

	// Ship management
	const std::list<std::shared_ptr<Ship>> &GetShips() const { return ships; }
	void AddShip(std::shared_ptr<Ship> ship);
	void RemoveShip(std::shared_ptr<Ship> ship);
	size_t GetShipCount() const { return ships.size(); }

	// Projectile management
	const std::list<Projectile> &GetProjectiles() const { return projectiles; }
	void AddProjectile(const Projectile &projectile);
	void RemoveProjectile(const Projectile &projectile);
	size_t GetProjectileCount() const { return projectiles.size(); }

	// Flotsam management (cargo/salvage floating in space)
	const std::list<std::shared_ptr<Flotsam>> &GetFlotsam() const { return flotsam; }
	void AddFlotsam(std::shared_ptr<Flotsam> item);
	void RemoveFlotsam(std::shared_ptr<Flotsam> item);

	// Visual effects (explosions, etc.)
	const std::list<Visual> &GetVisuals() const { return visuals; }
	void AddVisual(const Visual &visual);

	// Asteroid field
	const AsteroidField *GetAsteroids() const { return asteroids.get(); }
	void SetAsteroids(std::unique_ptr<AsteroidField> field);

	// Game time tracking
	uint64_t GetGameTick() const { return gameTick; }
	void SetGameTick(uint64_t tick) { gameTick = tick; }
	void IncrementGameTick() { ++gameTick; }

	// Clear all state (for system transitions)
	void Clear();

	// State validation
	bool IsValid() const;

	// Serialization support (for network transmission)
	// These will be implemented when we integrate with Phase 1 packet system
	// void Serialize(PacketWriter &writer) const;
	// void Deserialize(PacketReader &reader);


private:
	// Current system being simulated
	const System *currentSystem = nullptr;

	// Ships in current system (both player and NPC)
	std::list<std::shared_ptr<Ship>> ships;

	// Projectiles in flight
	std::list<Projectile> projectiles;

	// Flotsam (cargo, debris)
	std::list<std::shared_ptr<Flotsam>> flotsam;

	// Visual effects
	std::list<Visual> visuals;

	// Asteroid field (if present in current system)
	std::unique_ptr<AsteroidField> asteroids;

	// Game tick counter (60 Hz, increments every 1/60 second)
	uint64_t gameTick = 0;
};
