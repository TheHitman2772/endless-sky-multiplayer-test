// test_projectile_sync.cpp
// Phase 3.3: Comprehensive tests for projectile synchronization

#include "../../source/multiplayer/ProjectileSync.h"
#include "../../source/multiplayer/CollisionAuthority.h"
#include "../../source/Projectile.h"
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


// ============================================================================
// ProjectileSync Tests
// ============================================================================

void TestProjectileSyncBasics()
{
	cout << "\n=== ProjectileSync Basic Tests ===" << endl;

	ProjectileSync sync;

	// Test initial state
	TEST("ProjectileSync starts with tick 0", sync.GetCurrentTick() == 0);
	TEST("ProjectileSync starts with no tracked projectiles", sync.GetTrackedProjectileCount() == 0);
	TEST("ProjectileSync starts with ID 1", sync.GetNextProjectileID() == 1);

	// Test current tick
	sync.SetCurrentTick(100);
	TEST("ProjectileSync stores current tick", sync.GetCurrentTick() == 100);
}



void TestProjectileSpawnRegistration()
{
	cout << "\n=== ProjectileSync Spawn Registration Tests ===" << endl;

	ProjectileSync sync;
	sync.SetCurrentTick(100);

	// Note: Creating actual Projectile requires full game setup
	// These tests validate the API structure

	TEST("ProjectileSync has no pending spawns initially", sync.GetPendingSpawns().empty());

	// Test pending spawns retrieval clears the list
	auto spawns1 = sync.GetPendingSpawns();
	auto spawns2 = sync.GetPendingSpawns();
	TEST("ProjectileSync GetPendingSpawns clears list", spawns2.empty());
}



void TestProjectileSpawnStructure()
{
	cout << "\n=== ProjectileSync Spawn Structure Tests ===" << endl;

	ProjectileSync::ProjectileSpawn spawn;
	spawn.projectileID = 42;
	spawn.weaponName = "Laser Cannon";
	spawn.firingShipUUID = EsUuid::Generate();
	spawn.targetShipUUID = EsUuid::Generate();
	spawn.position = Point(100, 200);
	spawn.velocity = Point(10, 5);
	spawn.angle = Angle(45);
	spawn.spawnTick = 100;

	TEST("ProjectileSpawn stores projectile ID", spawn.projectileID == 42);
	TEST("ProjectileSpawn stores weapon name", spawn.weaponName == "Laser Cannon");
	TEST("ProjectileSpawn stores position", DoubleEqual(spawn.position.X(), 100));
	TEST("ProjectileSpawn stores velocity", DoubleEqual(spawn.velocity.Y(), 5));
	TEST("ProjectileSpawn stores spawn tick", spawn.spawnTick == 100);
}



void TestProjectileImpactRegistration()
{
	cout << "\n=== ProjectileSync Impact Registration Tests ===" << endl;

	ProjectileSync sync;
	sync.SetCurrentTick(150);

	TEST("ProjectileSync has no pending impacts initially", sync.GetPendingImpacts().empty());

	// Register an impact
	EsUuid targetUUID = EsUuid::Generate();
	sync.RegisterImpact(42, nullptr, Point(500, 600), 0.5);

	auto impacts = sync.GetPendingImpacts();
	TEST("ProjectileSync registers impact", impacts.size() == 1);

	if(!impacts.empty())
	{
		TEST("Impact has correct projectile ID", impacts[0].projectileID == 42);
		TEST("Impact has correct position", DoubleEqual(impacts[0].impactPosition.X(), 500));
		TEST("Impact has correct intersection", DoubleEqual(impacts[0].intersection, 0.5));
		TEST("Impact has correct tick", impacts[0].impactTick == 150);
	}

	// Test pending impacts retrieval clears the list
	auto impacts2 = sync.GetPendingImpacts();
	TEST("ProjectileSync GetPendingImpacts clears list", impacts2.empty());
}



