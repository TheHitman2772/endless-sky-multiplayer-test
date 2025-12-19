/* test_engine_multiplayer.cpp
 * Copyright (c) 2025 by Endless Sky Development Team
 *
 * Phase 3.1 integration test: Engine multiplayer mode support
 *
 * This test suite validates the Engine's multiplayer mode functionality:
 * - Mode enumeration and state management
 * - SetMode, GetMode, IsMultiplayer methods
 * - MultiplayerClient and GameState integration points
 */

#include "../../source/Engine.h"
#include "../../source/PlayerInfo.h"
#include "../../source/GameState.h"
#include "../../source/client/MultiplayerClient.h"

#include <iostream>

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


// Test 1: Engine defaults to SINGLEPLAYER mode
bool TestEngineDefaultMode()
{
	PlayerInfo player;
	Engine engine(player);

	if(engine.GetMode() != Engine::Mode::SINGLEPLAYER)
		return false;

	if(engine.IsMultiplayer())
		return false;

	return true;
}


// Test 2: Engine can be set to MULTIPLAYER_CLIENT mode
bool TestEngineSetMultiplayerMode()
{
	PlayerInfo player;
	Engine engine(player);

	engine.SetMode(Engine::Mode::MULTIPLAYER_CLIENT);

	if(engine.GetMode() != Engine::Mode::MULTIPLAYER_CLIENT)
		return false;

	if(!engine.IsMultiplayer())
		return false;

	return true;
}


// Test 3: Engine can be set to HEADLESS mode
bool TestEngineSetHeadlessMode()
{
	PlayerInfo player;
	Engine engine(player);

	engine.SetMode(Engine::Mode::HEADLESS);

	if(engine.GetMode() != Engine::Mode::HEADLESS)
		return false;

	// HEADLESS is not multiplayer client mode
	if(engine.IsMultiplayer())
		return false;

	return true;
}


// Test 4: Engine can switch modes
bool TestEngineSwitchModes()
{
	PlayerInfo player;
	Engine engine(player);

	// Start in SINGLEPLAYER
	if(engine.GetMode() != Engine::Mode::SINGLEPLAYER)
		return false;

	// Switch to MULTIPLAYER_CLIENT
	engine.SetMode(Engine::Mode::MULTIPLAYER_CLIENT);
	if(engine.GetMode() != Engine::Mode::MULTIPLAYER_CLIENT)
		return false;

	// Switch to HEADLESS
	engine.SetMode(Engine::Mode::HEADLESS);
	if(engine.GetMode() != Engine::Mode::HEADLESS)
		return false;

	// Switch back to SINGLEPLAYER
	engine.SetMode(Engine::Mode::SINGLEPLAYER);
	if(engine.GetMode() != Engine::Mode::SINGLEPLAYER)
		return false;

	return true;
}


// Test 5: Engine can accept GameState pointer
bool TestEngineSetGameState()
{
	PlayerInfo player;
	Engine engine(player);

	GameState state;
	engine.SetMultiplayerState(&state);

	// No crash = success
	// (We can't verify the pointer was stored without exposing private members)
	return true;
}


// Test 6: Engine can accept MultiplayerClient pointer
bool TestEngineSetMultiplayerClient()
{
	PlayerInfo player;
	Engine engine(player);

	MultiplayerClient client;
	engine.SetMultiplayerClient(&client);

	// No crash = success
	// (We can't verify the pointer was stored without exposing private members)
	return true;
}


// Test 7: IsMultiplayer returns correct value for each mode
bool TestIsMultiplayerForAllModes()
{
	PlayerInfo player;
	Engine engine(player);

	// SINGLEPLAYER
	engine.SetMode(Engine::Mode::SINGLEPLAYER);
	if(engine.IsMultiplayer())
		return false;

	// MULTIPLAYER_CLIENT
	engine.SetMode(Engine::Mode::MULTIPLAYER_CLIENT);
	if(!engine.IsMultiplayer())
		return false;

	// HEADLESS
	engine.SetMode(Engine::Mode::HEADLESS);
	if(engine.IsMultiplayer())
		return false;

	return true;
}


int main()
{
	cout << "===================================" << endl;
	cout << "Phase 3.1: Engine Multiplayer Tests" << endl;
	cout << "===================================" << endl;
	cout << endl;

	// Engine mode tests
	cout << "Engine Mode Tests:" << endl;
	ReportTest("Engine default mode", TestEngineDefaultMode());
	ReportTest("Engine set multiplayer mode", TestEngineSetMultiplayerMode());
	ReportTest("Engine set headless mode", TestEngineSetHeadlessMode());
	ReportTest("Engine switch modes", TestEngineSwitchModes());
	cout << endl;

	// Engine multiplayer integration tests
	cout << "Engine Multiplayer Integration Tests:" << endl;
	ReportTest("Engine set GameState", TestEngineSetGameState());
	ReportTest("Engine set MultiplayerClient", TestEngineSetMultiplayerClient());
	ReportTest("IsMultiplayer for all modes", TestIsMultiplayerForAllModes());
	cout << endl;

	// Summary
	cout << "===================================" << endl;
	cout << "Tests: " << testsPassed << "/" << testsRun << " passed";
	if(testsPassed == testsRun)
		cout << " ✓" << endl;
	else
		cout << " ✗" << endl;
	cout << "===================================" << endl;

	return (testsPassed == testsRun) ? 0 : 1;
}
