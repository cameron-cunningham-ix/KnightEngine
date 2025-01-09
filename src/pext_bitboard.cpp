#include "pext_bitboard.hpp"
#include "board_utility.hpp"

// Define static members
bool PEXT::initialized = false;
/// @brief Relevant occupancy masks for rooks (excludes edge squares)
std::array<U64, 64> PEXT::rookMasks;
/// @brief Relevant occupancy masks for bishops (excludes edge squares)
std::array<U64, 64> PEXT::bishopMasks;
/// @brief Possible rook attacks based on PEXT index occupancy
std::array<std::vector<U64>, 64> PEXT::rookMoves;
/// @brief Possible bishop attacks based on PEXT index occupancy
std::array<std::vector<U64>, 64> PEXT::bishopMoves;

/// @brief Initializes rookMoves and bishopMoves arrays
void PEXT::initialize() {
    // If PEXT has already been initialized before, you can return
    if (initialized) return;
    // Initialize attack masks and move tables
    for (int square = 0; square < 64; square++) {
        rookMasks[square] = generateRookMask(square);
        bishopMasks[square] = generateBishopMask(square);
        
        // Pre-calculate all possible moves for each occupancy pattern
        int rookBits = popcount(rookMasks[square]);
        // std::cout << "square " << square << " rook bits " << rookBits << "\n";
        int bishopBits = popcount(bishopMasks[square]);
        
        rookMoves[square].resize(1ULL << rookBits);
        // std::cout << (1ULL << rookBits) << "vars\n";
        bishopMoves[square].resize(1ULL << bishopBits);
        
        // Generate all possible occupancy patterns
        for (U64 occ = 0; occ < (1ULL << rookBits); occ++) {
            U64 actualOcc = _pdep_u64(occ, rookMasks[square]);
            rookMoves[square][occ] = generateRookAttacks(square, actualOcc);
        }
        
        for (U64 occ = 0; occ < (1ULL << bishopBits); occ++) {
            U64 actualOcc = _pdep_u64(occ, bishopMasks[square]);
            bishopMoves[square][occ] = generateBishopAttacks(square, actualOcc);
        }
    }
    initialized = true;
}
/// @brief 
/// @param square 
/// @param occupancy 
/// @return Bitboard of possible rook attacks based on occupancy
U64 PEXT::getRookAttacks(int square, U64 occupancy) {
    U64 index = _pext_u64(occupancy, rookMasks[square]);
    return rookMoves[square][index];
}

