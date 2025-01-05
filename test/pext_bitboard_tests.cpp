#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include "../src/pext_bitboard.hpp"
#include "../src/utility.hpp"

/// @brief Generate a random U64 number based on seed
/// @return 
static U64 randomU64() {
    U64 u1, u2, u3, u4;
    u1 = (U64)(rand()) & 0xFFFF;
    u2 = (U64)(rand()) & 0xFFFF;
    u3 = (U64)(rand()) & 0xFFFF;
    u4 = (U64)(rand()) & 0xFFFF;
    return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}
static U64 randomU64_fewbits() {
    return randomU64() & randomU64() & randomU64();
}

class PEXTTest : public ::testing::Test {

    void SetUp() override {
        PEXT::initialize();
    }

};

TEST_F(PEXTTest, GetRookAttacks) {
    std::ofstream outfile("rookMoves.txt");
    if (outfile.is_open()) {
        outfile << "Rook moves: " << std::endl;
    }
    std::cout.rdbuf(outfile.rdbuf());

    for (int square = 0; square < 64; square++) {
        int size = PEXT::rookMoves[square].size();
        for (int index = 0; index < size; index++) {
            std::cout << "square " << square << " index " << index << " - " << std::bitset<12>(index) << "\n";
            printBBLine(PEXT::rookMoves[square][index]);
        }
    }
}

TEST_F(PEXTTest, RookAttacks) {
    std::ofstream outfile("rookMoves.txt");
    if (outfile.is_open()) {
        outfile << "Rook moves: " << std::endl;
    }
    std::cout.rdbuf(outfile.rdbuf());

    for (int square = 0; square < 64; square++) {
        U64 random = randomU64_fewbits();
        printBitboard(random);
        U64 attacks = PEXT::getRookAttacks(square, random);
        printBitboard(attacks); 
    }
}

TEST_F(PEXTTest, BishopAttacks) {
    std::ofstream outfile("bishopMoves.txt");
    if (outfile.is_open()) {
        outfile << "Bishop moves: " << std::endl;
    }
    std::cout.rdbuf(outfile.rdbuf());

    for (int square = 0; square < 64; square++) {
        U64 random = randomU64_fewbits();
        printBitboard(random);
        U64 attacks = PEXT::getBishopAttacks(square, random);
        printBitboard(attacks); 
    }
}