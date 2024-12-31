#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include "../src/magic_bitboards.hpp"
#include "../src/utility.hpp"

class MagicTest : public ::testing::Test {

};

// Generate all rook movement masks for magic bitboards
TEST(MagicTest, generateRookMoveMasks) {
    std::vector<U64> rookMoveMasks = generateRookMoveMasks();
    for (int i = 0; i < 64; i++) {
        printBitboard(rookMoveMasks[i]);
    }
}

// Generate all rook movement masks for magic bitboards
TEST(MagicTest, generateBishopMoveMasks) {
    std::vector<U64> bishopMoveMasks = generateBishopMoveMasks();
    for (int i = 0; i < 64; i++) {
        printBitboard(bishopMoveMasks[i]);
    }
}

// Generate all rook blocker bitboards
TEST(MagicTest, GenerateRookBlockers) {
    std::ofstream outfile("rookBlockers.txt");
    if (outfile.is_open()) {
        outfile << "List of all possible rook blocker patterns" << std::endl;
    }
    
    std::vector<U64> rookMasks = generateRookMoveMasks();
    for (int i = 0; i < rookMasks.size(); i++) {
        std::vector<U64> blockers = createAllBlockerBitboards(rookMasks[i]);
        for (int j = 0; j < blockers.size(); j++) {
            std::bitset<64> bb (blockers[j]);
            std::cout << bb << "\n";
            outfile << bb << std::endl;
        }
        std::cout << "\n\nNext index: " << i << "\n\n";
    }
    
}

// Generate all bishop blocker bitboards
TEST(MagicTest, GenerateBishopBlockers) {
    std::ofstream outfile("bishopBlockers.txt");
    if (outfile.is_open()) {
        outfile << "List of all possible bishop blocker patterns" << std::endl;
    }
    
    std::vector<U64> bishopMasks = generateBishopMoveMasks();
    for (int i = 0; i < bishopMasks.size(); i++) {
        std::vector<U64> blockers = createAllBlockerBitboards(bishopMasks[i]);
        for (int j = 0; j < blockers.size(); j++) {
            std::bitset<64> bb (blockers[j]);
            std::cout << bb << "\n";
            outfile << bb << std::endl;
        }
        std::cout << "\n\nNext index: " << i << "\n\n";
    }
    
}

TEST(MagicTest, GenerateRookLUT) {
    std::ofstream outfile("rookLUT.txt");
    if (outfile.is_open()) {
        outfile << "List of all possible rook moves for blocking patterns" << std::endl;
    }
    
    std::unordered_map<std::pair<int, U64>, U64> rookMovesLookup = createRookLookupTable();
    std::unordered_map<std::pair<int, U64>, U64>::iterator itr;

    for (itr = rookMovesLookup.begin(); itr != rookMovesLookup.end(); itr++) {
        // outfile << "Square index: " << itr->first.first << std::endl;
        // outfile << "Occupancy: " << std::endl;
        // std::bitset<64> occ (itr->first.second);
        std::bitset<64> legal (itr->second);

        // outfile << "\n";
        // for (int j = 7; j >= 0; j--){
        //     for (int k = 0; k < 8; k++){
        //         outfile << occ[j*8 + k];
        //     }
        //     outfile << "\n";
        // }
        // outfile << "\n";

        // outfile << "\n";
        // for (int j = 7; j >= 0; j--){
        //     for (int k = 0; k < 8; k++){
        //         outfile << legal[j*8 + k];
        //     }
        //     outfile << "\n";
        // }
        // outfile << "\n";

        outfile << legal << std::endl;
    }
}