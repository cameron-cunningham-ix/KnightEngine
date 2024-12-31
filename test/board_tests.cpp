// test/board_tests.cpp
#include <gtest/gtest.h>
#include "../src/board_generation.hpp"
#include <utility.hpp>

class ChessBoardTest : public ::testing::Test {
protected:
    ChessBoard board;  // Fresh board for each test

    void SetUp() override {
        // Called before each test
        board = ChessBoard();
    }
};

// Test initial board setup
TEST_F(ChessBoardTest, InitialBoardSetup) {
    // Test white pieces initial positions
    EXPECT_EQ(board.getWhitePawns(), 0x000000000000FF00ULL);
    EXPECT_EQ(board.getWhiteKnights(), 0x0000000000000042ULL);
    EXPECT_EQ(board.getWhiteBishops(), 0x0000000000000024ULL);
    EXPECT_EQ(board.getWhiteRooks(), 0x0000000000000081ULL);
    EXPECT_EQ(board.getWhiteQueens(), 0x0000000000000008ULL);
    EXPECT_EQ(board.getWhiteKings(), 0x0000000000000010ULL);

    // Test black pieces initial positions
    EXPECT_EQ(board.getBlackPawns(), 0x00FF000000000000ULL);
    EXPECT_EQ(board.getBlackKnights(), 0x4200000000000000ULL);
    EXPECT_EQ(board.getBlackBishops(), 0x2400000000000000ULL);
    EXPECT_EQ(board.getBlackRooks(), 0x8100000000000000ULL);
    EXPECT_EQ(board.getBlackQueens(), 0x0800000000000000ULL);
    EXPECT_EQ(board.getBlackKings(), 0x1000000000000000ULL);

    EXPECT_EQ(board.getAllPieces(), 0xFFFF00000000FFFFULL);
}

// Test bitboard operations
TEST_F(ChessBoardTest, BitboardOperations) {
    // Test setBit
    U64 bb = 0ULL;
    board.setBit(bb, 8);  // Set a2 square
    EXPECT_EQ(bb, 0x0000000000000100ULL);

    // Test clearBit
    board.clearBit(bb, 8);
    EXPECT_EQ(bb, 0ULL);

    // Test isBitSet
    board.setBit(bb, 8);
    EXPECT_TRUE(board.isBitSet(bb, 8));
    EXPECT_FALSE(board.isBitSet(bb, 9));
}

// Test getPieceBitboard
TEST_F(ChessBoardTest, GetPieceSet) {
    // Test white pieces initial positions
    EXPECT_EQ(board.getPieceSet(W_PAWN), 0x000000000000FF00ULL);
    EXPECT_EQ(board.getPieceSet(W_KNIGHT), 0x0000000000000042ULL);
    EXPECT_EQ(board.getPieceSet(W_BISHOP), 0x0000000000000024ULL);
    EXPECT_EQ(board.getPieceSet(W_ROOK), 0x0000000000000081ULL);
    EXPECT_EQ(board.getPieceSet(W_QUEEN), 0x0000000000000008ULL);
    EXPECT_EQ(board.getPieceSet(W_KING), 0x0000000000000010ULL);

    // Test black pieces initial positions
    EXPECT_EQ(board.getPieceSet(B_PAWN), 0x00FF000000000000ULL);
    EXPECT_EQ(board.getPieceSet(B_KNIGHT), 0x4200000000000000ULL);
    EXPECT_EQ(board.getPieceSet(B_BISHOP), 0x2400000000000000ULL);
    EXPECT_EQ(board.getPieceSet(B_ROOK), 0x8100000000000000ULL);
    EXPECT_EQ(board.getPieceSet(B_QUEEN), 0x0800000000000000ULL);
    EXPECT_EQ(board.getPieceSet(B_KING), 0x1000000000000000ULL);
}

