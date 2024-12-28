#pragma once

#include "chess_engine_base.hpp"
#include <algorithm>

class MaterialEngine : public ChessEngineBase {
private:
    // Piece values (centipawns)
    static constexpr int PAWN_VALUE = 100;
    static constexpr int KNIGHT_VALUE = 320;
    static constexpr int BISHOP_VALUE = 330;
    static constexpr int ROOK_VALUE = 500;
    static constexpr int QUEEN_VALUE = 900;
    static constexpr int KING_VALUE = 20000;

    // Additional positional bonus/penalty
    static constexpr int DOUBLED_PAWN_PENALTY = -10;
    static constexpr int ISOLATED_PAWN_PENALTY = -20;
    static constexpr int BISHOP_PAIR_BONUS = 30;
    static constexpr int ROOK_OPEN_FILE_BONUS = 25;

public:
    MaterialEngine() 
        : ChessEngineBase("MaterialEngine", "0.1", "Cameron Cunningham", 3) {}

    Move findBestMove(const ChessBoard& board, 
                     const GameState& state,
                     int maxDepth = -1) override {
        startSearch();
        int actualDepth = (maxDepth > 0) ? maxDepth : searchDepth;
        
        // Generate all legal moves
        std::vector<Move> moves = generatePsuedoMoves(board, &state);
        MoveValidator validator(const_cast<ChessBoard&>(board), 
                              const_cast<GameState*>(&state));
        
        int bestScore = state.sideToMove == WHITE ? -999999 : 999999;
        Move bestMove;

        // Evaluate each move
        for (const Move& move : moves) {
            if (!validator.isMoveLegal(move)) continue;

            // Make move on temporary board
            ChessBoard tempBoard = board;
            GameState tempState = state;
            makeMove(tempBoard, move);
            validator.updateGameState(move);

            // Evaluate resulting position
            int score = -alphaBeta(tempBoard, tempState, actualDepth - 1, 
                                 -999999, 999999, state.sideToMove == BLACK);

            // Update best move if better score found
            if ((state.sideToMove == WHITE && score > bestScore) ||
                (state.sideToMove == BLACK && score < bestScore)) {
                bestScore = score;
                bestMove = move;
            }
        }

        this->bestMove = bestMove;
        endSearch();
        return bestMove;
    }

    int evaluatePosition(const ChessBoard& board, const GameState& state) override {
        int score = 0;

        // Material count
        score += countMaterial(board, WHITE) - countMaterial(board, BLACK);

        // Positional evaluation
        score += evaluatePositional(board, WHITE) - evaluatePositional(board, BLACK);

        return state.sideToMove == WHITE ? score : -score;
    }

private:
    int countMaterial(const ChessBoard& board, Color color) {
        int score = 0;
        U64 pieces;

        // Pawns
        pieces = color == WHITE ? board.getWhitePawns() : board.getBlackPawns();
        score += popcount(pieces) * PAWN_VALUE;

        // Knights
        pieces = color == WHITE ? board.getWhiteKnights() : board.getBlackKnights();
        score += popcount(pieces) * KNIGHT_VALUE;

        // Bishops
        pieces = color == WHITE ? board.getWhiteBishops() : board.getBlackBishops();
        score += popcount(pieces) * BISHOP_VALUE;

        // Rooks
        pieces = color == WHITE ? board.getWhiteRooks() : board.getBlackRooks();
        score += popcount(pieces) * ROOK_VALUE;

        // Queens
        pieces = color == WHITE ? board.getWhiteQueens() : board.getBlackQueens();
        score += popcount(pieces) * QUEEN_VALUE;

        return score;
    }

    int evaluatePositional(const ChessBoard& board, Color color) {
        int score = 0;

        // Bishop pair bonus
        U64 bishops = color == WHITE ? board.getWhiteBishops() : board.getBlackBishops();
        if (popcount(bishops) >= 2) {
            score += BISHOP_PAIR_BONUS;
        }

        // Evaluate pawn structure
        U64 pawns = color == WHITE ? board.getWhitePawns() : board.getBlackPawns();
        while (pawns) {
            int square = std::countr_zero(pawns);
            
            // Check for doubled pawns
            int file = square % 8;
            U64 fileMask = 0x0101010101010101ULL << file;
            if (popcount(fileMask & pawns) > 1) {
                score += DOUBLED_PAWN_PENALTY;
            }

            // Check for isolated pawns
            U64 adjacentFiles = 0;
            if (file > 0) adjacentFiles |= 0x0101010101010101ULL << (file - 1);
            if (file < 7) adjacentFiles |= 0x0101010101010101ULL << (file + 1);
            if (!(adjacentFiles & pawns)) {
                score += ISOLATED_PAWN_PENALTY;
            }

            pawns &= pawns - 1;  // Clear least significant bit
        }

        return score;
    }

    // Alpha-beta search implementation
    int alphaBeta(const ChessBoard& board, const GameState& state, 
                 int depth, int alpha, int beta, bool maximizing) {
        if (depth == 0 || !isSearching) {
            return evaluatePosition(board, state);
        }

        std::vector<Move> moves = generatePsuedoMoves(board, &state);
        MoveValidator validator(const_cast<ChessBoard&>(board), 
                              const_cast<GameState*>(&state));

        if (maximizing) {
            int maxEval = -999999;
            for (const Move& move : moves) {
                if (!validator.isMoveLegal(move)) continue;

                ChessBoard tempBoard = board;
                GameState tempState = state;
                makeMove(tempBoard, move);
                validator.updateGameState(move);

                int eval = alphaBeta(tempBoard, tempState, depth - 1, alpha, beta, false);
                maxEval = std::max(maxEval, eval);
                alpha = std::max(alpha, eval);
                if (beta <= alpha) break;
            }
            return maxEval;
        } else {
            int minEval = 999999;
            for (const Move& move : moves) {
                if (!validator.isMoveLegal(move)) continue;

                ChessBoard tempBoard = board;
                GameState tempState = state;
                makeMove(tempBoard, move);
                validator.updateGameState(move);

                int eval = alphaBeta(tempBoard, tempState, depth - 1, alpha, beta, true);
                minEval = std::min(minEval, eval);
                beta = std::min(beta, eval);
                if (beta <= alpha) break;
            }
            return minEval;
        }
    }
};