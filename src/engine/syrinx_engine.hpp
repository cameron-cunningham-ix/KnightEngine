#pragma once

#include "chess_engine_base.hpp"
#include "transposition_table.hpp"
#include "board_utility.hpp"
#include "pext_bitboard.hpp"
#include <immintrin.h>
#include <algorithm>
#include <chrono>
#include <unordered_set>

static constexpr int INF_POS = 999999999;
static constexpr int INF_NEG = -999999999;
static constexpr int TT_SIZE = 44739243;

/// @brief 
class Syrinx : public ChessEngineBase {
private:
    // Search statistics

    // Number of nodes searched
    U64 nodeCount;
    // Start time of current search
    std::chrono::steady_clock::time_point searchStartTime;
    // Amount of time previous depth took to search
    std::chrono::milliseconds prevDepthTime;
    DenseMove currentMove;
    int currentMoveNumber;
    int totalPiecesWithoutPawns;
    // Transposition table
    // 44 million 24-byte entries = ~1 GB
    std::array<TTEntry, TT_SIZE> transpositionTable;

    // Piece-Square tables
    // Pawns - Early game
    static constexpr std::array<int, 64> pawnSqTbEarly = {
         0,  0,  0,  0,  0,  0,  0,  0,
        50, 50, 50, 50, 50, 50, 50, 50,
        20, 20, 25, 25, 25, 25, 20, 20,
         5,  5, 10, 20, 20, 10,  5,  5,
         0,  0,  0, 25, 25,  0,  0,  0,
         5, -5,-10,  0,  0,-10, -5,  5,
        15, 25, 15,-10,-10, 15, 25,  15,
         0,  0,  0,  0,  0,  0,  0,  0
    };
    // Knights - Early game
    static constexpr std::array<int, 64> knightSqTbEarly = {
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 15, 15, 15,  5,-30,
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
         0,  0,  0,  5,  0, 10,  0,  0,
         5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
         0,  0,  0, 20,  5, 20,  0,  0
    };
    // Queens - Early game
    static constexpr std::array<int, 64> queenSqTbEarly = {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
         -5,  0,  5,  5,  5,  5,  0, -5,
         -5,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  0,  0,  0,  0,  0,-10,
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
        -10,-20,-20,-20,-20,-20,-20,-10,
         20, 30, 20,  0,  0, 10, 40, 20
    };
    // Pawns - Endgame
    static constexpr std::array<int, 64> pawnSqTbEnd = {
        0,  0,  0,  0,  0,  0,  0,  0,
        80, 80, 80, 80, 80, 80, 80, 80,
        50, 50, 50, 50, 50, 50, 50, 50,
        30, 30, 30, 30, 30, 30, 30, 30,
        15, 15, 15, 15, 15, 15, 15, 15,
        10, 10, 10, 10, 10, 10, 10, 10,
         5,  5,  5,  5,  5,  5,  5,  5,
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
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
         -5,  0,  5,  5,  5,  5,  0, -5,
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

    // Masks of light and dark squares
    static constexpr U64 lightSquareMask = 0xAA55AA55AA55AA55;
    static constexpr U64 darkSquareMask =  0x55AA55AA55AA55AA;
    // Initial count of major pieces
    static constexpr int INIT_MAJ_MIN_PIECES = 14;

     // Parameter values for evaluation
    struct Parameters {
        int pawnValue = 100;
        int knightValue = 320;
        int bishopValue = 330;
        int rookValue = 500;
        int queenValue = 900;
        int kingValue = 2000;

        int mateScore = 100000;
        int restrictKingBonus = 10;
        int kingShieldBonus = 50;
        int airyKingPenalty = -10;
        int supportedPawnBonus = 90;
        int supportingPawnBonus = 15;
        int supportingPieceBonus = 15;
        int doubledPawnPenalty = -50;
        int isolatedPawnPenalty = -80;
        int checkedPenalty = -300;
        int checkingBonus = 300;
        int bishopPairBonus = 150;
        int rookOpenFileBonus = 250;
    } params;

public:

    Syrinx() 
        : ChessEngineBase("Syrinx",
                          "1.03",
                          "Cameron Cunningham",
                          8,
                          std::chrono::milliseconds(200),
                          std::chrono::milliseconds(20000)) {
            // Register all engine options
            registerOption(EngineOption::createSpin("PawnValue", 100, 10, 1000));
            registerOption(EngineOption::createSpin("KnightValue", 320, 10, 1000));
            registerOption(EngineOption::createSpin("BishopValue", 330, 10, 1000));
            registerOption(EngineOption::createSpin("RookValue", 500, 20, 1500));
            registerOption(EngineOption::createSpin("QueenValue", 900, 30, 2000));
            registerOption(EngineOption::createSpin("KingValue", 2000, 100, 5000));

            registerOption(EngineOption::createSpin("MateScore", 100000, 50000, 200000));
            registerOption(EngineOption::createSpin("RestrictKingBonus", 10, 0, 500));
            registerOption(EngineOption::createSpin("KingShieldBonus", 50, 0, 300));
            registerOption(EngineOption::createSpin("AiryKingPenalty", -10, -100, 0));
            registerOption(EngineOption::createSpin("SupportedPawnBonus", 90, 0, 200));
            registerOption(EngineOption::createSpin("SupportingPawnBonus", 15, 0, 200));
            registerOption(EngineOption::createSpin("SupportingPieceBonus", 15, 0, 200));
            registerOption(EngineOption::createSpin("DoubledPawnPenalty", -50, -200, 0));
            registerOption(EngineOption::createSpin("IsolatedPawnPenalty", -80, -200, 0));
            registerOption(EngineOption::createSpin("CheckedPenalty", -300, -5000, 0));
            registerOption(EngineOption::createSpin("CheckingBonus", 300, 0, 5000));
            registerOption(EngineOption::createSpin("BishopPairBonus", 150, 0, 300));
            registerOption(EngineOption::createSpin("RookOpenFileBonus", 250, 0, 500));
    }

    // Handle option changes by updating corresponding parameter
    void onOptionChanged(const EngineOption& option) override {
        int value = std::get<int>(option.currentValue);  // All our options are Spin (integer)
        
        if (option.name == "PawnValue") params.pawnValue = value;
        else if (option.name == "KnightValue") params.knightValue = value;
        else if (option.name == "BishopValue") params.bishopValue = value;
        else if (option.name == "RookValue") params.rookValue = value;
        else if (option.name == "QueenValue") params.queenValue = value;
        else if (option.name == "KingValue") params.kingValue = value;
        else if (option.name == "MateScore") params.mateScore = value;
        else if (option.name == "RestrictKingBonus") params.restrictKingBonus = value;
        else if (option.name == "KingShieldBonus") params.kingShieldBonus = value;
        else if (option.name == "AiryKingPenalty") params.airyKingPenalty = value;
        else if (option.name == "SupportedPawnBonus") params.supportedPawnBonus = value;
        else if (option.name == "SupportingPawnBonus") params.supportingPawnBonus = value;
        else if (option.name == "SupportingPieceBonus") params.supportingPieceBonus = value;
        else if (option.name == "DoubledPawnPenalty") params.doubledPawnPenalty = value;
        else if (option.name == "IsolatedPawnPenalty") params.isolatedPawnPenalty = value;
        else if (option.name == "CheckedPenalty") params.checkedPenalty = value;
        else if (option.name == "CheckingBonus") params.checkingBonus = value;
        else if (option.name == "BishopPairBonus") params.bishopPairBonus = value;
        else if (option.name == "RookOpenFileBonus") params.rookOpenFileBonus = value;
    }

    /// @brief Alpha Beta pruning function
    /// @param board 
    /// @param depth 
    /// @param alpha 
    /// @param beta 
    /// @param ply 
    /// @return Evaluation score relative to current depth's side (positive better, negative worse)
    int alphaBeta(ChessBoard& board, int depth, int alpha, int beta, int ply) {
        
        Color sideToMove = board.getSideToMove();
        if (depth <= 0 || !isSearching) {
            return (sideToMove == WHITE ? evaluatePosition(board) : -evaluatePosition(board));
        }

        // Check for repetition and 50-move rule
        if (ply > 0) {
            // If the 50-move rule is reached in this position, or it has occured once on the actual board, return 0
            if (board.currentGameState.halfMoveClock >= 100 ||
                board.keySet.find(board.zobristKey) != board.keySet.end()) {
                return 0;
            }
        }

        // Check transposition table
        /// @todo Check what to do with score based on hash flag
        int score;
        DenseMove hashMove;
        if (checkTT(board, depth, alpha, beta, score, hashMove)) {
            return score;
        }

        // Generate and order moves
        int moveNum = 0;
        std::array<DenseMove, MAX_MOVES> moves = MoveGenerator::generatePsuedoMoves(board, moveNum);
        orderMoves(moves, moveNum, ply, hashMove);
        
        bool noLegalMoves = true;
        DenseMove bestMove;
        int flag = TTEntry::ALPHA;

        // Search through ordered moves
        for (int i = 0; i < moveNum; i++) {
            board.makeMove(moves[i], true);

            // Skip illegal moves
            if (board.isSideInCheck(sideToMove)) {
                board.unmakeMove(moves[i], true);
                continue;
            }
            noLegalMoves = false;

            int eval = -alphaBeta(board, depth - 1, -beta, -alpha, ply + 1);
            board.unmakeMove(moves[i], true);

            // Beta cutoff
            if (eval >= beta) {
                flag = TTEntry::BETA;
                RecordTTEntry(board, moves[i], depth, beta, flag);
                return beta;
            }
            // Update best score
            if (eval > alpha) {
                alpha = eval;
                bestMove = moves[i];
                flag = TTEntry::EXACT;
            }
        }

        // Handle checkmate/stalemate
        if (noLegalMoves) {
            if (board.isSideInCheck(sideToMove)) {
                // If the current side to move is in checkmate, 
                return -params.mateScore + ply;
            }
            return 0;  // Stalemate
        }

        // Store position in transposition table
        RecordTTEntry(board, bestMove, depth, alpha, flag);
        return alpha;
    }

    bool keepSearching(ChessClock& clock) const {
        // If clock is set to infinite, always keep searching
        if (clock.isInfinite()) return true;
        // Get remaining time for current player
        auto remainingTime = (clock.getActiveColor() == WHITE) ? 
                            clock.getWhiteTime() : 
                            clock.getBlackTime();
        
        // Estimate time for next depth based on previous depth
        // Each depth typically takes ~4-5x longer than the previous
        auto estimatedNextDepthTime = prevDepthTime * 5;

        // Leave some buffer time for move selection and communication
        auto bufferTime = std::chrono::milliseconds(20);

        // Don't use more than 20% of remaining time on one move
        auto maxTimeForMove = remainingTime / 5;

        return estimatedNextDepthTime + bufferTime < maxTimeForMove;
    }

    DenseMove findBestMove(ChessBoard& board, ChessClock& clock, int maxDepth = -1) {
        startSearch();
        searchStartTime = std::chrono::steady_clock::now();
        prevDepthTime = std::chrono::milliseconds(0);   // Reset any previous time
        
        nodeCount = 0;
        currentMoveNumber = 0;

        int actualDepth = (maxDepth > 0) ? maxDepth : searchDepth;
        DenseMove bestMoveOverall;
        int bestScoreOverall = 0;
        const int MIN_DEPTH = 2;

        // Generate and order moves
        int moveNum = 0;
        std::array<DenseMove, MAX_MOVES> moves = MoveGenerator::generateLegalMoves(board, moveNum);
        Color sideToMove = board.getSideToMove();

        // Iterative deepening
        for (int currDepth = MIN_DEPTH; currDepth <= actualDepth && isSearching; currDepth++) {
            nodeCount = 0;
            currentMoveNumber = 0;
            clock.updateTime();
            // If we're using too much time, stop iterating
            if (currDepth > MIN_DEPTH && !keepSearching(clock)) {
                break;
            }

            // Get move from transposition table if available
            TTEntry* entry = &transpositionTable[board.zobristKey % TT_SIZE];
            DenseMove hashMove;
            if (entry->key == board.zobristKey) {
                hashMove = entry->bestMove;
            }

            int alpha = INF_NEG;
            int beta = INF_POS;
            DenseMove bestMove;
            int bestScore = INF_NEG;

            orderMoves(moves, moveNum, 0, hashMove);

            // Search each move
            for (int i = 0; i < moveNum; i++) {
                currentMoveNumber = i + 1;
                currentMove = moves[i];

                sendInfo(std::format("currmove {} currmovenumber {}", 
                                   currentMove.toAlgebraic(), currentMoveNumber));

                board.makeMove(moves[i], true);
                int score = -alphaBeta(board, currDepth - 1, -beta, -alpha, 1);
                board.unmakeMove(moves[i], true);

                // Update best move if better score found
                if (score > bestScore) {
                    bestScore = score;
                    bestMove = moves[i];

                    if (score > alpha) alpha = score;

                    auto moveTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - searchStartTime).count();
                    
                    std::string infoStr = std::format("depth {} score cp {} time {} nodes {} {} ",
                        currDepth, bestScore, moveTime, nodeCount, bestMove.toAlgebraic());
                    
                    if (moveTime > 0) {
                        U64 nps = (nodeCount * 1000) / moveTime;
                        infoStr += std::format("nps {} ", nps);
                    }
                    
                    sendInfo(infoStr);
                }
            }

            // Store best move for this iteration
            RecordTTEntry(board, bestMove, currDepth, bestScore, TTEntry::EXACT);
            bestMoveOverall = bestMove;
            bestScoreOverall = bestScore;

            // Log iteration completion
            auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - searchStartTime);
            
            std::string iterStr = std::format("Completed depth {} in {}ms, score: {}, nodes: {}", 
                currDepth, totalTime.count(), bestScore, nodeCount);
            if (totalTime.count() > 0) {
                U64 nps = (nodeCount * 1000) / totalTime.count();
                iterStr += std::format(", nps: {}", nps);
            }
            sendInfo(iterStr);
            // Update how long this depth took
            prevDepthTime = totalTime;
        }

