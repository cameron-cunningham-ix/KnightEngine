// Thanks to Bruce Moreland via
// http://www.brucemo.com/compchess/programming/zobrist.htm

#include "types.hpp"
#include <random>
/// @return Returns a random 64-bit number
U64 rand64() {
}

/// @brief 
class Zobrist {
private:
    // RNG seed
    // DO NOT CHANGE
    const U64 RNG_SEED = 917346852;
    // Color offsets for indexing into zobrist numbers
    static constexpr int whiteOffset = 0;
    static constexpr int blackOffset = 384;

    // We can use the values of D_TYPE - 1 and the square index
    // to index the rest correctly
    std::array<U64, 768> zobristPieces;

    // 16 possibilities for castling rights of both players
    std::array<U64, 16> zobristCastle;
    // 
    U64 blackToMove;

    // Generate random 64-bit numbers to use in Zobrist hashing
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
    }

};