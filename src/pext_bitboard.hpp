#pragma once

#include "types.hpp"
#include "utility.hpp"
#include <immintrin.h>
#include <array>
#include <vector>

/// @brief Helper class to generate the PEXT bitboards
/// used for calculating sliding piece attacks
class PEXT {
public:

    static bool initialized;
    static void initialize();
    static U64 getRookAttacks(int square, U64 occupancy);
    static U64 getBishopAttacks(int square, U64 occupancy);

private:
    // Attack masks for each square
    static std::array<U64, 64> rookMasks;
    static std::array<U64, 64> bishopMasks;
    
    // Precomputed move tables
    static std::array<std::vector<U64>, 64> rookMoves;
    static std::array<std::vector<U64>, 64> bishopMoves;


    static U64 generateRookMask(int square);
    static U64 generateBishopMask(int square);
    static U64 generateRookAttacks(int square, U64 occupancy);
    static U64 generateBishopAttacks(int square, U64 occupancy);

};