/* ClientReconciliation.cpp
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

#include "ClientReconciliation.h"

#include <cmath>
#include <algorithm>

using namespace std;



ClientReconciliation::ClientReconciliation()
{
}



void ClientReconciliation::ReconcilePosition(const Point &predictedPos, const Point &serverPos)
{
	// Calculate error
	positionError = serverPos - predictedPos;
	double errorMagnitude = positionError.Length();

	// Update average error (exponential moving average)
	const double alpha = 0.1;
	averageError = alpha * errorMagnitude + (1.0 - alpha) * averageError;

	// Check if error is significant
	if(!IsSignificantError(errorMagnitude))
	{
		// Error too small, ignore
		positionError = Point();
		positionCorrectionProgress = 1.0;
		return;
	}

	// Check if error is too large (desync)
	if(errorMagnitude > snapThresholdPx)
	{
		// Snap immediately instead of smoothing
		positionError = Point();  // No gradual correction needed
		positionCorrectionProgress = 1.0;
		++totalSnaps;
		return;
	}

	// Start gradual correction
	positionCorrectionProgress = 0.0;
	++totalReconciliations;
}



void ClientReconciliation::ReconcileVelocity(const Point &predictedVel, const Point &serverVel)
{
	velocityError = serverVel - predictedVel;
	double errorMagnitude = velocityError.Length();

	// Velocity errors are typically smaller, use smaller threshold
	if(errorMagnitude < 0.1)
	{
		velocityError = Point();
		velocityCorrectionProgress = 1.0;
		return;
	}

	velocityCorrectionProgress = 0.0;
}



void ClientReconciliation::ReconcileFacing(const Angle &predictedAngle, const Angle &serverAngle)
{
	// Calculate angular error (shortest path)
	double predictedDegrees = predictedAngle.Degrees();
	double serverDegrees = serverAngle.Degrees();

	facingErrorDegrees = serverDegrees - predictedDegrees;

	// Handle wraparound
	if(facingErrorDegrees > 180.0)
		facingErrorDegrees -= 360.0;
	else if(facingErrorDegrees < -180.0)
		facingErrorDegrees += 360.0;

	// Check if error is significant (< 1 degree)
	if(abs(facingErrorDegrees) < 1.0)
	{
		facingErrorDegrees = 0.0;
		facingCorrectionProgress = 1.0;
		return;
	}

	facingCorrectionProgress = 0.0;
}



Point ClientReconciliation::GetCorrectedPosition(const Point &currentPos)
{
	if(positionCorrectionProgress >= 1.0)
		return currentPos;

	// Calculate how much error to apply this frame
	double step = GetCorrectionStepPerFrame();
	Point correction = positionError * step;

	return currentPos + correction;
}



Point ClientReconciliation::GetCorrectedVelocity(const Point &currentVel)
{
	if(velocityCorrectionProgress >= 1.0)
		return currentVel;

	double step = GetCorrectionStepPerFrame();
	Point correction = velocityError * step;

	return currentVel + correction;
}



Angle ClientReconciliation::GetCorrectedFacing(const Angle &currentAngle)
{
	if(facingCorrectionProgress >= 1.0)
		return currentAngle;

	double step = GetCorrectionStepPerFrame();
	double correction = facingErrorDegrees * step;

	return Angle(currentAngle.Degrees() + correction);
}



void ClientReconciliation::Update()
{
	double step = GetCorrectionStepPerFrame();

	// Advance position correction
	if(positionCorrectionProgress < 1.0)
	{
		positionCorrectionProgress += step;
		if(positionCorrectionProgress >= 1.0)
		{
			positionCorrectionProgress = 1.0;
			positionError = Point();
		}
	}

	// Advance velocity correction
	if(velocityCorrectionProgress < 1.0)
	{
		velocityCorrectionProgress += step;
		if(velocityCorrectionProgress >= 1.0)
		{
			velocityCorrectionProgress = 1.0;
			velocityError = Point();
		}
	}

	// Advance facing correction
	if(facingCorrectionProgress < 1.0)
	{
		facingCorrectionProgress += step;
		if(facingCorrectionProgress >= 1.0)
		{
			facingCorrectionProgress = 1.0;
			facingErrorDegrees = 0.0;
		}
	}
}



double ClientReconciliation::GetCorrectionStepPerFrame() const
{
	// Assuming 60 FPS
	// If correction time is 0.15 seconds, need 9 frames (0.15 * 60)
	// Each frame should advance by 1/9 = 0.111
	double framesForCorrection = correctionTimeSeconds * 60.0;
	if(framesForCorrection < 1.0)
		framesForCorrection = 1.0;

	return 1.0 / framesForCorrection;
}



bool ClientReconciliation::IsSignificantError(double errorMagnitude) const
{
	return errorMagnitude >= errorThresholdPx;
}
