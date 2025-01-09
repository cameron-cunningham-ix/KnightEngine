#pragma once

#include "types.hpp"
#include "utility.hpp"
#include <immintrin.h>
#include <array>
#include <vector>

class PEXT {
public:
    // Attack masks for each square
    static std::array<U64, 64> rookMasks;
    static std::array<U64, 64> bishopMasks;
    
    // Precomputed move tables
    static std::array<std::vector<U64>, 64> rookMoves;
    static std::array<std::vector<U64>, 64> bishopMoves;

    static void initialize();
    static U64 getRookAttacks(int square, U64 occupancy);
    static U64 getBishopAttacks(int square, U64 occupancy);

    static U64 generateRookMask(int square);
    static U64 generateBishopMask(int square);
    static U64 generateRookAttacks(int square, U64 occupancy);
    static U64 generateBishopAttacks(int square, U64 occupancy);

    static bool initialized;
};