// StateSync.cpp

#include "StateSync.h"

#include "../Ship.h"
#include "../GameState.h"
#include "../network/PacketWriter.h"

#include <algorithm>

using namespace std;



StateSync::StateSync()
	: interestManager(nullptr), currentTick(0)
{
}



void StateSync::SetInterestManager(InterestManager *manager)
{
	interestManager = manager;
}



void StateSync::SetCurrentTick(uint64_t tick)
{
	currentTick = tick;

	// Update all dead reckoning timestamps
	for(auto &pair : shipDeadReckoning)
		pair.second.SetCurrentTimestamp(tick);
}



uint64_t StateSync::GetCurrentTick() const
{
	return currentTick;
}



StateSync::ShipUpdate StateSync::CaptureShipState(const Ship &ship, UpdateScope scope)
{
	ShipUpdate update;
	update.shipUUID = ship.UUID();
	update.timestamp = currentTick;
	update.scope = scope;

	// Always include position data
	update.position = ship.Position();
	update.velocity = ship.Velocity();
	update.angle = ship.Facing();

	// Include additional data based on scope
	if(scope == UpdateScope::FULL || scope == UpdateScope::VITAL)
	{
		update.shields = static_cast<float>(ship.Shields());
		update.hull = static_cast<float>(ship.Hull());
		update.energy = static_cast<float>(ship.Energy());
		update.fuel = static_cast<float>(ship.Fuel());
	}

	if(scope == UpdateScope::FULL)
	{
		// Encode status flags
		update.flags = 0;
		// Note: These methods would need to be added to Ship class or we use existing ones
		// For now, just set flags to 0 (placeholder)
		// In real implementation: update.flags |= ship.IsThrusting() ? 0x0001 : 0;
	}

	return update;
}



vector<StateSync::ShipUpdate> StateSync::GetUpdatesForPlayer(const EsUuid &playerUUID,
	const vector<shared_ptr<Ship>> &allShips)
{
	if(!interestManager)
		return {};

	vector<ShipUpdate> updates;

	for(const auto &ship : allShips)
	{
		if(!ship)
			continue;

		// Get interest level for this ship
		InterestManager::InterestLevel interest = interestManager->GetShipInterest(playerUUID, *ship);

		// Skip if not interested
		if(interest == InterestManager::InterestLevel::NONE)
			continue;

		// Check if we should update this ship this tick
		if(!interestManager->ShouldUpdateThisTick(interest, currentTick))
			continue;

		// Determine update priority and scope
		UpdatePriority priority = InterestToPriority(interest);
		UpdateScope scope = DetermineUpdateScope(priority);

		// Capture and add update
		ShipUpdate update = CaptureShipState(*ship, scope);
		updates.push_back(update);
	}

	return updates;
}



void StateSync::ApplyShipUpdate(Ship &ship, const ShipUpdate &update)
{
	// Apply position data (always present)
	ship.SetPosition(update.position);
	ship.SetVelocity(update.velocity);
	ship.SetFacing(update.angle);

	// Apply vital data if present
	if(update.scope == UpdateScope::FULL || update.scope == UpdateScope::VITAL)
	{
		ship.SetShields(update.shields);
		ship.SetHull(update.hull);
		ship.SetEnergy(update.energy);
		ship.SetFuel(update.fuel);
	}

	// Apply flags if present (full update only)
	if(update.scope == UpdateScope::FULL)
	{
		// Decode status flags
		// Note: These methods would need to be added to Ship class
		// For now, this is a placeholder
	}

	// Update dead reckoning state
	DeadReckoning::State drState(update.position, update.velocity, update.angle, update.timestamp);
	UpdateDeadReckoning(update.shipUUID, drState);
}



void StateSync::UpdateDeadReckoning(const EsUuid &shipUUID, const DeadReckoning::State &state)
{
	auto &dr = shipDeadReckoning[shipUUID];
	dr.SetAuthoritativeState(state);
	dr.SetCurrentTimestamp(currentTick);
}



DeadReckoning::State StateSync::PredictShipState(const EsUuid &shipUUID, uint64_t targetTick)
{
	auto it = shipDeadReckoning.find(shipUUID);
	if(it == shipDeadReckoning.end())
	{
		// No dead reckoning data, return empty state
		return DeadReckoning::State();
	}

	return it->second.PredictState(targetTick);
}



void StateSync::WriteShipUpdate(PacketWriter &writer, const ShipUpdate &update)
{
	// Write ship UUID
	writer.WriteUuid(update.shipUUID);

	// Write timestamp
	writer.WriteUint64(update.timestamp);

	// Write scope
	writer.WriteUint8(static_cast<uint8_t>(update.scope));

	// Always write position data
	writer.WritePoint(update.position);
	writer.WritePoint(update.velocity);
	writer.WriteAngle(update.angle);

	// Write vital data if scope includes it
	if(update.scope == UpdateScope::FULL || update.scope == UpdateScope::VITAL)
	{
		writer.WriteFloat(update.shields);
		writer.WriteFloat(update.hull);
		writer.WriteFloat(update.energy);
		writer.WriteFloat(update.fuel);
	}

	// Write flags if full scope
	if(update.scope == UpdateScope::FULL)
	{
		writer.WriteUint16(update.flags);
	}
}



StateSync::UpdatePriority StateSync::GetUpdatePriority(const EsUuid &playerUUID, const Ship &ship)
{
	if(!interestManager)
		return UpdatePriority::NONE;

	InterestManager::InterestLevel interest = interestManager->GetShipInterest(playerUUID, ship);
	return InterestToPriority(interest);
}



StateSync::UpdateScope StateSync::DetermineUpdateScope(UpdatePriority priority)
{
	switch(priority)
	{
		case UpdatePriority::CRITICAL:
			return UpdateScope::FULL;  // Send everything for critical entities
		case UpdatePriority::HIGH:
			return UpdateScope::FULL;  // Also send full updates for high priority
		case UpdatePriority::MEDIUM:
			return UpdateScope::POSITION;  // Only position for medium priority
		case UpdatePriority::LOW:
			return UpdateScope::MINIMAL;  // Minimal data for low priority
		case UpdatePriority::NONE:
		default:
			return UpdateScope::MINIMAL;
	}
}



void StateSync::ClearDeadReckoning()
{
	shipDeadReckoning.clear();
}



size_t StateSync::GetTrackedShipCount() const
{
	return shipDeadReckoning.size();
}



StateSync::UpdatePriority StateSync::InterestToPriority(InterestManager::InterestLevel level)
{
	switch(level)
	{
		case InterestManager::InterestLevel::CRITICAL:
			return UpdatePriority::CRITICAL;
		case InterestManager::InterestLevel::HIGH:
			return UpdatePriority::HIGH;
		case InterestManager::InterestLevel::MEDIUM:
			return UpdatePriority::MEDIUM;
		case InterestManager::InterestLevel::LOW:
			return UpdatePriority::LOW;
		case InterestManager::InterestLevel::NONE:
		default:
			return UpdatePriority::NONE;
	}
}
