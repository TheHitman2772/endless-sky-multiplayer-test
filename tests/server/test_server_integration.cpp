/* test_server_integration.cpp
 * Copyright (c) 2025 by Endless Sky Development Team
 *
 * Phase 2.4 integration tests: Server implementation
 *
 * This test suite validates the dedicated server components:
 * - ServerConfig: Configuration loading and validation
 * - SnapshotManager: State snapshot management
 * - ServerLoop: Game timing and loop logic
 * - Server: Integration of all components
 */

#include "../../source/server/ServerConfig.h"
#include "../../source/server/SnapshotManager.h"
#include "../../source/server/ServerLoop.h"
#include "../../source/GameState.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

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


// Test 1: ServerConfig default values
bool TestServerConfigDefaults()
{
	ServerConfig config;

	if(config.GetPort() == 0)
		return false;

	if(config.GetMaxPlayers() == 0)
		return false;

	if(config.GetSimulationHz() != 60)
		return false;

	if(config.GetBroadcastHz() != 20)
		return false;

	if(!config.IsValid())
		return false;

	return true;
}


// Test 2: ServerConfig set and get
bool TestServerConfigSetGet()
{
	ServerConfig config;

	config.SetPort(12345);
	if(config.GetPort() != 12345)
		return false;

	config.SetMaxPlayers(64);
	if(config.GetMaxPlayers() != 64)
		return false;

	config.SetServerName("Test Server");
	if(config.GetServerName() != "Test Server")
		return false;

	config.SetPvPEnabled(true);
	if(!config.IsPvPEnabled())
		return false;

	return true;
}


// Test 3: ServerConfig file save and load
bool TestServerConfigFile()
{
	ServerConfig config1;
	config1.SetPort(55555);
	config1.SetServerName("Saved Server");
	config1.SetMaxPlayers(100);

	// Save to file
	string filename = "test_config.txt";
	if(!config1.SaveToFile(filename))
		return false;

	// Load from file
	ServerConfig config2;
	if(!config2.LoadFromFile(filename))
		return false;

	// Verify loaded values
	if(config2.GetPort() != 55555)
		return false;

	if(config2.GetServerName() != "Saved Server")
		return false;

	if(config2.GetMaxPlayers() != 100)
		return false;

	// Cleanup
	remove(filename.c_str());

	return true;
}


// Test 4: ServerConfig validation
bool TestServerConfigValidation()
{
	ServerConfig config;

	// Valid configuration
	if(!config.IsValid())
		return false;

	// Invalid port
	config.SetPort(0);
	if(config.IsValid())
		return false;

	config.SetPort(31337);  // Reset

	// Invalid simulation Hz
	config.SetSimulationHz(5);  // Too low
	if(config.IsValid())
		return false;

	config.SetSimulationHz(60);  // Reset

	// Invalid broadcast Hz (higher than simulation)
	config.SetBroadcastHz(100);
	if(config.IsValid())
		return false;

	config.SetBroadcastHz(20);  // Reset

	// Should be valid again
	if(!config.IsValid())
		return false;

	return true;
}


// Test 5: SnapshotManager basic functionality
bool TestSnapshotManagerBasic()
{
	SnapshotManager manager(10);  // Keep 10 snapshots

	GameState state1;
	state1.SetGameTick(100);

	// Create snapshot
	manager.CreateSnapshot(state1, 100);

	if(manager.GetSnapshotCount() != 1)
		return false;

	// Get latest snapshot
	const Snapshot *snap = manager.GetLatestSnapshot();
	if(!snap)
		return false;

	if(snap->gameTick != 100)
		return false;

	return true;
}


// Test 6: SnapshotManager history limit
bool TestSnapshotManagerHistory()
{
	SnapshotManager manager(5);  // Keep only 5 snapshots

	// Create 10 snapshots
	for(uint64_t i = 0; i < 10; ++i)
	{
		GameState state;
		state.SetGameTick(i);
		manager.CreateSnapshot(state, i);
	}

	// Should only have 5 (most recent)
	if(manager.GetSnapshotCount() != 5)
		return false;

	// Latest should be tick 9
	const Snapshot *latest = manager.GetLatestSnapshot();
	if(!latest || latest->gameTick != 9)
		return false;

	// Should not find old snapshot (tick 0)
	const Snapshot *old = manager.GetSnapshotAtTick(0);
	if(old != nullptr)
		return false;

	return true;
}


