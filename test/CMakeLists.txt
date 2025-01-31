# Download and build GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Create test executable
add_executable(chess_tests
    material_engine_tests.cpp
    board_tests.cpp
    chess_clock_tests.cpp
    engine_player_tests.cpp
    execution_time_tests.cpp
    game_state_tests.cpp
    move_generation_tests.cpp
    pext_bitboard_tests.cpp
    utility_tests.cpp
)

# Link against GoogleTest and chess engine library
target_link_libraries(chess_tests
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    KnightEngine
)

# Set properties for test executable
set_target_properties(chess_tests PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/test
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/test
)

# Just register the test directly
add_test(NAME chess_tests COMMAND chess_tests)