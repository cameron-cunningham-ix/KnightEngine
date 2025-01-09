add_executable(chess_benchmarks
    board_operations_bench.cpp
    move_generation_bench.cpp
    perft_bench.cpp
    pext_bench.cpp
    memory_manager.cpp
    memory_manager.hpp
    main_benchmark.cpp
)

target_link_libraries(chess_benchmarks
    PRIVATE
    KnightEngine
    benchmark::benchmark
    benchmark::benchmark_main
)

# Set output directory
set_target_properties(chess_benchmarks PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/benchmarks
)