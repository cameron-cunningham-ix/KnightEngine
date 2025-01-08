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
    static void generatePawnMoves(const ChessBoard& board, std::vector<DenseMove>& moves);
    static void generateEnPassantMoves(const ChessBoard &board, std::vector<DenseMove>& moves);
    static void generateCastlingMoves(const ChessBoard& board, std::vector<DenseMove>& moves);
    static void generatePieceMoves(const ChessBoard &board, std::vector<DenseMove>& moves, PieceType piece);
    static std::vector<DenseMove> generatePsuedoMoves(const ChessBoard &board);
    
public:
    static std::vector<DenseMove> generateLegalMoves(ChessBoard &board);
};

