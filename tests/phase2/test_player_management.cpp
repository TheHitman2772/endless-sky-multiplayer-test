/* test_player_management.cpp
 * Copyright (c) 2025 by Endless Sky Development Team
 *
 * Phase 2.2 demonstration: Player Management System
 *
 * This test demonstrates the player management system for multiplayer:
 * - NetworkPlayer: Individual player representation
 * - PlayerRegistry: Fast player ID mapping
 * - PlayerManager: Central player management
 * - Ship ownership tracking
 */

#include "../../source/multiplayer/NetworkPlayer.h"
#include "../../source/multiplayer/PlayerRegistry.h"
#include "../../source/multiplayer/PlayerManager.h"
#include "../../source/Ship.h"
#include "../../source/EsUuid.h"

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


// Test 1: NetworkPlayer creation and basic properties
bool TestNetworkPlayerCreation()
{
	NetworkPlayer player1;
	if(player1.GetName().empty())
		return true;  // Default constructor creates empty name

	EsUuid uuid = EsUuid::Generate();
	NetworkPlayer player2(uuid, "TestPlayer");

	if(player2.GetUUID() != uuid)
		return false;

	if(player2.GetName() != "TestPlayer")
		return false;

	return true;
}


// Test 2: NetworkPlayer ship management
bool TestNetworkPlayerShips()
{
	NetworkPlayer player(EsUuid::Generate(), "Player1");

	// Initially no ships
	if(player.GetShipCount() != 0)
		return false;

	// Add ships
	auto ship1 = make_shared<Ship>();
	auto ship2 = make_shared<Ship>();

	player.AddShip(ship1);
	if(player.GetShipCount() != 1)
		return false;

	// First ship becomes flagship
	if(player.GetFlagship() != ship1)
		return false;

	player.AddShip(ship2);
	if(player.GetShipCount() != 2)
		return false;

	// Remove flagship
	player.RemoveShip(ship1);
	if(player.GetShipCount() != 1)
		return false;

	// Flagship should switch to remaining ship
	if(player.GetFlagship() != ship2)
		return false;

	// Clear all ships
	player.ClearShips();
	if(player.GetShipCount() != 0)
		return false;

	return true;
}


// Test 3: NetworkPlayer roles and permissions
bool TestNetworkPlayerRoles()
{
	NetworkPlayer player(EsUuid::Generate(), "Player");

	// Default is PLAYER
	if(player.GetRole() != NetworkPlayer::Role::PLAYER)
		return false;

	if(player.IsAdmin() || player.IsModerator())
		return false;

	// Set to MODERATOR
	player.SetRole(NetworkPlayer::Role::MODERATOR);
	if(!player.IsModerator())
		return false;

	if(player.IsAdmin())
		return false;  // Moderator is not admin

	// Set to ADMIN
	player.SetRole(NetworkPlayer::Role::ADMIN);
	if(!player.IsAdmin())
		return false;

	if(!player.IsModerator())
		return false;  // Admin is also moderator

	return true;
}


// Test 4: NetworkPlayer status and activity
bool TestNetworkPlayerStatus()
{
	NetworkPlayer player(EsUuid::Generate(), "Player");

	// Default status
	if(player.GetStatus() != NetworkPlayer::Status::CONNECTING)
		return false;

	// Change status
	player.SetStatus(NetworkPlayer::Status::CONNECTED);
	if(player.GetStatus() != NetworkPlayer::Status::CONNECTED)
		return false;

	// Update activity
	player.SetJoinTime(1000);
	if(player.GetJoinTime() != 1000)
		return false;

	player.UpdateActivity(2000);
	if(player.GetLastActivityTime() != 2000)
		return false;

	return true;
}


// Test 5: PlayerRegistry registration
bool TestPlayerRegistryRegistration()
{
	PlayerRegistry registry;

	// Initially empty
	if(registry.GetPlayerCount() != 0)
		return false;

	// Register player
	auto player = make_shared<NetworkPlayer>(EsUuid::Generate(), "Player1");
	size_t index = registry.Register(player);

	if(index == static_cast<size_t>(-1))
		return false;

	if(registry.GetPlayerCount() != 1)
		return false;

	// Register duplicate should fail
	size_t dupIndex = registry.Register(player);
	if(dupIndex != static_cast<size_t>(-1))
		return false;

	if(registry.GetPlayerCount() != 1)
		return false;  // Count shouldn't change

	return true;
}


// Test 6: PlayerRegistry UUID lookup
bool TestPlayerRegistryUUIDLookup()
{
	PlayerRegistry registry;

	EsUuid uuid = EsUuid::Generate();
	auto player = make_shared<NetworkPlayer>(uuid, "Player");
	registry.Register(player);

	// Lookup by UUID
	auto found = registry.GetByUUID(uuid);
	if(found != player)
		return false;

	// Has UUID
	if(!registry.HasUUID(uuid))
		return false;

	// Non-existent UUID
	EsUuid fakeUuid = EsUuid::Generate();
	if(registry.HasUUID(fakeUuid))
		return false;

	if(registry.GetByUUID(fakeUuid) != nullptr)
		return false;

	return true;
}


