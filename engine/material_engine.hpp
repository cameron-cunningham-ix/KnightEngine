#pragma once

#include "chess_engine_base.hpp"
#include "transposition_table.hpp"
#include "board_utility.hpp"
#include <immintrin.h>
#include <algorithm>
#include <chrono>

static constexpr int INF_POS = 999999999;
static constexpr int INF_NEG = -999999999;
static constexpr int TT_SIZE = 44739243;

/// @brief 
class MaterialEngine : public ChessEngineBase {
private:
    // Search statistics
    U64 nodeCount;
    std::chrono::steady_clock::time_point searchStartTime;
    DenseMove currentMove;
    int currentMoveNumber;
    int totalPiecesWithoutPawns;
    // Transposition table
    // 44 million 24-byte entries = 1 GB
    std::array<TTEntry, TT_SIZE> transpositionTable;

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

     // Parameters struct without any option-related code
    struct Parameters {
        int pawnValue = 100;
        int knightValue = 320;
        int bishopValue = 330;
        int rookValue = 500;
        int queenValue = 900;
        int kingValue = 2000;

        int pawnPositionBonus = 50;
        int knightPositionBonus = 50;
        int bishopPositionBonus = 50;
        int rookPositionBonus = 50;
        int queenPositionBonus = 50;
        int kingPositionBonus = 50;

        int mateScore = 100000;
        int supportedPawnBonus = 75;
        int supportingPawnBonus = 90;
        int supportingPieceBonus = 100;
        int doubledPawnPenalty = -70;
        int isolatedPawnPenalty = -80;
        int checkedPenalty = -1000;
        int checkingBonus = 1500;
        int bishopPairBonus = 125;
        int rookOpenFileBonus = 250;
    } params;

public:

