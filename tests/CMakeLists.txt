include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(catch2)

# --- Schema/decoder unit tests: no network required, safe for every CI run. ---
add_executable(unit_tests test_decoder.cpp)
target_link_libraries(unit_tests PRIVATE binance_core Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(unit_tests)

# --- Connectivity integration test: opens a real TLS websocket to Binance. ---
# Excluded from the default `ctest` run; opt in with `ctest -L integration`.
add_executable(integration_tests test_connectivity.cpp)
target_link_libraries(integration_tests PRIVATE binance_core Catch2::Catch2WithMain)

catch_discover_tests(integration_tests
    TEST_PREFIX "integration/"
    PROPERTIES LABELS "integration"
)
