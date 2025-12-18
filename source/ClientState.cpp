/* ClientState.cpp
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

#include "ClientState.h"

#include "Ship.h"

#include <algorithm>

using namespace std;



ClientState::ClientState()
{
}



void ClientState::SetCameraZoom(double zoom)
{
	// Clamp zoom to reasonable range
	cameraZoom = max(0.25, min(4.0, zoom));
	targetCameraZoom = cameraZoom;
}



void ClientState::UpdateCamera(const Ship *ship, double zoomTarget)
{
	if(!ship)
		return;

	// Set target camera position to ship position
	targetCameraCenter = ship->Position();
	targetCameraZoom = max(0.25, min(4.0, zoomTarget));

	// Smoothly interpolate camera to target
	Point delta = targetCameraCenter - cameraCenter;
	cameraCenter += delta * CAMERA_SMOOTHING;

	// Smoothly interpolate zoom
	double zoomDelta = targetCameraZoom - cameraZoom;
	cameraZoom += zoomDelta * CAMERA_SMOOTHING;
}



void ClientState::SetInterpolationAlpha(double alpha)
{
	// Clamp to [0, 1] range
	interpolationAlpha = max(0.0, min(1.0, alpha));
}



void ClientState::Clear()
{
	cameraCenter = Point();
	cameraZoom = 1.0;
	targetCameraCenter = Point();
	targetCameraZoom = 1.0;
	playerShip.reset();
	radarOn = true;
	showLabels = true;
	showTooltips = true;
	predictionEnabled = true;
	lastConfirmedTick = 0;
	interpolationAlpha = 0.0;
}
