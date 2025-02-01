#include <benchmark/benchmark.h>
#include "../src/engine/material_engine.hpp"
#include "../src/chess_clock.hpp"
#include "../src/utility.hpp"
#include "../src/pext_bitboard.hpp"
#include <chrono>
#include <iostream>
#include <fstream>

// Benchmarking setup to share state between runs
class BM_MatEng : public benchmark::Fixture {
protected:
    std::unique_ptr<MaterialEngine> engine;
    ChessBoard board;
    ChessClock currClock;
    
    void SetUp(const ::benchmark::State& state) {
        PEXT::initialize();
        engine = std::make_unique<MaterialEngine>();
        board = ChessBoard();
        // Set up infinite time control for most tests
        TimeControl tc(std::chrono::hours(1), std::chrono::seconds(0), std::chrono::seconds(0), -1, true);
        currClock = ChessClock(tc);
    }
    
    void TearDown(const ::benchmark::State& state) {
        engine.reset();
    }
};

// Benchmark the base position evaluation
BENCHMARK_F(BM_MatEng, EvalStartPos)(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(engine->evaluatePosition(board));
    }
}

// Benchmark evaluation with complex material imbalance
BENCHMARK_F(BM_MatEng, EvalComplexPos)(benchmark::State& state) {
    board.setupPositionFromFEN("r1bqk2r/pp3ppp/2nppn2/2p5/2PP4/2N2NP1/PP2PPBP/R1BQK2R w KQkq - 0 1");
    for (auto _ : state) {
        benchmark::DoNotOptimize(engine->evaluatePosition(board));
    }
}

// Benchmark pawn structure evaluation
BENCHMARK_F(BM_MatEng, EvalPawnStruct)(benchmark::State& state) {
    // Position with interesting pawn structure
    board.setupPositionFromFEN("rnbqkbnr/ppp2ppp/4p3/3pP3/3P4/8/PPP2PPP/RNBQKBNR w KQkq d6 0 1");
    for (auto _ : state) {
        benchmark::DoNotOptimize(engine->evaluatePosition(board));
    }
}

// Benchmark king safety evaluation
BENCHMARK_F(BM_MatEng, EvalKingSafe)(benchmark::State& state) {
    // Position with exposed king
    board.setupPositionFromFEN("rnbqk2r/pppp1ppp/5n2/4p3/1b2P3/3P1N2/PPP2PPP/RNBQKB1R w KQkq - 0 1");
    for (auto _ : state) {
        benchmark::DoNotOptimize(engine->evaluatePosition(board));
    }
}

// Benchmark search at different depths
template<int Depth>
void BM_SearchAtDepth(benchmark::State& state) {
    PEXT::initialize();
    MaterialEngine engine;
    ChessBoard board;
    ChessClock currClock;
    // Set up infinite time control for most tests
    TimeControl tc(std::chrono::hours(1), std::chrono::seconds(0), std::chrono::seconds(0), -1, true);
    currClock = ChessClock(tc);
    engine.setSearchDepth(Depth);
    for (auto _ : state) {
        benchmark::DoNotOptimize(engine.findBestMove(board, currClock));
    }
}

// Register search depth benchmarks
BENCHMARK(BM_SearchAtDepth<1>)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_SearchAtDepth<2>)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_SearchAtDepth<3>)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_SearchAtDepth<4>)->Unit(benchmark::kMillisecond);

// Benchmark alpha-beta search performance
BENCHMARK_F(BM_MatEng, AlphaBetaSearch)(benchmark::State& state) {
    // Set up a middlegame position
    board.setupPositionFromFEN("r1bq1rk1/ppp2ppp/2n2n2/2bpp3/4P3/2PP1N2/PP1N1PPP/R1BQKB1R w KQ - 0 1");
    for (auto _ : state) {
        benchmark::DoNotOptimize(engine->findBestMove(board, currClock));
    }
}

// Benchmark mate in one search
BENCHMARK_F(BM_MatEng, MateInOneSearch)(benchmark::State& state) {
    // Scholar's mate position
    board.setupPositionFromFEN("r1bqkbnr/pppp1ppp/2n5/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR w KQkq - 0 1");
    for (auto _ : state) {
        benchmark::DoNotOptimize(engine->findBestMove(board, currClock));
    }
}

// Benchmark endgame position evaluation
BENCHMARK_F(BM_MatEng, EndgameEval)(benchmark::State& state) {
    // Complex endgame position
    board.setupPositionFromFEN("4k3/4P3/4K3/8/8/8/8/8 w - - 0 1");
    for (auto _ : state) {
        benchmark::DoNotOptimize(engine->evaluatePosition(board));
    }
}

// Benchmark move generation impact
BENCHMARK_F(BM_MatEng, MoveGen)(benchmark::State& state) {
    int moveNum = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            MoveGenerator::generateLegalMoves(board, moveNum)
        );
    }
}

// Profile time spent in different evaluation components
void BM_EvalComponents(benchmark::State& state) {
    PEXT::initialize();
    MaterialEngine engine;
    ChessBoard board;
    board.setupPositionFromFEN("r1bq1rk1/ppp2ppp/2n2n2/2bpp3/4P3/2PP1N2/PP1N1PPP/R1BQKB1R w KQ - 0 1");
    
    for (auto _ : state) {
        // Material count
        auto start = std::chrono::high_resolution_clock::now();
        benchmark::DoNotOptimize(engine.evaluatePosition(board));
        auto end = std::chrono::high_resolution_clock::now();
        
        // Report time for this iteration
        auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        state.SetIterationTime(elapsed.count() * 1e-9);
    }
}
BENCHMARK(BM_EvalComponents)->UseManualTime();