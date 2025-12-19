// DeadReckoning.cpp

#include "DeadReckoning.h"

#include <cmath>

using namespace std;



DeadReckoning::DeadReckoning()
	: currentTimestamp(0)
{
}



void DeadReckoning::SetAuthoritativeState(const State &state)
{
	authoritativeState = state;
}



const DeadReckoning::State &DeadReckoning::GetAuthoritativeState() const
{
	return authoritativeState;
}



DeadReckoning::State DeadReckoning::PredictState(uint64_t targetTimestamp) const
{
	// If target is at or before the authoritative state, return that state
	if(targetTimestamp <= authoritativeState.timestamp)
		return authoritativeState;

	// Calculate time delta in ticks
	uint64_t deltaTicks = targetTimestamp - authoritativeState.timestamp;

	// Linear extrapolation: newPosition = oldPosition + velocity * deltaTime
	// In Endless Sky, velocity is already in units per frame (60 FPS)
	Point predictedPosition = authoritativeState.position + authoritativeState.velocity * deltaTicks;

	// Angle prediction: for now, we assume angle doesn't change during dead reckoning
	// (ships turn slowly, and we'll get angle updates from server)
	// More advanced: could extrapolate angular velocity if we tracked it
	Angle predictedAngle = authoritativeState.angle;

	return State(predictedPosition, authoritativeState.velocity, predictedAngle, targetTimestamp);
}



DeadReckoning::State DeadReckoning::PredictStateAhead(uint64_t ticksAhead) const
{
	return PredictState(authoritativeState.timestamp + ticksAhead);
}



double DeadReckoning::GetPositionError(const State &predictedState, const State &actualState) const
{
	Point delta = actualState.position - predictedState.position;
	return sqrt(delta.X() * delta.X() + delta.Y() * delta.Y());
}



bool DeadReckoning::IsErrorExcessive(const State &predictedState, const State &actualState,
	double threshold) const
{
	return GetPositionError(predictedState, actualState) > threshold;
}



uint64_t DeadReckoning::GetCurrentTimestamp() const
{
	return currentTimestamp;
}



void DeadReckoning::SetCurrentTimestamp(uint64_t timestamp)
{
	currentTimestamp = timestamp;
}



void DeadReckoning::Reset()
{
	authoritativeState = State();
	currentTimestamp = 0;
}
