# KnightEngine/CMakeLists.txt (root)
cmake_minimum_required(VERSION 3.10)
project(KnightEngine)

# Add this section to force consistent runtime library settings
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create the chess engine library
add_library(KnightEngine
    src/board_generation.cpp
    src/board_generation.hpp
    src/moves.cpp
    src/moves.hpp
    src/types.hpp
    src/utility.cpp
    src/utility.hpp
    src/chess_clock.cpp
    src/chess_clock.hpp
)

# Include directories
target_include_directories(KnightEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Enable testing
enable_testing()
add_subdirectory(test)