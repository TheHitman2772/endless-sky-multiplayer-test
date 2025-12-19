// test_ship_state_sync.cpp
// Phase 3.2: Comprehensive tests for ship state synchronization

#include "../../source/multiplayer/InterestManager.h"
#include "../../source/multiplayer/DeadReckoning.h"
#include "../../source/multiplayer/StateSync.h"
#include "../../source/Ship.h"
#include "../../source/GameState.h"
#include "../../source/Point.h"
#include "../../source/Angle.h"
#include "../../source/EsUuid.h"

#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <memory>

using namespace std;

// Test counters
int testsRun = 0;
int testsPassed = 0;

void TEST(const string &name, bool condition)
{
	testsRun++;
	if(condition)
	{
		cout << "[PASS] " << name << endl;
		testsPassed++;
	}
	else
	{
		cout << "[FAIL] " << name << endl;
	}
}

// Helper function to compare doubles with tolerance
bool DoubleEqual(double a, double b, double epsilon = 0.001)
{
	return abs(a - b) < epsilon;
}

// Helper function to create a test ship
shared_ptr<Ship> CreateTestShip(const EsUuid &uuid, const Point &position, const Point &velocity,
	const Angle &angle, const EsUuid &ownerUUID = EsUuid())
{
	auto ship = make_shared<Ship>();
	ship->SetUUID(uuid);
	ship->SetPosition(position);
	ship->SetVelocity(velocity);
	ship->SetFacing(angle);
	ship->SetShields(1.0);
	ship->SetHull(1.0);
	ship->SetEnergy(1.0);
	ship->SetFuel(1.0);
	if(!ownerUUID.ToString().empty())
		ship->SetOwnerPlayerUUID(ownerUUID);
	return ship;
}


// ============================================================================
// InterestManager Tests
// ============================================================================

void TestInterestManagerBasics()
{
	cout << "\n=== InterestManager Basic Tests ===" << endl;

	InterestManager manager;
	InterestManager::Config config;
	config.criticalRange = 1000.0;
	config.highRange = 3000.0;
	config.mediumRange = 6000.0;
	config.lowRange = 10000.0;
	manager.SetConfig(config);

	EsUuid playerUUID = EsUuid::Generate();

	// Test player count initially zero
	TEST("InterestManager starts with zero players", manager.GetPlayerCount() == 0);

	// Add a player
	manager.SetPlayerInterestCenter(playerUUID, Point(0, 0));
	TEST("InterestManager adds player", manager.GetPlayerCount() == 1);

	// Test distance-based interest levels
	TEST("Interest CRITICAL at close range",
		manager.GetInterestLevelByDistance(500.0) == InterestManager::InterestLevel::CRITICAL);
	TEST("Interest HIGH at medium range",
		manager.GetInterestLevelByDistance(2000.0) == InterestManager::InterestLevel::HIGH);
	TEST("Interest MEDIUM at farther range",
		manager.GetInterestLevelByDistance(5000.0) == InterestManager::InterestLevel::MEDIUM);
	TEST("Interest LOW at far range",
		manager.GetInterestLevelByDistance(8000.0) == InterestManager::InterestLevel::LOW);
	TEST("Interest NONE beyond range",
		manager.GetInterestLevelByDistance(15000.0) == InterestManager::InterestLevel::NONE);

	// Remove player
	manager.RemovePlayer(playerUUID);
	TEST("InterestManager removes player", manager.GetPlayerCount() == 0);
}


