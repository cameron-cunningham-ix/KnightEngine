// test/game_state_tests.cpp
#include <gtest/gtest.h>
#include "../src/moves.hpp"

class GameStateTest : public ::testing::Test {
protected:
    ChessBoard board;
    GameState state;
    MoveValidator* validator;

    void SetUp() override {
        board = ChessBoard();
        state = GameState();
        validator = new MoveValidator(board, &state);
    }

    void setBoard(ChessBoard newBoard, GameState newState) {
        board = newBoard;
        state = newState;
        validator = new MoveValidator(board, &state);
        //validator = new MoveValidator(board, &state);
    }
};

// Test castling rights updates
TEST_F(GameStateTest, CastlingRightsUpdate) {
    // Move king's rook
    setupPosition(board, state, "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    setBoard(board, state);
    Move rookMove{W_ROOK, 7, 15, false, false};
    validator->updateGameState(rookMove);
    
    EXPECT_FALSE(state.canCastleWhiteKingside);
    EXPECT_TRUE(state.canCastleWhiteQueenside);
}

// Test en passant square updates
TEST_F(GameStateTest, EnPassantUpdate) {
    // Make a two-square pawn move
    Move pawnMove{W_PAWN, 8, 24, false, false};
    validator->updateGameState(pawnMove);
    
    EXPECT_EQ(state.enPassantSquare, 16);
}