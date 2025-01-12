#pragma once

#include "types.hpp"
#include "board_generation.hpp"
#include "utility.hpp"
#include <vector>
#include <array>
#include <string>

// Move validator class declaration
class MoveGenerator {
private:
    
    // Move generation functions
    static void generatePawnMoves(const ChessBoard& board, std::array<DenseMove, MAX_MOVES>& moves, int& moveNum);
    static void generateEnPassantMoves(const ChessBoard &board, std::array<DenseMove, MAX_MOVES>& moves, int& moveNum);
    static void generateCastlingMoves(const ChessBoard& board, std::array<DenseMove, MAX_MOVES>& moves, int& moveNum);
    static void generatePieceMoves(const ChessBoard &board, std::array<DenseMove, MAX_MOVES>& moves, int& moveNum);
    
public:
    static std::array<DenseMove, MAX_MOVES> generatePsuedoMoves(const ChessBoard &board, int& moveNum);
    static std::array<DenseMove, MAX_MOVES> generateLegalMoves(ChessBoard &board, int& moveNum);
};