void TestInterestManagerShipFiltering()
{
	cout << "\n=== InterestManager Ship Filtering Tests ===" << endl;

	InterestManager manager;
	EsUuid playerUUID = EsUuid::Generate();
	manager.SetPlayerInterestCenter(playerUUID, Point(0, 0));

	// Create test ships at various distances
	vector<shared_ptr<Ship>> allShips;

	// Ship 1: Very close (CRITICAL range)
	allShips.push_back(CreateTestShip(EsUuid::Generate(), Point(500, 0), Point(0, 0), Angle()));

	// Ship 2: Close (HIGH range)
	allShips.push_back(CreateTestShip(EsUuid::Generate(), Point(2000, 0), Point(0, 0), Angle()));

	// Ship 3: Medium distance
	allShips.push_back(CreateTestShip(EsUuid::Generate(), Point(5000, 0), Point(0, 0), Angle()));

	// Ship 4: Far (LOW range)
	allShips.push_back(CreateTestShip(EsUuid::Generate(), Point(8000, 0), Point(0, 0), Angle()));

	// Ship 5: Out of range (NONE)
	allShips.push_back(CreateTestShip(EsUuid::Generate(), Point(15000, 0), Point(0, 0), Angle()));

	// Get interested ships
	vector<const Ship *> interestedShips = manager.GetInterestedShips(playerUUID, allShips);

	// Should get all except the one out of range
	TEST("InterestManager filters out of range ships", interestedShips.size() == 4);

	// Test player's own ship is always CRITICAL
	auto ownShip = CreateTestShip(EsUuid::Generate(), Point(5000, 0), Point(0, 0), Angle(), playerUUID);
	InterestManager::InterestLevel ownInterest = manager.GetShipInterest(playerUUID, *ownShip);
	TEST("Player's own ship is CRITICAL interest", ownInterest == InterestManager::InterestLevel::CRITICAL);
}


void TestInterestManagerUpdateFrequency()
{
	cout << "\n=== InterestManager Update Frequency Tests ===" << endl;

	InterestManager manager;
	InterestManager::Config config;
	config.criticalFrequency = 1;  // Every tick
	config.highFrequency = 1;
	config.mediumFrequency = 2;    // Every 2nd tick
	config.lowFrequency = 5;       // Every 5th tick
	manager.SetConfig(config);

	// Test update frequencies
	TEST("CRITICAL updates every tick (tick 0)",
		manager.ShouldUpdateThisTick(InterestManager::InterestLevel::CRITICAL, 0));
	TEST("CRITICAL updates every tick (tick 1)",
		manager.ShouldUpdateThisTick(InterestManager::InterestLevel::CRITICAL, 1));

	TEST("MEDIUM updates every 2nd tick (tick 0)",
		manager.ShouldUpdateThisTick(InterestManager::InterestLevel::MEDIUM, 0));
	TEST("MEDIUM skips tick 1",
		!manager.ShouldUpdateThisTick(InterestManager::InterestLevel::MEDIUM, 1));
	TEST("MEDIUM updates tick 2",
		manager.ShouldUpdateThisTick(InterestManager::InterestLevel::MEDIUM, 2));

	TEST("LOW updates every 5th tick (tick 0)",
		manager.ShouldUpdateThisTick(InterestManager::InterestLevel::LOW, 0));
	TEST("LOW skips tick 1",
		!manager.ShouldUpdateThisTick(InterestManager::InterestLevel::LOW, 1));
	TEST("LOW updates tick 5",
		manager.ShouldUpdateThisTick(InterestManager::InterestLevel::LOW, 5));

	TEST("NONE never updates",
		!manager.ShouldUpdateThisTick(InterestManager::InterestLevel::NONE, 0));
}


// ============================================================================
// DeadReckoning Tests
// ============================================================================

void TestDeadReckoningBasics()
{
	cout << "\n=== DeadReckoning Basic Tests ===" << endl;

	DeadReckoning dr;

	// Set an initial state
	DeadReckoning::State initialState(Point(0, 0), Point(10, 5), Angle(45.0), 100);
	dr.SetAuthoritativeState(initialState);
	dr.SetCurrentTimestamp(100);

	// Get the state back
	const DeadReckoning::State &retrieved = dr.GetAuthoritativeState();
	TEST("DeadReckoning stores position", DoubleEqual(retrieved.position.X(), 0));
	TEST("DeadReckoning stores velocity", DoubleEqual(retrieved.velocity.X(), 10));
	TEST("DeadReckoning stores timestamp", retrieved.timestamp == 100);
}