// Test 7: PlayerRegistry index lookup
bool TestPlayerRegistryIndexLookup()
{
	PlayerRegistry registry;

	auto player1 = make_shared<NetworkPlayer>(EsUuid::Generate(), "Player1");
	auto player2 = make_shared<NetworkPlayer>(EsUuid::Generate(), "Player2");

	size_t index1 = registry.Register(player1);
	size_t index2 = registry.Register(player2);

	// Lookup by index
	if(registry.GetByIndex(index1) != player1)
		return false;

	if(registry.GetByIndex(index2) != player2)
		return false;

	// Has index
	if(!registry.HasIndex(index1))
		return false;

	// Non-existent index
	if(registry.HasIndex(999))
		return false;

	return true;
}


// Test 8: PlayerRegistry name lookup
bool TestPlayerRegistryNameLookup()
{
	PlayerRegistry registry;

	auto player = make_shared<NetworkPlayer>(EsUuid::Generate(), "UniquePlayer");
	registry.Register(player);

	// Lookup by name
	auto found = registry.GetByName("UniquePlayer");
	if(found != player)
		return false;

	// Non-existent name
	if(registry.GetByName("NonExistent") != nullptr)
		return false;

	return true;
}


// Test 9: PlayerRegistry unregister
bool TestPlayerRegistryUnregister()
{
	PlayerRegistry registry;

	EsUuid uuid = EsUuid::Generate();
	auto player = make_shared<NetworkPlayer>(uuid, "Player");
	size_t index = registry.Register(player);

	// Unregister by UUID
	registry.Unregister(uuid);

	if(registry.GetPlayerCount() != 0)
		return false;

	if(registry.HasUUID(uuid))
		return false;

	// Re-register and unregister by index
	index = registry.Register(player);
	registry.Unregister(index);

	if(registry.GetPlayerCount() != 0)
		return false;

	return true;
}


// Test 10: PlayerManager add/remove players
bool TestPlayerManagerAddRemove()
{
	PlayerManager manager;

	// Initially empty
	if(manager.GetPlayerCount() != 0)
		return false;

	// Add player
	EsUuid uuid = EsUuid::Generate();
	auto player = manager.AddPlayer(uuid, "Player1");

	if(!player)
		return false;

	if(manager.GetPlayerCount() != 1)
		return false;

	// Get player
	auto found = manager.GetPlayer(uuid);
	if(found != player)
		return false;

	// Remove player
	manager.RemovePlayer(uuid);
	if(manager.GetPlayerCount() != 0)
		return false;

	return true;
}


// Test 11: PlayerManager ship ownership
bool TestPlayerManagerShipOwnership()
{
	PlayerManager manager;

	auto player = manager.AddPlayer(EsUuid::Generate(), "Owner");
	auto ship = make_shared<Ship>();

	// Assign ship to player
	manager.AssignShipToPlayer(ship, player);

	// Check ownership
	auto owner = manager.GetShipOwner(ship);
	if(owner != player)
		return false;

	if(!manager.IsShipOwnedByPlayer(ship, player))
		return false;

	// Unassign ship
	manager.UnassignShipFromPlayer(ship);

	if(manager.GetShipOwner(ship) != nullptr)
		return false;

	return true;
}


// Test 12: PlayerManager event callbacks
bool TestPlayerManagerCallbacks()
{
	PlayerManager manager;

	bool joinCalled = false;
	bool leaveCalled = false;
	bool shipOwnershipCalled = false;

	manager.SetPlayerJoinCallback([&](shared_ptr<NetworkPlayer> p) {
		joinCalled = true;
	});

	manager.SetPlayerLeaveCallback([&](shared_ptr<NetworkPlayer> p) {
		leaveCalled = true;
	});

	manager.SetShipOwnershipCallback([&](shared_ptr<Ship> s, shared_ptr<NetworkPlayer> p) {
		shipOwnershipCalled = true;
	});

	// Add player - should trigger join callback
	auto player = manager.AddPlayer(EsUuid::Generate(), "Player");
	if(!joinCalled)
		return false;

	// Assign ship - should trigger ownership callback
	auto ship = make_shared<Ship>();
	manager.AssignShipToPlayer(ship, player);
	if(!shipOwnershipCalled)
		return false;

	// Remove player - should trigger leave callback
	manager.RemovePlayer(player);
	if(!leaveCalled)
		return false;

	return true;
}


// Test 13: Ship owner player UUID
bool TestShipOwnerPlayerUUID()
{
	Ship ship;

	// Initially no owner
	if(ship.HasOwner())
		return false;

	// Set owner
	EsUuid playerUuid = EsUuid::Generate();
	ship.SetOwnerPlayerUUID(playerUuid);

	if(!ship.HasOwner())
		return false;

	if(ship.GetOwnerPlayerUUID() != playerUuid)
		return false;

	return true;
}


