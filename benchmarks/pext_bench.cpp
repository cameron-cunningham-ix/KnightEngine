#include "pext_bitboard.hpp"
#include "chess_board.hpp"
#include <benchmark/benchmark.h>

// Benchmark rook attacks calculation
static void BM_RookAttacks(benchmark::State& state) {
    ChessBoard board;
    U64 occupancy = board.getAllPieces();
    for (auto _ : state) {
        for (int square = 0; square < 64; square++) {
            U64 attacks = PEXT::getRookAttacks(square, occupancy);
            benchmark::DoNotOptimize(attacks);
        }
    }
}
BENCHMARK(BM_RookAttacks);

// Benchmark bishop attacks calculation
static void BM_BishopAttacks(benchmark::State& state) {
    ChessBoard board;
    U64 occupancy = board.getAllPieces();
    for (auto _ : state) {
        for (int square = 0; square < 64; square++) {
            U64 attacks = PEXT::getBishopAttacks(square, occupancy);
            benchmark::DoNotOptimize(attacks);
        }
    }
}
BENCHMARK(BM_BishopAttacks);

// Benchmark queen attacks calculation (combines rook and bishop)
static void BM_QueenAttacks(benchmark::State& state) {
    ChessBoard board;
    U64 occupancy = board.getAllPieces();
    for (auto _ : state) {
        for (int square = 0; square < 64; square++) {
            U64 attacks = PEXT::getRookAttacks(square, occupancy) | 
                          PEXT::getBishopAttacks(square, occupancy);
            benchmark::DoNotOptimize(attacks);
        }
    }
}
BENCHMARK(BM_QueenAttacks);