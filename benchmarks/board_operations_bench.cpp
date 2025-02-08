#include "chess_board.hpp"
#include "moves.hpp"
#include <benchmark/benchmark.h>

// Benchmark making moves
static void BM_MakeMoveInit(benchmark::State& state) {
    ChessBoard board;
    DenseMove move(W_PAWN, 12, 28); // e2e4
    for (auto _ : state) {
        board.makeMove(move, true);
        benchmark::DoNotOptimize(board);
        board.unmakeMove(move, true);
    }
}
BENCHMARK(BM_MakeMoveInit);

// Benchmark FEN parsing
static void BM_FENParsing(benchmark::State& state) {
    ChessBoard board;
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    for (auto _ : state) {
        board.setupPositionFromFEN(fen);
        benchmark::DoNotOptimize(board);
    }
}
BENCHMARK(BM_FENParsing);

// Benchmark checking for check
static void BM_IsInCheck(benchmark::State& state) {
    ChessBoard board;
    board.setupPositionFromFEN("r1bqkb1r/pppp1ppp/2n2n2/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR w KQkq - 4 4");
    for (auto _ : state) {
        bool inCheck = board.isInCheck();
        benchmark::DoNotOptimize(inCheck);
    }
}
BENCHMARK(BM_IsInCheck);

// Benchmark attack calculations
static void BM_AttkCalcInit(benchmark::State& state) {
    ChessBoard board;
    for (auto _ : state) {
        U64 attacks = board.OppAttacksToSquare(4, WHITE); // Attacks to e1
        benchmark::DoNotOptimize(attacks);
    }
}
BENCHMARK(BM_AttkCalcInit);