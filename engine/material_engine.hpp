#pragma once

#include "chess_engine_base.hpp"
#include "board_utility.hpp"
#include <immintrin.h>
#include <algorithm>
#include <chrono>


static constexpr int INF_POS = 999999999;
static constexpr int INF_NEG = -999999999;

/// @brief 
class MaterialEngine : public ChessEngineBase {
private:
    // Search statistics
    U64 nodeCount;
    std::chrono::steady_clock::time_point searchStartTime;
    DenseMove currentMove;
    int currentMoveNumber;

    // Piece values (centipawns)
    static constexpr int PAWN_VALUE = 100;
    static constexpr int KNIGHT_VALUE = 320;
    static constexpr int BISHOP_VALUE = 330;
    static constexpr int ROOK_VALUE = 500;
    static constexpr int QUEEN_VALUE = 900;
    static constexpr int KING_VALUE = 2000;

    // Additional positional bonus/penalty
    static constexpr int MATE_SCORE = 100000;
    static constexpr int SUPPORTED_PAWN_BONUS = 90;
    static constexpr int DOUBLED_PAWN_PENALTY = -50;
    static constexpr int ISOLATED_PAWN_PENALTY = -80;
    static constexpr int CHECKED_PENALTY = -10000;
    static constexpr int CHECKING_BONUS = 15000;
    static constexpr int BISHOP_PAIR_BONUS = 150;
    static constexpr int ROOK_OPEN_FILE_BONUS = 250;

public:

    MaterialEngine() 
        : ChessEngineBase("MaterialEngine", "0.4", "Cameron Cunningham", 6) {}

    DenseMove findBestMove(ChessBoard& board, 
                           int maxDepth = -1) override {
        startSearch();
        searchStartTime = std::chrono::steady_clock::now();
        nodeCount = 0;
        currentMoveNumber = 0;

        int actualDepth = (maxDepth > 0) ? maxDepth : searchDepth;
        sendInfo(std::format("search to depth {}", actualDepth));

        // Generate all psuedo legal moves
        int moveNum = 0;
        std::array<DenseMove, MAX_MOVES> moves = MoveGenerator::generateLegalMoves(board, moveNum);
        Color sideToMove = board.getSideToMove();
        // bestScore will determine what the best move to play is
        // Negative bestScore means Black's position is better, positive means White's position is better
        int bestScore = sideToMove == WHITE ? INF_NEG : INF_POS;
        DenseMove bestMove;

        // Evaluate each move
        for (int i = 0; i < moveNum; i++) {
            currentMoveNumber = i + 1;
            currentMove = moves[i];

            // Send current move info
            sendInfo(std::format("currmove {} currmovenumber {}",
                                currentMove.toAlgebraic(), currentMoveNumber));

            // Make move on temporary board
            ChessBoard tempBoard = board;
            tempBoard.makeMove(moves[i], true);

            // // Check if move is illegal
            // if (tempBoard.isSideInCheck(sideToMove)) {
            //     std::cout << "go next\n";
            //     continue;
            // }

            // Store start time for this move
            auto moveStartTime = std::chrono::steady_clock::now();

            // Evaluate resulting position
            int score = alphaBeta(tempBoard, actualDepth - 1, 
                                 INF_NEG, INF_POS, board.getSideToMove() == WHITE);
            // Update best move if better score found
            if ((sideToMove == WHITE && score > bestScore) ||
                (sideToMove == BLACK && score < bestScore)) {
                bestScore = score;
                bestMove = moves[i];

                auto moveTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - searchStartTime).count();
                
                // Send updated info
                std::string infoStr = std::format("depth {} score cp {} time {} nodes {}",
                    actualDepth, bestScore, moveTime, nodeCount);
                
                // Add NPS if we have meaningful time elapsed
                if (moveTime > 0) {
                    U64 nps = (nodeCount * 1000) / moveTime;
                    infoStr += std::format("nps {} ", nps);
                }

                sendInfo(infoStr);
            }
        }

        this->bestMove = bestMove;
        endSearch();
        return bestMove;
    }

    int evaluatePosition(const ChessBoard& board) override {
        nodeCount++;
        // Send node count every 60K nodes
        if (nodeCount % 60000 == 0) {
            auto time = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - searchStartTime).count();
            if (time > 0) {
                long long nps = (nodeCount * 1000) / time;
                sendInfo(std::format("nodes {} nps {}", nodeCount, nps));
            }
        }

        int score = 0;

        // Material count
        score += countMaterial(board, WHITE);
        score -= countMaterial(board, BLACK);

        // Positional evaluation
        score += evaluatePositional(board, WHITE);
        score -= evaluatePositional(board, BLACK);

        return score;
    }