        this->bestMove = bestMoveOverall;
        endSearch();
        return bestMoveOverall;
    }


    /// @brief Called at the terminal nodes of alphaBeta function.
    /// 
    /// @param board 
    /// @return 
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
    // Struct to hold move with its score for sorting
    struct ScoredMove {
        DenseMove move;
        int score;
        
        ScoredMove(DenseMove m = DenseMove(), int s = 0) 
            : move(m), score(s) {}
        
        // Comparison operator for sorting (higher scores first)
        bool operator<(const ScoredMove& other) const {
            return score > other.score;  // Reverse order for highest first
        }
    };

    // Score values for different move types
    static constexpr int HASH_MOVE_SCORE = 1000000;
    static constexpr int CAPTURE_BASE_SCORE = 100000;
        
    // Orders moves based on various heuristics including PV
    void orderMoves(std::array<DenseMove, MAX_MOVES>& moves, int& moveCount, int ply, DenseMove hashMove) {
        std::vector<ScoredMove> scoredMoves;
        scoredMoves.reserve(moveCount);
        
        // Score each move
        for (int i = 0; i < moveCount; i++) {
            ScoredMove scored(moves[i], 0);
            
            if (moves[i] == hashMove) {
                scored.score = HASH_MOVE_SCORE;
            }
            // Captures scored by MVV/LVA (Most Valuable Victim/Least Valuable Attacker)
            else if (moves[i].isCapture()) {
                // Base capture score plus victim value minus attacker value (scaled)
                scored.score = CAPTURE_BASE_SCORE + 
                    getPieceValue(moves[i].getCaptPiece()) * 10 - 
                    getPieceValue(moves[i].getPieceType());
            }
            /// @todo Add other scoring criteria here (killer moves, history heuristic, etc.)
            
            scoredMoves.push_back(scored);
        }
        
        // Sort moves by score
        std::sort(scoredMoves.begin(), scoredMoves.end());
        
        // Copy back to moves array
        for (int i = 0; i < moveCount; i++) {
            moves[i] = scoredMoves[i].move;
        }
    }

    // Helper to get piece values for MVV/LVA scoring
    int getPieceValue(PieceType piece) const {
        switch (pieceCode(piece)) {
            case D_PAWN:   return params.pawnValue;
            case D_KNIGHT: return params.knightValue;
            case D_BISHOP: return params.bishopValue;
            case D_ROOK:   return params.rookValue;
            case D_QUEEN:  return params.queenValue;
            case D_KING:   return params.kingValue;
            default:       return 0;
        }
    }

    void sendInfo(const std::string& info) {
        std::cout << "info " << info << std::endl;
    }

    int countMaterial(const ChessBoard& board, Color color) {
        int score = 0;
        U64 pieces;

        if (color == WHITE) {
            // Pawns
            pieces = board.getWhitePawns();
            score += popcount(pieces) * params.pawnValue;
            // Knights
            pieces = board.getWhiteKnights();
            score += popcount(pieces) * params.knightValue;
            // Bishops
            pieces = board.getWhiteBishops();
            score += popcount(pieces) * params.bishopValue;
            // Rooks 
            pieces = board.getWhiteRooks();
            score += popcount(pieces) * params.rookValue;
            // Queens
            pieces = board.getWhiteQueens();
            score += popcount(pieces) * params.queenValue;
        } else {
            // Pawns
            pieces = board.getBlackPawns();
            score += popcount(pieces) * params.pawnValue;
            // Knights
            pieces = board.getBlackKnights();
            score += popcount(pieces) * params.knightValue;
            // Bishops
            pieces = board.getBlackBishops();
            score += popcount(pieces) * params.bishopValue;
            // Rooks 
            pieces = board.getBlackRooks();
            score += popcount(pieces) * params.rookValue;
            // Queens
            pieces = board.getBlackQueens();
            score += popcount(pieces) * params.queenValue;
        }
        // Get total piece count (not including pawns or kings)
        // for endgame lerp
        totalPiecesWithoutPawns = popcount(board.getAllPieces() & 
                                        (~(board.getDenseSet(D_PAWN) | 
                                         board.getDenseSet(D_KING))));
        return score;
    }

    int evaluatePositional(const ChessBoard& board, Color color) {
        int score = 0;
        // Calculate 'distance' along early game to late game by taking
        // current number of major and minor pieces and initial number of those pieces
        float earlygameLerp = (float)totalPiecesWithoutPawns/INIT_MAJ_MIN_PIECES;
        float endgameLerp = (float)(std::clamp(8 - totalPiecesWithoutPawns, 0, 8))/INIT_MAJ_MIN_PIECES;

        U64 pawns, pawnRef, knights, bishops, rooks, queens,
            attackingOppKing, attacksToKing;
        int kingSquare, oppKingIndex;
        // Flip is 0 for WHITE, and 56 (111000) for BLACK. This enables being able to flip the indicies for
        // black by XORing to get the correct PST score and not have to store extra tables for black.
        int flip;
        U64 occupancy = board.getAllPieces();

        if (color == WHITE) {
            flip = 0;
            pawns = board.getWhitePawns();
            pawnRef = pawns;
            knights = board.getWhiteKnights();
            bishops = board.getWhiteBishops();
            rooks = board.getWhiteRooks();
            queens = board.getWhiteRooks();
            kingSquare = board.getWhiteKingSquare();
            attacksToKing = board.OppAttacksToSquare(kingSquare, WHITE);
            oppKingIndex = board.getBlackKingSquare();
            attackingOppKing = board.OppAttacksToSquare(oppKingIndex, BLACK);
        } else {
            flip = 56;
            pawns = board.getBlackPawns();
            pawnRef = pawns;
            knights = board.getBlackKnights();
            bishops = board.getBlackBishops();
            rooks = board.getBlackRooks();
            queens = board.getBlackRooks();
            kingSquare = board.getBlackKingSquare() ^ flip;
            attacksToKing = board.OppAttacksToSquare(kingSquare, BLACK);
            oppKingIndex = board.getWhiteKingSquare();
            attackingOppKing = board.OppAttacksToSquare(oppKingIndex, WHITE);
        }
        while (pawns) {
            int square = std::countr_zero(pawns) ^ flip;
            // Add score from piece-square table
            // Lerp from early to endgame based on piece count
            score +=  pawnSqTbEarly[square]*(earlygameLerp) +
                pawnSqTbEnd[square]*(endgameLerp);

            square ^= flip;

            // Check for doubled pawns (more than one pawn in a file)
            int file = BUTIL::indexToFile(square);
            U64 fileMask = BUTIL::FileMask << file;
            if (popcount(fileMask & pawnRef) > 1) {
                score += params.doubledPawnPenalty;
            }

            // Check for isolated pawns
            U64 adjacentFiles = 0;
            if (file > 0) adjacentFiles |= BUTIL::FileMask << (file - 1);
            if (file < 7) adjacentFiles |= BUTIL::FileMask << (file + 1);
            // No pawns in adjacent files
            if (!(adjacentFiles & pawnRef)) {
                score += params.isolatedPawnPenalty;
            }

            // If this pawn is supporting another pawn
            U64 supports = color == WHITE ? ATKMASK_WPAWN[square] & pawnRef :
                                            ATKMASK_BPAWN[square] & pawnRef;
            if (supports) {
                score += params.supportingPawnBonus * popcount(supports);
            }
            pawns &= pawns - 1;
        }
        // Knights
        while (knights) {
            int square = std::countr_zero(knights) ^ flip;
            // Add score from piece-square table
            // Lerp from early to endgame based on piece count
            score +=  knightSqTbEarly[square]*(earlygameLerp) +
                knightSqTbEnd[square]*(endgameLerp);
            knights &= knights - 1;
        }
        // Bishops
        // Bishop pair bonus
        if (popcount(bishops) >= 2) {
            if ((lightSquareMask & bishops) && (darkSquareMask & bishops))
                    score += params.bishopPairBonus;
        }
        while (bishops) {
            int square = std::countr_zero(bishops) ^ flip;
            // Add score from piece-square table
            // Lerp from early to endgame based on piece count
            score +=  bishopSqTbEarly[square]*(earlygameLerp) +
                bishopSqTbEnd[square]*(endgameLerp);
            bishops &= bishops - 1;
        }
        // Rooks
        while (rooks) {
            int square = std::countr_zero(rooks) ^ flip;
            score += rookSqTbEarly[square]*(earlygameLerp) +
                rookSqTbEnd[square]*(endgameLerp);
            rooks &= rooks - 1;
        }
        // Queens
        while (queens) {
            int square = std::countr_zero(queens) ^ flip;
            score += queenSqTbEarly[square]*(earlygameLerp) +
                queenSqTbEnd[square]*(endgameLerp);
            queens &= queens - 1;
        }
        // King
        score += kingSqTbEarly[kingSquare]*(earlygameLerp) +
            kingSqTbEnd[kingSquare]*(endgameLerp);
        kingSquare ^= flip;
        // Give bonus for king behind pawns
        U64 kingShield = ATKMASK_KING[kingSquare] & pawnRef;
        if (popcount(kingShield) >= 2 && (kingSquare < 8 || kingSquare >= 56)) {
            score += params.kingShieldBonus;
        }
        // Give penalty for 'airy' king (empty squares to king)
        U64 kingAir = (PEXT::getRookAttacks(kingSquare, occupancy) |
                       PEXT::getBishopAttacks(kingSquare, occupancy));
        if (popcount(kingAir) > 12) {
            score += params.airyKingPenalty;
        }

        // Evaluate attacks to opposite king
        if (attackingOppKing) {
            // Using popcount means double checks should be worth more
            score += params.checkingBonus * popcount(attackingOppKing);
        }

        // Evaluate attacks to own king
        // Discourage getting checked
        if (attacksToKing) {
            score += params.checkedPenalty * popcount(attacksToKing);
        }

        return score;
    }

    /// @brief Records the given transposition table entry in the table
    /// @param key 
    /// @param best 
    /// @param depth 
    /// @param score 
    /// @param flag 
    void RecordTTEntry(ChessBoard& board, DenseMove best, int depth, int score, int flag) {
        TTEntry* entry = &transpositionTable[board.zobristKey % TT_SIZE];

        entry->key = board.zobristKey;
        entry->bestMove = best;
        entry->depth = depth;
        entry->score = score;
        entry->flag = flag;
    }
    /// @brief 
    /// @param board 
    /// @param depth 
    /// @param alpha 
    /// @param beta 
    /// @param score 
    /// @param hashMove 
    /// @return 
    bool checkTT(ChessBoard& board, int depth, int alpha, int beta,
        int& score, DenseMove& hashMove) {
        TTEntry* entry = &transpositionTable[board.zobristKey % TT_SIZE];

        // Check if this is the position we want
        if (entry->key == board.zobristKey) {
            // Only use the entry if its depth is greater or equal to our current
            // depth; lesser depth could be inaccurate due to other positions
            // searched at greater depth
            if (entry->depth >= depth) {
                // Use the bestMove first next time we search this position
                hashMove = entry->bestMove;

                // Based on the type of score stored, we might be able to use it
                if (entry->flag == TTEntry::EXACT) {
                    score = entry->score;
                    return true;
                }
                if (entry->flag == TTEntry::ALPHA && entry->score <= alpha) {
                    score = alpha;
                    return true;
                }
                if (entry->flag == TTEntry::BETA && entry->score >= beta) {
                    score = beta;
                    return true;
                }
            }
        }
        return false;
    }
};