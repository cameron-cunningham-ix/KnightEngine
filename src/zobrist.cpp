#include "zobrist.hpp"
// Whether Zobrist has been initialized yet or not
bool Zobrist::initialized = false;
std::array<U64, 768> Zobrist::zobristPieces;
U64 Zobrist::zobristWhiteToMove = 0ULL;
U64 Zobrist::zobristSideToMove = 0ULL;
// 16 possibilities for castling rights of both players
// Index corresponds to binary for rights: 0 - no rights
// 15 - both players can castle either side; KQkq
std::array<U64, 16> Zobrist::zobristCastle;
// 8 possible files for en passant
std::array<U64, 8> Zobrist::zobristEnPass;

/// @brief Generate random 64-bit numbers to use in Zobrist hashing
void Zobrist::initialize() {
    // Create and seed RNG
    std::mt19937_64 rng(RNG_SEED);
    // Generate random numbers for each piece, color, and square combo
    for (int square = 0; square < 64; square++) {   // Per square
        for (int color = 0; color < 2; color++) {
            for (int type = 1; type < 7; type++) {
                zobristPieces[(square*12) + (color*6) + (type - 1)] = rng();
            }
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
    zobristWhiteToMove = rng();
    zobristSideToMove = rng();
    initialized = true;
}

U64 Zobrist::getPieceSqKey(int sq, PieceType piece) {
    int color = colorCode(piece);
    int dtype = pieceCode(piece);
    // sq*12 - 12 possible pieces per square
    // color*6 - 6 possible pieces per color
    // dtype - 1 - type offset
    return Zobrist::zobristPieces[(sq*12) + (color*6) + (dtype - 1)];
}