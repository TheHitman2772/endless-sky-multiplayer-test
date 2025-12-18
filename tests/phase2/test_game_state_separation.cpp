/* test_game_state_separation.cpp
 * Copyright (c) 2025 by Endless Sky Development Team
 *
 * Phase 2.1 demonstration: Game State / Presentation Separation
 *
 * This test demonstrates the new architecture where:
 * - GameState = server-authoritative simulation (WHAT)
 * - ClientState = client-specific UI/camera (WHERE)
 * - Renderer = rendering logic (HOW)
 */

#include "../../source/GameState.h"
#include "../../source/ClientState.h"
#include "../../source/Renderer.h"
#include "../../source/Ship.h"
#include "../../source/System.h"
#include "../../source/Point.h"

#include <iostream>
#include <memory>

using namespace std;

// Test result tracking
int testsRun = 0;
int testsPassed = 0;

void ReportTest(const string &name, bool passed)
{
	testsRun++;
	if(passed)
	{
		testsPassed++;
		cout << "[PASS] " << name << endl;
	}
	else
	{
		cout << "[FAIL] " << name << endl;
	}
}


// Test 1: GameState creation and basic operations
bool TestGameStateBasics()
{
	GameState state;

	// Initially empty
	if(state.GetShipCount() != 0)
		return false;

	if(state.GetProjectileCount() != 0)
		return false;

	if(state.GetGameTick() != 0)
		return false;

	// Add a ship
	auto ship = make_shared<Ship>();
	state.AddShip(ship);

	if(state.GetShipCount() != 1)
		return false;

	// Increment tick
	state.IncrementGameTick();
	if(state.GetGameTick() != 1)
		return false;

	// Clear
	state.Clear();
	if(state.GetShipCount() != 0 || state.GetGameTick() != 0)
		return false;

	return true;
}


// Test 2: GameState copy constructor (for prediction/rollback)
bool TestGameStateCopy()
{
	GameState state1;

	// Add some ships
	state1.AddShip(make_shared<Ship>());
	state1.AddShip(make_shared<Ship>());
	state1.SetGameTick(42);

	// Copy
	GameState state2(state1);

	// Should have same data
	if(state2.GetShipCount() != 2)
		return false;

	if(state2.GetGameTick() != 42)
		return false;

	// Modifying state2 should not affect state1
	state2.AddShip(make_shared<Ship>());

	if(state1.GetShipCount() != 2)
		return false;

	if(state2.GetShipCount() != 3)
		return false;

	return true;
}


// Test 3: GameState copy assignment
bool TestGameStateAssignment()
{
	GameState state1;
	state1.AddShip(make_shared<Ship>());
	state1.SetGameTick(100);

	GameState state2;
	state2.AddShip(make_shared<Ship>());
	state2.AddShip(make_shared<Ship>());
	state2.SetGameTick(200);

	// Assign
	state2 = state1;

	// state2 should now match state1
	if(state2.GetShipCount() != 1)
		return false;

	if(state2.GetGameTick() != 100)
		return false;

	return true;
}


// Test 4: GameState move semantics
bool TestGameStateMove()
{
	GameState state1;
	state1.AddShip(make_shared<Ship>());
	state1.SetGameTick(50);

	// Move constructor
	GameState state2(move(state1));

	if(state2.GetShipCount() != 1)
		return false;

	if(state2.GetGameTick() != 50)
		return false;

	return true;
}


// Test 5: ClientState camera operations
bool TestClientStateCamera()
{
	ClientState client;

	// Initial state
	if(client.GetCameraZoom() != 1.0)
		return false;

	if(client.GetCameraCenter() != Point())
		return false;

	// Set camera
	client.SetCameraCenter(Point(100, 200));
	if(client.GetCameraCenter().X() != 100 || client.GetCameraCenter().Y() != 200)
		return false;

	// Move camera
	client.MoveCameraBy(Point(50, 50));
	if(client.GetCameraCenter().X() != 150 || client.GetCameraCenter().Y() != 250)
		return false;

	// Zoom
	client.SetCameraZoom(2.0);
	if(client.GetCameraZoom() != 2.0)
		return false;

	// Zoom clamping (should clamp to [0.25, 4.0])
	client.SetCameraZoom(10.0);
	if(client.GetCameraZoom() != 4.0)
		return false;

	client.SetCameraZoom(0.1);
	if(client.GetCameraZoom() != 0.25)
		return false;

	return true;
}


// Test 6: ClientState UI flags
bool TestClientStateUI()
{
	ClientState client;

	// Default state
	if(!client.IsRadarOn())
		return false;

	if(!client.ShowLabels())
		return false;

	// Toggle flags
	client.SetRadarOn(false);
	if(client.IsRadarOn())
		return false;

	client.SetShowLabels(false);
	if(client.ShowLabels())
		return false;

	return true;
}


// Test 7: ClientState prediction state
bool TestClientStatePrediction()
{
	ClientState client;

	// Default enabled
	if(!client.IsPredictionEnabled())
		return false;

	// Set tick
	client.SetLastConfirmedTick(42);
	if(client.GetLastConfirmedTick() != 42)
		return false;

	// Disable prediction
	client.SetPredictionEnabled(false);
	if(client.IsPredictionEnabled())
		return false;

	return true;
}


