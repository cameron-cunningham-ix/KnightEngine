// test/board_tests.cpp
#include "../src/board_generation.hpp"
#include "../src/pext_bitboard.hpp"
#include "../src/board_utility.hpp"
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
    board.makeMove(DenseMove(W_PAWN, BUTIL::E2, BUTIL::E3), true);
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

    board.makeMove(DenseMove(B_PAWN, BUTIL::E7, BUTIL::E5), true);
    EXPECT_EQ(board.currentGameState.sideToMove, WHITE);
    EXPECT_EQ(board.getSideToMove(), WHITE);
    EXPECT_EQ(board.getOppSide(), BLACK);
    EXPECT_EQ(board.currentGameState.enPassantSquare, BUTIL::E6);    // Square behind pawn
    EXPECT_EQ(board.currentGameState.fullMoveNumber, 2);
    EXPECT_EQ(board.currentGameState.halfMoveClock, 0);
    EXPECT_EQ(board.currentGameState.canCastleWhiteKingside, true);
    EXPECT_EQ(board.currentGameState.canCastleWhiteQueenside, true);
    EXPECT_EQ(board.currentGameState.canCastleBlackKingside, true);
    EXPECT_EQ(board.currentGameState.canCastleBlackQueenside, true);

    board.unmakeMove(DenseMove(B_PAWN, BUTIL::E7, BUTIL::E5), true);
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
    EXPECT_EQ(board.getCheckCount(), 0);

    board.makeMove(DenseMove(B_PAWN, BUTIL::E7, BUTIL::E5), false);
    EXPECT_EQ(board.isInCheck(), false);
    EXPECT_EQ(board.isSideInCheck(WHITE), false);
    EXPECT_EQ(board.isSideInCheck(BLACK), false);
    EXPECT_EQ(board.getCheckCount(), 0);

    board.makeMove(DenseMove(W_BISHOP, BUTIL::F1, BUTIL::B5), false);
    EXPECT_EQ(board.isInCheck(), false);
    EXPECT_EQ(board.isSideInCheck(WHITE), false);
    EXPECT_EQ(board.isSideInCheck(BLACK), false);
    EXPECT_EQ(board.getCheckCount(), 0);

    board.makeMove(DenseMove(B_BISHOP, BUTIL::F8, BUTIL::B4), false);
    EXPECT_EQ(board.isInCheck(), false);
    EXPECT_EQ(board.isSideInCheck(WHITE), false);
    EXPECT_EQ(board.isSideInCheck(BLACK), false);
    EXPECT_EQ(board.getCheckCount(), 0);

    board.makeMove(DenseMove(W_BISHOP, BUTIL::B5, BUTIL::D7, D_PAWN), false);
    EXPECT_EQ(board.isInCheck(), true);
    EXPECT_EQ(board.isSideInCheck(WHITE), false);
    EXPECT_EQ(board.isSideInCheck(BLACK), true);
    EXPECT_EQ(board.getCheckCount(), 1);

    board.unmakeMove(DenseMove(W_BISHOP, BUTIL::B5, BUTIL::D7, D_PAWN), false);
    EXPECT_EQ(board.isInCheck(), false);
    EXPECT_EQ(board.isSideInCheck(WHITE), false);
    EXPECT_EQ(board.isSideInCheck(BLACK), false);
    EXPECT_EQ(board.getCheckCount(), 0);
    
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