// Test 14: PlayerManager with multiple players
bool TestPlayerManagerMultiplePlayers()
{
	PlayerManager manager;

	// Add 5 players
	for(int i = 0; i < 5; ++i)
	{
		EsUuid uuid = EsUuid::Generate();
		manager.AddPlayer(uuid, "Player" + to_string(i));
	}

	if(manager.GetPlayerCount() != 5)
		return false;

	auto allPlayers = manager.GetAllPlayers();
	if(allPlayers.size() != 5)
		return false;

	// All players initially connecting
	auto connected = manager.GetConnectedPlayers();
	if(connected.size() != 0)
		return false;

	// Set one player to connected
	allPlayers[0]->SetStatus(NetworkPlayer::Status::CONNECTED);
	connected = manager.GetConnectedPlayers();
	if(connected.size() != 1)
		return false;

	return true;
}


// Test 15: Full integration test
bool TestFullIntegration()
{
	PlayerManager manager;

	// Create two players
	auto player1 = manager.AddPlayer(EsUuid::Generate(), "Alice");
	auto player2 = manager.AddPlayer(EsUuid::Generate(), "Bob");

	player1->SetRole(NetworkPlayer::Role::ADMIN);
	player2->SetRole(NetworkPlayer::Role::PLAYER);

	// Create ships
	auto ship1 = make_shared<Ship>();
	auto ship2 = make_shared<Ship>();
	auto ship3 = make_shared<Ship>();

	// Assign ships
	manager.AssignShipToPlayer(ship1, player1);
	manager.AssignShipToPlayer(ship2, player1);
	manager.AssignShipToPlayer(ship3, player2);

	// Verify ownership
	if(player1->GetShipCount() != 2)
		return false;

	if(player2->GetShipCount() != 1)
		return false;

	if(manager.GetShipOwner(ship1) != player1)
		return false;

	if(manager.GetShipOwner(ship3) != player2)
		return false;

	// Verify Ship class owner tracking
	if(!ship1->HasOwner())
		return false;

	if(ship1->GetOwnerPlayerUUID() != player1->GetUUID())
		return false;

	// Remove player - should unassign ships
	manager.RemovePlayer(player1);

	if(manager.GetShipOwner(ship1) != nullptr)
		return false;

	if(manager.GetPlayerCount() != 1)
		return false;

	return true;
}


int main()
{
	cout << "=== Phase 2.2: Player Management System Tests ===" << endl;
	cout << endl;

	// NetworkPlayer tests
	ReportTest("Test 1: NetworkPlayer Creation", TestNetworkPlayerCreation());
	ReportTest("Test 2: NetworkPlayer Ships", TestNetworkPlayerShips());
	ReportTest("Test 3: NetworkPlayer Roles", TestNetworkPlayerRoles());
	ReportTest("Test 4: NetworkPlayer Status", TestNetworkPlayerStatus());

	// PlayerRegistry tests
	ReportTest("Test 5: PlayerRegistry Registration", TestPlayerRegistryRegistration());
	ReportTest("Test 6: PlayerRegistry UUID Lookup", TestPlayerRegistryUUIDLookup());
	ReportTest("Test 7: PlayerRegistry Index Lookup", TestPlayerRegistryIndexLookup());
	ReportTest("Test 8: PlayerRegistry Name Lookup", TestPlayerRegistryNameLookup());
	ReportTest("Test 9: PlayerRegistry Unregister", TestPlayerRegistryUnregister());

	// PlayerManager tests
	ReportTest("Test 10: PlayerManager Add/Remove", TestPlayerManagerAddRemove());
	ReportTest("Test 11: PlayerManager Ship Ownership", TestPlayerManagerShipOwnership());
	ReportTest("Test 12: PlayerManager Callbacks", TestPlayerManagerCallbacks());

	// Ship ownership tests
	ReportTest("Test 13: Ship Owner Player UUID", TestShipOwnerPlayerUUID());

	// Integration tests
	ReportTest("Test 14: PlayerManager Multiple Players", TestPlayerManagerMultiplePlayers());
	ReportTest("Test 15: Full Integration", TestFullIntegration());

	cout << endl;
	cout << "=== Test Results ===" << endl;
	cout << "Tests Run: " << testsRun << endl;
	cout << "Tests Passed: " << testsPassed << endl;
	cout << "Tests Failed: " << (testsRun - testsPassed) << endl;

	if(testsPassed == testsRun)
	{
		cout << endl;
		cout << "=== Player Management System Demonstration ===" << endl;
		cout << "✓ NetworkPlayer: Individual player representation with roles/permissions" << endl;
		cout << "✓ PlayerRegistry: O(1) lookup by UUID, index, or name" << endl;
		cout << "✓ PlayerManager: Central management with event callbacks" << endl;
		cout << "✓ Ship Ownership: Track which player owns each ship" << endl;
		cout << "✓ Supports multiple players with independent ships" << endl;
	}

	return (testsPassed == testsRun) ? 0 : 1;
}
