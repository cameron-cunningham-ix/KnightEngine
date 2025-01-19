# KnightEngine/CMakeLists.txt (root)
cmake_minimum_required(VERSION 3.10)

# To use vcpkg
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()

project(KnightEngine)

# Add this section to force consistent runtime library settings
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set optimization flags for different build types
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        set(CMAKE_CXX_FLAGS_RELEASE "/O2 /GL")
    else()
        set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    if(MSVC)
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Zi /GL")
    else()
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
    endif()
endif()


# Create the chess engine library
add_library(KnightEngine
    src/board_generation.cpp
    src/board_generation.hpp
    src/board_utility.hpp
    src/chess_clock.cpp
    src/chess_clock.hpp
    src/chess_engine_base.cpp
    src/chess_engine_base.hpp
    src/chess_match.cpp
    src/chess_match.hpp
    src/engine_player.cpp
    src/engine_player.hpp
    src/human_player.cpp
    src/human_player.hpp
    src/i_chess_engine.hpp
    src/i_player.hpp
    src/move_history.cpp
    src/move_history.hpp
    src/moves.cpp
    src/moves.hpp
    src/pext_bitboard.cpp
    src/pext_bitboard.hpp
    src/transposition_table.hpp
    src/types.cpp
    src/types.hpp
    src/utility.cpp
    src/utility.hpp
    src/zobrist.cpp
    src/zobrist.hpp
    engine/random_engine.hpp
    engine/material_engine.hpp
)

# Include directories
target_include_directories(KnightEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(KnightEngine PRIVATE
)

# Add the executable
add_executable(KnightEngineApp
    src/main.cpp
)

# Link the library
target_link_libraries(KnightEngineApp PRIVATE 
    KnightEngine
)

# Enable testing
enable_testing()
add_subdirectory(test)
file(MAKE_DIRECTORY build/TestOutput)

# Enable benchmarking
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)


FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3  # Use the latest stable version
)
set(BENCHMARK_ENABLE_TESTING OFF)
FetchContent_MakeAvailable(benchmark)

# Create benchmarks directory
add_subdirectory(benchmarks)
file(MAKE_DIRECTORY build/BenchOutput)


# Create UCI handler library
add_library(uci_handler
    engine/uci/uci_handler.cpp
    engine/uci/uci_handler.hpp
)

target_link_libraries(uci_handler PUBLIC
    KnightEngine
)

# Create UCI engine executables
add_executable(random_engine
    engine/uci/random_engine_main.cpp
)

add_executable(material_engine
    engine/uci/material_engine_main.cpp
)

# Link against the chess engine library and UCI handler
target_link_libraries(random_engine PRIVATE 
    KnightEngine
    uci_handler
)

target_link_libraries(material_engine PRIVATE 
    KnightEngine
    uci_handler
)

# Set properties for the engine executables
set_target_properties(random_engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/engines
)

set_target_properties(material_engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/engines
)