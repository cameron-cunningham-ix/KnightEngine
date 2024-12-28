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

    Move findBestMove(const ChessBoard& board, 
                     const GameState& state,
                     int maxDepth = -1) override {
        startSearch();

        // Generate all legal moves
        std::vector<Move> moves = generatePsuedoMoves(board, &state);
        std::vector<Move> legalMoves;
        MoveValidator validator(const_cast<ChessBoard&>(board), 
                              const_cast<GameState*>(&state));

        // Filter to legal moves only
        for (const Move& move : moves) {
            if (validator.isMoveLegal(move)) {
                legalMoves.push_back(move);
            }
        }

        // Select random move
        if (!legalMoves.empty()) {
            std::uniform_int_distribution<size_t> dist(0, legalMoves.size() - 1);
            bestMove = legalMoves[dist(rng)];
        }

        endSearch();
        return bestMove;
    }

    // Simple material count evaluation - not really used but required by interface
    int evaluatePosition(const ChessBoard& board, const GameState& state) override {
        return 0;  // Random engine doesn't evaluate positions
    }

    // setOption is not needed for this engine
    bool setOption(const std::string& name, const std::string& value) override {
        return false;
    }
};