    MaterialEngine() 
        : ChessEngineBase("MaterialEngine", "0.72", "Cameron Cunningham", 8) {
            // Register all engine options
            registerOption(EngineOption::createSpin("PawnValue", 100, 0, 1000));
            registerOption(EngineOption::createSpin("KnightValue", 320, 0, 1000));
            registerOption(EngineOption::createSpin("BishopValue", 330, 0, 1000));
            registerOption(EngineOption::createSpin("RookValue", 500, 0, 1500));
            registerOption(EngineOption::createSpin("QueenValue", 900, 0, 2000));
            registerOption(EngineOption::createSpin("KingValue", 2000, 1000, 5000));

            registerOption(EngineOption::createSpin("PawnPositionBonus", 50, 0, 200));
            registerOption(EngineOption::createSpin("KnightPositionBonus", 50, 0, 200));
            registerOption(EngineOption::createSpin("BishopPositionBonus", 50, 0, 200));
            registerOption(EngineOption::createSpin("RookPositionBonus", 50, 0, 200));
            registerOption(EngineOption::createSpin("QueenPositionBonus", 50, 0, 200));
            registerOption(EngineOption::createSpin("KingPositionBonus", 50, 0, 200));

            registerOption(EngineOption::createSpin("MateScore", 100000, 50000, 200000));
            registerOption(EngineOption::createSpin("SupportedPawnBonus", 75, 0, 200));
            registerOption(EngineOption::createSpin("SupportingPawnBonus", 90, 0, 200));
            registerOption(EngineOption::createSpin("SupportingPieceBonus", 100, 0, 200));
            registerOption(EngineOption::createSpin("DoubledPawnPenalty", -70, -200, 0));
            registerOption(EngineOption::createSpin("IsolatedPawnPenalty", -80, -200, 0));
            registerOption(EngineOption::createSpin("CheckedPenalty", -1000, -2000, 0));
            registerOption(EngineOption::createSpin("CheckingBonus", 1500, 0, 3000));
            registerOption(EngineOption::createSpin("BishopPairBonus", 125, 0, 300));
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
        else if (option.name == "PawnPositionBonus") params.pawnPositionBonus = value;
        else if (option.name == "KnightPositionBonus") params.knightPositionBonus = value;
        else if (option.name == "BishopPositionBonus") params.bishopPositionBonus = value;
        else if (option.name == "RookPositionBonus") params.rookPositionBonus = value;
        else if (option.name == "QueenPositionBonus") params.queenPositionBonus = value;
        else if (option.name == "KingPositionBonus") params.kingPositionBonus = value;
        else if (option.name == "MateScore") params.mateScore = value;
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

    


     // Enhanced alpha-beta search with move ordering and quiescence
    int alphaBeta(ChessBoard& board, int depth, int alpha, int beta, int ply) {
        if (!isSearching) {
            return evaluatePosition(board);
        }

        // Check for checkmate/stalemate at leaf nodes
        if (depth <= 0) {
            pv[ply].length = 0;  // Clear PV at leaf nodes
            return quiescence(board, alpha, beta, ply);
        }

        // Check transposition table
        int score;
        TTEntry* entry = &transpositionTable[board.zobristKey % TT_SIZE];
        if (checkTT(board, depth, alpha, beta, score)) {
            return score;
        }

        // Get hash move from transposition table
        DenseMove hashMove;
        if (entry->key == board.zobristKey) {
            hashMove = entry->bestMove;
        }

        // Generate and order moves
        int moveNum = 0;
        std::array<DenseMove, MAX_MOVES> moves = MoveGenerator::generatePsuedoMoves(board, moveNum);
        orderMoves(moves, moveNum, ply, hashMove);
        
        bool noLegalMoves = true;
        DenseMove bestMove;
        int flag = TTEntry::ALPHA;
        Color sideToMove = board.getSideToMove();

        // Search through ordered moves
        for (int i = 0; i < moveNum; i++) {
            ChessBoard tempBoard = board;
            tempBoard.makeMove(moves[i], true);

            // Skip illegal moves
            if (tempBoard.isSideInCheck(sideToMove)) {
                tempBoard.unmakeMove(moves[i], true);
                continue;
            }
            noLegalMoves = false;

            // Full-depth search
            int eval = -alphaBeta(tempBoard, depth - 1, -beta, -alpha, ply + 1);
            tempBoard.unmakeMove(moves[i], true);

            // Update best score
            if (eval > alpha) {
                alpha = eval;
                bestMove = moves[i];
                flag = TTEntry::EXACT;
                
                // Update principal variation
                updatePV(ply, moves[i]);
                
                // Beta cutoff
                if (alpha >= beta) {
                    flag = TTEntry::BETA;
                    RecordTTEntry(board, moves[i], depth, beta, flag);
                    return beta;
                }
            }
        }

        // Handle checkmate/stalemate
        if (noLegalMoves) {
            if (board.isSideInCheck(sideToMove)) {
                return sideToMove ? -params.mateScore - depth : params.mateScore + depth;
            }
            return 0;  // Stalemate
        }

        // Store position in transposition table
        RecordTTEntry(board, bestMove, depth, alpha, flag);
        return alpha;
    }

    // Modified findBestMove to use PVS
    DenseMove findBestMove(ChessBoard& board, int maxDepth = -1) {
        startSearch();
        searchStartTime = std::chrono::steady_clock::now();
        nodeCount = 0;
        currentMoveNumber = 0;
        clearPV();

        int actualDepth = (maxDepth > 0) ? maxDepth : searchDepth;
        DenseMove bestMoveOverall;
        int bestScoreOverall = 0;
        const int MIN_DEPTH = 2;

        // Iterative deepening
        for (int currDepth = MIN_DEPTH; currDepth <= actualDepth && isSearching; currDepth++) {
            nodeCount = 0;
            currentMoveNumber = 0;

            // Get move from transposition table if available
            TTEntry* entry = &transpositionTable[board.zobristKey % TT_SIZE];
            DenseMove ttMove;
            if (entry->key == board.zobristKey) {
                ttMove = entry->bestMove;
            }

            // Generate and order moves
            int moveNum = 0;
            std::array<DenseMove, MAX_MOVES> moves = MoveGenerator::generateLegalMoves(board, moveNum);
            Color sideToMove = board.getSideToMove();
            
            int alpha = INF_NEG;
            int beta = INF_POS;
            DenseMove bestMove;
            int bestScore = sideToMove == WHITE ? INF_NEG : INF_POS;

            // Try TT move first if available
            if (ttMove != DenseMove()) {
                for (int i = 0; i < moveNum; i++) {
                    if (moves[i] == ttMove) {
                        if (i != 0) std::swap(moves[0], moves[i]);
                        break;
                    }
                }
            }

            // Search each move
            for (int i = 0; i < moveNum; i++) {
                currentMoveNumber = i + 1;
                currentMove = moves[i];

                sendInfo(std::format("currmove {} currmovenumber {}", 
                                   currentMove.toAlgebraic(), currentMoveNumber));

                ChessBoard tempBoard = board;
                tempBoard.makeMove(moves[i], true);

                int score = -alphaBeta(tempBoard, currDepth - 1, -beta, -alpha, 1);

                // Update best move if better score found
                if ((sideToMove == WHITE && score > bestScore) ||
                    (sideToMove == BLACK && score < bestScore)) {
                    bestScore = score;
                    bestMove = moves[i];

                    auto moveTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - searchStartTime).count();
                    
                    std::string infoStr = std::format("depth {} score cp {} time {} nodes {} {} ",
                        currDepth, bestScore, moveTime, nodeCount, bestMove.toAlgebraic());
                    
                    if (moveTime > 0) {
                        U64 nps = (nodeCount * 1000) / moveTime;
                        infoStr += std::format("nps {} ", nps);
                    }
                    
                    infoStr += getPVString(0);
                    sendInfo(infoStr);
                }
            }

            // Store best move for this iteration
            RecordTTEntry(board, bestMove, currDepth, bestScore, TTEntry::EXACT);
            bestMoveOverall = bestMove;
            bestScoreOverall = bestScore;

            // Log iteration completion
            auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - searchStartTime).count();
            
            std::string iterStr = std::format("Completed depth {} in {}ms, score: {}, nodes: {}", 
                currDepth, totalTime, bestScore, nodeCount);
            if (totalTime > 0) {
                U64 nps = (nodeCount * 1000) / totalTime;
                iterStr += std::format(", nps: {}", nps);
            }
            sendInfo(iterStr);
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
    // Maximum length of principal variation line
    static const int MAX_PV_LENGTH = 64;

    // Struct to store PV info at each ply
    struct PVEntry {
        DenseMove pvMoves[MAX_PV_LENGTH];   // Array of moves in the PV
        int length;     // Number of moves in this PV

        PVEntry() : length(0) {}

        void clear() {
            length = 0;
        }
    };

    // Array of PV entries for each ply of search
    std::array<PVEntry, MAX_PLY> pv;

    // Updates the PV at the current ply
    void updatePV(int ply, DenseMove move) {
        // Copy this move and the PV from child ply
        pv[ply].pvMoves[0] = move;
        if (ply + 1 < MAX_PLY && pv[ply + 1].length > 0) {
            memcpy(pv[ply].pvMoves + 1,
                   pv[ply + 1].pvMoves,
                   pv[ply + 1].length * sizeof(DenseMove));
            pv[ply].length = pv[ply + 1].length + 1;
        } else {
            pv[ply].length = 1;
        }
    }

    // Get string representation of current PV line for UCI output
    std::string getPVString(int ply) {
        std::string pvStr = "pv";
        for (int i = 0; i < pv[ply].length; i++) {
            pvStr += " " + pv[ply].pvMoves[i].toAlgebraic();
        }
        return pvStr;
    }

    // Clear PV entries for a new search
    void clearPV() {
        for (int i = 0; i < MAX_PLY; i++) {
            pv[i].clear();
        }
    }

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
    static constexpr int PV_MOVE_SCORE = 2000000;
    static constexpr int HASH_MOVE_SCORE = 1000000;
    static constexpr int CAPTURE_BASE_SCORE = 100000;
        
    // Orders moves based on various heuristics including PV
    void orderMoves(std::array<DenseMove, MAX_MOVES>& moves, int& moveCount, int ply, DenseMove hashMove) {
        std::vector<ScoredMove> scoredMoves;
        scoredMoves.reserve(moveCount);
        
        // Get PV move for this position if it exists
        DenseMove pvMove = (ply < MAX_PLY && pv[ply].length > 0) ? pv[ply].pvMoves[0] : DenseMove();
        
        // Score each move
        for (int i = 0; i < moveCount; i++) {
            ScoredMove scored(moves[i], 0);
            
            // PV move gets highest priority
            if (moves[i] == pvMove) {
                scored.score = PV_MOVE_SCORE;
            }
            // Hash move gets second priority (if different from PV move)
            else if (moves[i] == hashMove) {
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

    // Add to MaterialEngine class private section:

    // Constants for quiescence search
    static constexpr int MAX_QSEARCH_DEPTH = 8;    // Maximum depth for qsearch
    static constexpr int DELTA_MARGIN = 200;        // Margin for delta pruning (in centipawns)

    // Static Exchange Evaluation
    // Returns estimated score after sequence of captures on a square
    int staticExchangeEvaluation(ChessBoard& board, int square, Color side) {
        PieceType target = board.getPieceAt(square);
        if (target == EMPTY) return 0;

        int targetValue = getPieceValue(target);
        U64 attackers = board.getAttacksForSide((Color)!side);

        // No attackers means no capture possible
        if (!(attackers & (1ULL << square))) return 0;

        // Find least valuable attacker
        PieceType nextAttacker = EMPTY;
        int lvaValue = INF_POS;
        U64 pieces = board.getWhitePawns() | board.getBlackPawns();
        if (pieces & attackers) {
            nextAttacker = side == WHITE ? W_PAWN : B_PAWN;
            lvaValue = getPieceValue(nextAttacker);
        }
        pieces = board.getWhiteKnights() | board.getBlackKnights();
        if (pieces & attackers && getPieceValue(side == WHITE ? W_KNIGHT : B_KNIGHT) < lvaValue) {
            nextAttacker = side == WHITE ? W_KNIGHT : B_KNIGHT;
            lvaValue = getPieceValue(nextAttacker);
        }
        pieces = board.getWhiteBishops() | board.getBlackBishops();
        if (pieces & attackers && getPieceValue(side == WHITE ? W_BISHOP : B_BISHOP) < lvaValue) {
            nextAttacker = side == WHITE ? W_BISHOP : B_BISHOP;
            lvaValue = getPieceValue(nextAttacker);
        }
        pieces = board.getWhiteRooks() | board.getBlackRooks();
        if (pieces & attackers && getPieceValue(side == WHITE ? W_ROOK : B_ROOK) < lvaValue) {
            nextAttacker = side == WHITE ? W_ROOK : B_ROOK;
            lvaValue = getPieceValue(nextAttacker);
        }
        pieces = board.getWhiteQueens() | board.getBlackQueens();
        if (pieces & attackers && getPieceValue(side == WHITE ? W_QUEEN : B_QUEEN) < lvaValue) {
            nextAttacker = side == WHITE ? W_QUEEN : B_QUEEN;
            lvaValue = getPieceValue(nextAttacker);
        }
        pieces = board.getWhiteKings() | board.getBlackKings();
        if (pieces & attackers && getPieceValue(side == WHITE ? W_KING : B_KING) < lvaValue) {
            nextAttacker = side == WHITE ? W_KING : B_KING;
            lvaValue = getPieceValue(nextAttacker);
        }

        if (nextAttacker == EMPTY) return targetValue;

        // Recursively evaluate subsequent captures
        int score = targetValue - staticExchangeEvaluation(board, square, (Color)!side);
        return score > 0 ? score : 0;
    }

    // Enhanced quiescence search with SEE and delta pruning
    int quiescence(ChessBoard& board, int alpha, int beta, int ply, int qDepth = 0) {
        nodeCount++;

        // Check qsearch depth limit
        if (qDepth >= MAX_QSEARCH_DEPTH) {
            return evaluatePosition(board);
        }

        // Stand pat score
        int standPat = evaluatePosition(board);
        
        // Fail-high if standing pat exceeds beta
        if (standPat >= beta) {
            return beta;
        }

        // Delta pruning
        // If even capturing queen can't raise alpha, stop here
        if (standPat + params.queenValue + DELTA_MARGIN < alpha) {
            return alpha;
        }

        // Update alpha if stand pat is better
        if (standPat > alpha) {
            alpha = standPat;
        }

        // Generate and score captures
        int moveNum = 0;
        std::array<DenseMove, MAX_MOVES> moves = MoveGenerator::generatePsuedoMoves(board, moveNum);
        std::vector<ScoredMove> captureMoves;

        // Filter captures and score them
        for (int i = 0; i < moveNum; i++) {
            if (moves[i].isCapture()) {
                // Get initial MVV/LVA score
                int score = getPieceValue(moves[i].getCaptPiece()) * 10 - 
                        getPieceValue(moves[i].getPieceType());
                
                // Delta pruning per move
                if (standPat + getPieceValue(moves[i].getCaptPiece()) + DELTA_MARGIN < alpha) {
                    continue;
                }

                // Static Exchange Evaluation
                int see = staticExchangeEvaluation(board, moves[i].getTo(), board.getSideToMove());
                if (see <= 0) {
                    continue;  // Skip losing captures
                }

                // Final move score combines MVV/LVA and SEE
                score += see;
                captureMoves.emplace_back(moves[i], score);
            }
        }

        // Sort captures by score
        std::sort(captureMoves.begin(), captureMoves.end());

        // Search captures
        for (const auto& scored : captureMoves) {
            ChessBoard tempBoard = board;
            tempBoard.makeMove(scored.move, true);

            // Skip if illegal (leaves us in check)
            if (tempBoard.isSideInCheck(board.getSideToMove())) {
                continue;
            }

            int score = -quiescence(tempBoard, -beta, -alpha, ply + 1, qDepth + 1);
            
            if (score >= beta) {
                return beta;
            }
            if (score > alpha) {
                alpha = score;
            }
        }

        return alpha;
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
                                        (~board.getDenseSet(D_PAWN) | 
                                         ~board.getDenseSet(D_KING)));
        return score;
    }

    int evaluatePositional(const ChessBoard& board, Color color) {
        int score = 0;
        // Calculate 'distance' along early game to late game by taking
        // current number of major and minor pieces and initial number of those pieces
        int earlygameLerp = totalPiecesWithoutPawns/INIT_MAJ_MIN_PIECES;
        int endgameLerp = (std::clamp(14 - totalPiecesWithoutPawns, 0, 14))/INIT_MAJ_MIN_PIECES;
        if (color == WHITE) {
            // Evaluate pawn structure
            U64 pawns = board.getWhitePawns();
            U64 pawnRef = pawns;
            while (pawns) {
                int square = std::countr_zero(pawns);

                // Add score from piece-square table
                // Lerp from early to endgame based on piece count
                score +=  pawnSqTbEarly[square]*(earlygameLerp) +
                    pawnSqTbEnd[square]*(endgameLerp);

                // Check for doubled pawns (more than one pawn in a file)
                int file = BUTIL::squareToFileIndex(square);
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
                U64 supports = ATKMASK_WPAWN[square] & pawnRef;
                if (supports) {
                    score += params.supportingPawnBonus * popcount(supports);
                }
                // If this pawn is supported by other pawns
                U64 supported = ATKMASK_BPAWN[square] & pawnRef;
                if (supported) {
                    score += params.supportedPawnBonus * popcount(supported);
                }
                // If this pawn is supporting a piece
                /// @todo See if only using this with minor pieces is better
                supports = ATKMASK_WPAWN[square] & (board.getWhitePieces() & ~pawnRef);
                if (supports) {
                    score += params.supportingPieceBonus * popcount(supports);
                }
                pawns &= pawns - 1;
            }
            // Knights
            U64 knights = board.getWhiteKnights();
            while (knights) {
                int square = std::countr_zero(knights);
                // Add score from piece-square table
                // Lerp from early to endgame based on piece count
                score +=  knightSqTbEarly[square]*(earlygameLerp) +
                    knightSqTbEnd[square]*(endgameLerp);
                knights &= knights - 1;
            }
            // Bishops
            U64 bishops = board.getWhiteBishops();
            // Bishop pair bonus
            if (popcount(bishops) >= 2) {
                if ((lightSquareMask & bishops) && (darkSquareMask & bishops))
                     score += params.bishopPairBonus;
            }
            while (bishops) {
                int square = std::countr_zero(bishops);
                // Add score from piece-square table
                // Lerp from early to endgame based on piece count
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
                score += params.checkingBonus * popcount(attackingOppKing);
            }

            // Evaluate attacks to own king
            // Discourage getting checked
            U64 attacksToKing = board.OppAttacksToSquare(board.getWhiteKingSquare(), WHITE);
            if (attacksToKing) {
                score += params.checkedPenalty * popcount(attacksToKing);
            }
        } else {
            // Evaluate pawn structure
            U64 pawns = board.getBlackPawns();
            U64 pawnRef = pawns;
            while (pawns) {
                int square = std::countr_zero(pawns);

                // Add score from piece-square table
                // Lerp from early to endgame based on piece count
                // For now 63 - is a hack for not having flipped piece-square tables
                score +=  pawnSqTbEarly[63-square]*(earlygameLerp) +
                    pawnSqTbEnd[63-square]*(endgameLerp);

                // Check for doubled pawns (more than one pawn in a file)
                int file = BUTIL::squareToFileIndex(square);
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

                // If this pawn is supporting another
                U64 supports = ATKMASK_BPAWN[square] & pawnRef;
                if (supports) {
                    score += params.supportingPawnBonus * popcount(supports);
                }
                // If this pawn is supported by others
                U64 supported = ATKMASK_WPAWN[square] & pawnRef;
                if (supported) {
                    score += params.supportedPawnBonus * popcount(supported);
                }
                // If this pawn is supporting a piece
                supports = ATKMASK_BPAWN[square] & (board.getBlackPieces() & ~pawnRef);
                if (supports) {
                    score += params.supportingPieceBonus * popcount(supports);
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
            if (popcount(bishops) >= 2) {
                if ((lightSquareMask & bishops) && (darkSquareMask & bishops))
                     score += params.bishopPairBonus;
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
                score += params.checkingBonus * popcount(attackingOppKing);
            }

            // Evaluate attacks to own king
            // Discourage getting checked
            U64 attacksToKing = board.OppAttacksToSquare(board.getBlackKingSquare(), BLACK);
            if (attacksToKing) {
                score += params.checkedPenalty * popcount(attacksToKing);
            }
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

    bool checkTT(ChessBoard& board, int depth, int& alpha, int& beta, int& score) {
        TTEntry* entry = &transpositionTable[board.zobristKey & TT_SIZE];

        // Check if this is the position we want
        if (entry->key == board.zobristKey) {
            // Only use the entry if its depth is greater or equal to our current
            // depth; lesser depth could be inaccurate due to other positions
            // searched at greater depth
            if (entry->depth >= depth) {
                // Use the bestMove first next time we search this position
                bestMove = entry->bestMove;

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



    // /// @brief Alpha-Beta search algorithm.
    // /// This is a recursively called function used to score a chess position.
    // /// Positive means the position is better for White, negative better for Black
    // /// @param board Current board to search
    // /// @param depth Depth to search to. Position is evaluated at depth 0
    // /// @param alpha The minimum score the maximizing player (W) is guranteed
    // /// in the position
    // /// @param beta The maximum score the minimizing player (B) is guranteed
    // /// in the position
    // /// @param maximizing 
    // /// @return 
};