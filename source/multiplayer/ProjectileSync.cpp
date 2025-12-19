// ProjectileSync.cpp

#include "ProjectileSync.h"

#include "../Projectile.h"
#include "../Weapon.h"
#include "../Ship.h"
#include "../GameState.h"
#include "../network/PacketWriter.h"
#include "../network/PacketReader.h"

using namespace std;



ProjectileSync::ProjectileSync()
	: currentTick(0), nextProjectileID(1)
{
}



void ProjectileSync::SetCurrentTick(uint64_t tick)
{
	currentTick = tick;
}



uint64_t ProjectileSync::GetCurrentTick() const
{
	return currentTick;
}



uint32_t ProjectileSync::RegisterProjectileSpawn(const Projectile &projectile,
	const Ship *firingShip, const string &weaponName)
{
	uint32_t networkID = nextProjectileID++;

	// Track this projectile
	networkIDToProjectile[networkID] = &projectile;
	projectileToNetworkID[&projectile] = networkID;

	// Create spawn event
	ProjectileSpawn spawn;
	spawn.projectileID = networkID;
	spawn.weaponName = weaponName;
	spawn.firingShipUUID = firingShip ? firingShip->UUID() : EsUuid();
	spawn.targetShipUUID = projectile.Target() ? projectile.Target()->UUID() : EsUuid();
	spawn.position = projectile.Position();
	spawn.velocity = projectile.Velocity();
	spawn.angle = projectile.Facing();
	spawn.spawnTick = currentTick;

	pendingSpawns.push_back(spawn);

	return networkID;
}



vector<ProjectileSync::ProjectileSpawn> ProjectileSync::GetPendingSpawns()
{
	vector<ProjectileSpawn> result = pendingSpawns;
	pendingSpawns.clear();
	return result;
}



void ProjectileSync::RegisterImpact(uint32_t projectileID, const Ship *target,
	const Point &impactPos, double intersection)
{
	ProjectileImpact impact;
	impact.projectileID = projectileID;
	impact.targetUUID = target ? target->UUID() : EsUuid();
	impact.impactPosition = impactPos;
	impact.intersection = intersection;
	impact.impactTick = currentTick;

	pendingImpacts.push_back(impact);
}



void ProjectileSync::RegisterDeath(uint32_t projectileID, const Point &deathPos)
{
	ProjectileDeath death;
	death.projectileID = projectileID;
	death.deathPosition = deathPos;
	death.deathTick = currentTick;

	pendingDeaths.push_back(death);
}



vector<ProjectileSync::ProjectileImpact> ProjectileSync::GetPendingImpacts()
{
	vector<ProjectileImpact> result = pendingImpacts;
	pendingImpacts.clear();
	return result;
}



vector<ProjectileSync::ProjectileDeath> ProjectileSync::GetPendingDeaths()
{
	vector<ProjectileDeath> result = pendingDeaths;
	pendingDeaths.clear();
	return result;
}



void ProjectileSync::ApplySpawn(const ProjectileSpawn &spawn, GameState &gameState)
{
	// Find the weapon by name
	// const Weapon *weapon = GameData::Weapons().Get(spawn.weaponName);
	// if(!weapon)
	//     return;

	// Find the firing ship
	// auto firingShip = gameState.FindShipByUUID(spawn.firingShipUUID);

	// Create the projectile (this would need proper Projectile construction)
	// For now, this is a placeholder showing the integration point
	// In full implementation, we'd create the projectile and add it to gameState

	// Note: Actual implementation would require:
	// - GameData::Weapons() lookup
	// - Proper Projectile construction
	// - Adding to gameState.projectiles vector

	// Placeholder for now (full implementation in actual integration)
}



void ProjectileSync::ApplyImpact(const ProjectileImpact &impact, GameState &gameState)
{
	// Find the projectile by network ID
	Projectile *projectile = FindProjectileByNetworkID(impact.projectileID, gameState);
	if(!projectile)
		return;

	// Find the target ship
	// auto target = gameState.FindShipByUUID(impact.targetUUID);

	// Create visual effects for the impact
	// projectile->Explode(gameState.visuals, impact.intersection);

	// Mark projectile for removal
	// projectile->Kill();

	// Placeholder for now (full implementation in actual integration)
}



