// test/move_generation_tests.cpp
#include "../src/moves.hpp"
#include "../src/pext_bitboard.hpp"
#include <gtest/gtest.h>
#include <algorithm>

class MoveGenerationTest : public ::testing::Test {
protected:
    ChessBoard board;
    std::vector<DenseMove> moves;

    void SetUp() override {
        // Initialize PEXT
        PEXT::initialize();
        board = ChessBoard();
    }

    // Helper function to find specific move in generated moves
    bool containsMove(const std::vector<DenseMove>& moves, int from, int to) {
        return std::any_of(moves.begin(), moves.end(),
            [from, to](const DenseMove& m) { return m.getFrom() == from && m.getTo() == to; });
    }

    void setBoard(ChessBoard newBoard) {
        board = newBoard;
    }
};

// Test initial white pawn move generation
TEST_F(MoveGenerationTest, InitialWhitePawnMoves) {
    moves = MoveGenerator::generateLegalMoves(board);
    int pawnMoves = 0;
    // White pawn moves
    for (int from = 8, to = 16; from < 16; from ++, to ++) {
        EXPECT_TRUE(containsMove(moves, from, to));     // Single pawn push
        EXPECT_TRUE(containsMove(moves, from, to + 8)); // Double pawn push
        pawnMoves += 2;
    }


    EXPECT_EQ(pawnMoves, 16);  // All initial pawn moves
}

// Test initial black pawn move generation
TEST_F(MoveGenerationTest, InitialBlackPawnMoves) {
    board.setupPositionFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");
    moves = MoveGenerator::generateLegalMoves(board);

    // Black pawn moves
    for (int from = 48, to = 40; from < 56; from ++, to ++) {
        EXPECT_TRUE(containsMove(moves, from, to));     // Single pawn push
        EXPECT_TRUE(containsMove(moves, from, to - 8)); // Double pawn push
    }

    EXPECT_EQ(moves.size(), 16);  // All initial pawn moves
}

// Test initial knight move generation
TEST_F(MoveGenerationTest, InitialKnightMoves) {
    moves = MoveGenerator::generateLegalMoves(board);

    // Initial knight moves
    EXPECT_TRUE(containsMove(moves, 1, 16));  // Nb1-c3
    EXPECT_TRUE(containsMove(moves, 1, 18));  // Nb1-a3
    EXPECT_EQ(moves.size(), 4);  // Two knights with two moves each
}

//
TEST_F(MoveGenerationTest, AllInitialMoves) {
    board.setupPositionFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1");
    moves = MoveGenerator::generateLegalMoves(board);

    EXPECT_EQ(moves.size(), 20);
}



// Test en passant generation
TEST_F(MoveGenerationTest, EnPassantMoves) {
    board.setupPositionFromFEN("r1bqkbnr/ppp1pppp/2n5/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3");
    moves = MoveGenerator::generateLegalMoves(board);

    EXPECT_TRUE(containsMove(moves, 36, 43));


    moves.clear();
    board.setupPositionFromFEN("r1bqkbnr/ppp1pppp/2n5/3pP3/8/P7/1PPP1PPP/RNBQKBNR b KQkq - 0 3");
    moves = MoveGenerator::generateLegalMoves(board);

    EXPECT_FALSE(containsMove(moves, 36, 43));
    EXPECT_FALSE(containsMove(moves, 36, 45));

    
    moves.clear();
    board.setupPositionFromFEN("r1bqkbnr/ppp1p1pp/2n5/3pPp2/8/P7/1PPP1PPP/RNBQKBNR w KQkq f6 0 4");

    moves = MoveGenerator::generateLegalMoves(board);

    EXPECT_TRUE(containsMove(moves, 36, 45));

    
    moves.clear();
    board.setupPositionFromFEN("r1bqkbnr/ppp1p1pp/2n5/3pP3/5pP1/P1N5/1PPP1P1P/R1BQKBNR b KQkq g3 0 5");

    moves = MoveGenerator::generateLegalMoves(board);

    EXPECT_TRUE(containsMove(moves, 29, 22));
}

// // Test attack masks
// TEST_F(MoveGenerationTest, AttackMask) {

// }
