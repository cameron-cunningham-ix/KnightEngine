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
    int totalPiecesWithoutPawns;

    // Piece-Square tables
    // Pawns - Early game
    static constexpr std::array<int, 64> pawnSqTbEarly = {
         0,  0,  0,  0,  0,  0,  0,  0,
        50, 50, 50, 50, 50, 50, 50, 50,
        20, 20, 25, 25, 25, 25, 20, 20,
         5,  5, 10, 20, 20, 10,  5,  5,
         0,  0,  0, 20, 20,  0,  0,  0,
         5, -5,-10,  0,  0,-10, -5,  5,
         5, 15, 15,-10,-10, 15, 15,  5,
         0,  0,  0,  0,  0,  0,  0,  0
    };
    // Knights - Early game
    static constexpr std::array<int, 64> knightSqTbEarly = {
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 15, 15, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50
    };
    // Bishops - Early game
    static constexpr std::array<int, 64> bishopSqTbEarly = {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20
    };
    // Rooks - Early game
    static constexpr std::array<int, 64> rookSqTbEarly = {
        0,  0,  0,  0,  0,  0,  0,  0,
        5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        0,  0,  0,  5,  5,  0,  0,  0
    };
    // Queens - Early game
    static constexpr std::array<int, 64> queenSqTbEarly = {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -5,  0,  5,  5,  5,  5,  0, -5,
        0,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    };
    // King - Early game
    static constexpr std::array<int, 64> kingSqTbEarly = {
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
        20, 20,  0,  0,  0,  0, 20, 20,
        20, 30, 10,  0,  0, 10, 30, 20
    };
    // Piece-Square tables for endgame
    // Pawns - Endgame
    static constexpr std::array<int, 64> pawnSqTbEnd = {
        0,  0,  0,  0,  0,  0,  0,  0,
        80, 80, 80, 80, 80, 80, 80, 80,
        50, 50, 50, 50, 50, 50, 50, 50,
        30, 30, 30, 30, 30, 30, 30, 30,
        20, 20, 20, 20, 20, 20, 20, 20,
        10, 10, 10, 10, 10, 10, 10, 10,
        10, 10, 10, 10, 10, 10, 10, 10,
        0,  0,  0,  0,  0,  0,  0,  0
    };
    // Knights - Endgame
    static constexpr std::array<int, 64> knightSqTbEnd = {
        -40,-30,-20,-20,-20,-20,-30,-40,
        -30,-20,  0,  0,  0,  0,-20,-30,
        -20,  0, 10, 15, 15, 10,  0,-20,
        -20,  5, 15, 20, 20, 15,  5,-20,
        -20,  0, 15, 20, 20, 15,  0,-20,
        -20,  5, 10, 15, 15, 10,  5,-20,
        -30,-20,  0,  5,  5,  0,-20,-30,
        -40,-30,-20,-20,-20,-20,-30,-40
    };
    // Bishops - Endgame
    static constexpr std::array<int, 64> bishopSqTbEnd = {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20
    };
    // Rooks - Endgame
    static constexpr std::array<int, 64> rookSqTbEnd = {
        0,  0,  0,  0,  0,  0,  0,  0,
        5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        0,  0,  0,  5,  5,  0,  0,  0
    };
    // Queens - Endgame
    static constexpr std::array<int, 64> queenSqTbEnd = {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -10,  5,  5,  5,  5,  5,  0,-10,
        0,  0,  5,  5,  5,  5,  0, -5,
        -5,  0,  5,  5,  5,  5,  0, -5,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    };
    // King - Endgame
    static constexpr std::array<int, 64> kingSqTbEnd = {
        -50,-40,-30,-20,-20,-30,-40,-50,
        -30,-20,-10,  0,  0,-10,-20,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-30,  0,  0,  0,  0,-30,-30,
        -50,-30,-30,-30,-30,-30,-30,-50
    };

    // Piece values (centipawns)
    static constexpr int PAWN_VALUE = 100;
    static constexpr int KNIGHT_VALUE = 320;
    static constexpr int BISHOP_VALUE = 330;
    static constexpr int ROOK_VALUE = 500;
    static constexpr int QUEEN_VALUE = 900;
    static constexpr int KING_VALUE = 2000;

    // Additional positional bonus/penalty
    static constexpr int ENDGAME_LERP = 14;
    static constexpr int MATE_SCORE = 100000;
    static constexpr int SUPPORTED_PAWN_BONUS = 90;
    static constexpr int SUPPORTING_PAWN_BONUS = 75;
    static constexpr int SUPPORTING_PIECE_BONUS = 100;
    static constexpr int DOUBLED_PAWN_PENALTY = -70;
    static constexpr int ISOLATED_PAWN_PENALTY = -80;
    static constexpr int CHECKED_PENALTY = -1000;
    static constexpr int CHECKING_BONUS = 1500;
    static constexpr int BISHOP_PAIR_BONUS = 150;
    static constexpr int ROOK_OPEN_FILE_BONUS = 250;

