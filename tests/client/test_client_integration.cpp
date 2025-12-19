/* test_client_integration.cpp
 * Copyright (c) 2025 by Endless Sky Development Team
 *
 * Phase 2.5 integration tests: Client implementation
 *
 * This test suite validates the multiplayer client components:
 * - ConnectionMonitor: Network quality tracking
 * - EntityInterpolator: Smooth remote entity movement
 * - ClientReconciliation: Prediction error correction
 */

#include "../../source/client/ConnectionMonitor.h"
#include "../../source/client/EntityInterpolator.h"
#include "../../source/client/ClientReconciliation.h"
#include "../../source/EsUuid.h"
#include "../../source/Point.h"
#include "../../source/Angle.h"

#include <iostream>
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


// Test 1: ConnectionMonitor basic functionality
bool TestConnectionMonitorBasic()
{
	ConnectionMonitor monitor;

	// Initially disconnected
	if(monitor.GetQuality() != ConnectionMonitor::Quality::DISCONNECTED)
		return false;

	// Record some pings
	monitor.RecordPing(50);
	if(monitor.GetPing() != 50)
		return false;

	monitor.RecordPing(60);
	monitor.RecordPing(55);

	// Average should be around 55
	uint32_t avg = monitor.GetAveragePing();
	if(avg < 50 || avg > 60)
		return false;

	return true;
}


// Test 2: ConnectionMonitor quality assessment
bool TestConnectionMonitorQuality()
{
	ConnectionMonitor monitor;

	// Excellent connection
	for(int i = 0; i < 10; ++i)
		monitor.RecordPing(30);

	monitor.Update();

	// Should be excellent quality
	if(monitor.GetQuality() != ConnectionMonitor::Quality::EXCELLENT)
		return false;

	// Poor connection
	for(int i = 0; i < 10; ++i)
		monitor.RecordPing(300);

	monitor.Update();

	// Should degrade to poor
	auto quality = monitor.GetQuality();
	if(quality != ConnectionMonitor::Quality::POOR &&
	   quality != ConnectionMonitor::Quality::TERRIBLE)
		return false;

	return true;
}


// Test 3: ConnectionMonitor packet loss tracking
bool TestConnectionMonitorPacketLoss()
{
	ConnectionMonitor monitor;

	// Send 10 packets
	for(uint64_t i = 0; i < 10; ++i)
		monitor.RecordPacketSent(i);

	// Receive 8 packets
	for(uint64_t i = 0; i < 8; ++i)
		monitor.RecordPacketReceived(i);

	// Should have 20% packet loss
	double loss = monitor.GetPacketLoss();
	if(loss < 15.0 || loss > 25.0)  // Allow some tolerance
		return false;

	return true;
}


// Test 4: EntityInterpolator basic functionality
bool TestEntityInterpolatorBasic()
{
	EntityInterpolator interpolator;

	EsUuid entityId = EsUuid::MakeUuid();

	// Add some snapshots
	EntityState state1(100, Point(0, 0), Point(1, 0), Angle(0));
	EntityState state2(110, Point(10, 0), Point(1, 0), Angle(0));

	interpolator.AddSnapshot(entityId, state1);
	interpolator.AddSnapshot(entityId, state2);

	if(interpolator.GetTrackedEntityCount() != 1)
		return false;

	if(interpolator.GetTotalSnapshotsStored() != 2)
		return false;

	return true;
}


// Test 5: EntityInterpolator history limiting
bool TestEntityInterpolatorHistory()
{
	EntityInterpolator interpolator;
	interpolator.SetMaxSnapshotHistory(3);

	EsUuid entityId = EsUuid::MakeUuid();

	// Add 5 snapshots
	for(uint64_t i = 0; i < 5; ++i)
	{
		EntityState state(i, Point(i * 10, 0), Point(1, 0), Angle(0));
		interpolator.AddSnapshot(entityId, state);
	}

	// Should only keep last 3
	if(interpolator.GetTotalSnapshotsStored() != 3)
		return false;

	return true;
}


