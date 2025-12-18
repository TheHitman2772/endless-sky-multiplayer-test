/* ClientState.h
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

#include "Point.h"

#include <cstdint>
#include <memory>

class Ship;


// ClientState: Client-specific presentation and UI state
//
// This class represents state that is specific to a client and does not need
// to be synchronized with the server. It includes:
// - Camera position and zoom
// - UI state and selections
// - Client-side prediction state
// - Local player ship reference
// - Rendering preferences
//
// Design Goals:
// - Separate from GameState (server-authoritative simulation)
// - Not synchronized over network
// - Supports smooth camera movement and UI
// - Enables client-side prediction for responsive controls
class ClientState {
public:
	ClientState();
	~ClientState() = default;

	// Camera control
	const Point &GetCameraCenter() const { return cameraCenter; }
	void SetCameraCenter(const Point &center) { cameraCenter = center; }
	void MoveCameraBy(const Point &offset) { cameraCenter += offset; }

	double GetCameraZoom() const { return cameraZoom; }
	void SetCameraZoom(double zoom);

	// Update camera to follow a ship smoothly
	void UpdateCamera(const Ship *ship, double zoomTarget = 1.0);

	// Local player ship
	std::shared_ptr<Ship> GetPlayerShip() const { return playerShip.lock(); }
	void SetPlayerShip(std::shared_ptr<Ship> ship) { playerShip = ship; }

	// UI state
	bool IsRadarOn() const { return radarOn; }
	void SetRadarOn(bool on) { radarOn = on; }

	bool ShowLabels() const { return showLabels; }
	void SetShowLabels(bool show) { showLabels = show; }

	bool ShowTooltips() const { return showTooltips; }
	void SetShowTooltips(bool show) { showTooltips = show; }

	// Client-side prediction
	bool IsPredictionEnabled() const { return predictionEnabled; }
	void SetPredictionEnabled(bool enabled) { predictionEnabled = enabled; }

	uint64_t GetLastConfirmedTick() const { return lastConfirmedTick; }
	void SetLastConfirmedTick(uint64_t tick) { lastConfirmedTick = tick; }

	// Frame interpolation (for smooth rendering between physics steps)
	double GetInterpolationAlpha() const { return interpolationAlpha; }
	void SetInterpolationAlpha(double alpha);

	// Clear all state
	void Clear();


private:
	// Camera state
	Point cameraCenter;
	double cameraZoom = 1.0;

	// Smooth camera movement
	Point targetCameraCenter;
	double targetCameraZoom = 1.0;
	static constexpr double CAMERA_SMOOTHING = 0.2;

	// Local player ship (weak_ptr to avoid ownership issues)
	std::weak_ptr<Ship> playerShip;

	// UI state
	bool radarOn = true;
	bool showLabels = true;
	bool showTooltips = true;

	// Client-side prediction state
	bool predictionEnabled = true;
	uint64_t lastConfirmedTick = 0;

	// Frame interpolation (0.0 = previous frame, 1.0 = current frame)
	double interpolationAlpha = 0.0;
};