void TestDeadReckoningPrediction()
{
	cout << "\n=== DeadReckoning Prediction Tests ===" << endl;

	DeadReckoning dr;

	// Start at position (0, 0) with velocity (10, 5) at tick 100
	DeadReckoning::State initialState(Point(0, 0), Point(10, 5), Angle(0), 100);
	dr.SetAuthoritativeState(initialState);

	// Predict 10 ticks ahead (should be at position (100, 50))
	DeadReckoning::State predicted = dr.PredictState(110);

	TEST("DeadReckoning predicts X position", DoubleEqual(predicted.position.X(), 100.0));
	TEST("DeadReckoning predicts Y position", DoubleEqual(predicted.position.Y(), 50.0));
	TEST("DeadReckoning preserves velocity", DoubleEqual(predicted.velocity.X(), 10.0));
	TEST("DeadReckoning updates timestamp", predicted.timestamp == 110);

	// Test predicting backwards (should return authoritative state)
	DeadReckoning::State past = dr.PredictState(90);
	TEST("DeadReckoning returns auth state for past prediction",
		DoubleEqual(past.position.X(), 0) && past.timestamp == 100);
}


void TestDeadReckoningError()
{
	cout << "\n=== DeadReckoning Error Detection Tests ===" << endl;

	DeadReckoning dr;
	DeadReckoning::State predictedState(Point(100, 50), Point(10, 5), Angle(0), 110);
	DeadReckoning::State actualState(Point(110, 55), Point(10, 5), Angle(0), 110);

	// Calculate error (distance between predicted and actual)
	double error = dr.GetPositionError(predictedState, actualState);

	// Error should be sqrt((110-100)^2 + (55-50)^2) = sqrt(100 + 25) = sqrt(125) ~= 11.18
	TEST("DeadReckoning calculates position error", DoubleEqual(error, 11.18, 0.1));

	// Test error threshold
	TEST("DeadReckoning detects excessive error (threshold 10)",
		dr.IsErrorExcessive(predictedState, actualState, 10.0));
	TEST("DeadReckoning accepts small error (threshold 20)",
		!dr.IsErrorExcessive(predictedState, actualState, 20.0));
}


void TestDeadReckoningReset()
{
	cout << "\n=== DeadReckoning Reset Tests ===" << endl;

	DeadReckoning dr;
	dr.SetAuthoritativeState(DeadReckoning::State(Point(100, 100), Point(10, 10), Angle(45), 100));
	dr.SetCurrentTimestamp(100);

	// Reset
	dr.Reset();

	const DeadReckoning::State &state = dr.GetAuthoritativeState();
	TEST("DeadReckoning reset clears position", DoubleEqual(state.position.X(), 0));
	TEST("DeadReckoning reset clears timestamp", state.timestamp == 0);
	TEST("DeadReckoning reset clears current timestamp", dr.GetCurrentTimestamp() == 0);
}


// ============================================================================
// StateSync Tests
// ============================================================================

void TestStateSyncBasics()
{
	cout << "\n=== StateSync Basic Tests ===" << endl;

	StateSync stateSync;
	InterestManager manager;
	stateSync.SetInterestManager(&manager);

	// Test current tick
	stateSync.SetCurrentTick(100);
	TEST("StateSync stores current tick", stateSync.GetCurrentTick() == 100);

	// Test ship tracking
	TEST("StateSync starts with no tracked ships", stateSync.GetTrackedShipCount() == 0);
}


void TestStateSyncCaptureShipState()
{
	cout << "\n=== StateSync Capture Ship State Tests ===" << endl;

	StateSync stateSync;
	stateSync.SetCurrentTick(100);

	// Create a test ship
	auto ship = CreateTestShip(EsUuid::Generate(), Point(100, 200), Point(5, 10), Angle(90));
	ship->SetShields(0.8);
	ship->SetHull(0.9);
	ship->SetEnergy(0.7);
	ship->SetFuel(0.6);

	// Capture full state
	StateSync::ShipUpdate update = stateSync.CaptureShipState(*ship, StateSync::UpdateScope::FULL);

	TEST("StateSync captures position", DoubleEqual(update.position.X(), 100));
	TEST("StateSync captures velocity", DoubleEqual(update.velocity.Y(), 10));
	TEST("StateSync captures shields", DoubleEqual(update.shields, 0.8));
	TEST("StateSync captures hull", DoubleEqual(update.hull, 0.9));
	TEST("StateSync captures energy", DoubleEqual(update.energy, 0.7));
	TEST("StateSync captures fuel", DoubleEqual(update.fuel, 0.6));
	TEST("StateSync sets timestamp", update.timestamp == 100);

	// Capture position-only state
	StateSync::ShipUpdate posUpdate = stateSync.CaptureShipState(*ship, StateSync::UpdateScope::POSITION);
	TEST("StateSync position scope includes position", DoubleEqual(posUpdate.position.X(), 100));
	TEST("StateSync position scope sets correct scope", posUpdate.scope == StateSync::UpdateScope::POSITION);
}


