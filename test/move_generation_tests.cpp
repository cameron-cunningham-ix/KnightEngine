// test/move_generation_tests.cpp
#include <gtest/gtest.h>
#include <algorithm>
#include "../src/moves.hpp"

class MoveGenerationTest : public ::testing::Test {
protected:
    ChessBoard board;
    GameState state;
    std::vector<Move> moves;

    void SetUp() override {
        board = ChessBoard();
        state = GameState();
    }

    // Helper function to find specific move in generated moves
    bool containsMove(const std::vector<Move>& moves, int from, int to) {
        return std::any_of(moves.begin(), moves.end(),
            [from, to](const Move& m) { return m.from == from && m.to == to; });
    }

    void setBoard(ChessBoard newBoard, GameState newState) {
        board = newBoard;
        state = newState;
        //validator = new MoveValidator(board, &state);
    }
};

// Test initial white pawn move generation
TEST_F(MoveGenerationTest, InitialWhitePawnMoves) {
    generatePawnMoves(board, &state, moves);

    // White pawn moves
    for (int from = 8, to = 16; from < 16; from ++, to ++) {
        EXPECT_TRUE(containsMove(moves, from, to));     // Single pawn push
        EXPECT_TRUE(containsMove(moves, from, to + 8)); // Double pawn push
    }

    EXPECT_EQ(moves.size(), 16);  // All initial pawn moves
}

// Test initial black pawn move generation
TEST_F(MoveGenerationTest, InitialBlackPawnMoves) {
    setupPosition(board, state, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");
    setBoard(board, state);
    generatePawnMoves(board, &state, moves);

    // Black pawn moves
    for (int from = 48, to = 40; from < 56; from ++, to ++) {
        EXPECT_TRUE(containsMove(moves, from, to));     // Single pawn push
        EXPECT_TRUE(containsMove(moves, from, to - 8)); // Double pawn push
    }

    EXPECT_EQ(moves.size(), 16);  // All initial pawn moves
}

// Test initial knight move generation
TEST_F(MoveGenerationTest, InitialKnightMoves) {
    generatePieceMoves(board, moves, W_KNIGHT);

    // Initial knight moves
    EXPECT_TRUE(containsMove(moves, 1, 16));  // Nb1-c3
    EXPECT_TRUE(containsMove(moves, 1, 18));  // Nb1-a3
    EXPECT_EQ(moves.size(), 4);  // Two knights with two moves each
}

//
TEST_F(MoveGenerationTest, AllInitialMoves) {
    setupPosition(board, state, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1");
    setBoard(board, state);
    moves = generatePsuedoMoves(board, &state);

    EXPECT_EQ(moves.size(), 20);
}



// Test en passant generation
TEST_F(MoveGenerationTest, EnPassantMoves) {
    setupPosition(board, state, "r1bqkbnr/ppp1pppp/2n5/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3");
    setBoard(board, state);

    generateEnPassantMoves(board, &state, moves);

    EXPECT_TRUE(containsMove(moves, 36, 43));


    moves.clear();
    setupPosition(board, state, "r1bqkbnr/ppp1pppp/2n5/3pP3/8/P7/1PPP1PPP/RNBQKBNR b KQkq - 0 3");
    setBoard(board, state);

    generateEnPassantMoves(board, &state, moves);

    EXPECT_FALSE(containsMove(moves, 36, 43));
    EXPECT_FALSE(containsMove(moves, 36, 45));

    
    moves.clear();
    setupPosition(board, state, "r1bqkbnr/ppp1p1pp/2n5/3pPp2/8/P7/1PPP1PPP/RNBQKBNR w KQkq f6 0 4");
    setBoard(board, state);

    generateEnPassantMoves(board, &state, moves);

    EXPECT_TRUE(containsMove(moves, 36, 45));

    
    moves.clear();
    setupPosition(board, state, "r1bqkbnr/ppp1p1pp/2n5/3pP3/5pP1/P1N5/1PPP1P1P/R1BQKBNR b KQkq g3 0 5");
    setBoard(board, state);

    generateEnPassantMoves(board, &state, moves);

    EXPECT_TRUE(containsMove(moves, 29, 22));
}

// // Test attack masks
// TEST_F(MoveGenerationTest, AttackMask) {

// }
