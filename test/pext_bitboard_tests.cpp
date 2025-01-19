#include "../src/pext_bitboard.hpp"
#include "../src/utility.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <fstream>

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

TEST_F(PEXTTest, RookAttacks) {
    std::ofstream outfile("TestOutput/rookMoves.txt");
    std::streambuf* coutBuf = std::cout.rdbuf();
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
    std::cout.rdbuf(coutBuf);
    outfile.close();
}

TEST_F(PEXTTest, BishopAttacks) {
    std::ofstream outfile("TestOutput/bishopMoves.txt");
    std::streambuf* coutBuf = std::cout.rdbuf();
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
    std::cout.rdbuf(coutBuf);
    outfile.close();
}