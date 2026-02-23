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
    // Transposition table
    // 44 million 24-byte entries = ~1 GB
    std::array<TTEntry, TT_SIZE> transpositionTable;
    
    static constexpr int KM_PLY = 64;
    // Stores the two best non-capturing moves per ply
    DenseMove killerMoves[KM_PLY * 2];

    //
    static constexpr int HISTORY_MAX = 65536;
    // Keeps history of moves that have caused good beta cutoffs in the search tree
    int historyTable[2][64][64] = {0};   // Indexed by [Color][From][To]

    // Masks of light and dark squares
    static constexpr U64 lightSquareMask = 0xAA55AA55AA55AA55;
    static constexpr U64 darkSquareMask =  0x55AA55AA55AA55AA;
    // Initial count of major pieces
    static constexpr int INIT_MAJ_MIN_PIECES = 14;
    // Current count of major pieces
    int totalPiecesWithoutPawns;
    float earlygameLerp;
    float endgameLerp;


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
        int passedPawnBonus = 30;
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
                          "1.25",
                          "Cameron Cunningham",
                          8,
                          std::chrono::milliseconds(200),
                          std::chrono::milliseconds(20000)) {
            // Register all engine options
            registerOption(EngineOption::createSpin("PawnValue", 100, 10, 300));
            registerOption(EngineOption::createSpin("KnightValue", 320, 100, 500));
            registerOption(EngineOption::createSpin("BishopValue", 330, 110, 510));
            registerOption(EngineOption::createSpin("RookValue", 500, 500, 800));
            registerOption(EngineOption::createSpin("QueenValue", 900, 810, 1500));
            registerOption(EngineOption::createSpin("KingValue", 2000, 2000, 5000));

            registerOption(EngineOption::createSpin("MateScore", 100000, 50000, 200000));
            registerOption(EngineOption::createSpin("RestrictKingBonus", 10, 1, 100));
            registerOption(EngineOption::createSpin("KingShieldBonus", 50, 1, 300));
            registerOption(EngineOption::createSpin("AiryKingPenalty", -10, -100, -1));
            registerOption(EngineOption::createSpin("SupportedPawnBonus", 90, 1, 200));
            registerOption(EngineOption::createSpin("SupportingPawnBonus", 15, 1, 200));
            registerOption(EngineOption::createSpin("PassedPawnBonus", 30, 1, 200));
            registerOption(EngineOption::createSpin("SupportingPieceBonus", 15, 1, 200));
            registerOption(EngineOption::createSpin("DoubledPawnPenalty", -50, -200, -1));
            registerOption(EngineOption::createSpin("IsolatedPawnPenalty", -80, -200, -1));
            registerOption(EngineOption::createSpin("CheckedPenalty", -300, -5000, -1));
            registerOption(EngineOption::createSpin("CheckingBonus", 300, 1, 5000));
            registerOption(EngineOption::createSpin("BishopPairBonus", 150, 1, 300));
            registerOption(EngineOption::createSpin("RookOpenFileBonus", 250, 1, 500));
    }
    // Reset the game-based search histories
    // Note: We don't have to reset transposition table, since if we transpose
    // into the same position in a different game, we'll be able to use the same move and score
    void clearForNewGame() {
        std::fill_n(&historyTable[0][0][0], 2*64*64, 0);
        std::fill(std::begin(killerMoves), std::end(killerMoves), DenseMove());
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
        else if (option.name == "PassedPawnBonus") params.passedPawnBonus = value;
        else if (option.name == "SupportingPieceBonus") params.supportingPieceBonus = value;
        else if (option.name == "DoubledPawnPenalty") params.doubledPawnPenalty = value;
        else if (option.name == "IsolatedPawnPenalty") params.isolatedPawnPenalty = value;
        else if (option.name == "CheckedPenalty") params.checkedPenalty = value;
        else if (option.name == "CheckingBonus") params.checkingBonus = value;
        else if (option.name == "BishopPairBonus") params.bishopPairBonus = value;
        else if (option.name == "RookOpenFileBonus") params.rookOpenFileBonus = value;
    }
    /// @brief Updates the history heuristic table value for a given element.
    /// From CPW.
    /// @param sideToMove 
    /// @param from 
    /// @param to 
    /// @param bonus 
    void historyTableUpdate(Color sideToMove, int from, int to, int bonus) {
        int clampedBonus = std::clamp(bonus, -HISTORY_MAX, HISTORY_MAX);
        historyTable[sideToMove][from][to]  += clampedBonus
            - (historyTable[sideToMove][from][to] * abs(clampedBonus) / HISTORY_MAX);
    }
    /// @brief 
    /// @param depth 
    /// @return 
    int calculateHistBonus(int depth) {
        return depth*depth;
    }

    /// @brief Alpha Beta pruning function
    /// @param board 
    /// @param depth 
    /// @param alpha 
    /// @param beta 
    /// @param ply 
    /// @return Evaluation score relative to current depth's side (positive better, negative worse)
    int alphaBeta(ChessBoard& board, int depth, int alpha, int beta, int ply, bool isPV) {
        if (depth <= 0 || !isSearching) {
            return quiescenceSearch(board, alpha, beta);
        }
    
        // Check for repetition and 50-move rule
        if (ply > 0 && (board.currentGameState.halfMoveClock >= 100 ||
                        board.keySet.find(board.zobristKey) != board.keySet.end())) {
            return 0;
        }
    
        // Check transposition table
        int score;
        DenseMove hashMove;
        if (checkTT(board, depth, alpha, beta, score, hashMove)) {
            return score;
        }
    
        // Generate and order moves
        int moveNum = 0;
        int quietNum = 0;
        std::array<DenseMove, MAX_MOVES> moves = MoveGenerator::generatePsuedoMoves(board, moveNum);
        std::array<DenseMove, MAX_MOVES> quietsSearched = {0};
        orderMoves(moves, moveNum, ply, hashMove);
    
        bool noLegalMoves = true;
        DenseMove bestMove;
        int flag = TTEntry::ALPHA;
        Color sideToMove = board.getSideToMove();
        bool firstMove = true;  // Track if it's the first move for PVS
    
        for (int i = 0; i < moveNum; i++) {
            board.makeMove(moves[i], true);
    
            // Skip illegal moves
            if (board.isSideInCheck(sideToMove)) {
                board.unmakeMove(moves[i], true);
                continue;
            }
            noLegalMoves = false;
    
            int eval;
    
            if (firstMove) {
                eval = -alphaBeta(board, depth - 1, -beta, -alpha, ply + 1, isPV);
                firstMove = false;
            } else {
                // Late Move Reduction
                // Essentially, if the move is 'less important' to fully check, we can
                // reduce the depth (and therefore branching factor)
                int reduction = 0;
                if (depth >= 3 && !moves[i].isCapture() && !board.isSideInCheck(sideToMove) &&
                !board.isSideInCheck((Color)!sideToMove) && moves[i] != killerMoves[ply] && 
                moves[i] != killerMoves[ply + 1]) {
                    reduction = std::min(2, depth / 2); // Reduce depth slightly for late moves
                }
            
                eval = -alphaBeta(board, depth - 1 - reduction, -alpha - 1, -alpha, ply + 1, false);
            
                if (eval > alpha && eval < beta) {
                    eval = -alphaBeta(board, depth - 1, -beta, -alpha, ply + 1, true);  // Re-search
                }
            }
    
            board.unmakeMove(moves[i], true);
    
            // Beta cutoff
            if (eval >= beta) {
                if (!moves[i].isCapture()) {
                    // Update killer moves
                    if (killerMoves[ply] != moves[i]) {
                        killerMoves[ply + 1] = killerMoves[ply];  // Shift previous killer move
                        killerMoves[ply] = moves[i];
                    }
                    // History update for quites moves that cause beta cutoffs
                    int bonus = calculateHistBonus(depth);
                    Color color = moves[i].getColor();
                    int from = moves[i].getFrom();
                    int to = moves[i].getTo();
                    historyTableUpdate(color, from, to, bonus);

                    // Penalize any other quiet moves searched that didn't cause beta cutoff
                    for (int j = 0; j < quietNum; j++) {
                        if (quietsSearched[j] == moves[i]) continue;
                        Color color = quietsSearched[j].getColor();
                        int from = quietsSearched[j].getFrom();
                        int to = quietsSearched[j].getTo();
                        historyTableUpdate(color, from, to, -bonus);
                    }
                }
                flag = TTEntry::BETA;
                RecordTTEntry(board, moves[i], depth, beta, flag);
                return beta;
            }
    
            if (eval > alpha) {
                alpha = eval;
                bestMove = moves[i];
                flag = TTEntry::EXACT;
            }
            // If this was a quiet move, add it to quietsSearched
            if (!moves[i].isCapture()) {
                quietsSearched[quietNum] = moves[i];
                quietNum++;
            }
        }
    
        // Handle checkmate/stalemate
        if (noLegalMoves) {
            return board.isSideInCheck(sideToMove) ? -params.mateScore + ply : 0;
        }
    
        // Store best move in transposition table
        RecordTTEntry(board, bestMove, depth, alpha, flag);
        return alpha;
    }
    
    /// @brief 
    /// @param board 
    /// @param alpha 
    /// @param beta 
    /// @return 
    int quiescenceSearch(ChessBoard& board, int alpha, int beta) {
        nodeCount++;  // Track nodes searched
    
        // Static evaluation at the root of quiescence search
        int standPat = evaluatePosition(board);
        
        // Beta cutoff - if the current position is already too good
        if (standPat >= beta) {
            return beta; // Cutoff
        }
        
        // Update alpha if the standing evaluation is better
        if (standPat > alpha) {
            alpha = standPat;
        }

        Color sideToMove = board.getSideToMove();
        // Generate capture moves only
        int moveNum = 0;
        std::array<DenseMove, MAX_MOVES> captureMoves = MoveGenerator::generateCaptureMoves(board, moveNum);
    
        // Order captures using MVV-LVA heuristic
        orderCaptures(captureMoves, moveNum);
    
        for (int i = 0; i < moveNum; i++) {
            board.makeMove(captureMoves[i], true);
            // SEE Pruning: Skip bad captures
            if (staticExchangeEvaluation(board, captureMoves[i].getTo(), sideToMove) < 0) {
                board.unmakeMove(captureMoves[i], true);
                continue;
            }
    
            // Skip illegal moves (e.g., moving into check)
            if (board.isSideInCheck(sideToMove)) {
                board.unmakeMove(captureMoves[i], true);
                continue;
            }
    
            // Recursively evaluate captures
            int score = -quiescenceSearch(board, -beta, -alpha);
            board.unmakeMove(captureMoves[i], true);
    
            // Beta cutoff
            if (score >= beta) {
                return beta;  // Prune the remaining captures
            }
    
            // Update alpha
            if (score > alpha) {
                alpha = score;
            }
        }
    
        return alpha;
    }
    /// @brief 
    /// @param board 
    /// @param square 
    /// @param sideToMove 
    /// @return 
    int staticExchangeEvaluation(const ChessBoard& board, int square, Color sideToMove) {
        U64 attackers = board.OppAttacksToSquare(square, sideToMove);
        
        // If no attackers, there's no exchange happening
        if (attackers == 0) return 0;
    
        // Get the piece on the target square (the initial victim)
        PieceType victim = board.getPieceAt(square);
        int gain = getPieceValue(victim);
        
        // Store attackers in a local bitboard for modification
        U64 mutableAttackers = attackers;
        Color currentTurn = sideToMove;
    
        int pieceExchanges[32]; // Stack to store exchange sequence
        int depth = 0;
        pieceExchanges[depth++] = gain;
    
        // Keep swapping turns and resolving exchanges
        while (mutableAttackers) {
            int attackerSquare = std::countr_zero(mutableAttackers);
            mutableAttackers &= mutableAttackers - 1; // Remove attacker from set
    
            PieceType attacker = board.getPieceAt(attackerSquare);
            int attackerValue = getPieceValue(attacker);
    
            // Calculate net gain if the attacker captures
            gain = attackerValue - gain;
            
            // If net gain is negative, stop
            if (gain < 0) break;
    
            // Store the exchange
            pieceExchanges[depth++] = gain;
    
            // Flip turn (next attacker belongs to the opponent)
            currentTurn = (currentTurn == WHITE) ? BLACK : WHITE;
        }
    
        // Find the best exchange sequence
        while (--depth > 0) {
            pieceExchanges[depth - 1] = std::min(-pieceExchanges[depth], pieceExchanges[depth - 1]);
        }
    
        return pieceExchanges[0];
    }
    
    bool keepSearching(ChessClock& clock) const {
        // If clock is set to infinite, always keep searching
        if (clock.isInfinite()) return true;
        // Get remaining time for current player
        auto remainingTime = (clock.getActiveColor() == WHITE) ? 
                            clock.getWhiteTime() : 
                            clock.getBlackTime();
        
        // Estimate time for next depth based on previous depth
        // Each depth typically takes ~4-5x longer than the previous,
        // but some cases 10x, so plan for that
        auto estimatedNextDepthTime = prevDepthTime * 10;

        // Leave some buffer time for move selection and communication
        auto bufferTime = std::chrono::milliseconds(20);

        // Don't use more than 10% of remaining time on one move
        auto maxTimeForMove = remainingTime / 10;

        return estimatedNextDepthTime + bufferTime < maxTimeForMove;
    }

    DenseMove findBestMove(ChessBoard& board, ChessClock& clock, int maxDepth = -1) {
        startSearch();
        searchStartTime = std::chrono::steady_clock::now();
        prevDepthTime = std::chrono::milliseconds(0);  // Reset previous depth time
    
        nodeCount = 0;
        currentMoveNumber = 0;
    
        int actualDepth = (maxDepth > 0) ? maxDepth : searchDepth;
        DenseMove bestMoveOverall;
        int bestScoreOverall = 0;
        const int MIN_DEPTH = 2;
    
        // Generate and order moves
        int moveNum = 0;
        std::array<DenseMove, MAX_MOVES> moves = MoveGenerator::generateLegalMoves(board, moveNum);
    
        // Iterative Deepening Loop
        for (int currDepth = MIN_DEPTH; currDepth <= actualDepth && isSearching; currDepth++) {
            nodeCount = 0;
            currentMoveNumber = 0;
            clock.updateTime();
    
            // Stop search if time is running out
            if (currDepth > MIN_DEPTH && !keepSearching(clock)) {
                break;
            }
    
            // Retrieve principal variation move from the transposition table
            TTEntry* entry = &transpositionTable[board.zobristKey % TT_SIZE];
            DenseMove hashMove;
            if (entry->key == board.zobristKey) {
                hashMove = entry->bestMove;
            }
    
            int alpha = INF_NEG;
            int beta = INF_POS;
            DenseMove bestMove;
            int bestScore = INF_NEG;
            bool firstMove = true;
    
            orderMoves(moves, moveNum, 0, hashMove);
    
            // Search each move
            for (int i = 0; i < moveNum; i++) {
                currentMoveNumber = i + 1;
                currentMove = moves[i];
    
                sendInfo(std::format("currmove {} currmovenumber {}", 
                                       currentMove.toAlgebraic(), currentMoveNumber));
    
                board.makeMove(moves[i], true);
                int score;
    
                if (firstMove) {
                    // First move uses a full window search
                    score = -alphaBeta(board, currDepth - 1, -beta, -alpha, 1, true);
                    firstMove = false;
                } else {
                    // All other moves use PVS
                    score = -alphaBeta(board, currDepth - 1, -alpha - 1, -alpha, 1, false);
                    if (score > alpha && score < beta) {
                        score = -alphaBeta(board, currDepth - 1, -beta, -alpha, 1, true); // Re-search
                    }
                }
    
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
        
        // Get total piece count (not including pawns or kings)
        // for endgame lerp
        totalPiecesWithoutPawns = popcount(board.getAllPieces() & 
                                        (~(board.getDenseSet(D_PAWN) | 
                                         board.getDenseSet(D_KING))));
        // Calculate 'distance' along early game to late game by taking
        // current number of major and minor pieces and initial number of those pieces
        earlygameLerp = (float)totalPiecesWithoutPawns/INIT_MAJ_MIN_PIECES;
        endgameLerp = (float)(std::clamp(8 - totalPiecesWithoutPawns, 0, 8))/INIT_MAJ_MIN_PIECES;

        int score = 0;

        // Material count
        score += countMaterial(board);

        // Positional evaluation
        score += evaluatePositional(board, WHITE);
        score -= evaluatePositional(board, BLACK);

        //Adjust the score based on current side to move
        return (board.getSideToMove() == WHITE) ? score: -score;
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
    static constexpr int KILLER_MOVE_SCORE = 90000;
    static constexpr int PRIORITY_SCORE = 10000;
        
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
            } else {
                // Check for killer moves
                if (moves[i] == killerMoves[ply]) {
                    scored.score = KILLER_MOVE_SCORE;   // Higher priority than normal moves
                } else if (moves[i] == killerMoves[ply+1]) {
                    scored.score = KILLER_MOVE_SCORE - PRIORITY_SCORE;   // Second priority
                }
                // Check and order moves by history
                if (moves[i] != hashMove) {
                    int piece = moves[i].getDenseType() - 1;
                    int color = moves[i].getColor();
                    int to = moves[i].getTo();
                    scored.score = historyTable[color][piece][to];  // Use history heuristic score
                }
                
            }
            
            scoredMoves.push_back(scored);
        }
        
        // Sort moves by score
        std::sort(scoredMoves.begin(), scoredMoves.end());
        
        // Copy back to moves array
        for (int i = 0; i < moveCount; i++) {
            moves[i] = scoredMoves[i].move;
        }
    }
    /// @brief 
    /// @param moves 
    /// @param moveCount 
    void orderCaptures(std::array<DenseMove, MAX_MOVES>& moves, int moveCount) {
        std::vector<ScoredMove> scoredMoves;
        scoredMoves.reserve(moveCount);
    
        for (int i = 0; i < moveCount; i++) {
            DenseMove move = moves[i];
            int score = 0;
    
            if (move.isCapture()) {
                score = getPieceValue(move.getCaptPiece()) * 10 - getPieceValue(move.getPieceType());
            }
    
            scoredMoves.emplace_back(move, score);
        }
    
        // Sort captures in descending order
        std::sort(scoredMoves.begin(), scoredMoves.end());
    
        // Copy back to original moves array
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

    int countMaterial(const ChessBoard& board) {
        int score = 0;
        U64 pieces;

        // White
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
        // Black
        // Pawns
        pieces = board.getBlackPawns();
        score -= popcount(pieces) * params.pawnValue;
        // Knights
        pieces = board.getBlackKnights();
        score -= popcount(pieces) * params.knightValue;
        // Bishops
        pieces = board.getBlackBishops();
        score -= popcount(pieces) * params.bishopValue;
        // Rooks 
        pieces = board.getBlackRooks();
        score -= popcount(pieces) * params.rookValue;
        // Queens
        pieces = board.getBlackQueens();
        score -= popcount(pieces) * params.queenValue;
        
        return score;
    }

    int evaluatePositional(const ChessBoard& board, Color color) {
        int score = 0;

        U64 pawns, pawnRef, knights, bishops, rooks, queens;
        int kingSquare;
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
        } else {
            flip = 56;
            pawns = board.getBlackPawns();
            pawnRef = pawns;
            knights = board.getBlackKnights();
            bishops = board.getBlackBishops();
            rooks = board.getBlackRooks();
            queens = board.getBlackRooks();
            kingSquare = board.getBlackKingSquare() ^ flip;
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

            // Passed pawn bonus
            U64 enemyPawns = color == WHITE ? board.getBlackPawns() : board.getWhitePawns();
            // Include the pawn's own file in to make sure its a fully passed pawn
            adjacentFiles |= BUTIL::FileMask << file;
            if (!(adjacentFiles & enemyPawns)) {
                score += params.passedPawnBonus;
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
            score += knightSqTbEarly[square]*earlygameLerp + knightSqTbEnd[square]*endgameLerp;
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
            score +=  bishopSqTbEarly[square]*earlygameLerp + bishopSqTbEnd[square]*endgameLerp;
            bishops &= bishops - 1;
        }
        // Rooks
        while (rooks) {
            int square = std::countr_zero(rooks) ^ flip;
            score += rookSqTbEarly[square]*earlygameLerp + rookSqTbEnd[square]*endgameLerp;
            // Open file bonus
            int file = BUTIL::indexToFile(square);
            if (!((BUTIL::FileMask << file) & occupancy)) score += params.rookOpenFileBonus;

            rooks &= rooks - 1;
        }
        // Queens
        while (queens) {
            int square = std::countr_zero(queens) ^ flip;
            score += queenSqTbEarly[square]*earlygameLerp + queenSqTbEnd[square]*endgameLerp;
            queens &= queens - 1;
        }
        // King
        score += kingSqTbEarly[kingSquare]*earlygameLerp + kingSqTbEnd[kingSquare]*endgameLerp;
        kingSquare ^= flip;
        // Give bonus for king behind pawns
        U64 kingShield = ATKMASK_KING[kingSquare] & pawnRef;
        if (popcount(kingShield) >= 2 && (kingSquare < 8 || kingSquare >= 56)) {
            score += params.kingShieldBonus;
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
       -10,-25,-40,-40,-40,-40,-25,-10,
        20, 30, 10,  0,  0, 10, 40, 20
   };
   // Pawns - Endgame
   static constexpr std::array<int, 64> pawnSqTbEnd = {
       0,  0,  0,  0,  0,  0,  0,  0,
     120,120,120,120,120,120,120,120,
       80, 80, 80, 80, 80, 80, 80, 80,
       50, 50, 50, 50, 50, 50, 50, 50,
       30, 30, 30, 30, 30, 30, 30, 30,
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
};