void TestStateSyncApplyUpdate()
{
	cout << "\n=== StateSync Apply Update Tests ===" << endl;

	StateSync stateSync;
	stateSync.SetCurrentTick(100);

	// Create a ship with initial state
	auto ship = CreateTestShip(EsUuid::Generate(), Point(0, 0), Point(0, 0), Angle(0));

	// Create an update
	StateSync::ShipUpdate update;
	update.shipUUID = ship->UUID();
	update.position = Point(100, 200);
	update.velocity = Point(5, 10);
	update.angle = Angle(45);
	update.shields = 0.8f;
	update.hull = 0.9f;
	update.energy = 0.7f;
	update.fuel = 0.6f;
	update.scope = StateSync::UpdateScope::FULL;
	update.timestamp = 100;

	// Apply the update
	stateSync.ApplyShipUpdate(*ship, update);

	// Verify ship state was updated
	TEST("StateSync apply updates position", DoubleEqual(ship->Position().X(), 100));
	TEST("StateSync apply updates velocity", DoubleEqual(ship->Velocity().Y(), 10));
	TEST("StateSync apply updates shields", DoubleEqual(ship->Shields(), 0.8));
	TEST("StateSync apply updates hull", DoubleEqual(ship->Hull(), 0.9));
	TEST("StateSync apply updates energy", DoubleEqual(ship->Energy(), 0.7));
	TEST("StateSync apply updates fuel", DoubleEqual(ship->Fuel(), 0.6));
}


void TestStateSyncUpdatePriority()
{
	cout << "\n=== StateSync Update Priority Tests ===" << endl;

	StateSync stateSync;
	InterestManager manager;
	stateSync.SetInterestManager(&manager);

	EsUuid playerUUID = EsUuid::Generate();
	manager.SetPlayerInterestCenter(playerUUID, Point(0, 0));

	// Create ships at different distances
	auto closeShip = CreateTestShip(EsUuid::Generate(), Point(500, 0), Point(0, 0), Angle());
	auto mediumShip = CreateTestShip(EsUuid::Generate(), Point(5000, 0), Point(0, 0), Angle());
	auto farShip = CreateTestShip(EsUuid::Generate(), Point(15000, 0), Point(0, 0), Angle());

	// Test priority levels
	StateSync::UpdatePriority closePriority = stateSync.GetUpdatePriority(playerUUID, *closeShip);
	StateSync::UpdatePriority mediumPriority = stateSync.GetUpdatePriority(playerUUID, *mediumShip);
	StateSync::UpdatePriority farPriority = stateSync.GetUpdatePriority(playerUUID, *farShip);

	TEST("StateSync close ship has CRITICAL priority", closePriority == StateSync::UpdatePriority::CRITICAL);
	TEST("StateSync medium ship has MEDIUM priority", mediumPriority == StateSync::UpdatePriority::MEDIUM);
	TEST("StateSync far ship has NONE priority", farPriority == StateSync::UpdatePriority::NONE);
}


