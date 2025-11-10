cmake_minimum_required(VERSION 3.15)
project(MarketSimulation VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/Account.cpp
    src/Trader.cpp
    src/OrderBook.cpp
    src/MatchingEngine.cpp
    src/SettlementEngine.cpp
    src/MarketServer.cpp
    src/WebServer.cpp
    src/OrderLogger.cpp
    src/main.cpp
)

# Create executable
add_executable(market_simulation ${SOURCES})

# Link required libraries
target_link_libraries(market_simulation pthread pq)

# Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Test executable
enable_testing()

add_executable(market_tests
    tests/test_market.cpp
    tests/TestClient.cpp
    src/Account.cpp
    src/Trader.cpp
    src/OrderBook.cpp
    src/MatchingEngine.cpp
    src/SettlementEngine.cpp
    src/MarketServer.cpp
    src/OrderLogger.cpp
)

target_link_libraries(market_tests
    GTest::gtest
    GTest::gtest_main
    pthread
    pq
)

include(GoogleTest)
gtest_discover_tests(market_tests)

