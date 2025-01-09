#include "pext_bitboard.hpp"
#include "memory_manager.hpp"
#include <benchmark/benchmark.h>

/***
 * Reminder: Commands and options for running benchmarks:
 * .\benchmarks\Debug\chess_benchmarks.exe
 * 
 * Options:
 * --benchmark_filter=<regex>               Filter to run benchmarks that match <regex>
 * --benchmark_repetitions=N                Run each benchmark N times
 * --benchmark_format=<console|json|csv>    Output results in (format)
 * --benchmark_out=<file>                   Save results to (file)
 * --benchmark_out_format=<console|json|csv>
 */

// Function declared in memory_manager.cpp
void SetGlobalMemoryManager(ChessMemoryManager* manager);

int main(int argc, char** argv) {
    try {
        // Initialize PEXT tables before setting up memory tracking
        PEXT::initialize();
        
        // Create memory manager
        static ChessMemoryManager memoryManager;
        
        // Register memory manager with Google Benchmark
        benchmark::RegisterMemoryManager(&memoryManager);
        
        // Set as global memory manager for operator new/delete tracking
        SetGlobalMemoryManager(&memoryManager);

        ::benchmark::Initialize(&argc, argv);
        if (::benchmark::ReportUnrecognizedArguments(argc, argv)) {
            return 1;
        }
        ::benchmark::RunSpecifiedBenchmarks();
        ::benchmark::Shutdown();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error during benchmark execution: " << e.what() << std::endl;
        return 1;
    }
}