# Phase 2.4: Server Integration Tests

# Test for server integration
add_executable(test_server_integration
	test_server_integration.cpp
	../../source/server/ServerConfig.cpp
	../../source/server/SnapshotManager.cpp
	../../source/server/ServerLoop.cpp
	../../source/GameState.cpp
	../../source/Ship.cpp
	../../source/Body.cpp
	../../source/Point.cpp
	../../source/Angle.cpp
	../../source/EsUuid.cpp
)

target_include_directories(test_server_integration PRIVATE
	${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(test_server_integration PRIVATE
	pthread
)

# Add to test suite
add_test(NAME ServerIntegration COMMAND test_server_integration)