private:
    void sendInfo(const std::string& info) {
        std::cout << "info " << info << std::endl;
    }

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

                // If this pawn is supporting another
                U64 supports = ATKMASK_WPAWN[square] & pawns;
                if (supports) {
                    score += SUPPORTED_PAWN_BONUS * popcount(supports);
                }

                pawns &= pawns - 1;
            }

            // Evaluate attacks to opposite king
            /// @todo apparently this does fuck all to encourage checks
            U64 attackingOppKing = board.OppAttacksToSquare(board.getBlackKingSquare(), BLACK);
            if (attackingOppKing) {
                // Using popcount means double checks should be worth more
                score += CHECKING_BONUS * popcount(attackingOppKing);
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

                // If this pawn is supporting another
                U64 supports = ATKMASK_BPAWN[square] & pawns;
                if (supports) {
                    score += SUPPORTED_PAWN_BONUS * popcount(pawns);
                }

                pawns &= pawns - 1;
            }

            // Evaluate attacks to opposite king
            U64 attackingOppKing = board.OppAttacksToSquare(board.getWhiteKingSquare(), WHITE);
            if (attackingOppKing) {
                // Using popcount means double checks should be worth more
                score += CHECKING_BONUS * popcount(attackingOppKing);
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

    /// @brief Alpha-Beta search algorithm.
    /// This is a recursively called function used to score a chess position.
    /// Positive means the position is better for White, negative better for Black
    /// @param board Current board to search
    /// @param depth Depth to search to. Position is evaluated at depth 0
    /// @param alpha The minimum score the maximizing player (W) is guranteed
    /// in the position
    /// @param beta The maximum score the minimizing player (B) is guranteed
    /// in the position
    /// @param maximizing 
    /// @return 
    int alphaBeta(ChessBoard& board, int depth, int alpha, 
                  int beta, bool maximizing) {
        if (depth == 0 || !isSearching) {
            return evaluatePosition(board);
        }

        int moveNum = 0;
        std::array<DenseMove, MAX_MOVES> moves = MoveGenerator::generatePsuedoMoves(board, moveNum);

        bool noLegalMoves = true;

        // // No legal moves
        // if (moveNum == 0) {
        //     // If in check, this is checkmate
        //     if (board.isSideInCheck((Color)maximizing)) {
        //         /// @todo Figure out why maximizing side wants big score?
        //         return maximizing ? 999999 : -999999;
        //     }
        //     // Otherwise stalemate
        //     return 0;
        // }

        // White to move
        // Trying to improve alpha value by finding moves that give a higher score
        if (maximizing) {
            // maxEval is the best score that's been found so far in this node
            int maxEval = INF_NEG;
            ChessBoard tempBoard = board;
            // Test every move in the position
            for (int i = 0; i < moveNum; i++) {
                tempBoard.makeMove(moves[i], true);

                // Check move legality
                if (tempBoard.isSideInCheck(WHITE)) {
                    // isMate = false;
                    tempBoard.unmakeMove(moves[i], true);
                    continue;
                }
                noLegalMoves = false;
                // Evaluation of next alphabeta is minimizing
                int eval = alphaBeta(tempBoard, depth - 1, alpha, beta, false);

                tempBoard.unmakeMove(moves[i], true);
                // If eval is greater than any other score so far in this node,
                // maxEval gets set to eval
                if (eval > maxEval) maxEval = eval;

                // If maxEval is now greater than alpha (best maximizing score 
                // guranteed across the search tree so far), set alpha to eval
                if (maxEval > alpha) alpha = maxEval;

                // If alpha is now so good that it's greater than beta (best minimizing 
                // score guaranteed across the search tree so far), then minimizing
                // player will never allow this position to be reached if playing optimally
                // and we can return alpha early
                if (alpha >= beta) return alpha;
            }
            // If there was at least 1 legal move, return eval
            if (!noLegalMoves)
                return maxEval;
            else {
                // No legal moves means we're either in checkmate or stalemate
                if (board.isSideInCheck(WHITE)) {
                    // If we're trying to maximize White's score and are in checkmate,
                    // this is the worst possible outcome and should be avoided for White
                    // and wanted for Black
                    // Subtract depth so Black prefers faster checkmates
                    return -MATE_SCORE - depth;
                }
                // Otherwise stalemate
                return 0;
            }
        } 
        // Black to move
        // Trying to improve beta value by finding moves that give a lower score
        else {
            // minEval is the lowest score that's been found so far in this node
            int minEval = INF_POS;
            ChessBoard tempBoard = board;
            // Test every move in the position
            for (int i = 0; i < moveNum; i++) {
                tempBoard.makeMove(moves[i], true);

                // Check move legality
                if (tempBoard.isSideInCheck(BLACK)) {
                    tempBoard.unmakeMove(moves[i], true);
                    continue;
                }
                noLegalMoves = false;
                // Evaluation of next alphabeta is maximizing
                int eval = alphaBeta(tempBoard, depth - 1, alpha, beta, true);

                // If eval is less than any other score so far in this node,
                // minEval gets set to eval
                if (eval < minEval) minEval = eval;

                // If minEval is now less than beta (best minimizing score guranteed across
                // the search tree so far), set beta to minEval
                if (minEval < beta) beta = minEval;

                // If beta is now so good that it's less than alpha (best maximizing score
                // guranteed across the search tree so far), then maximizing player
                // will never allow this position to be reached if playing optimally
                if (beta <= alpha) return beta;

            }
            // If there was at least 1 legal move, return eval
            if (!noLegalMoves)
                return minEval;
            else {
                // No legal moves means we're either in checkmate or stalemate
                if (board.isSideInCheck(BLACK)) {
                    // If we're trying to minimize Black's score and are in checkmate,
                    // this is the worst possible outcome and should be avoided for Black
                    // and wanted for White
                    // Add depth so White prefers faster checkmates
                    return MATE_SCORE + depth;
                }
                // Otherwise stalemate
                return 0;
            }
            return minEval;
        }
    }
};