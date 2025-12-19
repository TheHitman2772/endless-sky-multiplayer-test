// DeadReckoning.h
// Phase 3.2: Dead reckoning for entity position prediction
// Predicts entity positions between server updates for smooth client-side rendering

#ifndef DEAD_RECKONING_H_
#define DEAD_RECKONING_H_

#include "../Point.h"
#include "../Angle.h"

#include <cstdint>



// DeadReckoning predicts entity state between server updates.
// This allows smooth client-side movement even at lower network update rates.
class DeadReckoning {
public:
	// State snapshot at a specific point in time
	struct State {
		Point position;
		Point velocity;
		Angle angle;
		uint64_t timestamp;  // Game tick when this state was recorded

		State() : timestamp(0) {}
		State(const Point &pos, const Point &vel, const Angle &ang, uint64_t tick)
			: position(pos), velocity(vel), angle(ang), timestamp(tick) {}
	};

	DeadReckoning();

	// Set the last known authoritative state from the server
	void SetAuthoritativeState(const State &state);

	// Get the last authoritative state
	const State &GetAuthoritativeState() const;

	// Predict the state at a given timestamp using dead reckoning
	// (linear extrapolation based on velocity)
	State PredictState(uint64_t targetTimestamp) const;

	// Predict the state N ticks into the future
	State PredictStateAhead(uint64_t ticksAhead) const;

	// Get error between predicted and actual state (for error correction)
	double GetPositionError(const State &predictedState, const State &actualState) const;

	// Check if prediction error exceeds threshold (indicates need for correction)
	bool IsErrorExcessive(const State &predictedState, const State &actualState,
		double threshold = 100.0) const;

	// Get the current timestamp (for internal use)
	uint64_t GetCurrentTimestamp() const;

	// Set current timestamp (updated each game tick)
	void SetCurrentTimestamp(uint64_t timestamp);

	// Reset dead reckoning state
	void Reset();


private:
	State authoritativeState;  // Last known server state
	uint64_t currentTimestamp; // Current game tick
};



#endif