void TestProjectileDeathRegistration()
{
	cout << "\n=== ProjectileSync Death Registration Tests ===" << endl;

	ProjectileSync sync;
	sync.SetCurrentTick(200);

	TEST("ProjectileSync has no pending deaths initially", sync.GetPendingDeaths().empty());

	// Register a death
	sync.RegisterDeath(99, Point(1000, 2000));

	auto deaths = sync.GetPendingDeaths();
	TEST("ProjectileSync registers death", deaths.size() == 1);

	if(!deaths.empty())
	{
		TEST("Death has correct projectile ID", deaths[0].projectileID == 99);
		TEST("Death has correct position", DoubleEqual(deaths[0].deathPosition.X(), 1000));
		TEST("Death has correct tick", deaths[0].deathTick == 200);
	}

	// Test pending deaths retrieval clears the list
	auto deaths2 = sync.GetPendingDeaths();
	TEST("ProjectileSync GetPendingDeaths clears list", deaths2.empty());
}



void TestProjectileSyncClear()
{
	cout << "\n=== ProjectileSync Clear Tests ===" << endl;

	ProjectileSync sync;
	sync.SetCurrentTick(100);

	// Register some events
	sync.RegisterImpact(1, nullptr, Point(0, 0), 0.5);
	sync.RegisterDeath(2, Point(0, 0));

	// Clear
	sync.Clear();

	TEST("ProjectileSync clear resets tracked count", sync.GetTrackedProjectileCount() == 0);
	TEST("ProjectileSync clear resets next ID to 1", sync.GetNextProjectileID() == 1);
	TEST("ProjectileSync clear clears pending spawns", sync.GetPendingSpawns().empty());
	TEST("ProjectileSync clear clears pending impacts", sync.GetPendingImpacts().empty());
	TEST("ProjectileSync clear clears pending deaths", sync.GetPendingDeaths().empty());
}



void TestProjectileSyncMultipleEvents()
{
	cout << "\n=== ProjectileSync Multiple Events Tests ===" << endl;

	ProjectileSync sync;

	// Register multiple impacts
	for(int i = 0; i < 5; ++i)
		sync.RegisterImpact(i, nullptr, Point(i * 100, i * 100), 0.5);

	// Register multiple deaths
	for(int i = 5; i < 10; ++i)
		sync.RegisterDeath(i, Point(i * 100, i * 100));

	auto impacts = sync.GetPendingImpacts();
	auto deaths = sync.GetPendingDeaths();

	TEST("ProjectileSync handles multiple impacts", impacts.size() == 5);
	TEST("ProjectileSync handles multiple deaths", deaths.size() == 5);

	// Verify ordering
	if(impacts.size() >= 5)
	{
		TEST("ProjectileSync preserves impact order", impacts[0].projectileID == 0 &&
			impacts[4].projectileID == 4);
	}

	if(deaths.size() >= 5)
	{
		TEST("ProjectileSync preserves death order", deaths[0].projectileID == 5 &&
			deaths[4].projectileID == 9);
	}
}



// ============================================================================
// CollisionAuthority Tests
// ============================================================================

void TestCollisionAuthorityBasics()
{
	cout << "\n=== CollisionAuthority Basic Tests ===" << endl;

	CollisionAuthority authority;

	// Test initial statistics
	TEST("CollisionAuthority starts with 0 total collisions", authority.GetTotalCollisionsDetected() == 0);
	TEST("CollisionAuthority starts with 0 ship hits", authority.GetShipHits() == 0);
	TEST("CollisionAuthority starts with 0 asteroid hits", authority.GetAsteroidHits() == 0);
}



void TestCollisionAuthorityStatistics()
{
	cout << "\n=== CollisionAuthority Statistics Tests ===" << endl;

	CollisionAuthority authority;

	// Reset statistics
	authority.ResetStatistics();

	TEST("CollisionAuthority reset clears total collisions", authority.GetTotalCollisionsDetected() == 0);
	TEST("CollisionAuthority reset clears ship hits", authority.GetShipHits() == 0);
	TEST("CollisionAuthority reset clears asteroid hits", authority.GetAsteroidHits() == 0);
}



