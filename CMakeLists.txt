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


# Create the chess engine library
add_library(KnightEngine
    src/board_generation.cpp
    src/board_generation.hpp
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
    src/material_engine.hpp
    src/move_history.cpp
    src/move_history.hpp
    src/moves.cpp
    src/moves.hpp
    src/player.hpp
    src/random_engine.hpp
    src/types.hpp
    src/utility.cpp
    src/utility.hpp
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

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1  # or another stable release
)
FetchContent_MakeAvailable(googletest)