void TestStateSyncUpdateScope()
{
	cout << "\n=== StateSync Update Scope Tests ===" << endl;

	StateSync stateSync;

	// Test scope determination based on priority
	StateSync::UpdateScope criticalScope = stateSync.DetermineUpdateScope(StateSync::UpdatePriority::CRITICAL);
	StateSync::UpdateScope highScope = stateSync.DetermineUpdateScope(StateSync::UpdatePriority::HIGH);
	StateSync::UpdateScope mediumScope = stateSync.DetermineUpdateScope(StateSync::UpdatePriority::MEDIUM);
	StateSync::UpdateScope lowScope = stateSync.DetermineUpdateScope(StateSync::UpdatePriority::LOW);

	TEST("StateSync CRITICAL priority gets FULL scope", criticalScope == StateSync::UpdateScope::FULL);
	TEST("StateSync HIGH priority gets FULL scope", highScope == StateSync::UpdateScope::FULL);
	TEST("StateSync MEDIUM priority gets POSITION scope", mediumScope == StateSync::UpdateScope::POSITION);
	TEST("StateSync LOW priority gets MINIMAL scope", lowScope == StateSync::UpdateScope::MINIMAL);
}


void TestStateSyncGetUpdatesForPlayer()
{
	cout << "\n=== StateSync Get Updates For Player Tests ===" << endl;

	StateSync stateSync;
	InterestManager manager;
	stateSync.SetInterestManager(&manager);
	stateSync.SetCurrentTick(100);

	EsUuid playerUUID = EsUuid::Generate();
	manager.SetPlayerInterestCenter(playerUUID, Point(0, 0));

	// Create test ships
	vector<shared_ptr<Ship>> allShips;
	allShips.push_back(CreateTestShip(EsUuid::Generate(), Point(500, 0), Point(0, 0), Angle()));     // Close
	allShips.push_back(CreateTestShip(EsUuid::Generate(), Point(2000, 0), Point(0, 0), Angle()));    // Medium
	allShips.push_back(CreateTestShip(EsUuid::Generate(), Point(15000, 0), Point(0, 0), Angle()));   // Far (out of range)

	// Get updates for this player
	vector<StateSync::ShipUpdate> updates = stateSync.GetUpdatesForPlayer(playerUUID, allShips);

	// Should get updates for 2 ships (close and medium, but not far)
	TEST("StateSync gets updates for interested ships", updates.size() == 2);
}


void TestStateSyncDeadReckoningIntegration()
{
	cout << "\n=== StateSync Dead Reckoning Integration Tests ===" << endl;

	StateSync stateSync;
	EsUuid shipUUID = EsUuid::Generate();

	// Update dead reckoning state
	DeadReckoning::State drState(Point(0, 0), Point(10, 5), Angle(0), 100);
	stateSync.UpdateDeadReckoning(shipUUID, drState);

	TEST("StateSync tracks ship for dead reckoning", stateSync.GetTrackedShipCount() == 1);

	// Predict future state
	DeadReckoning::State predicted = stateSync.PredictShipState(shipUUID, 110);
	TEST("StateSync predicts ship position via dead reckoning", DoubleEqual(predicted.position.X(), 100.0));

	// Clear dead reckoning
	stateSync.ClearDeadReckoning();
	TEST("StateSync clears dead reckoning data", stateSync.GetTrackedShipCount() == 0);
}


// ============================================================================
// Main
// ============================================================================

int main()
{
	cout << "======================================" << endl;
	cout << "Phase 3.2: Ship State Synchronization Tests" << endl;
	cout << "======================================" << endl;

	// InterestManager tests
	TestInterestManagerBasics();
	TestInterestManagerShipFiltering();
	TestInterestManagerUpdateFrequency();

	// DeadReckoning tests
	TestDeadReckoningBasics();
	TestDeadReckoningPrediction();
	TestDeadReckoningError();
	TestDeadReckoningReset();

	// StateSync tests
	TestStateSyncBasics();
	TestStateSyncCaptureShipState();
	TestStateSyncApplyUpdate();
	TestStateSyncUpdatePriority();
	TestStateSyncUpdateScope();
	TestStateSyncGetUpdatesForPlayer();
	TestStateSyncDeadReckoningIntegration();

	// Summary
	cout << "\n======================================" << endl;
	cout << "Tests: " << testsPassed << "/" << testsRun << " passed";
	if(testsPassed == testsRun)
		cout << " ✓" << endl;
	else
		cout << " ✗" << endl;
	cout << "======================================" << endl;

	return (testsPassed == testsRun) ? 0 : 1;
}
