#include "../src/moves.hpp"
#include "../src/pext_bitboard.hpp"
#include "../src/board_utility.hpp"
#include <gtest/gtest.h>
#include <algorithm>

class MoveGenerationTest : public ::testing::Test {
protected:
    ChessBoard board;
    std::array<DenseMove, MAX_MOVES> moves;

    void SetUp() override {
        // Initialize PEXT
        PEXT::initialize();
        board = ChessBoard();
    }

    void TearDown() override {
        moves.fill(DenseMove());
    }

    // Helper function to find specific move in generated moves
    bool containsMove(const std::array<DenseMove, MAX_MOVES>& moves, int from, int to) {
        return std::any_of(moves.begin(), moves.end(),
            [from, to](const DenseMove& m) { return m.getFrom() == from && m.getTo() == to; });
    }

    void setBoard(ChessBoard newBoard) {
        board = newBoard;
    }
};

// Test initial white pawn move generation
TEST_F(MoveGenerationTest, InitialWhitePawnMoves) {
    int moveNum = 0;
    moves = MoveGenerator::generateLegalMoves(board, moveNum);
    int pawnMoves = 0;
    // White pawn moves
    for (int i = 0; i < moveNum; i++) {
        if (moves[i].getPieceType() == W_PAWN) pawnMoves++;
    }

    EXPECT_EQ(pawnMoves, 16);  // All initial white pawn moves
}

// Test initial black pawn move generation
TEST_F(MoveGenerationTest, InitialBlackPawnMoves) {
    board.setupPositionFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");
    int moveNum = 0;
    moves = MoveGenerator::generateLegalMoves(board, moveNum);
    int pawnMoves = 0;
    // Black pawn moves
    for (int i = 0; i < moveNum; i++) {
        if (moves[i].getPieceType() == B_PAWN) pawnMoves++;
    }

    EXPECT_EQ(pawnMoves, 16);  // All initial black pawn moves
}

// Test initial knight move generation
TEST_F(MoveGenerationTest, InitialKnightMoves) {
    int moveNum = 0;
    moves = MoveGenerator::generateLegalMoves(board, moveNum);
    int knightMoves = 0;
    // Initial knight moves
    for (int i = 0; i < moveNum; i++) {
        if (moves[i].getPieceType() == W_KNIGHT) knightMoves++;
    }
    EXPECT_TRUE(containsMove(moves, 1, 16));  // Nb1-c3
    EXPECT_TRUE(containsMove(moves, 1, 18));  // Nb1-a3
    EXPECT_EQ(knightMoves, 4);  // Two knights with two moves each
}

//
TEST_F(MoveGenerationTest, AllInitialMoves) {
    board.setupPositionFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1");
    int moveNum = 0;
    moves = MoveGenerator::generateLegalMoves(board, moveNum);
    EXPECT_EQ(moveNum, 20);
}

//
TEST_F(MoveGenerationTest, NotStalemate) {
    board.setupPositionFromFEN("7Q/8/8/2k5/P1P1B1P1/N3PP2/P6P/R1B1K1NR b KQ - 0 24");
    int moveNum = 0;
    moves = MoveGenerator::generateLegalMoves(board, moveNum);

    EXPECT_EQ(moveNum, 3);
}

TEST_F(MoveGenerationTest, Illegal1) {
    board.setupPositionFromFEN("r1br4/p1p3k1/1p5p/2p1b2q/2P1B3/4NR2/PP4PP/R3Q1K1 w - - 4 13");
    int moveNum = 0;
    moves = MoveGenerator::generateLegalMoves(board, moveNum);

    EXPECT_FALSE(containsMove(moves, BUTIL::F4, BUTIL::E5));
}

TEST_F(MoveGenerationTest, Illegal2) {
    board.setupPositionFromFEN("3r4/1ppk1rb1/3p2pp/n2Pp3/4P2P/2P2pP1/PP1N1P1R/4R1K1 b - - 0 23");
    
    board.makeMove(DenseMove(B_PAWN, BUTIL::C7, BUTIL::C5), false);
    board.makeMove(DenseMove(W_PAWN, BUTIL::D5, BUTIL::C6, D_PAWN, false, true), false);
    board.makeMove(DenseMove(B_KNIGHT, BUTIL::A5, BUTIL::C6, D_PAWN), false);
    board.makeMove(DenseMove(W_PAWN, BUTIL::B2, BUTIL::B4), false);
    board.makeMove(DenseMove(B_ROOK, BUTIL::F7, BUTIL::E7), false);
    
    int moveNum = 0;
    moves = MoveGenerator::generatePsuedoMoves(board, moveNum);

    EXPECT_FALSE(containsMove(moves, BUTIL::B4, BUTIL::C5));
}



// Test en passant generation
TEST_F(MoveGenerationTest, EnPassantMoves) {
    board.setupPositionFromFEN("r1bqkbnr/ppp1pppp/2n5/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3");
    int moveNum = 0;
    moves = MoveGenerator::generateLegalMoves(board, moveNum);

    EXPECT_TRUE(containsMove(moves, 36, 43));


    moves.fill(DenseMove());
    board.setupPositionFromFEN("r1bqkbnr/ppp1pppp/2n5/3pP3/8/P7/1PPP1PPP/RNBQKBNR b KQkq - 0 3");
    moves = MoveGenerator::generateLegalMoves(board, moveNum);

    EXPECT_FALSE(containsMove(moves, 36, 43));
    EXPECT_FALSE(containsMove(moves, 36, 45));

    
    moves.fill(DenseMove());
    board.setupPositionFromFEN("r1bqkbnr/ppp1p1pp/2n5/3pPp2/8/P7/1PPP1PPP/RNBQKBNR w KQkq f6 0 4");

    moves = MoveGenerator::generateLegalMoves(board, moveNum);

    EXPECT_TRUE(containsMove(moves, 36, 45));

    
    moves.fill(DenseMove());
    board.setupPositionFromFEN("r1bqkbnr/ppp1p1pp/2n5/3pP3/5pP1/P1N5/1PPP1P1P/R1BQKBNR b KQkq g3 0 5");

    moves = MoveGenerator::generateLegalMoves(board, moveNum);

    EXPECT_TRUE(containsMove(moves, 29, 22));
}


