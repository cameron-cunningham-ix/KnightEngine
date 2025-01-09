// test/human_player_tests.cpp
#include "../src/human_player.hpp"
#include "../src/utility.hpp"
#include "../src/pext_bitboard.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>

// Test fixture for HumanPlayer tests that handles input/output redirection
class HumanPlayerTest : public ::testing::Test {
protected:
    // The player being tested
    std::unique_ptr<HumanPlayer> player;
    
    // Game state components
    ChessBoard board;
    std::unique_ptr<ChessClock> clock;
    
    // Input/output stream handling
    std::stringstream inputStream;
    std::stringstream outputStream;
    std::streambuf* originalCin;
    std::streambuf* originalCout;

    void SetUp() override {
        // Initialize PEXT
        PEXT::initialize();
        // Create player instance
        player = std::make_unique<HumanPlayer>("Test Player");
        
        // Initialize game components
        board = ChessBoard();
        TimeControl tc(std::chrono::minutes(5), std::chrono::seconds(3));
        clock = std::make_unique<ChessClock>(tc);
        
        // Redirect standard input/output for testing
        originalCin = std::cin.rdbuf();
        originalCout = std::cout.rdbuf();
        std::cin.rdbuf(inputStream.rdbuf());
        std::cout.rdbuf(outputStream.rdbuf());
    }

    void TearDown() override {
        // Restore original input/output
        std::cin.rdbuf(originalCin);
        std::cout.rdbuf(originalCout);
        player.reset();
        clock.reset();
    }
    
    // Helper to simulate user input and get move
    DenseMove simulateInput(const std::string& input) {
        inputStream.str("");  // Clear previous input
        inputStream.clear();  // Clear any error flags
        outputStream.str("");  // Clear previous output
        inputStream << input << std::endl;
        return player->getMove(board, *clock);
    }
    
    // Helper to get captured output
    std::string getOutput() {
        return outputStream.str();
    }
};

// Test basic player initialization
TEST_F(HumanPlayerTest, Initialization) {
    EXPECT_EQ(player->getName(), "Test Player");
    EXPECT_EQ(player->getType(), PlayerType::Human);
    EXPECT_FALSE(player->acceptsDraw());
}

// Test basic pawn move input
TEST_F(HumanPlayerTest, BasicPawnMove) {
    // From starting position
    DenseMove move = simulateInput("e4");
    
    EXPECT_EQ(move.getPieceType(), W_PAWN);
    EXPECT_EQ(move.getFrom(), 12);  // e2
    EXPECT_EQ(move.getTo(), 28);    // e4
    EXPECT_FALSE(move.getCaptDense());
    EXPECT_FALSE(move.getPromoteDense());
}