public:


    MaterialEngine() 
        : ChessEngineBase("MaterialEngine", "0.41", "Cameron Cunningham", 6) {}

    DenseMove findBestMove(ChessBoard& board, 
                           int maxDepth = -1) override {
        startSearch();
        searchStartTime = std::chrono::steady_clock::now();
        nodeCount = 0;
        currentMoveNumber = 0;

        int actualDepth = (maxDepth > 0) ? maxDepth : searchDepth;

        // Generate all psuedo legal moves
        int moveNum = 0;
        std::array<DenseMove, MAX_MOVES> moves = MoveGenerator::generateLegalMoves(board, moveNum);
        Color sideToMove = board.getSideToMove();
        
        int alpha = INF_NEG;
        int beta = INF_POS;
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

            // Store start time for this move
            auto moveStartTime = std::chrono::steady_clock::now();

            // Evaluate resulting position
            int score = alphaBeta(tempBoard, actualDepth - 1, 
                                 alpha, beta, tempBoard.getSideToMove() == WHITE);
            // Update best move if better score found
            if ((sideToMove == WHITE && score > bestScore) ||
                (sideToMove == BLACK && score < bestScore)) {
                bestScore = score;
                bestMove = moves[i];

                if (sideToMove == WHITE) {
                    if (score > alpha) alpha = score;
                } else {
                    if (score < beta) beta = score;
                }

                auto moveTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - searchStartTime).count();
                
                // Send updated info
                std::string infoStr = std::format("depth {} score cp {} time {} nodes {} ",
                    actualDepth, bestScore, moveTime, nodeCount, bestMove.toAlgebraic());
                
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
        // Get total piece count (not including pawns or kings)
        // for endgame lerp
        totalPiecesWithoutPawns = popcount(board.getAllPieces() & 
                                        (~board.getDenseSet(D_PAWN) | 
                                         ~board.getDenseSet(D_KING)));
        return score;
    }

    int evaluatePositional(const ChessBoard& board, Color color) {
        int score = 0;
        int earlygameLerp = totalPiecesWithoutPawns/ENDGAME_LERP;
        int endgameLerp = (std::clamp(14 - totalPiecesWithoutPawns, 0, 14))/ENDGAME_LERP;
        if (color == WHITE) {
            // Evaluate pawn structure
            U64 pawns, pawnRef = board.getWhitePawns();
            while (pawns) {
                int square = std::countr_zero(pawns);

                // Add score from piece-square table
                // Lerp from early to endgame based on total piece count
                // not including pawns
                score +=  pawnSqTbEarly[square]*(earlygameLerp) +
                    pawnSqTbEnd[square]*(endgameLerp);

                // Check for doubled pawns (more than one pawn in a file)
                int file = BUTIL::squareToFileIndex(square);
                U64 fileMask = BUTIL::FileMask << file;
                if (popcount(fileMask & pawnRef) > 1) {
                    score += DOUBLED_PAWN_PENALTY;
                }

                // Check for isolated pawns
                U64 adjacentFiles = 0;
                if (file > 0) adjacentFiles |= BUTIL::FileMask << (file - 1);
                if (file < 7) adjacentFiles |= BUTIL::FileMask << (file + 1);
                // No pawns in adjacent files
                if (!(adjacentFiles & pawnRef)) {
                    score += ISOLATED_PAWN_PENALTY;
                }

                // If this pawn is supporting another pawn
                U64 supports = ATKMASK_WPAWN[square] & pawnRef;
                if (supports) {
                    score += SUPPORTING_PAWN_BONUS * popcount(supports);
                }
                // If this pawn is supported by other pawns
                U64 supported = ATKMASK_BPAWN[square] & pawnRef;
                if (supported) {
                    score += SUPPORTED_PAWN_BONUS * popcount(supported);
                }
                // If this pawn is supporting a piece
                supports = ATKMASK_WPAWN[square] & (board.getWhitePieces() & ~pawnRef);
                if (supports) {
                    score += SUPPORTING_PIECE_BONUS * popcount(supports);
                }
                pawns &= pawns - 1;
            }
            // Knights
            U64 knights = board.getWhiteKnights();
            while (knights) {
                int square = std::countr_zero(knights);
                // Add score from piece-square table
                // Lerp from early to endgame based on total piece count
                score +=  knightSqTbEarly[square]*(earlygameLerp) +
                    knightSqTbEnd[square]*(endgameLerp);
                knights &= knights - 1;
            }
            // Bishops
            U64 bishops = board.getWhiteBishops();
            // Bishop pair bonus
            /// @todo Consider light square / dark square mask to determine real pair
            if (popcount(bishops) >= 2) {
                score += BISHOP_PAIR_BONUS;
            }
            while (bishops) {
                int square = std::countr_zero(bishops);
                // Add score from piece-square table
                // Lerp from early to endgame based on total piece count
                score +=  bishopSqTbEarly[square]*(earlygameLerp) +
                    bishopSqTbEnd[square]*(endgameLerp);
                bishops &= bishops - 1;
            }
            // Rooks
            U64 rooks = board.getWhiteRooks();
            while (rooks) {
                int square = std::countr_zero(rooks);
                score += rookSqTbEarly[square]*(earlygameLerp) +
                    rookSqTbEnd[square]*(endgameLerp);
                rooks &= rooks - 1;
            }
            // Queens
            U64 queens = board.getWhiteQueens();
            while (queens) {
                int square = std::countr_zero(queens);
                score += queenSqTbEarly[square]*(earlygameLerp) +
                    queenSqTbEnd[square]*(endgameLerp);
                queens &= queens - 1;
            }
            // King
            int kingSquare = board.getWhiteKingSquare();
            score += kingSqTbEarly[kingSquare]*(earlygameLerp) +
                kingSqTbEnd[kingSquare]*(endgameLerp);

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
            // Evaluate pawn structure
            U64 pawns, pawnRef = board.getBlackPawns();
            while (pawns) {
                int square = std::countr_zero(pawns);

                // Add score from piece-square table
                // Lerp from early to endgame based on total piece count
                // not including pawns
                // For now 63 - is a hack for not having flipped piece-square tables
                score +=  pawnSqTbEarly[63-square]*(earlygameLerp) +
                    pawnSqTbEnd[63-square]*(endgameLerp);

                // Check for doubled pawns (more than one pawn in a file)
                int file = BUTIL::squareToFileIndex(square);
                U64 fileMask = BUTIL::FileMask << file;
                if (popcount(fileMask & pawnRef) > 1) {
                    score += DOUBLED_PAWN_PENALTY;
                }

                // Check for isolated pawns
                U64 adjacentFiles = 0;
                if (file > 0) adjacentFiles |= BUTIL::FileMask << (file - 1);
                if (file < 7) adjacentFiles |= BUTIL::FileMask << (file + 1);
                // No pawns in adjacent files
                if (!(adjacentFiles & pawnRef)) {
                    score += ISOLATED_PAWN_PENALTY;
                }

                // If this pawn is supporting another
                U64 supports = ATKMASK_BPAWN[square] & pawnRef;
                if (supports) {
                    score += SUPPORTING_PAWN_BONUS * popcount(supports);
                }
                // If this pawn is supported by others
                U64 supported = ATKMASK_WPAWN[square] & pawnRef;
                if (supported) {
                    score += SUPPORTED_PAWN_BONUS * popcount(supported);
                }
                // If this pawn is supporting a piece
                supports = ATKMASK_BPAWN[square] & (board.getBlackPieces() & ~pawnRef);
                if (supports) {
                    score += SUPPORTING_PIECE_BONUS * popcount(supports);
                }

                pawns &= pawns - 1;
            }
            // Knights
            U64 knights = board.getBlackKnights();
            while (knights) {
                int square = std::countr_zero(knights);
                // Add score from piece-square table
                // Lerp from early to endgame based on total piece count
                score +=  knightSqTbEarly[63-square]*(earlygameLerp) +
                    knightSqTbEnd[63-square]*(endgameLerp);
                knights &= knights - 1;
            }
            // Bishops
            U64 bishops = board.getBlackBishops();
            // Bishop pair bonus
            /// @todo Consider light square / dark square mask to determine real pair
            if (popcount(bishops) >= 2) {
                score += BISHOP_PAIR_BONUS;
            }
            while (bishops) {
                int square = std::countr_zero(bishops);
                // Add score from piece-square table
                // Lerp from early to endgame based on total piece count
                score +=  bishopSqTbEarly[63-square]*(earlygameLerp) +
                    bishopSqTbEnd[63-square]*(endgameLerp);
                bishops &= bishops - 1;
            }
            // Rooks
            U64 rooks = board.getBlackRooks();
            while (rooks) {
                int square = std::countr_zero(rooks);
                score += rookSqTbEarly[63-square]*(earlygameLerp) +
                    rookSqTbEnd[63-square]*(endgameLerp);

                rooks &= rooks - 1;
            }
            // Queens
            U64 queens = board.getBlackQueens();
            while (queens) {
                int square = std::countr_zero(queens);
                score += queenSqTbEarly[63-square]*(earlygameLerp) +
                    queenSqTbEnd[63-square]*(endgameLerp);
                queens &= queens - 1;
            }
            // King
            int kingSquare = board.getBlackKingSquare();
            score += kingSqTbEarly[63-kingSquare]*(earlygameLerp) +
                kingSqTbEnd[63-kingSquare]*(endgameLerp);

            // Evaluate attacks to opposite king
            /// @todo apparently this does fuck all to encourage checks
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
                // and we can break early
                if (alpha >= beta) break;
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

                tempBoard.unmakeMove(moves[i], true);

                // If eval is less than any other score so far in this node,
                // minEval gets set to eval
                if (eval < minEval) minEval = eval;

                // If minEval is now less than beta (best minimizing score guranteed across
                // the search tree so far), set beta to minEval
                if (minEval < beta) beta = minEval;

                // If beta is now so good that it's less than alpha (best maximizing score
                // guranteed across the search tree so far), then maximizing player
                // will never allow this position to be reached if playing optimally
                if (beta <= alpha) break;
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