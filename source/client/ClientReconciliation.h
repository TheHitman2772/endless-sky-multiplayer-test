/* ClientReconciliation.h
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

#include "../Point.h"
#include "../Angle.h"

#include <cstdint>


// ClientReconciliation: Smooth correction of prediction errors
//
// Problem: Client-side prediction isn't perfect
// - Client predicts: "I'm at position X"
// - Server says: "Actually, you're at position Y"
// - Teleporting to Y looks janky
//
// Solution: Gradual error correction
// - Calculate error: difference = Y - X
// - Apply correction over N frames
// - Player experiences smooth adjustment instead of teleport
//
// Techniques:
// 1. Error smoothing - Gradually blend towards server position
// 2. Error threshold - Ignore tiny errors (< 1 pixel)
// 3. Snap threshold - Teleport if error is huge (desync)
//
// Trade-offs:
// - Slower correction = smoother but less accurate
// - Faster correction = more accurate but jerkier
// - Typical: 100-200ms correction time
class ClientReconciliation {
public:
	ClientReconciliation();

	// Start reconciliation for position error
	void ReconcilePosition(const Point &predictedPos, const Point &serverPos);

	// Start reconciliation for velocity error
	void ReconcileVelocity(const Point &predictedVel, const Point &serverVel);

	// Start reconciliation for facing error
	void ReconcileFacing(const Angle &predictedAngle, const Angle &serverAngle);

	// Get corrected position (call every frame)
	Point GetCorrectedPosition(const Point &currentPos);

	// Get corrected velocity (call every frame)
	Point GetCorrectedVelocity(const Point &currentVel);

	// Get corrected facing (call every frame)
	Angle GetCorrectedFacing(const Angle &currentAngle);

	// Update (call every frame to advance correction)
	void Update();

	// Check if reconciliation in progress
	bool IsReconcilingPosition() const { return positionCorrectionProgress < 1.0; }
	bool IsReconcilingVelocity() const { return velocityCorrectionProgress < 1.0; }
	bool IsReconcilingFacing() const { return facingCorrectionProgress < 1.0; }

	// Configuration
	void SetCorrectionTime(double seconds) { correctionTimeSeconds = seconds; }
	double GetCorrectionTime() const { return correctionTimeSeconds; }

	void SetErrorThreshold(double pixels) { errorThresholdPx = pixels; }
	double GetErrorThreshold() const { return errorThresholdPx; }

	void SetSnapThreshold(double pixels) { snapThresholdPx = pixels; }
	double GetSnapThreshold() const { return snapThresholdPx; }

	// Statistics
	uint64_t GetTotalReconciliations() const { return totalReconciliations; }
	uint64_t GetTotalSnaps() const { return totalSnaps; }
	double GetAverageError() const { return averageError; }


private:
	// Position correction
	Point positionError;
	double positionCorrectionProgress = 1.0;  // 0.0 = start, 1.0 = done

	// Velocity correction
	Point velocityError;
	double velocityCorrectionProgress = 1.0;

	// Facing correction
	double facingErrorDegrees = 0.0;
	double facingCorrectionProgress = 1.0;

	// Configuration
	double correctionTimeSeconds = 0.15;  // 150ms correction time
	double errorThresholdPx = 1.0;        // Ignore errors < 1 pixel
	double snapThresholdPx = 500.0;       // Teleport if error > 500 pixels

	// Statistics
	uint64_t totalReconciliations = 0;
	uint64_t totalSnaps = 0;
	double averageError = 0.0;

	// Helper: Calculate correction step per frame (assuming 60 FPS)
	double GetCorrectionStepPerFrame() const;

	// Helper: Check if error is significant
	bool IsSignificantError(double errorMagnitude) const;
};
