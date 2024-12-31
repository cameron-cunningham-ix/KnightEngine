#include <vector>
#include <unordered_map>
#include "types.hpp"
#include "utility.hpp"

namespace std {
    template<>
    struct hash<std::pair<int, U64>> {
        size_t operator()(const std::pair<int, U64>& p) const {
            return hash<int>()(p.first) ^ hash<U64>()(p.second);
        }
    };
}

struct MagicEntry {
    U64 mask;
    U64 magic;
    unsigned char index_bits;
};


// Generates the move mask of a rook for one square.
// This mask excludes squares on the edge of the board
U64 generateRookMoveMask(int square) {
    U64 attacks = 0ULL;
    // Rank mask: all squares on the same rank (horizontal)
    // except for squares at edge
    attacks |= 0x000000000000007EULL << (8*(square / 8));
    
    // File mask: all squares on the same file (vertical)
    // except for squares at edge
    attacks |= 0x0001010101010100ULL << (square % 8);

    // Remove the rook's current position from the attack mask
    attacks &= ~(1ULL << square);
    return attacks;
}

// Generates the move mask of a rook for one square based on occupancy.
// This mask excludes squares on the edge of the board
U64 getRookAttackMask(int square, U64 blockers) {
    U64 attacks = generateRookMoveMask(square);

    // Check each direction (N, S, E, W) for blockers
    // South
    for (int sq = square - 8; sq >= 0; sq -= 8) {
        if (blockers & (1ULL << sq)) {
            U64 file = 0ULL;
            for (int sSq = sq - 8; sSq >= 0; sSq -= 8) {
                file |= 1ULL << sSq;
            }
            attacks &= ~file;
            break;
        }
    }

    // West
    for (int sq = square - 1; sq % 8 != 7 && sq >= 0; sq--) {
        if (blockers & (1ULL << sq)) {
            U64 rank = 0ULL;
            for (int rankSq = sq - 1; rankSq % 8 != 7 && rankSq >= 0; rankSq--) {
                rank |= 1ULL << rankSq;
            }
            attacks &= ~rank;
            break;
        }
    }

    // East
    for (int sq = square + 1; sq % 8 != 0 && sq < 64; sq++) {
        if (blockers & (1ULL << sq)) {
            U64 rank = 0ULL;
            for (int rankSq = sq + 1; rankSq % 8 != 0 && rankSq < 64; rankSq++) {
                rank |= 1ULL << rankSq;
            }
            attacks &= ~rank;
            break;
        }
    }

    // North
    for (int sq = square + 8; sq < 64; sq += 8) {
        if (blockers & (1ULL << sq)) {
            U64 file = 0ULL;
            for (int nSq = sq + 8; nSq < 64; nSq += 8){
                file |= 1ULL << nSq;
            }
            attacks &= ~file;
            break;
        }
    }
    
    return attacks;
}

// Generates the move mask of a rook for all 64 squares.
// Masks exclude squares on the edge of the board
std::vector<U64> generateRookMoveMasks() {
    std::vector<U64> rookMoveMasks = std::vector<U64>();
    for (int i = 0; i < 64; i++) {
        rookMoveMasks.emplace_back(generateRookMoveMask(i));
    }
    return rookMoveMasks;
};

// Generates the move mask of a bishop for one square.
// This mask excludes squares on the edge of the board ( I think, need to check!!!)
std::vector<U64> generateBishopMoveMasks() {
    std::vector<U64> bishopMoveMasks = std::vector<U64>();
    for (int i = 0; i < 64; i++) {
        U64 attacks = 0ULL;
        // Top right diagonal
        int tr = 9;
        while ((i + tr) < 64 && ((i + tr) % 8 != 0)) {
            attacks |= (1ULL << (i + tr));
            tr += 9;
        }
        // Clear top right square
        attacks &= ~(1ULL << (i + tr - 9));
        // Bottom right diagonal
        int br = 7;
        while ((i - br) >= 0 && ((i - br) % 8 != 0)) {
            attacks |= (1ULL << (i - br));
            br += 7;
        }
        attacks &= ~(1ULL << (i - br + 7));
        // Bottom left diagonal
        int bl = 9;
        while ((i - bl) >= 0 && ((i - bl) % 8 != 7)) {
            attacks |= (1ULL << (i - bl));
            bl += 9;
        }
        attacks &= ~(1ULL << (i - bl + 9));
        // Top left diagonal
        int tl = 7;
        while ((i + tl) < 64 && ((i + tl) % 8 != 7)) {
            attacks |= (1ULL << (i + tl));
            tl += 7;
        }
        attacks &= ~(1ULL << (i + tl - 7));
        bishopMoveMasks.emplace_back(attacks);
    }
    return bishopMoveMasks;
}

// Create all of the possible patterns of blocking pieces based on the given mask
std::vector<U64> createAllBlockerBitboards(U64 mask) {
    // Create a vector of the indices of the bits that are set in mask
    std::vector<int> moveSquareIndices = std::vector<int>();
    for (int i = 0; i < 64; i++) {
        if ((mask >> i) & 1) {
            moveSquareIndices.emplace_back(i);
        }
    }

    // Calculate total number of different bitboards (2^n)
    int numBB = 1 << moveSquareIndices.size();

    std::vector<U64> blockers = std::vector<U64>(numBB);
    // Create all blocker bitboards
    for (int numIndex = 0; numIndex < numBB; numIndex++) {
        for (int bitIndex = 0; bitIndex < moveSquareIndices.size(); bitIndex++) {
            int bit = (numIndex >> bitIndex) & 1;
            blockers[numIndex] |= (U64)bit << moveSquareIndices[bitIndex];
        }
    }
    return blockers;
}

// 
std::unordered_map<std::pair<int, U64>, U64> createRookLookupTable() {
    std::unordered_map<std::pair<int, U64>, U64> rookMovesLookup = std::unordered_map<std::pair<int, U64>, U64>();

    std::vector<U64> movementMasks = generateRookMoveMasks();

    for (int square = 0; square < 64; square++) {
        std::vector<U64> blockerPatterns = createAllBlockerBitboards(movementMasks[square]);
        for (int i = 0; i < blockerPatterns.size(); i++) {
            U64 legalMoveBitboard = getRookAttackMask(square, blockerPatterns[i]);
            std::pair<int, U64> key = std::pair<int, U64>(square, blockerPatterns[i]);
            rookMovesLookup.insert(std::make_pair(key, legalMoveBitboard));
        }
    }
    return rookMovesLookup;
}