// test/board_tests.cpp
#include "../src/board_generation.hpp"
#include "../src/pext_bitboard.hpp"
#include "../src/board_utility.hpp"
#include "../src/utility.hpp"
#include "../src/zobrist.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <utility.hpp>

class ChessBoardTest : public ::testing::Test {
protected:
    ChessBoard board;  // Fresh board for each test

    void SetUp() override {
        // Called before each test
        PEXT::initialize();
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
    EXPECT_EQ(board.getEmptySquares(), 0x0000FFFFFFFF0000ULL);

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
    board.setupPositionFromFEN("1nb2r2/rppkqp1p/p2p1npb/4p2Q/1PB1P3/P1N5/1BPPNPPP/2KR1R2 w - - 6 11");
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

// Test getDenseTypeAt
TEST_F(ChessBoardTest, GetDenseTypeAt) {
    // Test initial position
    EXPECT_EQ(board.getDenseTypeAt(0), D_ROOK);
    EXPECT_EQ(board.getDenseTypeAt(1), D_KNIGHT);
    EXPECT_EQ(board.getDenseTypeAt(2), D_BISHOP);
    EXPECT_EQ(board.getDenseTypeAt(3), D_QUEEN);
    EXPECT_EQ(board.getDenseTypeAt(4), D_KING);
    EXPECT_EQ(board.getDenseTypeAt(5), D_BISHOP);
    EXPECT_EQ(board.getDenseTypeAt(6), D_KNIGHT);
    EXPECT_EQ(board.getDenseTypeAt(7), D_ROOK);
    EXPECT_EQ(board.getDenseTypeAt(8), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(9), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(10), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(11), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(12), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(13), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(14), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(15), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(56), D_ROOK);
    EXPECT_EQ(board.getDenseTypeAt(57), D_KNIGHT);
    EXPECT_EQ(board.getDenseTypeAt(58), D_BISHOP);
    EXPECT_EQ(board.getDenseTypeAt(59), D_QUEEN);
    EXPECT_EQ(board.getDenseTypeAt(60), D_KING);
    EXPECT_EQ(board.getDenseTypeAt(61), D_BISHOP);
    EXPECT_EQ(board.getDenseTypeAt(62), D_KNIGHT);
    EXPECT_EQ(board.getDenseTypeAt(63), D_ROOK);
    EXPECT_EQ(board.getDenseTypeAt(48), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(49), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(50), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(51), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(52), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(53), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(54), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(55), D_PAWN);

    // Random position
    board.setupPositionFromFEN("1nb2r2/rppkqp1p/p2p1npb/4p2Q/1PB1P3/P1N5/1BPPNPPP/2KR1R2 w - - 6 11");
    EXPECT_EQ(board.getDenseTypeAt(3), D_ROOK);
    EXPECT_EQ(board.getDenseTypeAt(12), D_KNIGHT);
    EXPECT_EQ(board.getDenseTypeAt(9), D_BISHOP);
    EXPECT_EQ(board.getDenseTypeAt(39), D_QUEEN);
    EXPECT_EQ(board.getDenseTypeAt(2), D_KING);
    EXPECT_EQ(board.getDenseTypeAt(26), D_BISHOP);
    EXPECT_EQ(board.getDenseTypeAt(18), D_KNIGHT);
    EXPECT_EQ(board.getDenseTypeAt(5), D_ROOK);
    EXPECT_EQ(board.getDenseTypeAt(16), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(25), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(10), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(11), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(28), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(13), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(14), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(15), D_PAWN);

    EXPECT_EQ(board.getDenseTypeAt(61), D_ROOK);
    EXPECT_EQ(board.getDenseTypeAt(57), D_KNIGHT);
    EXPECT_EQ(board.getDenseTypeAt(58), D_BISHOP);
    EXPECT_EQ(board.getDenseTypeAt(52), D_QUEEN);
    EXPECT_EQ(board.getDenseTypeAt(51), D_KING);
    EXPECT_EQ(board.getDenseTypeAt(47), D_BISHOP);
    EXPECT_EQ(board.getDenseTypeAt(45), D_KNIGHT);
    EXPECT_EQ(board.getDenseTypeAt(48), D_ROOK);
    EXPECT_EQ(board.getDenseTypeAt(40), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(49), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(50), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(43), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(36), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(53), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(46), D_PAWN);
    EXPECT_EQ(board.getDenseTypeAt(55), D_PAWN);
}

// Test makeMove
TEST_F(ChessBoardTest, MakeMove1) {
    testing::internal::CaptureStdout();

    std::ofstream outfile("TestOutput/MakeMove1.txt");
    if (outfile.is_open()) {
        outfile << "MakeMove1 test: " << std::endl;
    }
    // Test position, lots of possible moves
    board.setupPositionFromFEN("1qrnb2k/5ppp/8/8/8/8/PPP5/K2BNRQ1 w - - 0 1");
    board.printBoardInfo(false);
    
    board.makeMove(DenseMove(W_PAWN, BUTIL::A2, BUTIL::A3), false);
    outfile << "White pawn move\n";
    board.printBoardInfo(false);

    board.unmakeMove(DenseMove(W_PAWN, BUTIL::A2, BUTIL::A3), false);
    outfile << "White pawn move unmade\nAll pieces: ";
    board.printBoardInfo(false);

    board.makeMove(DenseMove(B_PAWN, BUTIL::H7, BUTIL::H6), false);
    outfile << "Black pawn move\nAll pieces: ";
    board.printBoardInfo(false);

    board.unmakeMove(DenseMove(B_PAWN, BUTIL::H7, BUTIL::H6), false);
    outfile << "Black pawn move unmade\nAll pieces: ";
    board.printBoardInfo(false);

    board.makeMove(DenseMove(W_BISHOP, BUTIL::D1, BUTIL::E2), false);
    outfile << "White bishop move\nAll pieces: ";
    board.printBoardInfo(false);

    board.unmakeMove(DenseMove(W_BISHOP, BUTIL::D1, BUTIL::E2), false);
    outfile << "White bishop move unmade\nAll pieces: ";
    board.printBoardInfo(false);

    board.makeMove(DenseMove(B_BISHOP, BUTIL::E8, BUTIL::D7), false);
    outfile << "Black bishop move\nAll pieces: ";
    board.printBoardInfo(false);

    board.unmakeMove(DenseMove(B_BISHOP, BUTIL::E8, BUTIL::D7), false);
    outfile << "Black bishop move unmade\nAll pieces: ";
    board.printBoardInfo(false);

    board.makeMove(DenseMove(W_KNIGHT, BUTIL::E1, BUTIL::F3), false);
    outfile << "White knight move\nAll pieces: ";
    board.printBoardInfo(false);

    board.unmakeMove(DenseMove(W_KNIGHT, BUTIL::E1, BUTIL::F3), false);
    outfile << "White knight move unmade\nAll pieces: ";
    board.printBoardInfo(false);

    board.makeMove(DenseMove(B_KNIGHT, BUTIL::D8, BUTIL::E6), false);
    outfile << "Black knight move\nAll pieces: ";
    board.printBoardInfo(false);
    
    board.unmakeMove(DenseMove(B_KNIGHT, BUTIL::D8, BUTIL::E6), false);
    outfile << "Black knight move unmade\nAll pieces: ";
    board.printBoardInfo(false);

    board.makeMove(DenseMove(W_ROOK, BUTIL::F1, BUTIL::F2), false);
    outfile << "White rook move\nAll pieces: ";
    board.printBoardInfo(false);

    board.unmakeMove(DenseMove(W_ROOK, BUTIL::F1, BUTIL::F2), false);
    outfile << "White rook move unmade\nAll pieces: ";
    board.printBoardInfo(false);

    board.makeMove(DenseMove(B_ROOK, BUTIL::C8, BUTIL::C7), false);
    outfile << "Black rook move\nAll pieces: ";
    board.printBoardInfo(false);

    board.unmakeMove(DenseMove(B_ROOK, BUTIL::C8, BUTIL::C7), false);
    outfile << "Black rook move unmade\nAll pieces: ";
    board.printBoardInfo(false);

    board.makeMove(DenseMove(W_QUEEN, BUTIL::G1, BUTIL::G3), false);
    outfile << "White queen move\nAll pieces: ";
    board.printBoardInfo(false);

    board.unmakeMove(DenseMove(W_QUEEN, BUTIL::G1, BUTIL::G3), false);
    outfile << "White queen move unmade\nAll pieces: ";
    board.printBoardInfo(false);


    board.makeMove(DenseMove(B_QUEEN, BUTIL::B8, BUTIL::B5), false);
    outfile << "Black queen move\nAll pieces: ";
    board.printBoardInfo(false);

    board.unmakeMove(DenseMove(B_QUEEN, BUTIL::B8, BUTIL::B5), false);
    outfile << "Black queen move unmade\nAll pieces: ";
    board.printBoardInfo(false);


    board.makeMove(DenseMove(W_KING, BUTIL::A1, BUTIL::B1), false);
    outfile << "White king move\nAll pieces: ";
    board.printBoardInfo(false);

    board.unmakeMove(DenseMove(W_KING, BUTIL::A1, BUTIL::B1), false);
    outfile << "White king move unmade\nAll pieces: ";
    board.printBoardInfo(false);


    DenseMove b_king(B_KING, BUTIL::H8, BUTIL::G8);
    outfile << "B King Move: \n";
    outfile << " DType " << b_king.getDenseType() <<
         " PType " << b_king.getPieceType() <<
         " getColor " << b_king.getColor() <<
         " getFrom " << b_king.getFrom() << 
         " getTo " << b_king.getTo() <<
         " getCaptDense " << b_king.getCaptDense() <<
         " getCaptPiece " << b_king.getCaptPiece() <<
         " isCastle " << b_king.isCastle() <<
         " getPromoteDense " << b_king.getPromoteDense() <<
         " getPromotePiece " << b_king.getPromotePiece() << "\n";
    board.makeMove(b_king, false);
    board.printBoardInfo(false);

    board.unmakeMove(b_king, false);
    outfile << "Black king move unmade\nAll pieces: ";
    board.printBoardInfo(false);

    std::string output = testing::internal::GetCapturedStdout();
    outfile << output;
    outfile.close();
}

// Test makeMove and unmakeMove captures
TEST_F(ChessBoardTest, MakeMove2) {
    testing::internal::CaptureStdout();

    std::ofstream outfile("TestOutput/MakeMove2.txt");
    if (outfile.is_open()) {
        outfile << "MakeMove2 test: " << std::endl;
    }
    // Test position, lots of possible moves
    board.setupPositionFromFEN("1qrnb2k/5ppp/8/8/8/8/PPP5/K2BNRQ1 w - - 0 1");
    printBoard(board);
    board.printBoardInfo(true);

    DenseMove W_RookCapt(W_ROOK, BUTIL::F1, BUTIL::F7, D_PAWN);
    DenseMove B_KnightCapt(B_KNIGHT, BUTIL::D8, BUTIL::F7, D_ROOK);

    board.makeMove(W_RookCapt, false);
    printBoard(board);
    board.printBoardInfo(true);

    board.makeMove(B_KnightCapt, false);
    printBoard(board);
    board.printBoardInfo(true);

    board.unmakeMove(B_KnightCapt, false);
    printBoard(board);
    board.printBoardInfo(true);

    board.unmakeMove(W_RookCapt, false);
    printBoard(board);
    board.printBoardInfo(true);

    std::string output = testing::internal::GetCapturedStdout();
    outfile << output;
    outfile.close();
}

// Test makeMove and unmakeMove captures
TEST_F(ChessBoardTest, MakeMove3) {
    testing::internal::CaptureStdout();

    std::ofstream outfile("TestOutput/MakeMove3.txt");
    if (outfile.is_open()) {
        outfile << "MakeMove3 test: " << std::endl;
    }
    // Test position, lots of possible moves
    board.setupPositionFromFEN("k7/pppq1Q2/5rr1/2p1p2b/1PnP2B1/5RR1/PPPN4/K7 w - - 0 1");
    printBoard(board);
    board.printBoardInfo(true);

    DenseMove W_PawnCapt1(W_PAWN, BUTIL::B4, BUTIL::C5, D_PAWN);
    DenseMove W_PawnCapt2(W_PAWN, BUTIL::D4, BUTIL::E5, D_PAWN);
    DenseMove W_KnightCapt(W_KNIGHT, BUTIL::D2, BUTIL::C4, D_KNIGHT);
    DenseMove W_BishopCapt1(W_BISHOP, BUTIL::G4, BUTIL::H5, D_BISHOP);
    DenseMove W_BishopCapt2(W_BISHOP, BUTIL::G4, BUTIL::D7, D_QUEEN);
    DenseMove W_RookCapt(W_ROOK, BUTIL::F3, BUTIL::F6, D_ROOK);
    DenseMove W_QueenCapt1(W_QUEEN, BUTIL::F7, BUTIL::D7, D_QUEEN);

    DenseMove B_PawnCapt1(B_PAWN, BUTIL::C5, BUTIL::B4, D_PAWN);
    DenseMove B_PawnCapt2(W_PAWN, BUTIL::E5, BUTIL::D4, D_PAWN);
    DenseMove B_KnightCapt(B_KNIGHT, BUTIL::C4, BUTIL::D2, D_KNIGHT);
    DenseMove B_BishopCapt1(B_BISHOP, BUTIL::H5, BUTIL::G4, D_BISHOP);
    DenseMove B_RookCapt(B_ROOK, BUTIL::F6, BUTIL::F3, D_ROOK);
    DenseMove B_QueenCapt1(B_QUEEN, BUTIL::D7, BUTIL::F7, D_QUEEN);

    board.makeMove(W_PawnCapt1, false);
    printBoard(board);
    board.printBoardInfo(true);

    board.makeMove(B_KnightCapt, false);
    printBoard(board);
    board.printBoardInfo(true);

    board.unmakeMove(B_KnightCapt, false);
    printBoard(board);
    board.printBoardInfo(true);

    board.unmakeMove(W_RookCapt, false);
    printBoard(board);
    board.printBoardInfo(true);

    std::string output = testing::internal::GetCapturedStdout();
    outfile << output;
    outfile.close();
}

TEST_F(ChessBoardTest, GetKingSquare1) {
    // Initial position
    EXPECT_EQ(board.getWhiteKingSquare(), 4);
    EXPECT_EQ(board.getBlackKingSquare(), 60);
    
    // Random position
    board.setupPositionFromFEN("rnbq1bnr/pppp1ppp/5k2/4p3/4P3/3K4/PPPP1PPP/RNBQ1BNR w - - 4 4");
    EXPECT_EQ(board.getWhiteKingSquare(), BUTIL::D3);
    EXPECT_EQ(board.getBlackKingSquare(), BUTIL::F6);
    // Check after makeMove
    board.makeMove(DenseMove(W_KING, BUTIL::D3, BUTIL::C4), false);
    EXPECT_EQ(board.getWhiteKingSquare(), BUTIL::C4);
    board.makeMove(DenseMove(B_KING, BUTIL::F6, BUTIL::G6), false);
    // Check after unmakeMove
    board.unmakeMove(DenseMove(B_KING, BUTIL::F6, BUTIL::G6), false);
    EXPECT_EQ(board.getBlackKingSquare(), BUTIL::F6);

    board.unmakeMove(DenseMove(W_KING, BUTIL::D3, BUTIL::C4), false);
    EXPECT_EQ(board.getWhiteKingSquare(), BUTIL::D3);
}

// Test basic OppAttacksToSquare
TEST_F(ChessBoardTest, AttacksToSquare1) {
    // Initial position, white king - no attacks
    EXPECT_EQ(board.OppAttacksToSquare(4, WHITE), 0x0ULL);
    // Test position, white pawn on b5
    board.setupPositionFromFEN("8/2p5/3p4/KP5r/1R3p1k/4P3/6P1/8 w - - 0 1");
    // printBitboard(board.OppAttacksToSquare(33, WHITE));
    EXPECT_EQ(board.OppAttacksToSquare(33, WHITE), 0b0000000000000000000000001000000000000000000000000000000000000000);
}

// Test more OppAttacksToSquare
TEST_F(ChessBoardTest, AttacksToSquare2) {
    // Test position, white pawn on b5
    board.setupPositionFromFEN("1r6/2p5/3n4/KP5r/1R3p1k/4P3/6P1/8 w - - 0 1");
    // printBitboard(board.OppAttacksToSquare(33, WHITE));
    EXPECT_EQ(board.OppAttacksToSquare(33, WHITE), 0b0000001000000000000010001000000000000000000000000000000000000000);
}

// Test initial game state
TEST_F(ChessBoardTest, GameState1) {
    EXPECT_EQ(board.currentGameState.sideToMove, WHITE);
    EXPECT_EQ(board.getSideToMove(), WHITE);
    EXPECT_EQ(board.getOppSide(), BLACK);
    EXPECT_EQ(board.currentGameState.enPassantSquare, -1);
    EXPECT_EQ(board.currentGameState.fullMoveNumber, 1);
    EXPECT_EQ(board.currentGameState.halfMoveClock, 0);
    EXPECT_EQ(board.currentGameState.canCastleWhiteKingside, true);
    EXPECT_EQ(board.currentGameState.canCastleWhiteQueenside, true);
    EXPECT_EQ(board.currentGameState.canCastleBlackKingside, true);
    EXPECT_EQ(board.currentGameState.canCastleBlackQueenside, true);
}

// Test game state from FEN setup
TEST_F(ChessBoardTest, GameState2) {
    // Starting position
    board.setupPositionFromFEN("rnbqkbnr/ppp2ppp/8/3pp3/2B1P3/8/PPPP1PPP/RNBQK1NR w KQkq - 0 3");
    EXPECT_EQ(board.currentGameState.sideToMove, WHITE);
    EXPECT_EQ(board.getSideToMove(), WHITE);
    EXPECT_EQ(board.getOppSide(), BLACK);
    EXPECT_EQ(board.currentGameState.enPassantSquare, -1);
    EXPECT_EQ(board.currentGameState.fullMoveNumber, 3);
    EXPECT_EQ(board.currentGameState.halfMoveClock, 0);
    EXPECT_EQ(board.currentGameState.canCastleWhiteKingside, true);
    EXPECT_EQ(board.currentGameState.canCastleWhiteQueenside, true);
    EXPECT_EQ(board.currentGameState.canCastleBlackKingside, true);
    EXPECT_EQ(board.currentGameState.canCastleBlackQueenside, true);
}

// Test game state from FEN setup
TEST_F(ChessBoardTest, GameState3) {
    // Starting position
    board.setupPositionFromFEN("rnbqkbnr/ppp2ppp/8/3pp3/2B1P3/8/PPPP1PPP/RNBQK1NR b KQkq - 0 3");
    EXPECT_EQ(board.currentGameState.sideToMove, BLACK);
    EXPECT_EQ(board.getSideToMove(), BLACK);
    EXPECT_EQ(board.getOppSide(), WHITE);
    EXPECT_EQ(board.currentGameState.enPassantSquare, -1);
    EXPECT_EQ(board.currentGameState.fullMoveNumber, 3);
    EXPECT_EQ(board.currentGameState.halfMoveClock, 0);
    EXPECT_EQ(board.currentGameState.canCastleWhiteKingside, true);
    EXPECT_EQ(board.currentGameState.canCastleWhiteQueenside, true);
    EXPECT_EQ(board.currentGameState.canCastleBlackKingside, true);
    EXPECT_EQ(board.currentGameState.canCastleBlackQueenside, true);
}

// Test game state from FEN setup
TEST_F(ChessBoardTest, GameState4) {
    // Starting position
    board.setupPositionFromFEN("rnbqkbnr/pp3ppp/8/2pPp3/2B5/8/PPPP1PPP/RNBQK1NR w KQkq c6 0 4");
    EXPECT_EQ(board.currentGameState.sideToMove, WHITE);
    EXPECT_EQ(board.getSideToMove(), WHITE);
    EXPECT_EQ(board.getOppSide(), BLACK);
    EXPECT_EQ(board.currentGameState.enPassantSquare, BUTIL::C6);    // Square behind pawn
    EXPECT_EQ(board.currentGameState.fullMoveNumber, 4);
    EXPECT_EQ(board.currentGameState.halfMoveClock, 0);
    EXPECT_EQ(board.currentGameState.canCastleWhiteKingside, true);
    EXPECT_EQ(board.currentGameState.canCastleWhiteQueenside, true);
    EXPECT_EQ(board.currentGameState.canCastleBlackKingside, true);
    EXPECT_EQ(board.currentGameState.canCastleBlackQueenside, true);
}

// Test game state from FEN setup
TEST_F(ChessBoardTest, GameState5) {
    // Starting position
    board.setupPositionFromFEN("rnbqkbnr/pp3ppp/8/2pPp3/2B5/8/PPPPKPPP/RNBQ2NR b kq - 1 4");
    EXPECT_EQ(board.currentGameState.sideToMove, BLACK);
    EXPECT_EQ(board.getSideToMove(), BLACK);
    EXPECT_EQ(board.getOppSide(), WHITE);
    EXPECT_EQ(board.currentGameState.enPassantSquare, -1);    // Square behind pawn
    EXPECT_EQ(board.currentGameState.fullMoveNumber, 4);
    EXPECT_EQ(board.currentGameState.halfMoveClock, 1);
    EXPECT_EQ(board.currentGameState.canCastleWhiteKingside, false);
    EXPECT_EQ(board.currentGameState.canCastleWhiteQueenside, false);
    EXPECT_EQ(board.currentGameState.canCastleBlackKingside, true);
    EXPECT_EQ(board.currentGameState.canCastleBlackQueenside, true);
}

// Test game state from play
TEST_F(ChessBoardTest, GameState6) {
    // Starting position
    board.makeMove(DenseMove(W_PAWN, BUTIL::D2, BUTIL::D4), true);
    EXPECT_EQ(board.currentGameState.sideToMove, BLACK);
    EXPECT_EQ(board.getSideToMove(), BLACK);
    EXPECT_EQ(board.getOppSide(), WHITE);
    EXPECT_EQ(board.currentGameState.enPassantSquare, -1);    // Square behind pawn
    EXPECT_EQ(board.currentGameState.fullMoveNumber, 1);
    EXPECT_EQ(board.currentGameState.halfMoveClock, 0);     // Pawns don't increase HMC
    EXPECT_EQ(board.currentGameState.canCastleWhiteKingside, true);
    EXPECT_EQ(board.currentGameState.canCastleWhiteQueenside, true);
    EXPECT_EQ(board.currentGameState.canCastleBlackKingside, true);
    EXPECT_EQ(board.currentGameState.canCastleBlackQueenside, true);

    board.makeMove(DenseMove(B_PAWN, BUTIL::A7, BUTIL::A5), true);
    EXPECT_EQ(board.currentGameState.sideToMove, WHITE);
    EXPECT_EQ(board.getSideToMove(), WHITE);
    EXPECT_EQ(board.getOppSide(), BLACK);
    EXPECT_EQ(board.currentGameState.enPassantSquare, -1);
    EXPECT_EQ(board.currentGameState.fullMoveNumber, 2);
    EXPECT_EQ(board.currentGameState.halfMoveClock, 0);
    EXPECT_EQ(board.currentGameState.canCastleWhiteKingside, true);
    EXPECT_EQ(board.currentGameState.canCastleWhiteQueenside, true);
    EXPECT_EQ(board.currentGameState.canCastleBlackKingside, true);
    EXPECT_EQ(board.currentGameState.canCastleBlackQueenside, true);

    board.makeMove(DenseMove(W_PAWN, BUTIL::D4, BUTIL::D5), true);
    EXPECT_EQ(board.currentGameState.sideToMove, BLACK);
    EXPECT_EQ(board.getSideToMove(), BLACK);
    EXPECT_EQ(board.getOppSide(), WHITE);
    EXPECT_EQ(board.currentGameState.enPassantSquare, -1);
    EXPECT_EQ(board.currentGameState.fullMoveNumber, 2);
    EXPECT_EQ(board.currentGameState.halfMoveClock, 0);
    EXPECT_EQ(board.currentGameState.canCastleWhiteKingside, true);
    EXPECT_EQ(board.currentGameState.canCastleWhiteQueenside, true);
    EXPECT_EQ(board.currentGameState.canCastleBlackKingside, true);
    EXPECT_EQ(board.currentGameState.canCastleBlackQueenside, true);

    board.makeMove(DenseMove(B_PAWN, BUTIL::E7, BUTIL::E5), true);
    EXPECT_EQ(board.currentGameState.sideToMove, WHITE);
    EXPECT_EQ(board.getSideToMove(), WHITE);
    EXPECT_EQ(board.getOppSide(), BLACK);
    EXPECT_EQ(board.currentGameState.enPassantSquare, BUTIL::E6);    // Square behind pawn
    EXPECT_EQ(board.currentGameState.fullMoveNumber, 3);
    EXPECT_EQ(board.currentGameState.halfMoveClock, 0);     // Pawns don't increase HMC
    EXPECT_EQ(board.currentGameState.canCastleWhiteKingside, true);
    EXPECT_EQ(board.currentGameState.canCastleWhiteQueenside, true);
    EXPECT_EQ(board.currentGameState.canCastleBlackKingside, true);
    EXPECT_EQ(board.currentGameState.canCastleBlackQueenside, true);

    board.unmakeMove(DenseMove(B_PAWN, BUTIL::E7, BUTIL::E5), true);
    EXPECT_EQ(board.currentGameState.sideToMove, BLACK);
    EXPECT_EQ(board.getSideToMove(), BLACK);
    EXPECT_EQ(board.getOppSide(), WHITE);
    EXPECT_EQ(board.currentGameState.enPassantSquare, -1);    // Square behind pawn
    EXPECT_EQ(board.currentGameState.fullMoveNumber, 2);
    EXPECT_EQ(board.currentGameState.halfMoveClock, 0);     // Pawns don't increase HMC
    EXPECT_EQ(board.currentGameState.canCastleWhiteKingside, true);
    EXPECT_EQ(board.currentGameState.canCastleWhiteQueenside, true);
    EXPECT_EQ(board.currentGameState.canCastleBlackKingside, true);
    EXPECT_EQ(board.currentGameState.canCastleBlackQueenside, true);

}

// Test checks in initial states
TEST_F(ChessBoardTest, IsInCheck1) {
    // Test initial position
    EXPECT_EQ(board.isInCheck(), false);
    EXPECT_EQ(board.isSideInCheck(WHITE), false);
    EXPECT_EQ(board.isSideInCheck(BLACK), false);
    EXPECT_EQ(board.getCheckCount(), 0);

    // Test random position not in check
    board.setupPositionFromFEN("r1bq1rk1/p1pp2pp/2n2n2/1p2pp2/3PPB2/b1N2N2/PPP1QPPP/R2K1B1R w - - 3 8");
    EXPECT_EQ(board.isInCheck(), false);
    EXPECT_EQ(board.isSideInCheck(WHITE), false);
    EXPECT_EQ(board.isSideInCheck(BLACK), false);
    // EXPECT_EQ(board.getCheckCount(), 0);

    // Test position in check
    board.setupPositionFromFEN("r1bq1rk1/p1pp2pp/2n2n2/1p2pp2/2QPPB2/b1N2N2/PPP2PPP/R2K1B1R b - - 4 8");
    EXPECT_EQ(board.isInCheck(), true);
    EXPECT_EQ(board.isSideInCheck(WHITE), false);
    EXPECT_EQ(board.isSideInCheck(BLACK), true);
    // EXPECT_EQ(board.getCheckCount(), 1);

}

// Test checks in play states
TEST_F(ChessBoardTest, IsInCheck2) {
    board.makeMove(DenseMove(W_PAWN, BUTIL::E2, BUTIL::E4), false);
    EXPECT_EQ(board.isInCheck(), false);
    EXPECT_EQ(board.isSideInCheck(WHITE), false);
    EXPECT_EQ(board.isSideInCheck(BLACK), false);
    // EXPECT_EQ(board.getCheckCount(), 0);

    board.makeMove(DenseMove(B_PAWN, BUTIL::E7, BUTIL::E5), false);
    EXPECT_EQ(board.isInCheck(), false);
    EXPECT_EQ(board.isSideInCheck(WHITE), false);
    EXPECT_EQ(board.isSideInCheck(BLACK), false);
    // EXPECT_EQ(board.getCheckCount(), 0);

    board.makeMove(DenseMove(W_BISHOP, BUTIL::F1, BUTIL::B5), false);
    EXPECT_EQ(board.isInCheck(), false);
    EXPECT_EQ(board.isSideInCheck(WHITE), false);
    EXPECT_EQ(board.isSideInCheck(BLACK), false);
    // EXPECT_EQ(board.getCheckCount(), 0);

    board.makeMove(DenseMove(B_BISHOP, BUTIL::F8, BUTIL::B4), false);
    EXPECT_EQ(board.isInCheck(), false);
    EXPECT_EQ(board.isSideInCheck(WHITE), false);
    EXPECT_EQ(board.isSideInCheck(BLACK), false);
    // EXPECT_EQ(board.getCheckCount(), 0);

    board.makeMove(DenseMove(W_BISHOP, BUTIL::B5, BUTIL::D7, D_PAWN), false);
    EXPECT_EQ(board.isInCheck(), true);
    EXPECT_EQ(board.isSideInCheck(WHITE), false);
    EXPECT_EQ(board.isSideInCheck(BLACK), true);
    // EXPECT_EQ(board.getCheckCount(), 1);

    board.unmakeMove(DenseMove(W_BISHOP, BUTIL::B5, BUTIL::D7, D_PAWN), false);
    EXPECT_EQ(board.isInCheck(), false);
    EXPECT_EQ(board.isSideInCheck(WHITE), false);
    EXPECT_EQ(board.isSideInCheck(BLACK), false);
    // EXPECT_EQ(board.getCheckCount(), 0);
    
}

// Test getFEN from initial position
TEST_F(ChessBoardTest, GetFEN1) {
    EXPECT_EQ(board.getFEN(), "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

// Test getFEN from random position
TEST_F(ChessBoardTest, GetFEN2) {
    board.setupPositionFromFEN("r1bq1rk1/p1pp2pp/2n2n2/1p2pp2/3PPB2/b1N2N2/PPP1QPPP/R2K1B1R w - - 3 8");
    EXPECT_EQ(board.getFEN(), "r1bq1rk1/p1pp2pp/2n2n2/1p2pp2/3PPB2/b1N2N2/PPP1QPPP/R2K1B1R w - - 3 8");
}

// Test Zobrist key equality
TEST_F(ChessBoardTest, Zobrist1) {
    ChessBoard board1 = ChessBoard();
    ChessBoard board2 = ChessBoard();

    EXPECT_EQ(board1.zobristKey, board2.zobristKey);
}

// Test Zobrist key equality
TEST_F(ChessBoardTest, Zobrist2) {
    testing::internal::CaptureStdout();
    std::ofstream outfile("TestOutput/ChessBoardTest_Zobrist2.txt");
    std::streambuf* coutBuf = std::cout.rdbuf();
    if (outfile.is_open()) {
        outfile << "ChessBoardTest_Zobrist2.txt\n";
        std::cout.rdbuf(outfile.rdbuf());
    }

    ChessBoard board1 = ChessBoard();
    ChessBoard board2 = ChessBoard();

    DenseMove pawn = DenseMove(W_PAWN, BUTIL::E2, BUTIL::E4);
    std::cout << "Board2 key before makeMove:\n";
    printBBLine(board2.zobristKey);
    board2.makeMove(pawn, false);
    std::cout << "Board2 key after makeMove:\n";
    printBBLine(board2.zobristKey);
    board2.unmakeMove(pawn, false);
    std::cout << "Board2 key after unmakeMove:\n";
    printBBLine(board2.zobristKey);

    EXPECT_EQ(board1.zobristKey, board2.zobristKey);

    std::cout << "Board1 key:\n";
    printBBLine(board1.zobristKey);
    std::cout << "Board2 key:\n";
    printBBLine(board2.zobristKey);

    std::string output = testing::internal::GetCapturedStdout();
    outfile << output;
    outfile.close();
    std::cout.rdbuf(coutBuf);
}

// Test Zobrist key equality
TEST_F(ChessBoardTest, Zobrist3) {
    testing::internal::CaptureStdout();
    std::ofstream outfile("TestOutput/ChessBoardTest_Zobrist3.txt");
    std::streambuf* coutBuf = std::cout.rdbuf();
    if (outfile.is_open()) {
        outfile << "ChessBoardTest_Zobrist3.txt\n";
        std::cout.rdbuf(outfile.rdbuf());
    }

    ChessBoard board1 = ChessBoard();
    board1.setupPositionFromFEN("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1");
    ChessBoard board2 = ChessBoard();

    DenseMove pawn = DenseMove(W_PAWN, BUTIL::E2, BUTIL::E4);

    std::cout << "Board2 key before makeMove:\n";
    printBBLine(board2.zobristKey);
    board2.makeMove(pawn, false);
    std::cout << "Board2 key after makeMove:\n";
    printBBLine(board2.zobristKey);

    EXPECT_EQ(board1.zobristKey, board2.zobristKey);

    std::cout << "Board1 key:\n";
    printBBLine(board1.zobristKey);
    std::cout << "Board2 key:\n";
    printBBLine(board2.zobristKey);

    std::cout << std::format("\n\nWhite Pawn at e2 key: {}\nWhite Pawn at e4 key: {}\nInitial ZKey: {}\n",
        Zobrist::getPieceSqKey(BUTIL::E2, W_PAWN), Zobrist::getPieceSqKey(BUTIL::E4, W_PAWN), board.zobristKey);
    board.zobristKey ^= Zobrist::getPieceSqKey(BUTIL::E2, W_PAWN);

    std::cout << std::format("ZKey after XOR wpe2: {}\n", board.zobristKey);

    board.zobristKey ^= Zobrist::getPieceSqKey(BUTIL::E4, W_PAWN);
    std::cout << std::format("ZKey after XOR wpe4: {}\n", board.zobristKey);

    std::cout << std::format("Black to move key: {}\n", Zobrist::zobristSideToMove);

    board.zobristKey ^= Zobrist::zobristSideToMove;
    std::cout << std::format("ZKey after XOR black: {}\n", board.zobristKey);



    std::string output = testing::internal::GetCapturedStdout();
    outfile << output;
    outfile.close();
    std::cout.rdbuf(coutBuf);
}

// Test Zobrist key equality
TEST_F(ChessBoardTest, Zobrist4) {
    ChessBoard board1 = ChessBoard();

    board.setupPositionFromFEN("r1bqkb1r/pppppppp/2n2n2/8/8/2N2N2/PPPPPPPP/R1BQKB1R w KQkq - 4 3");

    board1.makeMove(DenseMove(W_KNIGHT, BUTIL::B1, BUTIL::C3), false);
    board1.makeMove(DenseMove(B_KNIGHT, BUTIL::G8, BUTIL::F6), false);
    board1.makeMove(DenseMove(W_KNIGHT, BUTIL::G1, BUTIL::F3), false);
    board1.makeMove(DenseMove(B_KNIGHT, BUTIL::B8, BUTIL::C6), false);
    EXPECT_EQ(board.zobristKey, board1.zobristKey);

    board.setupPositionFromFEN("r1bqkb1r/pppppppp/2n2n2/8/8/2N2N2/PPPPPPPP/1RBQKB1R b Kkq - 5 3");

    board1.makeMove(DenseMove(W_ROOK, BUTIL::A1, BUTIL::B1), false);
    EXPECT_EQ(board.zobristKey, board1.zobristKey);
}

// Test Zobrist key equality with castling rights
TEST_F(ChessBoardTest, Zobrist5) {
    ChessBoard board1 = ChessBoard();
    // Position where both white rooks moved
    board1.makeMove(DenseMove(W_PAWN, BUTIL::A2, BUTIL::A3), false);
    board1.makeMove(DenseMove(B_PAWN, BUTIL::A7, BUTIL::A6), false);
    board1.makeMove(DenseMove(W_ROOK, BUTIL::A1, BUTIL::A2), false);
    board1.makeMove(DenseMove(B_ROOK, BUTIL::A8, BUTIL::A7), false);
    board1.makeMove(DenseMove(W_PAWN, BUTIL::H2, BUTIL::H3), false);
    board1.makeMove(DenseMove(B_PAWN, BUTIL::H7, BUTIL::H6), false);
    board1.makeMove(DenseMove(W_ROOK, BUTIL::H1, BUTIL::H2), false);
    board1.makeMove(DenseMove(B_ROOK, BUTIL::H8, BUTIL::H7), false);

    // Set up same position directly - should have no castling rights
    board.setupPositionFromFEN("1nbqkbn1/rppppppr/p6p/8/8/P6P/RPPPPPPR/1NBQKBN1 w - - 2 5");
    EXPECT_EQ(board.zobristKey, board1.zobristKey);

    // Get back to initial pos
    board.setupPositionFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    board1.unmakeMove(DenseMove(B_ROOK, BUTIL::H8, BUTIL::H7), false);
    board1.unmakeMove(DenseMove(W_ROOK, BUTIL::H1, BUTIL::H2), false);
    board1.unmakeMove(DenseMove(B_PAWN, BUTIL::H7, BUTIL::H6), false);
    board1.unmakeMove(DenseMove(W_PAWN, BUTIL::H2, BUTIL::H3), false);
    board1.unmakeMove(DenseMove(B_ROOK, BUTIL::A8, BUTIL::A7), false);
    board1.unmakeMove(DenseMove(W_ROOK, BUTIL::A1, BUTIL::A2), false);
    board1.unmakeMove(DenseMove(B_PAWN, BUTIL::A7, BUTIL::A6), false);
    board1.unmakeMove(DenseMove(W_PAWN, BUTIL::A2, BUTIL::A3), false);

    EXPECT_EQ(board.zobristKey, board1.zobristKey);
}

// Test Zobrist key equality with en passant
TEST_F(ChessBoardTest, Zobrist6) {
    ChessBoard board1 = ChessBoard();
    board.setupPositionFromFEN("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3");
    
    // Reach same position through moves
    board1.makeMove(DenseMove(W_PAWN, BUTIL::E2, BUTIL::E4), false);
    board1.makeMove(DenseMove(B_PAWN, BUTIL::D7, BUTIL::D5), false);
    board1.makeMove(DenseMove(W_PAWN, BUTIL::E4, BUTIL::E5), false);
    board1.makeMove(DenseMove(B_PAWN, BUTIL::F7, BUTIL::F5), false);
    
    EXPECT_EQ(board.zobristKey, board1.zobristKey);
}

// Test Zobrist key equality
TEST_F(ChessBoardTest, Zobrist7) {
    board.setupPositionFromFEN("rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b KQkq - 0 1");

    ChessBoard board1 = ChessBoard();
    board1.makeMove(DenseMove(W_PAWN, BUTIL::D2, BUTIL::D4), false);
    EXPECT_EQ(board.zobristKey, board1.zobristKey);


    board.setupPositionFromFEN("rnbqkbnr/pppp1ppp/8/4p3/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 2");
    board1.makeMove(DenseMove(B_PAWN, BUTIL::E7, BUTIL::E5), false);
    EXPECT_EQ(board.zobristKey, board1.zobristKey);


    board.setupPositionFromFEN("rnbqkbnr/pppp1ppp/8/4P3/8/8/PPP1PPPP/RNBQKBNR b KQkq - 0 2");

    board1.makeMove(DenseMove(W_PAWN, BUTIL::D4, BUTIL::E5, D_PAWN), false);
    EXPECT_EQ(board.zobristKey, board1.zobristKey);
}

// Test Zobrist key equality with promotions
TEST_F(ChessBoardTest, Zobrist8) {
    ChessBoard board1 = ChessBoard();
    board.setupPositionFromFEN("5k2/8/8/8/8/8/8/4K2q w - - 0 2");

    // Reach same position through promotion
    board1.setupPositionFromFEN("5k2/8/8/8/8/8/6p1/4K2Q b - - 0 1");
    DenseMove promo(B_PAWN, BUTIL::G2, BUTIL::H1, D_QUEEN);
    promo.setPromoteTo(D_QUEEN);
    board1.makeMove(promo, false);

    EXPECT_EQ(board.zobristKey, board1.zobristKey);
}

// Test Zobrist key equality with different move orders
TEST_F(ChessBoardTest, Zobrist9) {
    ChessBoard board1 = ChessBoard();

    // Reach same position through different moves
    DenseMove nc3(W_KNIGHT, BUTIL::B1, BUTIL::C3);
    DenseMove nf3(W_KNIGHT, BUTIL::G1, BUTIL::F3);
    DenseMove nf6(B_KNIGHT, BUTIL::G8, BUTIL::F6);
    DenseMove nc6(B_KNIGHT, BUTIL::B8, BUTIL::C6);

    board.makeMove(nc3, false);
    board.makeMove(nc6, false);
    board.makeMove(nf3, false);
    board.makeMove(nf6, false);

    board1.makeMove(nf3, false);
    board1.makeMove(nc6, false);
    board1.makeMove(nc3, false);
    board1.makeMove(nf6, false);

    EXPECT_EQ(board.zobristKey, board1.zobristKey);
}

// Test Zobrist key equality with different move orders
TEST_F(ChessBoardTest, Zobrist10) {
    ChessBoard board1 = ChessBoard();

    // Reach same position through different moves
    DenseMove g4(W_PAWN, BUTIL::G2, BUTIL::G4);
    DenseMove h4(W_PAWN, BUTIL::H2, BUTIL::H4);
    DenseMove h5(B_PAWN, BUTIL::H7, BUTIL::H5);

    board.makeMove(g4, false);
    board.makeMove(h5, false);
    board.makeMove(h4, false);

    board1.setupPositionFromFEN("rnbqkbnr/ppppppp1/8/7p/6PP/8/PPPPPP2/RNBQKBNR b KQkq - 0 2");

    EXPECT_EQ(board.zobristKey, board1.zobristKey);
}

// Test Zobrist keys
TEST_F(ChessBoardTest, CheckZobrist1) {
    testing::internal::CaptureStdout();
    std::ofstream outfile("TestOutput/ChessBoardTest_CheckZobrist1.txt");
    std::streambuf* coutBuf = std::cout.rdbuf();
    if (outfile.is_open()) {
        outfile << "ChessBoardTest_CheckZobrist1.txt\n";
        std::cout.rdbuf(outfile.rdbuf());
    }
    
    debugZobristKeys(board, 5);

    std::string output = testing::internal::GetCapturedStdout();
    outfile << output;
    outfile.close();
    std::cout.rdbuf(coutBuf);
}

// Test Zobrist keys
TEST_F(ChessBoardTest, CheckZobrist2) {
    testing::internal::CaptureStdout();
    std::ofstream outfile("TestOutput/ChessBoardTest_CheckZobrist2.txt");
    std::streambuf* coutBuf = std::cout.rdbuf();
    if (outfile.is_open()) {
        outfile << "ChessBoardTest_CheckZobrist2.txt\n";
        std::cout.rdbuf(outfile.rdbuf());
    }
    board.setupPositionFromFEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    debugZobristKeys(board, 5);

    std::string output = testing::internal::GetCapturedStdout();
    outfile << output;
    outfile.close();
    std::cout.rdbuf(coutBuf);
}

// Test Zobrist keys
TEST_F(ChessBoardTest, CheckZobrist3) {
    testing::internal::CaptureStdout();
    std::ofstream outfile("TestOutput/ChessBoardTest_CheckZobrist3.txt");
    std::streambuf* coutBuf = std::cout.rdbuf();
    if (outfile.is_open()) {
        outfile << "ChessBoardTest_CheckZobrist3.txt\n";
        std::cout.rdbuf(outfile.rdbuf());
    }
    board.setupPositionFromFEN("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    debugZobristKeys(board, 5);

    std::string output = testing::internal::GetCapturedStdout();
    outfile << output;
    outfile.close();
    std::cout.rdbuf(coutBuf);
}


// Test Zobrist keys
TEST_F(ChessBoardTest, CheckZobrist4) {
    testing::internal::CaptureStdout();
    std::ofstream outfile("TestOutput/ChessBoardTest_CheckZobrist4.txt");
    std::streambuf* coutBuf = std::cout.rdbuf();
    if (outfile.is_open()) {
        outfile << "ChessBoardTest_CheckZobrist4.txt\n";
        std::cout.rdbuf(outfile.rdbuf());
    }
    board.setupPositionFromFEN("r1bqkbnr/pppppppp/8/8/3nP3/8/PPP2PPP/RNBQKBNR w KQkq - 0 3");
    debugZobristKeys(board, 5);

    std::string output = testing::internal::GetCapturedStdout();
    outfile << output;
    outfile.close();
    std::cout.rdbuf(coutBuf);
}