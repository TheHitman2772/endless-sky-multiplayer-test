# Phase 2 tests for Game State / Presentation Separation

cmake_minimum_required(VERSION 3.19)

# Phase 2.1: Game State Separation test
add_executable(test_game_state_separation
	test_game_state_separation.cpp
	../../source/GameState.cpp
	../../source/ClientState.cpp
	../../source/Renderer.cpp
	../../source/Ship.cpp
	../../source/Body.cpp
	../../source/Point.cpp
	../../source/Angle.cpp
	../../source/Armament.cpp
	../../source/CargoHold.cpp
	../../source/Outfit.cpp
	../../source/Government.cpp
	../../source/Personality.cpp
	../../source/System.cpp
	../../source/Planet.cpp
	../../source/EsUuid.cpp
	../../source/Port.cpp
	../../source/ship/ShipAICache.cpp
	../../source/ShipJumpNavigation.cpp
)
target_include_directories(test_game_state_separation PRIVATE ../../source)
target_compile_features(test_game_state_separation PRIVATE cxx_std_20)

# Platform-specific UUID libraries
if(WIN32)
	target_link_libraries(test_game_state_separation PRIVATE rpcrt4)
else()
	target_link_libraries(test_game_state_separation PRIVATE uuid)
endif()

# Phase 2.2: Player Management System test
add_executable(test_player_management
	test_player_management.cpp
	../../source/multiplayer/NetworkPlayer.cpp
	../../source/multiplayer/PlayerRegistry.cpp
	../../source/multiplayer/PlayerManager.cpp
	../../source/Ship.cpp
	../../source/Body.cpp
	../../source/Point.cpp
	../../source/Angle.cpp
	../../source/Armament.cpp
	../../source/CargoHold.cpp
	../../source/Outfit.cpp
	../../source/Government.cpp
	../../source/Personality.cpp
	../../source/System.cpp
	../../source/Planet.cpp
	../../source/EsUuid.cpp
	../../source/Port.cpp
	../../source/Account.cpp
	../../source/Mission.cpp
	../../source/ship/ShipAICache.cpp
	../../source/ShipJumpNavigation.cpp
)
target_include_directories(test_player_management PRIVATE ../../source)
target_compile_features(test_player_management PRIVATE cxx_std_20)

# Platform-specific UUID libraries for test_player_management
if(WIN32)
	target_link_libraries(test_player_management PRIVATE rpcrt4)
else()
	target_link_libraries(test_player_management PRIVATE uuid)
endif()

# Add tests
enable_testing()
add_test(NAME GameStateSeparation COMMAND test_game_state_separation)
set_tests_properties(GameStateSeparation PROPERTIES TIMEOUT 10 LABELS "architecture;phase2.1")

add_test(NAME PlayerManagement COMMAND test_player_management)
set_tests_properties(PlayerManagement PROPERTIES TIMEOUT 10 LABELS "architecture;phase2.2")

message(STATUS "Phase 2 tests configured (Phase 2.1 + 2.2)")
