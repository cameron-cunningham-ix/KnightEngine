// Thanks to Bruce Moreland via
// http://www.brucemo.com/compchess/programming/zobrist.htm

#include "types.hpp"
#include <random>

/// @brief Helper class for generating and using Zobrist keys
class Zobrist {
/// Zobrist keys are 64-bit numbers that represent a position on the board
/// They 
private:
    // RNG seed
    // DO NOT CHANGE
    static constexpr U64 RNG_SEED = 917346853;
    // Color offsets for indexing into zobristPieces
    static constexpr int whiteOffset = 0;
    static constexpr int blackOffset = 1;
    // We use these values along with D_TYPE - 1 and the square index
    // to index zobristPieces correctly

public:
    // Whether Zobrist has been initialized yet or not
    static bool initialized;
    static std::array<U64, 768> zobristPieces;
    static U64 zobristWhiteToMove;
    // When it's Black's turn, we also XOR this into the key
    static U64 zobristSideToMove;
    // 16 possibilities for castling rights of both players
    // Index corresponds to binary for rights: 0 - no rights
    // 15 - both players can castle either side; KQkq
    static std::array<U64, 16> zobristCastle;
    // 8 possible files for en passant
    static std::array<U64, 8> zobristEnPass;

    /// @brief Generate random 64-bit numbers to use in Zobrist hashing
    static void initialize();
    static U64 getPieceSqKey(int sq, PieceType piece);
};