void ProjectileSync::ApplyDeath(const ProjectileDeath &death, GameState &gameState)
{
	// Find the projectile by network ID
	Projectile *projectile = FindProjectileByNetworkID(death.projectileID, gameState);
	if(!projectile)
		return;

	// Mark projectile as dead
	projectile->Kill();
}



Projectile *ProjectileSync::FindProjectileByNetworkID(uint32_t networkID, GameState &gameState)
{
	auto it = networkIDToProjectile.find(networkID);
	if(it == networkIDToProjectile.end())
		return nullptr;

	// Note: This returns a pointer that may be invalidated if the projectile was removed
	// Full implementation would need proper lifetime management
	return const_cast<Projectile *>(it->second);
}



uint32_t ProjectileSync::GetNetworkID(const Projectile *projectile) const
{
	auto it = projectileToNetworkID.find(projectile);
	if(it == projectileToNetworkID.end())
		return 0;
	return it->second;
}



bool ProjectileSync::IsTracked(const Projectile *projectile) const
{
	return projectileToNetworkID.find(projectile) != projectileToNetworkID.end();
}



void ProjectileSync::WriteSpawn(PacketWriter &writer, const ProjectileSpawn &spawn)
{
	writer.WriteUint32(spawn.projectileID);
	writer.WriteString(spawn.weaponName);
	writer.WriteUuid(spawn.firingShipUUID);
	writer.WriteUuid(spawn.targetShipUUID);
	writer.WritePoint(spawn.position);
	writer.WritePoint(spawn.velocity);
	writer.WriteAngle(spawn.angle);
	writer.WriteUint64(spawn.spawnTick);
}



ProjectileSync::ProjectileSpawn ProjectileSync::ReadSpawn(PacketReader &reader)
{
	ProjectileSpawn spawn;
	spawn.projectileID = reader.ReadUint32();
	spawn.weaponName = reader.ReadString();
	spawn.firingShipUUID = reader.ReadUuid();
	spawn.targetShipUUID = reader.ReadUuid();
	spawn.position = reader.ReadPoint();
	spawn.velocity = reader.ReadPoint();
	spawn.angle = reader.ReadAngle();
	spawn.spawnTick = reader.ReadUint64();
	return spawn;
}



void ProjectileSync::WriteImpact(PacketWriter &writer, const ProjectileImpact &impact)
{
	writer.WriteUint32(impact.projectileID);
	writer.WriteUuid(impact.targetUUID);
	writer.WritePoint(impact.impactPosition);
	writer.WriteDouble(impact.intersection);
	writer.WriteUint64(impact.impactTick);
}



ProjectileSync::ProjectileImpact ProjectileSync::ReadImpact(PacketReader &reader)
{
	ProjectileImpact impact;
	impact.projectileID = reader.ReadUint32();
	impact.targetUUID = reader.ReadUuid();
	impact.impactPosition = reader.ReadPoint();
	impact.intersection = reader.ReadDouble();
	impact.impactTick = reader.ReadUint64();
	return impact;
}



void ProjectileSync::WriteDeath(PacketWriter &writer, const ProjectileDeath &death)
{
	writer.WriteUint32(death.projectileID);
	writer.WritePoint(death.deathPosition);
	writer.WriteUint64(death.deathTick);
}



ProjectileSync::ProjectileDeath ProjectileSync::ReadDeath(PacketReader &reader)
{
	ProjectileDeath death;
	death.projectileID = reader.ReadUint32();
	death.deathPosition = reader.ReadPoint();
	death.deathTick = reader.ReadUint64();
	return death;
}



void ProjectileSync::Clear()
{
	pendingSpawns.clear();
	pendingImpacts.clear();
	pendingDeaths.clear();
	networkIDToProjectile.clear();
	projectileToNetworkID.clear();
	nextProjectileID = 1;
}



size_t ProjectileSync::GetTrackedProjectileCount() const
{
	return networkIDToProjectile.size();
}



uint32_t ProjectileSync::GetNextProjectileID() const
{
	return nextProjectileID;
}
