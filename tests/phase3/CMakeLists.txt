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