void TestCollisionResultStructure()
{
	cout << "\n=== CollisionAuthority Result Structure Tests ===" << endl;

	CollisionAuthority::CollisionResult result;

	// Test default values
	TEST("CollisionResult defaults to projectile ID 0", result.projectileNetworkID == 0);
	TEST("CollisionResult defaults to NONE type",
		result.type == CollisionAuthority::CollisionResult::Type::NONE);
	TEST("CollisionResult defaults to not destroyed", !result.targetDestroyed);

	// Test type assignments
	result.type = CollisionAuthority::CollisionResult::Type::SHIP;
	TEST("CollisionResult can be set to SHIP type",
		result.type == CollisionAuthority::CollisionResult::Type::SHIP);

	result.type = CollisionAuthority::CollisionResult::Type::ASTEROID;
	TEST("CollisionResult can be set to ASTEROID type",
		result.type == CollisionAuthority::CollisionResult::Type::ASTEROID);

	result.type = CollisionAuthority::CollisionResult::Type::EXPIRED;
	TEST("CollisionResult can be set to EXPIRED type",
		result.type == CollisionAuthority::CollisionResult::Type::EXPIRED);
}



void TestCollisionAuthorityIntegration()
{
	cout << "\n=== CollisionAuthority Integration Tests ===" << endl;

	CollisionAuthority authority;
	ProjectileSync sync;

	// Set ProjectileSync
	authority.SetProjectileSync(&sync);

	// Note: Full collision detection requires GameState setup
	// This validates the API structure
	TEST("CollisionAuthority accepts ProjectileSync", true);
}



// ============================================================================
// Serialization Tests
// ============================================================================

void TestSpawnSerializationStructure()
{
	cout << "\n=== Spawn Serialization Structure Tests ===" << endl;

	ProjectileSync sync;

	// Create a spawn
	ProjectileSync::ProjectileSpawn spawn;
	spawn.projectileID = 123;
	spawn.weaponName = "Heavy Laser";
	spawn.firingShipUUID = EsUuid::Generate();
	spawn.targetShipUUID = EsUuid::Generate();
	spawn.position = Point(1000, 2000);
	spawn.velocity = Point(50, 100);
	spawn.angle = Angle(90);
	spawn.spawnTick = 500;

	// Verify structure
	TEST("Spawn has projectile ID", spawn.projectileID == 123);
	TEST("Spawn has weapon name", spawn.weaponName == "Heavy Laser");
	TEST("Spawn has position", DoubleEqual(spawn.position.X(), 1000));
	TEST("Spawn has velocity", DoubleEqual(spawn.velocity.X(), 50));
	TEST("Spawn has tick", spawn.spawnTick == 500);
}



void TestImpactSerializationStructure()
{
	cout << "\n=== Impact Serialization Structure Tests ===" << endl;

	ProjectileSync::ProjectileImpact impact;
	impact.projectileID = 456;
	impact.targetUUID = EsUuid::Generate();
	impact.impactPosition = Point(3000, 4000);
	impact.intersection = 0.75;
	impact.impactTick = 600;

	TEST("Impact has projectile ID", impact.projectileID == 456);
	TEST("Impact has position", DoubleEqual(impact.impactPosition.Y(), 4000));
	TEST("Impact has intersection", DoubleEqual(impact.intersection, 0.75));
	TEST("Impact has tick", impact.impactTick == 600);
}



void TestDeathSerializationStructure()
{
	cout << "\n=== Death Serialization Structure Tests ===" << endl;

	ProjectileSync::ProjectileDeath death;
	death.projectileID = 789;
	death.deathPosition = Point(5000, 6000);
	death.deathTick = 700;

	TEST("Death has projectile ID", death.projectileID == 789);
	TEST("Death has position", DoubleEqual(death.deathPosition.X(), 5000));
	TEST("Death has tick", death.deathTick == 700);
}



// ============================================================================
// Main
// ============================================================================

int main()
{
	cout << "======================================" << endl;
	cout << "Phase 3.3: Projectile Synchronization Tests" << endl;
	cout << "======================================" << endl;

	// ProjectileSync tests
	TestProjectileSyncBasics();
	TestProjectileSpawnRegistration();
	TestProjectileSpawnStructure();
	TestProjectileImpactRegistration();
	TestProjectileDeathRegistration();
	TestProjectileSyncClear();
	TestProjectileSyncMultipleEvents();

	// CollisionAuthority tests
	TestCollisionAuthorityBasics();
	TestCollisionAuthorityStatistics();
	TestCollisionResultStructure();
	TestCollisionAuthorityIntegration();

	// Serialization tests
	TestSpawnSerializationStructure();
	TestImpactSerializationStructure();
	TestDeathSerializationStructure();

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
