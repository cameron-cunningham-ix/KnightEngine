#include "moves.hpp"
#include "board_generation.hpp"
#include "utility.hpp"
#include <benchmark/benchmark.h>

// Benchmark move generation from initial position
static void BM_InitialMoveGen(benchmark::State& state) {
    ChessBoard board;
    for (auto _ : state) {
        auto moves = MoveGenerator::generateLegalMoves(board);
        benchmark::DoNotOptimize(moves);
    }
}
BENCHMARK(BM_InitialMoveGen);

// Benchmark move generation from Kiwipete position
static void BM_KiwipeteMoveGen(benchmark::State& state) {
    ChessBoard board;
    board.setupPositionFromFEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    for (auto _ : state) {
        auto moves = MoveGenerator::generateLegalMoves(board);
        benchmark::DoNotOptimize(moves);
    }
}
BENCHMARK(BM_KiwipeteMoveGen);

// Benchmark move generation from initial position
static void BM_MGMakeUnmakeSearch(benchmark::State& state) {
    ChessBoard board;
    for (auto _ : state) {
        auto moves = MoveGenerator::generateLegalMoves(board);
        benchmark::DoNotOptimize(moves);
        for (const DenseMove& move : moves) {
            board.makeMove(move, true);
            benchmark::DoNotOptimize(board);
            board.unmakeMove(move, true);
        }
    }
}
BENCHMARK(BM_MGMakeUnmakeSearch);

// Benchmark move generation from initial position
static void BM_MGMakeUnmakeNoSearch(benchmark::State& state) {
    ChessBoard board;
    for (auto _ : state) {
        auto moves = MoveGenerator::generateLegalMoves(board);
        benchmark::DoNotOptimize(moves);
        for (const DenseMove& move : moves) {
            board.makeMove(move, false);
            benchmark::DoNotOptimize(board);
            board.unmakeMove(move, false);
        }
    }
}
BENCHMARK(BM_MGMakeUnmakeNoSearch);