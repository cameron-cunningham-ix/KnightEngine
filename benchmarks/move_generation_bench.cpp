#include "moves.hpp"
#include "chess_board.hpp"
#include "utility.hpp"
#include <benchmark/benchmark.h>

// Benchmark move generation from initial position
static void BM_InitialMoveGen(benchmark::State& state) {
    ChessBoard board;
    for (auto _ : state) {
        int moveNum = 0;
        auto moves = MoveGenerator::generateLegalMoves(board, moveNum);
        benchmark::DoNotOptimize(moves);
    }
}
BENCHMARK(BM_InitialMoveGen);

// Benchmark move generation from Kiwipete position
static void BM_KiwipeteMoveGen(benchmark::State& state) {
    ChessBoard board;
    board.setupPositionFromFEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    for (auto _ : state) {
        int moveNum = 0;
        auto moves = MoveGenerator::generateLegalMoves(board, moveNum);
        benchmark::DoNotOptimize(moves);
    }
}
BENCHMARK(BM_KiwipeteMoveGen);

// Benchmark move generation from initial position
static void BM_MGMakeUnmakeSearch(benchmark::State& state) {
    ChessBoard board;
    int moveNum = 0;
    auto moves = MoveGenerator::generateLegalMoves(board, moveNum);
    benchmark::DoNotOptimize(moves);
    for (auto _ : state) {
        for (int i = 0; i < moveNum + 1; i++) {
            board.makeMove(moves[i], true);
            benchmark::DoNotOptimize(board);
            board.unmakeMove(moves[i], true);
        }
    }
}
BENCHMARK(BM_MGMakeUnmakeSearch);

// Benchmark move generation from initial position
static void BM_MGMakeUnmakeNoSearch(benchmark::State& state) {
    ChessBoard board;
    int moveNum = 0;
    auto moves = MoveGenerator::generateLegalMoves(board, moveNum);
    benchmark::DoNotOptimize(moves);
    for (auto _ : state) {
        for (int i = 0; i < moveNum + 1; i++) {
            board.makeMove(moves[i], false);
            benchmark::DoNotOptimize(board);
            board.unmakeMove(moves[i], false);
        }
    }
}
BENCHMARK(BM_MGMakeUnmakeNoSearch);