// Test 8: ClientState interpolation
bool TestClientStateInterpolation()
{
	ClientState client;

	// Initial state
	if(client.GetInterpolationAlpha() != 0.0)
		return false;

	// Set alpha
	client.SetInterpolationAlpha(0.5);
	if(client.GetInterpolationAlpha() != 0.5)
		return false;

	// Clamping to [0, 1]
	client.SetInterpolationAlpha(1.5);
	if(client.GetInterpolationAlpha() != 1.0)
		return false;

	client.SetInterpolationAlpha(-0.5);
	if(client.GetInterpolationAlpha() != 0.0)
		return false;

	return true;
}


// Test 9: Renderer creation
bool TestRendererCreation()
{
	Renderer renderer;

	// Should be created successfully
	// FPS should default to 60
	if(renderer.GetFPS() != 60.0)
		return false;

	return true;
}


// Test 10: Renderer debug mode
bool TestRendererDebug()
{
	Renderer renderer;

	// Initially off
	if(renderer.IsShowDebug())
		return false;

	// Toggle on
	renderer.SetShowDebug(true);
	if(!renderer.IsShowDebug())
		return false;

	return true;
}


// Test 11: Full architecture integration
bool TestFullArchitectureIntegration()
{
	// Create all three components
	GameState gameState;
	ClientState clientState;
	Renderer renderer;

	// Setup game state
	auto ship1 = make_shared<Ship>();
	auto ship2 = make_shared<Ship>();
	gameState.AddShip(ship1);
	gameState.AddShip(ship2);
	gameState.SetGameTick(100);

	// Setup client state
	clientState.SetPlayerShip(ship1);
	clientState.SetCameraCenter(Point(500, 500));
	clientState.SetCameraZoom(1.5);

	// Render (should not crash)
	renderer.Render(gameState, clientState);

	// Verify states are independent
	if(gameState.GetShipCount() != 2)
		return false;

	if(clientState.GetCameraZoom() != 1.5)
		return false;

	if(renderer.GetFPS() != 60.0)
		return false;

	return true;
}


// Test 12: Separation of concerns demonstration
bool TestSeparationOfConcerns()
{
	GameState gameState;
	ClientState clientState1;
	ClientState clientState2;

	// One game state, multiple client states
	// (demonstrating server with multiple clients)

	auto ship = make_shared<Ship>();
	gameState.AddShip(ship);

	// Client 1 views from position A
	clientState1.SetCameraCenter(Point(100, 100));
	clientState1.SetCameraZoom(1.0);

	// Client 2 views from position B
	clientState2.SetCameraCenter(Point(500, 500));
	clientState2.SetCameraZoom(2.0);

	// Both can render the same game state
	Renderer renderer;
	renderer.Render(gameState, clientState1);
	renderer.Render(gameState, clientState2);

	// Game state unchanged
	if(gameState.GetShipCount() != 1)
		return false;

	// Client states are independent
	if(clientState1.GetCameraCenter() == clientState2.GetCameraCenter())
		return false;

	return true;
}


int main()
{
	cout << "=== Phase 2.1: Game State / Presentation Separation Tests ===" << endl;
	cout << endl;

	// GameState tests
	ReportTest("Test 1: GameState Basics", TestGameStateBasics());
	ReportTest("Test 2: GameState Copy Constructor", TestGameStateCopy());
	ReportTest("Test 3: GameState Copy Assignment", TestGameStateAssignment());
	ReportTest("Test 4: GameState Move Semantics", TestGameStateMove());

	// ClientState tests
	ReportTest("Test 5: ClientState Camera", TestClientStateCamera());
	ReportTest("Test 6: ClientState UI Flags", TestClientStateUI());
	ReportTest("Test 7: ClientState Prediction", TestClientStatePrediction());
	ReportTest("Test 8: ClientState Interpolation", TestClientStateInterpolation());

	// Renderer tests
	ReportTest("Test 9: Renderer Creation", TestRendererCreation());
	ReportTest("Test 10: Renderer Debug Mode", TestRendererDebug());

	// Integration tests
	ReportTest("Test 11: Full Architecture Integration", TestFullArchitectureIntegration());
	ReportTest("Test 12: Separation of Concerns", TestSeparationOfConcerns());

	cout << endl;
	cout << "=== Test Results ===" << endl;
	cout << "Tests Run: " << testsRun << endl;
	cout << "Tests Passed: " << testsPassed << endl;
	cout << "Tests Failed: " << (testsRun - testsPassed) << endl;

	if(testsPassed == testsRun)
	{
		cout << endl;
		cout << "=== Architecture Demonstration ===" << endl;
		cout << "✓ GameState: Server-authoritative simulation (copyable for prediction)" << endl;
		cout << "✓ ClientState: Client-specific camera and UI state" << endl;
		cout << "✓ Renderer: Decoupled rendering system" << endl;
		cout << "✓ Multiple clients can view the same GameState independently" << endl;
	}

	return (testsPassed == testsRun) ? 0 : 1;
}