// Test pawn capture
TEST_F(HumanPlayerTest, PawnCapture) {
    board.setupPositionFromFEN("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
    
    DenseMove move = simulateInput("exd5");
    
    EXPECT_EQ(move.getPieceType(), W_PAWN);
    EXPECT_EQ(move.getFrom(), 28);  // e4
    EXPECT_EQ(move.getTo(), 35);    // d5
    EXPECT_EQ(move.getCaptPiece(), B_PAWN);
    EXPECT_FALSE(move.getPromoteDense());
}

// Test pawn promotion
TEST_F(HumanPlayerTest, PawnPromotion) {
    // testing::internal::CaptureStdout();
    std::streambuf* coutBuf = std::cout.rdbuf();
    std::ofstream outfile("TestOutput/HumanPlayerTest_PawnPromotion.txt");
    if (outfile.is_open()) {
        outfile << "TestOutput/HumanPlayerTest_PawnPromotion.txt\n";
        std::cout.rdbuf(outfile.rdbuf());
    }

    board.setupPositionFromFEN("8/4P3/8/8/8/8/8/k6K w - - 0 1");
    
    DenseMove move = simulateInput("e8=Q");
    
    EXPECT_EQ(move.getPieceType(), W_PAWN);
    EXPECT_EQ(move.getFrom(), 52);  // e7
    EXPECT_EQ(move.getTo(), 60);    // e8
    EXPECT_TRUE(move.getCaptDense() == D_EMPTY);
    EXPECT_TRUE(move.getPromoteDense() == D_QUEEN);
    EXPECT_TRUE(move.getPromotePiece() == W_QUEEN);

    // std::string output = testing::internal::GetCapturedStdout();
    // outfile << output;
    std::cout.rdbuf(coutBuf);
    outfile.close();
}

// Test pawn promotion with capture
TEST_F(HumanPlayerTest, PromotionWithCapture) {
    board.setupPositionFromFEN("3r4/4P3/8/8/8/8/8/k6K w - - 0 1");
    
    DenseMove move = simulateInput("exd8=Q");
    
    EXPECT_EQ(move.getPieceType(), W_PAWN);
    EXPECT_EQ(move.getFrom(), 52);  // e7
    EXPECT_EQ(move.getTo(), 59);    // d8
    EXPECT_TRUE(move.getCaptPiece() == B_ROOK);
    EXPECT_TRUE(move.getPromoteDense() != D_EMPTY);
    EXPECT_EQ(move.getPromotePiece(), W_QUEEN);
}

// Test knight moves
TEST_F(HumanPlayerTest, KnightMoves) {
    // From starting position
    DenseMove move = simulateInput("Nf3");
    
    EXPECT_EQ(move.getPieceType(), W_KNIGHT);
    EXPECT_EQ(move.getFrom(), 6);   // g1
    EXPECT_EQ(move.getTo(), 21);    // f3
    EXPECT_FALSE(move.getCaptDense());
}

// Test bishop moves
TEST_F(HumanPlayerTest, BishopMoves) {
    board.setupPositionFromFEN("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
    
    DenseMove move = simulateInput("Bc4");
    
    EXPECT_EQ(move.getPieceType(), W_BISHOP);
    EXPECT_EQ(move.getFrom(), 5);   // f1
    EXPECT_EQ(move.getTo(), 26);    // c4
    EXPECT_FALSE(move.getCaptDense());
}

// Test castling
TEST_F(HumanPlayerTest, Castling) {
    // White to move
    board.setupPositionFromFEN("r3k2r/pppqbppp/2np1n2/2b1p3/2B1P3/2NP1N2/PPPBQPPP/R3K2R w KQkq - 6 8");
    
    // Test kingside castling
    DenseMove WKingsideCastle = simulateInput("O-O");
    EXPECT_EQ(WKingsideCastle.getPieceType(), W_KING);
    EXPECT_EQ(WKingsideCastle.getFrom(), 4);
    EXPECT_EQ(WKingsideCastle.getTo(), 6);
    EXPECT_TRUE(WKingsideCastle.isCastle());
    
    // Test queenside castling
    DenseMove WQueensideCastle = simulateInput("O-O-O");
    EXPECT_EQ(WQueensideCastle.getPieceType(), W_KING);
    EXPECT_EQ(WQueensideCastle.getFrom(), 4);
    EXPECT_EQ(WQueensideCastle.getTo(), 2);
    EXPECT_TRUE(WQueensideCastle.isCastle());

    // Black to move
    board.setupPositionFromFEN("r3k2r/pppqbppp/2np1n2/2b1p3/2B1P3/2NP1N2/PPPBQPPP/R3K2R b KQkq - 6 8");
    
    // Test kingside castling
    DenseMove BKingsideCastle = simulateInput("O-O");
    EXPECT_EQ(BKingsideCastle.getPieceType(), B_KING);
    EXPECT_EQ(BKingsideCastle.getFrom(), 60);
    EXPECT_EQ(BKingsideCastle.getTo(), 62);
    EXPECT_TRUE(BKingsideCastle.isCastle());
    
    // Test queenside castling
    DenseMove BQueensideCastle = simulateInput("O-O-O");
    EXPECT_EQ(BQueensideCastle.getPieceType(), B_KING);
    EXPECT_EQ(BQueensideCastle.getFrom(), 60);
    EXPECT_EQ(BQueensideCastle.getTo(), 58);
    EXPECT_TRUE(BQueensideCastle.isCastle());
}

// Test invalid input handling
TEST_F(HumanPlayerTest, InvalidInput) {
    // Test completely invalid move
    inputStream << "invalid\ne4\n";  // First invalid, then valid move
    DenseMove move = player->getMove(board, *clock);
    
    std::string output = getOutput();
    EXPECT_TRUE(output.find("Invalid move") != std::string::npos);
    
    // The valid move should be processed
    EXPECT_EQ(move.getPieceType(), W_PAWN);
    EXPECT_EQ(move.getFrom(), 12);
    EXPECT_EQ(move.getTo(), 28);
}

// Test resignation
TEST_F(HumanPlayerTest, Resignation) {
    EXPECT_THROW({
        simulateInput("resign");
    }, std::runtime_error);
    
    EXPECT_TRUE(player->getIsResigned());
}

// Test draw offer
TEST_F(HumanPlayerTest, DrawOffer) {
    inputStream << "draw\ne4\n";  // Offer draw, then make a move
    DenseMove move = player->getMove(board, *clock);
    
    EXPECT_TRUE(player->acceptsDraw());
    
    // Should still make the move after draw offer
    EXPECT_EQ(move.getPieceType(), W_PAWN);
    EXPECT_EQ(move.getFrom(), 12);
    EXPECT_EQ(move.getTo(), 28);
}

// Test piece disambiguation
TEST_F(HumanPlayerTest, PieceDisambiguation) {
    // Position with two knights that can move to same square
    board.setupPositionFromFEN("4k3/8/8/4N3/8/8/3N4/4K3 w - - 0 1");
    
    // Test file disambiguation
    DenseMove move = simulateInput("Ndf3");
    EXPECT_EQ(move.getPieceType(), W_KNIGHT);
    EXPECT_EQ(move.getFrom(), 11);  // d2
    EXPECT_EQ(move.getTo(), 21);    // f3
    
    // Test rank disambiguation
    board.setupPositionFromFEN("4k3/8/8/8/3N4/8/3N4/4K3 w - - 0 1");
    move = simulateInput("N4f3");
    EXPECT_EQ(move.getPieceType(), W_KNIGHT);
    EXPECT_EQ(move.getFrom(), 27);  // d4
    EXPECT_EQ(move.getTo(), 21);    // f3
}

// Test en passant capture
TEST_F(HumanPlayerTest, EnPassant) {
    board.setupPositionFromFEN("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3");
    
    DenseMove move = simulateInput("exf6");
    
    EXPECT_EQ(move.getPieceType(), W_PAWN);
    EXPECT_EQ(move.getFrom(), 36);  // e5
    EXPECT_EQ(move.getTo(), 45);    // f6
    EXPECT_TRUE(move.getCaptPiece() == B_PAWN);
    EXPECT_TRUE(move.isEnPassant());
}

// Test game end notification
TEST_F(HumanPlayerTest, GameEndNotification) {
    player->onGameEnd();
    EXPECT_FALSE(player->acceptsDraw());  // Draw acceptance should be reset
}

// Test opponent move notification
TEST_F(HumanPlayerTest, OpponentMoveNotification) {
    DenseMove opponentMove(B_PAWN, 52, 36);  // e7-e5
    player->notifyOpponentMove(opponentMove);
    
    // Should not affect player state
    EXPECT_EQ(player->getName(), "Test Player");
    EXPECT_FALSE(player->acceptsDraw());
}

// Test handling of check notation
TEST_F(HumanPlayerTest, CheckNotation) {
    board.setupPositionFromFEN("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
    
    DenseMove move = simulateInput("Qh5+");
    
    EXPECT_EQ(move.getPieceType(), W_QUEEN);
    EXPECT_EQ(move.getFrom(), 3);   // d1
    EXPECT_EQ(move.getTo(), 39);    // h5
    EXPECT_FALSE(move.getCaptDense());
}

// Test handling of checkmate notation
TEST_F(HumanPlayerTest, CheckmateNotation) {
    board.setupPositionFromFEN("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
    
    DenseMove move = simulateInput("Qh5#");
    
    EXPECT_EQ(move.getPieceType(), W_QUEEN);
    EXPECT_EQ(move.getFrom(), 3);   // d1
    EXPECT_EQ(move.getTo(), 39);    // h5
    EXPECT_FALSE(move.getCaptDense());
}

// Test move input with full coordinates
TEST_F(HumanPlayerTest, CoordinateNotation) {
    // Test e2e4 style notation
    DenseMove move = simulateInput("e2e4");
    
    EXPECT_EQ(move.getPieceType(), W_PAWN);
    EXPECT_EQ(move.getFrom(), 12);  // e2
    EXPECT_EQ(move.getTo(), 28);    // e4
    EXPECT_FALSE(move.getCaptDense());
}

// Test clock display
TEST_F(HumanPlayerTest, ClockDisplay) {
    DenseMove move = simulateInput("e4");
    std::string output = getOutput();
    
    // Should show remaining time
    EXPECT_TRUE(output.find("Time remaining") != std::string::npos);
}