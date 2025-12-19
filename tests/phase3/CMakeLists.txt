# Phase 3.1: Engine Multiplayer Mode Integration Tests

# Note: This test requires the full EndlessSkyLib to build Engine
# For now, we document the test structure without full compilation

# Test for Engine multiplayer mode
add_executable(test_engine_multiplayer
	test_engine_multiplayer.cpp
)

target_include_directories(test_engine_multiplayer PRIVATE
	${CMAKE_SOURCE_DIR}/source
)

# Require full library for Engine
target_link_libraries(test_engine_multiplayer PRIVATE
	EndlessSkyLib
	pthread
)

# Add to test suite
add_test(NAME EngineMultiplayerMode COMMAND test_engine_multiplayer)


# Phase 3.2: Ship State Synchronization Tests

# Test for ship state synchronization (InterestManager, DeadReckoning, StateSync)
add_executable(test_ship_state_sync
	test_ship_state_sync.cpp
)

target_include_directories(test_ship_state_sync PRIVATE
	${CMAKE_SOURCE_DIR}/source
)

# Require full library for Ship and multiplayer components
target_link_libraries(test_ship_state_sync PRIVATE
	EndlessSkyLib
	pthread
)

# Add to test suite
add_test(NAME ShipStateSync COMMAND test_ship_state_sync)


# Phase 3.3: Projectile Synchronization Tests

# Test for projectile synchronization (ProjectileSync, CollisionAuthority)
add_executable(test_projectile_sync
	test_projectile_sync.cpp
)

target_include_directories(test_projectile_sync PRIVATE
	${CMAKE_SOURCE_DIR}/source
)

# Require full library for Projectile and multiplayer components
target_link_libraries(test_projectile_sync PRIVATE
	EndlessSkyLib
	pthread
)

# Add to test suite
add_test(NAME ProjectileSync COMMAND test_projectile_sync)
