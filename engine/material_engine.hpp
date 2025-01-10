#pragma once

#include "chess_engine_base.hpp"
#include "board_utility.hpp"
#include <immintrin.h>
#include <algorithm>

/// @brief 
class MaterialEngine : public ChessEngineBase {
private:
    // Piece values (centipawns)
    static constexpr int PAWN_VALUE = 100;
    static constexpr int KNIGHT_VALUE = 320;
    static constexpr int BISHOP_VALUE = 330;
    static constexpr int ROOK_VALUE = 500;
    static constexpr int QUEEN_VALUE = 900;
    static constexpr int KING_VALUE = 2000;

    // Additional positional bonus/penalty
    static constexpr int DOUBLED_PAWN_PENALTY = -10;
    static constexpr int ISOLATED_PAWN_PENALTY = -20;
    static constexpr int CHECKED_PENALTY = -300;
    static constexpr int BISHOP_PAIR_BONUS = 30;
    static constexpr int ROOK_OPEN_FILE_BONUS = 25;

public:
    MaterialEngine() 
        : ChessEngineBase("MaterialEngine", "0.2", "Cameron Cunningham", BLACK, 4) {}

    DenseMove findBestMove(ChessBoard& board, 
                           int maxDepth = -1) override {
        startSearch();
        int actualDepth = (maxDepth > 0) ? maxDepth : searchDepth;

        // Generate all legal moves
        int moveNum = 0;
        std::array<DenseMove, MAX_MOVES> moves = MoveGenerator::generateLegalMoves(board, moveNum);
        
        // 
        int bestScore = board.currentGameState.sideToMove == getSide() ? -999999 : 999999;
        DenseMove bestMove;

        // Evaluate each move
        for (int i = 0; i < moveNum; i++) {
            // Make move on temporary board
            ChessBoard tempBoard = board;
            tempBoard.makeMove(moves[i], true);

            // Evaluate resulting position
            int score = -alphaBeta(tempBoard, actualDepth - 1, 
                                 -999999, 999999, board.currentGameState.sideToMove == getSide());

            // Update best move if better score found
            if ((board.currentGameState.sideToMove == getSide() && score > bestScore) ||
                (board.currentGameState.sideToMove != getSide() && score < bestScore)) {
                bestScore = score;
                bestMove = moves[i];
            }
        }

        this->bestMove = bestMove;
        endSearch();
        return bestMove;
    }

    int evaluatePosition(const ChessBoard& board) override {
        int score = 0;

        // Material count
        score += countMaterial(board, getSide()) - countMaterial(board, (Color)!getSide());

        // Positional evaluation
        score += evaluatePositional(board, getSide()) - evaluatePositional(board, (Color)!getSide());

        return board.currentGameState.sideToMove == getSide() ? score : -score;
    }

private:
    int countMaterial(const ChessBoard& board, Color color) {
        int score = 0;
        U64 pieces;

        if (color == WHITE) {
            // Pawns
            pieces = board.getWhitePawns();
            score += popcount(pieces) * PAWN_VALUE;
            // Knights
            pieces = board.getWhiteKnights();
            score += popcount(pieces) * KNIGHT_VALUE;
            // Bishops
            pieces = board.getWhiteBishops();
            score += popcount(pieces) * BISHOP_VALUE;
            // Rooks 
            pieces = board.getWhiteRooks();
            score += popcount(pieces) * ROOK_VALUE;
            // Queens
            pieces = board.getWhiteQueens();
            score += popcount(pieces) * QUEEN_VALUE;
        } else {
            // Pawns
            pieces = board.getBlackPawns();
            score += popcount(pieces) * PAWN_VALUE;
            // Knights
            pieces = board.getBlackKnights();
            score += popcount(pieces) * KNIGHT_VALUE;
            // Bishops
            pieces = board.getBlackBishops();
            score += popcount(pieces) * BISHOP_VALUE;
            // Rooks 
            pieces = board.getBlackRooks();
            score += popcount(pieces) * ROOK_VALUE;
            // Queens
            pieces = board.getBlackQueens();
            score += popcount(pieces) * QUEEN_VALUE;
        }

        return score;
    }