// Test getPieceAt
TEST_F(ChessBoardTest, GetPieceAt) {
    // Test initial position
    EXPECT_EQ(board.getPieceAt(0), W_ROOK);
    EXPECT_EQ(board.getPieceAt(1), W_KNIGHT);
    EXPECT_EQ(board.getPieceAt(2), W_BISHOP);
    EXPECT_EQ(board.getPieceAt(3), W_QUEEN);
    EXPECT_EQ(board.getPieceAt(4), W_KING);
    EXPECT_EQ(board.getPieceAt(5), W_BISHOP);
    EXPECT_EQ(board.getPieceAt(6), W_KNIGHT);
    EXPECT_EQ(board.getPieceAt(7), W_ROOK);
    EXPECT_EQ(board.getPieceAt(8), W_PAWN);
    EXPECT_EQ(board.getPieceAt(9), W_PAWN);
    EXPECT_EQ(board.getPieceAt(10), W_PAWN);
    EXPECT_EQ(board.getPieceAt(11), W_PAWN);
    EXPECT_EQ(board.getPieceAt(12), W_PAWN);
    EXPECT_EQ(board.getPieceAt(13), W_PAWN);
    EXPECT_EQ(board.getPieceAt(14), W_PAWN);
    EXPECT_EQ(board.getPieceAt(15), W_PAWN);

    EXPECT_EQ(board.getPieceAt(56), B_ROOK);
    EXPECT_EQ(board.getPieceAt(57), B_KNIGHT);
    EXPECT_EQ(board.getPieceAt(58), B_BISHOP);
    EXPECT_EQ(board.getPieceAt(59), B_QUEEN);
    EXPECT_EQ(board.getPieceAt(60), B_KING);
    EXPECT_EQ(board.getPieceAt(61), B_BISHOP);
    EXPECT_EQ(board.getPieceAt(62), B_KNIGHT);
    EXPECT_EQ(board.getPieceAt(63), B_ROOK);
    EXPECT_EQ(board.getPieceAt(48), B_PAWN);
    EXPECT_EQ(board.getPieceAt(49), B_PAWN);
    EXPECT_EQ(board.getPieceAt(50), B_PAWN);
    EXPECT_EQ(board.getPieceAt(51), B_PAWN);
    EXPECT_EQ(board.getPieceAt(52), B_PAWN);
    EXPECT_EQ(board.getPieceAt(53), B_PAWN);
    EXPECT_EQ(board.getPieceAt(54), B_PAWN);
    EXPECT_EQ(board.getPieceAt(55), B_PAWN);

    // Random position
    GameState state;
    setupPosition(board, state, "1nb2r2/rppkqp1p/p2p1npb/4p2Q/1PB1P3/P1N5/1BPPNPPP/2KR1R2 w - - 6 11");
    EXPECT_EQ(board.getPieceAt(3), W_ROOK);
    EXPECT_EQ(board.getPieceAt(12), W_KNIGHT);
    EXPECT_EQ(board.getPieceAt(9), W_BISHOP);
    EXPECT_EQ(board.getPieceAt(39), W_QUEEN);
    EXPECT_EQ(board.getPieceAt(2), W_KING);
    EXPECT_EQ(board.getPieceAt(26), W_BISHOP);
    EXPECT_EQ(board.getPieceAt(18), W_KNIGHT);
    EXPECT_EQ(board.getPieceAt(5), W_ROOK);
    EXPECT_EQ(board.getPieceAt(16), W_PAWN);
    EXPECT_EQ(board.getPieceAt(25), W_PAWN);
    EXPECT_EQ(board.getPieceAt(10), W_PAWN);
    EXPECT_EQ(board.getPieceAt(11), W_PAWN);
    EXPECT_EQ(board.getPieceAt(28), W_PAWN);
    EXPECT_EQ(board.getPieceAt(13), W_PAWN);
    EXPECT_EQ(board.getPieceAt(14), W_PAWN);
    EXPECT_EQ(board.getPieceAt(15), W_PAWN);

    EXPECT_EQ(board.getPieceAt(61), B_ROOK);
    EXPECT_EQ(board.getPieceAt(57), B_KNIGHT);
    EXPECT_EQ(board.getPieceAt(58), B_BISHOP);
    EXPECT_EQ(board.getPieceAt(52), B_QUEEN);
    EXPECT_EQ(board.getPieceAt(51), B_KING);
    EXPECT_EQ(board.getPieceAt(47), B_BISHOP);
    EXPECT_EQ(board.getPieceAt(45), B_KNIGHT);
    EXPECT_EQ(board.getPieceAt(48), B_ROOK);
    EXPECT_EQ(board.getPieceAt(40), B_PAWN);
    EXPECT_EQ(board.getPieceAt(49), B_PAWN);
    EXPECT_EQ(board.getPieceAt(50), B_PAWN);
    EXPECT_EQ(board.getPieceAt(43), B_PAWN);
    EXPECT_EQ(board.getPieceAt(36), B_PAWN);
    EXPECT_EQ(board.getPieceAt(53), B_PAWN);
    EXPECT_EQ(board.getPieceAt(46), B_PAWN);
    EXPECT_EQ(board.getPieceAt(55), B_PAWN);
}

// Test basic OppAttacksToSquare
TEST_F(ChessBoardTest, AttacksToSquare1) {
    // Initial position, white king - no attacks
    EXPECT_EQ(board.OppAttacksToSquare(4, WHITE), 0x0ULL);
    GameState state;
    // Test position, white pawn on b5
    setupPosition(board, state, "8/2p5/3p4/KP5r/1R3p1k/4P3/6P1/8 w - - 0 1");
    printBitboard(board.OppAttacksToSquare(33, WHITE));
    EXPECT_EQ(board.OppAttacksToSquare(33, WHITE), 0b0000000000000000000000001000000000000000000000000000000000000000);
}

// Test more OppAttacksToSquare
TEST_F(ChessBoardTest, AttacksToSquare2) {
    GameState state;
    // Test position, white pawn on b5
    setupPosition(board, state, "1r6/2p5/3n4/KP5r/1R3p1k/4P3/6P1/8 w - - 0 1");
    printBitboard(board.OppAttacksToSquare(33, WHITE));
    EXPECT_EQ(board.OppAttacksToSquare(33, WHITE), 0b0000001000000000000010001000000000000000000000000000000000000000);
}