// Test 6: ClientReconciliation position error
bool TestClientReconciliationPosition()
{
	ClientReconciliation reconciliation;

	Point predicted(100, 100);
	Point server(110, 105);

	// Start reconciliation
	reconciliation.ReconcilePosition(predicted, server);

	if(!reconciliation.IsReconcilingPosition())
		return false;

	// Apply correction
	Point corrected = reconciliation.GetCorrectedPosition(predicted);

	// Should be moving towards server position
	double distBefore = (predicted - server).Length();
	double distAfter = (corrected - server).Length();

	if(distAfter >= distBefore)
		return false;

	return true;
}


// Test 7: ClientReconciliation error threshold
bool TestClientReconciliationThreshold()
{
	ClientReconciliation reconciliation;
	reconciliation.SetErrorThreshold(10.0);  // Ignore errors < 10 pixels

	Point predicted(100, 100);
	Point server(102, 101);  // Only 2-3 pixels off

	// Should ignore this error
	reconciliation.ReconcilePosition(predicted, server);

	if(reconciliation.IsReconcilingPosition())
		return false;  // Should not be reconciling

	return true;
}


// Test 8: ClientReconciliation snap threshold
bool TestClientReconciliationSnap()
{
	ClientReconciliation reconciliation;
	reconciliation.SetSnapThreshold(100.0);  // Snap if error > 100 pixels

	Point predicted(100, 100);
	Point server(300, 300);  // 200+ pixels off

	// Should snap immediately
	reconciliation.ReconcilePosition(predicted, server);

	// Snap means no gradual reconciliation
	if(reconciliation.IsReconcilingPosition())
		return false;

	if(reconciliation.GetTotalSnaps() != 1)
		return false;

	return true;
}


// Test 9: ClientReconciliation velocity correction
bool TestClientReconciliationVelocity()
{
	ClientReconciliation reconciliation;

	Point predictedVel(10, 0);
	Point serverVel(15, 5);

	reconciliation.ReconcileVelocity(predictedVel, serverVel);

	if(!reconciliation.IsReconcilingVelocity())
		return false;

	Point corrected = reconciliation.GetCorrectedVelocity(predictedVel);

	// Should be moving towards server velocity
	double distBefore = (predictedVel - serverVel).Length();
	double distAfter = (corrected - serverVel).Length();

	if(distAfter >= distBefore)
		return false;

	return true;
}


// Test 10: ClientReconciliation facing correction
bool TestClientReconciliationFacing()
{
	ClientReconciliation reconciliation;

	Angle predicted(0);
	Angle server(45);

	reconciliation.ReconcileFacing(predicted, server);

	if(!reconciliation.IsReconcilingFacing())
		return false;

	Angle corrected = reconciliation.GetCorrectedFacing(predicted);

	// Should be rotating towards server angle
	double angleDiff = abs(corrected.Degrees() - server.Degrees());
	if(angleDiff >= 45.0)
		return false;

	return true;
}


int main()
{
	cout << "==================================" << endl;
	cout << "Phase 2.5: Client Integration Tests" << endl;
	cout << "==================================" << endl;
	cout << endl;

	// ConnectionMonitor tests
	cout << "ConnectionMonitor Tests:" << endl;
	ReportTest("ConnectionMonitor basic", TestConnectionMonitorBasic());
	ReportTest("ConnectionMonitor quality", TestConnectionMonitorQuality());
	ReportTest("ConnectionMonitor packet loss", TestConnectionMonitorPacketLoss());
	cout << endl;

	// EntityInterpolator tests
	cout << "EntityInterpolator Tests:" << endl;
	ReportTest("EntityInterpolator basic", TestEntityInterpolatorBasic());
	ReportTest("EntityInterpolator history", TestEntityInterpolatorHistory());
	cout << endl;

	// ClientReconciliation tests
	cout << "ClientReconciliation Tests:" << endl;
	ReportTest("ClientReconciliation position", TestClientReconciliationPosition());
	ReportTest("ClientReconciliation threshold", TestClientReconciliationThreshold());
	ReportTest("ClientReconciliation snap", TestClientReconciliationSnap());
	ReportTest("ClientReconciliation velocity", TestClientReconciliationVelocity());
	ReportTest("ClientReconciliation facing", TestClientReconciliationFacing());
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
