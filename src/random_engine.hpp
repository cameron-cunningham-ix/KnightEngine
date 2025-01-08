#pragma once

#include "chess_engine_base.hpp"
#include <random>

class RandomEngine : public ChessEngineBase {
private:
    std::mt19937 rng;    // Random number generator

public:
    RandomEngine() 
        : ChessEngineBase("RandomEngine", "1.0", "Cameron Cunningham", 1)  // Only needs depth 1
        , rng(std::random_device{}()) {}

    DenseMove findBestMove(ChessBoard& board, 
                           int maxDepth = -1) override {
        startSearch();
        // Generate all legal moves
        std::vector<DenseMove> moves = MoveGenerator::generateLegalMoves(board);

        // Select random move
        if (!moves.empty()) {
            std::uniform_int_distribution<size_t> dist(0, moves.size() - 1);
            bestMove = moves[dist(rng)];
        }

        endSearch();
        return bestMove;
    }

    // Simple material count evaluation - not really used but required by interface
    int evaluatePosition(const ChessBoard& board) override {
        return 0;  // Random engine doesn't evaluate positions
    }

    // setOption is not needed for this engine
    bool setOption(const std::string& name, const std::string& value) override {
        return false;
    }
};