// Test 7: SnapshotManager keyframes
bool TestSnapshotManagerKeyframes()
{
	SnapshotManager manager(100);
	manager.SetKeyframeInterval(5);  // Every 5th snapshot is keyframe

	// Create 10 snapshots
	for(uint64_t i = 0; i < 10; ++i)
	{
		GameState state;
		state.SetGameTick(i);
		manager.CreateSnapshot(state, i);
	}

	// Should have 2 keyframes (at 0 and 5)
	if(manager.GetTotalKeyframes() != 2)
		return false;

	// Total snapshots should be 10
	if(manager.GetTotalSnapshots() != 10)
		return false;

	return true;
}


// Test 8: ServerLoop timing configuration
bool TestServerLoopTiming()
{
	ServerLoop loop(60, 20);  // 60 Hz sim, 20 Hz broadcast

	if(loop.GetTargetSimulationHz() != 60)
		return false;

	if(loop.GetTargetBroadcastHz() != 20)
		return false;

	// Change rates
	loop.SetSimulationHz(120);
	loop.SetBroadcastHz(30);

	if(loop.GetTargetSimulationHz() != 120)
		return false;

	if(loop.GetTargetBroadcastHz() != 30)
		return false;

	return true;
}


// Test 9: ServerLoop callbacks
bool TestServerLoopCallbacks()
{
	ServerLoop loop(60, 20);

	int simulationCount = 0;
	int broadcastCount = 0;

	loop.SetSimulationCallback([&](uint64_t tick) {
		++simulationCount;
	});

	loop.SetBroadcastCallback([&](uint64_t tick) {
		++broadcastCount;
	});

	// Run loop in separate thread
	thread loopThread([&loop]() {
		loop.Run();
	});

	// Let it run for a short time
	this_thread::sleep_for(chrono::milliseconds(100));

	// Stop the loop
	loop.Stop();
	loopThread.join();

	// Should have called simulation callback multiple times
	if(simulationCount == 0)
		return false;

	// Broadcast should be called less frequently than simulation
	if(broadcastCount == 0)
		return false;

	if(broadcastCount >= simulationCount)
		return false;  // Broadcast should be less frequent

	return true;
}


// Test 10: ServerLoop game tick increment
bool TestServerLoopGameTick()
{
	ServerLoop loop(100, 20);  // Fast rate for testing

	uint64_t lastTick = 0;

	loop.SetSimulationCallback([&](uint64_t tick) {
		lastTick = tick;
	});

	// Run loop
	thread loopThread([&loop]() {
		loop.Run();
	});

	// Let it run briefly
	this_thread::sleep_for(chrono::milliseconds(50));

	loop.Stop();
	loopThread.join();

	// Game tick should have incremented
	if(loop.GetGameTick() == 0)
		return false;

	if(lastTick != loop.GetGameTick())
		return false;

	return true;
}


int main()
{
	cout << "==================================" << endl;
	cout << "Phase 2.4: Server Integration Tests" << endl;
	cout << "==================================" << endl;
	cout << endl;

	// ServerConfig tests
	cout << "ServerConfig Tests:" << endl;
	ReportTest("ServerConfig defaults", TestServerConfigDefaults());
	ReportTest("ServerConfig set/get", TestServerConfigSetGet());
	ReportTest("ServerConfig file I/O", TestServerConfigFile());
	ReportTest("ServerConfig validation", TestServerConfigValidation());
	cout << endl;

	// SnapshotManager tests
	cout << "SnapshotManager Tests:" << endl;
	ReportTest("SnapshotManager basic", TestSnapshotManagerBasic());
	ReportTest("SnapshotManager history limit", TestSnapshotManagerHistory());
	ReportTest("SnapshotManager keyframes", TestSnapshotManagerKeyframes());
	cout << endl;

	// ServerLoop tests
	cout << "ServerLoop Tests:" << endl;
	ReportTest("ServerLoop timing", TestServerLoopTiming());
	ReportTest("ServerLoop callbacks", TestServerLoopCallbacks());
	ReportTest("ServerLoop game tick", TestServerLoopGameTick());
	cout << endl;

	// Summary
	cout << "==================================" << endl;
	cout << "Tests: " << testsPassed << "/" << testsRun << " passed";
	if(testsPassed == testsRun)
		cout << " ✓" << endl;
	else
		cout << " ✗" << endl;
	cout << "==================================" << endl;

	return (testsPassed == testsRun) ? 0 : 1;
}
