// Thanks to Bruce Moreland via
// http://www.brucemo.com/compchess/programming/zobrist.htm

#include "types.hpp"
#include "board_generation.hpp"
#include <random>

/// @brief Helper class for generating and using Zobrist keys
class Zobrist {
/// Zobrist keys are 64-bit numbers that represent a position on the board
/// They 
private:
    // RNG seed
    // DO NOT CHANGE
    const U64 RNG_SEED = 917346852;
    // Color offsets for indexing into zobrist numbers
    static constexpr int whiteOffset = 0;
    static constexpr int blackOffset = 384;

public:
    // We can use the values of D_TYPE - 1 and the square index
    // to index the rest correctly
    static std::array<U64, 768> zobristPieces;
    // When it's Black's turn, we also XOR this into the key
    static U64 zobristBlackToMove;
    // 16 possibilities for castling rights of both players
    // Index corresponds to binary for rights: 0 - no rights
    // 15 - both players can castle either side; KQkq
    static std::array<U64, 16> zobristCastle;
    // 8 possible files for en passant
    static std::array<U64, 8> zobristEnPass;

    /// @brief Generate random 64-bit numbers to use in Zobrist hashing
    Zobrist() {
        // Create and seed RNG
        std::mt19937_64 rng(RNG_SEED);
        // Generate random numbers for each piece, color, and square combo
        for (int sq = 0; sq < 64; sq++) {      // Per square
            // Per dense type
            for (int dtype = 0; dtype < 6; dtype++) {
                zobristPieces[whiteOffset + sq + dtype] = rng();
                zobristPieces[blackOffset + sq + dtype] = rng();
            }
        }
        // Generate for castling rights
        for (int i = 0; i < 16; i++) {
            zobristCastle[i] = rng();
        }
        // Generate en passant files
        for (int i = 0; i < 8; i++) {
            zobristEnPass[i] = rng();
        }
        zobristBlackToMove = rng();
    }

    /// @brief 
    /// @param board 
    /// @return 
    U64 GenerateZobristKey(ChessBoard board) {
        U64 zobristKey = 0ULL;

        for (int sq = 0; sq < 64; sq++) {
            PieceType piece = board.getPieceAt(sq);
            if (piece == PieceType::EMPTY) continue;
            int color = colorCode(piece);
            int dtype = pieceCode(piece);
            zobristKey ^= zobristPieces[color + sq + dtype];
        }

        if (board.getSideToMove() == BLACK)
            zobristKey ^= zobristBlackToMove;

        zobristKey ^= zobristCastle[board.currentGameState.getCastleRights()];

        if (board.currentGameState.enPassantSquare != -1) {
            zobristKey ^= zobristEnPass[board.currentGameState.getEnPassantFileIndex()];
        }

        return zobristKey;
    }

};