# Phase 2.5: Client Integration Tests

# Test for client integration
add_executable(test_client_integration
	test_client_integration.cpp
	../../source/client/ConnectionMonitor.cpp
	../../source/client/EntityInterpolator.cpp
	../../source/client/ClientReconciliation.cpp
	../../source/Point.cpp
	../../source/Angle.cpp
	../../source/EsUuid.cpp
)

target_include_directories(test_client_integration PRIVATE
	${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(test_client_integration PRIVATE
	pthread
)

# Add to test suite
add_test(NAME ClientIntegration COMMAND test_client_integration)