    int evaluatePositional(const ChessBoard& board, Color color) {
        int score = 0;
        if (color == WHITE) {
            // Bishop pair bonus
            U64 bishops = board.getWhiteBishops();

            /// @todo Consider light square / dark square mask to determine real pair
            if (popcount(bishops) >= 2) {
                score += BISHOP_PAIR_BONUS;
            }

            // Evaluate pawn structure
            U64 pawns = board.getWhitePawns();
            while (pawns) {
                int square = std::countr_zero(pawns);

                // Check for doubled pawns (more than one pawn in a file)
                int file = BUTIL::squareToFileIndex(square);
                U64 fileMask = BUTIL::FileMask << file;
                if (popcount(fileMask & pawns) > 1) {
                    score += DOUBLED_PAWN_PENALTY;
                }

                // Check for isolated pawns
                U64 adjacentFiles = 0;
                if (file > 0) adjacentFiles |= BUTIL::FileMask << (file - 1);
                if (file < 7) adjacentFiles |= BUTIL::FileMask << (file + 1);
                // No pawns in adjacent files
                if (!(adjacentFiles & pawns)) {
                    score += ISOLATED_PAWN_PENALTY;
                }

                pawns &= pawns - 1;
            }

            // Evaluate attacks to opposite king
            U64 attackingOppKing = board.OppAttacksToSquare(board.getBlackKingSquare(), BLACK);
            if (attackingOppKing) {
                // Using popcount means double checks should be worth more
                score += KING_VALUE * popcount(attackingOppKing);
            }

            // Evaluate attacks to own king
            // Discourage getting checked
            U64 attacksToKing = board.OppAttacksToSquare(board.getWhiteKingSquare(), WHITE);
            if (attacksToKing) {
                score += CHECKED_PENALTY * popcount(attacksToKing);
            }
        } else {
            // Bishop pair bonus
            U64 bishops = board.getBlackBishops();

            /// @todo Consider light square / dark square mask to determine real pair
            if (popcount(bishops) >= 2) {
                score += BISHOP_PAIR_BONUS;
            }

            // Evaluate pawn structure
            U64 pawns = board.getBlackPawns();
            while (pawns) {
                int square = std::countr_zero(pawns);

                // Check for doubled pawns (more than one pawn in a file)
                int file = BUTIL::squareToFileIndex(square);
                U64 fileMask = BUTIL::FileMask << file;
                if (popcount(fileMask & pawns) > 1) {
                    score += DOUBLED_PAWN_PENALTY;
                }

                // Check for isolated pawns
                U64 adjacentFiles = 0;
                if (file > 0) adjacentFiles |= BUTIL::FileMask << (file - 1);
                if (file < 7) adjacentFiles |= BUTIL::FileMask << (file + 1);
                // No pawns in adjacent files
                if (!(adjacentFiles & pawns)) {
                    score += ISOLATED_PAWN_PENALTY;
                }

                pawns &= pawns - 1;
            }

            // Evaluate attacks to opposite king
            U64 attackingOppKing = board.OppAttacksToSquare(board.getWhiteKingSquare(), WHITE);
            if (attackingOppKing) {
                // Using popcount means double checks should be worth more
                score += KING_VALUE * popcount(attackingOppKing);
            }

            // Evaluate attacks to own king
            // Discourage getting checked
            U64 attacksToKing = board.OppAttacksToSquare(board.getBlackKingSquare(), BLACK);
            if (attacksToKing) {
                score += CHECKED_PENALTY * popcount(attacksToKing);
            }
        }
        


        return score;
    }

    // Alpha-beta search implementation
    int alphaBeta(ChessBoard& board, int depth, int alpha, 
                  int beta, bool maximizing) {
        if (depth == 0 || !isSearching) {
            return evaluatePosition(board);
        }

        int moveNum = 0;
        std::array<DenseMove, MAX_MOVES> moves = MoveGenerator::generateLegalMoves(board, moveNum);

        if (maximizing) {
            int maxEval = -999999;
            for (int i = 0; i < moveNum; i++) {
                ChessBoard tempBoard = board;
                tempBoard.makeMove(moves[i], true);

                int eval = alphaBeta(tempBoard, depth - 1, alpha, beta, false);
                maxEval = std::max(maxEval, eval);
                alpha = std::max(alpha, eval);
                if (beta <= alpha) break;
            }
            return maxEval;
        } else {
            int minEval = 999999;
            for (int i = 0; i < moveNum; i++) {
                ChessBoard tempBoard = board;
                tempBoard.makeMove(moves[i], true);

                int eval = alphaBeta(tempBoard, depth - 1, alpha, beta, true);
                minEval = std::min(minEval, eval);
                beta = std::min(beta, eval);
                if (beta <= alpha) break;
            }
            return minEval;
        }
    }
};