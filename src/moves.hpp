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
    static void generatePawnMoves(const ChessBoard& board, std::array<DenseMove, MAX_MOVES>& moves, int& moveIndex);
    static void generateEnPassantMoves(const ChessBoard &board, std::array<DenseMove, MAX_MOVES>& moves, int& moveIndex);
    static void generateCastlingMoves(const ChessBoard& board, std::array<DenseMove, MAX_MOVES>& moves, int& moveIndex);
    static void generatePieceMoves(const ChessBoard &board, std::array<DenseMove, MAX_MOVES>& moves, int& moveIndex, PieceType piece);
    static std::array<DenseMove, MAX_MOVES> generatePsuedoMoves(ChessBoard &board);
    
public:
    static std::array<DenseMove, MAX_MOVES> generateLegalMoves(ChessBoard &board);
};

