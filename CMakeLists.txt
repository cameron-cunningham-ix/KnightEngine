cmake_minimum_required(VERSION 3.10)

# To use vcpkg
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()

project(KnightEngine)

# Add warning flags based on compiler
if(MSVC)
    # Warning level 4 and all warnings as errors
    add_compile_options(
        /W4     # All reasonable warnings
        #/WX     # Treat warnings as errors
        /w14242 # 'identfier': conversion from 'type1' to 'type1', possible loss of data
        /w14254 # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits'
        /w14263 # 'function': member function does not override any base class virtual member function
        /w14265 # 'classname': class has virtual functions, but destructor is not virtual
        /w14287 # 'operator': unsigned/negative constant mismatch
        /we4289 # nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside the for-loop scope
        /w14296 # 'operator': expression is always 'boolean_value'
        /w14311 # 'variable': pointer truncation from 'type1' to 'type2'
        /w14545 # expression before comma evaluates to a function which is missing an argument list
        /w14546 # function call before comma missing argument list
        /w14547 # 'operator': operator before comma has no effect; expected operator with side-effect
        /w14549 # 'operator': operator before comma has no effect; did you intend 'operator'?
        /w14555 # expression has no effect; expected expression with side-effect
        /w14619 # pragma warning: there is no warning number 'number'
        /w14640 # Enable warning on thread un-safe static member initialization
        /w14826 # Conversion from 'type1' to 'type_2' is sign-extended. This may cause unexpected runtime behavior
        /w14905 # wide string literal cast to 'LPSTR'
        /w14906 # string literal cast to 'LPWSTR'
        /w14928 # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
    )
else()
    # GCC/Clang warnings
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
        -Wmisleading-indentation
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wuseless-cast
    )
endif()

# To force consistent runtime library settings
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

message("CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")


# Create the chess engine library
add_library(KnightEngine
    src/board_generation.cpp
    src/board_generation.hpp
    src/board_utility.hpp
    src/chess_clock.cpp
    src/chess_clock.hpp
    src/chess_engine_base.cpp
    src/chess_engine_base.hpp
    src/engine_player.cpp
    src/engine_player.hpp
    src/i_chess_engine.hpp
    src/i_player.hpp
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
    src/engine/random_engine.hpp
    src/engine/material_engine.hpp
)

# Include directories
target_include_directories(KnightEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(KnightEngine PRIVATE
)

# Create UCI handler library
add_library(uci_handler
    src/engine/uci/uci_handler.cpp
    src/engine/uci/uci_handler.hpp
)

target_link_libraries(uci_handler PUBLIC
    KnightEngine
)

# Create UCI engine executables
add_executable(random_engine
    src/engine/uci/random_engine_main.cpp
)

add_executable(material_engine
    src/engine/uci/material_engine_main.cpp
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


