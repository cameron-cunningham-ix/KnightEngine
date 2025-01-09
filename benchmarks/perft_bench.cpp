#include "utility.hpp"
#include "pext_bitboard.hpp"
#include "board_generation.hpp"
#include <benchmark/benchmark.h>

// Benchmark perft 1 on initial position
static void BM_PerftD1Init(benchmark::State& state) {
    ChessBoard board = ChessBoard();
    for (auto _ : state) {
        U64 nodes = perft(board, 1, 1, false);
        benchmark::DoNotOptimize(nodes);
    }
}
BENCHMARK(BM_PerftD1Init);

// Benchmark perft 3 on initial position
static void BM_PerftD3Init(benchmark::State& state) {
    ChessBoard board = ChessBoard();
    for (auto _ : state) {
        U64 nodes = perft(board, 3, 3, false);
        benchmark::DoNotOptimize(nodes);
    }
}
BENCHMARK(BM_PerftD3Init);

// Benchmark perft 5 on initial position
static void BM_PerftD5Init(benchmark::State& state) {
    ChessBoard board = ChessBoard();
    for (auto _ : state) {
        U64 nodes = perft(board, 5, 5, false);
        benchmark::DoNotOptimize(nodes);
    }
}
BENCHMARK(BM_PerftD5Init);

// // Benchmark perft 6 on initial position
// static void BM_PerftD6Init(benchmark::State& state) {
//     ChessBoard board = ChessBoard();
//     for (auto _ : state) {
//         U64 nodes = perft(board, 6, 6, false);
//         benchmark::DoNotOptimize(nodes);
//     }
// }
// BENCHMARK(BM_PerftD6Init);