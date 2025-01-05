// test/game_state_tests.cpp
#include "../src/moves.hpp"
#include "../src/pext_bitboard.hpp"
#include <gtest/gtest.h>

class GameStateTest : public ::testing::Test {
protected:
    ChessBoard board;

    void SetUp() override {
        // Initialize PEXT
        PEXT::initialize();
        board = ChessBoard();
    }

    void setBoard(ChessBoard newBoard) {
        board = newBoard;
    }
};

// Test castling rights updates
TEST_F(GameStateTest, CastlingRightsUpdate) {
    // Move king's rook
    board.setupPositionFromFEN("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    DenseMove rookMove{W_ROOK, 7, 15};
    board.makeMove(rookMove, false);
    
    EXPECT_FALSE(board.currentGameState.canCastleWhiteKingside);
    EXPECT_TRUE(board.currentGameState.canCastleWhiteQueenside);
}

// Test en passant square updates
TEST_F(GameStateTest, EnPassantUpdate) {
    // Make a two-square pawn move
    DenseMove pawnMove{W_PAWN, 8, 24};
    board.makeMove(pawnMove, false);
    
    EXPECT_EQ(board.currentGameState.enPassantSquare, 16);
}