// U64 getBlockingRookMask(int square, U64 occupancy, U64 RQPieces) {
//     U64 rookMoves = PEXT::getRookAttacks(square, occupancy);
//     // Mask east
//     if (RQPieces & rookMoves & )
// }
/// @brief 
/// @param square 
/// @param occupancy 
/// @return Bitboard of possible bishop attacks based on occupancy
U64 PEXT::getBishopAttacks(int square, U64 occupancy) {
    U64 index = _pext_u64(occupancy, bishopMasks[square]);
    return bishopMoves[square][index];
}
/// @brief Generates the relevant occupancy mask of a rook on square.
/// This mask excludes squares on the edge of the board
/// @param square 
/// @return Bitboard of relevant occupancy mask
U64 PEXT::generateRookMask(int square) {
    U64 attacks = 0ULL;
    // Rank mask: all squares on the same rank (horizontal)
    // except for squares at edge
    attacks |= 0x000000000000007EULL << (8*(square / 8));
    _pext_u64(0ULL, 1ULL);
    
    // File mask: all squares on the same file (vertical)
    // except for squares at edge
    attacks |= 0x0001010101010100ULL << (square % 8);

    // Remove the rook's current position from the attack mask
    attacks &= ~(1ULL << square);
    return attacks;
}
/// @brief Generates the relevant occupancy mask of a bishop on square.
/// This mask excludes squares on the edge of the board
/// @param square 
/// @return Bitboard of relevant occupancy mask
U64 PEXT::generateBishopMask(int square) {
    U64 attacks = 0ULL;
    // Top right diagonal
    int tr = 9;
    while ((square + tr) < 64 && ((square + tr) % 8 != 0)) {
        attacks |= (1ULL << (square + tr));
        tr += 9;
    }
    // Clear top right square
    attacks &= ~(1ULL << (square + tr - 9));
    // Bottom right diagonal
    int br = 7;
    while ((square - br) >= 0 && ((square - br) % 8 != 0)) {
        attacks |= (1ULL << (square - br));
        br += 7;
    }
    attacks &= ~(1ULL << (square - br + 7));
    // Bottom left diagonal
    int bl = 9;
    while ((square - bl) >= 0 && ((square - bl) % 8 != 7)) {
        attacks |= (1ULL << (square - bl));
        bl += 9;
    }
    attacks &= ~(1ULL << (square - bl + 9));
    // Top left diagonal
    int tl = 7;
    while ((square + tl) < 64 && ((square + tl) % 8 != 7)) {
        attacks |= (1ULL << (square + tl));
        tl += 7;
    }
    attacks &= ~(1ULL << (square + tl - 7));
    return attacks;
}
/// @brief Generates possible rook attacks / moves from square based on occupancy
/// @param square 
/// @param occupancy 
/// @return Bitboard of possible rook attacks and moves
U64 PEXT::generateRookAttacks(int square, U64 occupancy) {
    U64 attacks = ATKMASK_ROOK[square];
    // Check each direction (N, S, E, W) for blockers
    // South
    for (int sq = square - 8; sq >= 0; sq -= 8) {
        if (occupancy & (1ULL << sq)) {
            U64 file = 0ULL;
            for (int sSq = sq - 8; sSq >= 0; sSq -= 8) {
                file |= 1ULL << sSq;
            }
            attacks ^= file;
            break;
        }
    
    }

    // West
    for (int sq = square - 1; sq % 8 != 7 && sq >= 0; sq--) {
        if (occupancy & (1ULL << sq)) {
            U64 rank = 0ULL;
            for (int rankSq = sq - 1; rankSq % 8 != 7 && rankSq >= 0; rankSq--) {
                rank |= 1ULL << rankSq;
            }
            attacks ^= rank;
            break;
        }
    }

    // East
    for (int sq = square + 1; sq % 8 != 0 && sq < 64; sq++) {
        if (occupancy & (1ULL << sq)) {
            U64 rank = 0ULL;
            for (int rankSq = sq + 1; rankSq % 8 != 0 && rankSq < 64; rankSq++) {
                rank |= 1ULL << rankSq;
            }
            attacks ^= rank;
            break;
        }
    }

    // North
    for (int sq = square + 8; sq < 64; sq += 8) {
        if (occupancy & (1ULL << sq)) {
            U64 file = 0ULL;
            for (int nSq = sq + 8; nSq < 64; nSq += 8){
                file |= 1ULL << nSq;
            }
            attacks ^= file;
            break;
        }
    }
    
    return attacks;
}
/// @brief Generates possible bishop attacks / moves from square based on occupancy
/// @param square 
/// @param occupancy 
/// @return Bitboard of possible bishop attacks and moves
U64 PEXT::generateBishopAttacks(int square, U64 occupancy) {
    U64 attacks = 0;
    int rank = square / 8;
    int file = square % 8;

    // Northeast
    for (int r = rank + 1, f = file + 1; r < 8 && f < 8; r++, f++) {
        U64 target = 1ULL << (r * 8 + f);
        attacks |= target;
        if (occupancy & target) break;
    }

    // Southeast
    for (int r = rank - 1, f = file + 1; r >= 0 && f < 8; r--, f++) {
        U64 target = 1ULL << (r * 8 + f);
        attacks |= target;
        if (occupancy & target) break;
    }

    // Southwest
    for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--) {
        U64 target = 1ULL << (r * 8 + f);
        attacks |= target;
        if (occupancy & target) break;
    }

    // Northwest
    for (int r = rank + 1, f = file - 1; r < 8 && f >= 0; r++, f--) {
        U64 target = 1ULL << (r * 8 + f);
        attacks |= target;
        if (occupancy & target) break;
    }

    return attacks;
}