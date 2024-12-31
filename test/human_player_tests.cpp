// test/human_player_tests.cpp
#include <gtest/gtest.h>
#include "../src/human_player.hpp"
#include "../src/utility.hpp"
#include <sstream>
#include <chrono>

// Test fixture for HumanPlayer tests that handles input/output redirection
class HumanPlayerTest : public ::testing::Test {
protected:
    // The player being tested
    std::unique_ptr<HumanPlayer> player;
    
    // Game state components
    ChessBoard board;
    GameState state;
    std::unique_ptr<ChessClock> clock;
    
    // Input/output stream handling
    std::stringstream inputStream;
    std::stringstream outputStream;
    std::streambuf* originalCin;
    std::streambuf* originalCout;

    void SetUp() override {
        // Create player instance
        player = std::make_unique<HumanPlayer>("Test Player");
        
        // Initialize game components
        board = ChessBoard();
        state = GameState();
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
    }
    
    // Helper to simulate user input and get move
    Move simulateInput(const std::string& input) {
        inputStream.str("");  // Clear previous input
        inputStream.clear();  // Clear any error flags
        outputStream.str("");  // Clear previous output
        inputStream << input << std::endl;
        return player->getMove(board, state, *clock);
    }
    
    // Helper to get captured output
    std::string getOutput() {
        return outputStream.str();
    }
    
    // Helper to setup a specific position
    void setPosition(const std::string& fen) {
        setupPosition(board, state, fen);
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
    Move move = simulateInput("e4");
    
    EXPECT_EQ(move.piece, W_PAWN);
    EXPECT_EQ(move.from, 12);  // e2
    EXPECT_EQ(move.to, 28);    // e4
    EXPECT_FALSE(move.isCapture);
    EXPECT_FALSE(move.isPromotion);
}

// Test pawn capture
TEST_F(HumanPlayerTest, PawnCapture) {
    setPosition("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
    
    Move move = simulateInput("exd5");
    
    EXPECT_EQ(move.piece, W_PAWN);
    EXPECT_EQ(move.from, 28);  // e4
    EXPECT_EQ(move.to, 35);    // d5
    EXPECT_TRUE(move.isCapture);
    EXPECT_FALSE(move.isPromotion);
}

// Test pawn promotion
TEST_F(HumanPlayerTest, PawnPromotion) {
    setPosition("8/4P3/8/8/8/8/8/k6K w - - 0 1");
    
    Move move = simulateInput("e8=Q");
    
    EXPECT_EQ(move.piece, W_PAWN);
    EXPECT_EQ(move.from, 52);  // e7
    EXPECT_EQ(move.to, 60);    // e8
    EXPECT_FALSE(move.isCapture);
    EXPECT_TRUE(move.isPromotion);
    EXPECT_EQ(move.promoteTo, W_QUEEN);
}

// Test pawn promotion with capture
TEST_F(HumanPlayerTest, PromotionWithCapture) {
    setPosition("3r4/4P3/8/8/8/8/8/k6K w - - 0 1");
    
    Move move = simulateInput("exd8=Q");
    
    EXPECT_EQ(move.piece, W_PAWN);
    EXPECT_EQ(move.from, 52);  // e7
    EXPECT_EQ(move.to, 59);    // d8
    EXPECT_TRUE(move.isCapture);
    EXPECT_TRUE(move.isPromotion);
    EXPECT_EQ(move.promoteTo, W_QUEEN);
}

// Test knight moves
TEST_F(HumanPlayerTest, KnightMoves) {
    // From starting position
    Move move = simulateInput("Nf3");
    
    EXPECT_EQ(move.piece, W_KNIGHT);
    EXPECT_EQ(move.from, 6);   // g1
    EXPECT_EQ(move.to, 21);    // f3
    EXPECT_FALSE(move.isCapture);
}

// Test bishop moves
TEST_F(HumanPlayerTest, BishopMoves) {
    setPosition("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
    
    Move move = simulateInput("Bc4");
    
    EXPECT_EQ(move.piece, W_BISHOP);
    EXPECT_EQ(move.from, 5);   // f1
    EXPECT_EQ(move.to, 26);    // c4
    EXPECT_FALSE(move.isCapture);
}

// Test castling
TEST_F(HumanPlayerTest, Castling) {
    setPosition("r3k2r/pppqbppp/2np1n2/2b1p3/2B1P3/2NP1N2/PPPBQPPP/R3K2R w KQkq - 6 8");
    
    // Test kingside castling
    Move kingsideCastle = simulateInput("O-O");
    EXPECT_EQ(kingsideCastle.piece, W_KING);
    EXPECT_EQ(kingsideCastle.from, 4);
    EXPECT_EQ(kingsideCastle.to, 6);
    EXPECT_TRUE(kingsideCastle.isCastle);
    
    // Test queenside castling
    state.sideToMove = BLACK;
    Move queensideCastle = simulateInput("O-O-O");
    EXPECT_EQ(queensideCastle.piece, B_KING);
    EXPECT_EQ(queensideCastle.from, 60);
    EXPECT_EQ(queensideCastle.to, 58);
    EXPECT_TRUE(queensideCastle.isCastle);
}

// Test invalid input handling
TEST_F(HumanPlayerTest, InvalidInput) {
    // Test completely invalid move
    inputStream << "invalid\ne4\n";  // First invalid, then valid move
    Move move = player->getMove(board, state, *clock);
    
    std::string output = getOutput();
    EXPECT_TRUE(output.find("Invalid move") != std::string::npos);
    
    // The valid move should be processed
    EXPECT_EQ(move.piece, W_PAWN);
    EXPECT_EQ(move.from, 12);
    EXPECT_EQ(move.to, 28);
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
    Move move = player->getMove(board, state, *clock);
    
    EXPECT_TRUE(player->acceptsDraw());
    
    // Should still make the move after draw offer
    EXPECT_EQ(move.piece, W_PAWN);
    EXPECT_EQ(move.from, 12);
    EXPECT_EQ(move.to, 28);
}

// Test piece disambiguation
TEST_F(HumanPlayerTest, PieceDisambiguation) {
    // Position with two knights that can move to same square
    setPosition("4k3/8/8/4N3/8/8/3N4/4K3 w - - 0 1");
    
    // Test file disambiguation
    Move move = simulateInput("Ndf3");
    EXPECT_EQ(move.piece, W_KNIGHT);
    EXPECT_EQ(move.from, 11);  // d2
    EXPECT_EQ(move.to, 21);    // f3
    
    // Test rank disambiguation
    setPosition("4k3/8/8/8/3N4/8/3N4/4K3 w - - 0 1");
    move = simulateInput("N4f3");
    EXPECT_EQ(move.piece, W_KNIGHT);
    EXPECT_EQ(move.from, 27);  // d4
    EXPECT_EQ(move.to, 21);    // f3
}

// Test en passant capture
TEST_F(HumanPlayerTest, EnPassant) {
    setPosition("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3");
    
    Move move = simulateInput("exf6");
    
    EXPECT_EQ(move.piece, W_PAWN);
    EXPECT_EQ(move.from, 36);  // e5
    EXPECT_EQ(move.to, 45);    // f6
    EXPECT_TRUE(move.isCapture);
    EXPECT_TRUE(move.isEnPassant);
}

// Test game end notification
TEST_F(HumanPlayerTest, GameEndNotification) {
    player->onGameEnd();
    EXPECT_FALSE(player->acceptsDraw());  // Draw acceptance should be reset
}

// Test opponent move notification
TEST_F(HumanPlayerTest, OpponentMoveNotification) {
    Move opponentMove(B_PAWN, 52, 36);  // e7-e5
    player->notifyOpponentMove(opponentMove);
    
    // Should not affect player state
    EXPECT_EQ(player->getName(), "Test Player");
    EXPECT_FALSE(player->acceptsDraw());
}

// Test handling of check notation
TEST_F(HumanPlayerTest, CheckNotation) {
    setPosition("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
    
    Move move = simulateInput("Qh5+");
    
    EXPECT_EQ(move.piece, W_QUEEN);
    EXPECT_EQ(move.from, 3);   // d1
    EXPECT_EQ(move.to, 39);    // h5
    EXPECT_FALSE(move.isCapture);
}

// Test handling of checkmate notation
TEST_F(HumanPlayerTest, CheckmateNotation) {
    setPosition("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
    
    Move move = simulateInput("Qh5#");
    
    EXPECT_EQ(move.piece, W_QUEEN);
    EXPECT_EQ(move.from, 3);   // d1
    EXPECT_EQ(move.to, 39);    // h5
    EXPECT_FALSE(move.isCapture);
}

// Test move input with full coordinates
TEST_F(HumanPlayerTest, CoordinateNotation) {
    // Test e2e4 style notation
    Move move = simulateInput("e2e4");
    
    EXPECT_EQ(move.piece, W_PAWN);
    EXPECT_EQ(move.from, 12);  // e2
    EXPECT_EQ(move.to, 28);    // e4
    EXPECT_FALSE(move.isCapture);
}

// Test clock display
TEST_F(HumanPlayerTest, ClockDisplay) {
    Move move = simulateInput("e4");
    std::string output = getOutput();
    
    // Should show remaining time
    EXPECT_TRUE(output.find("Time remaining") != std::string::npos);
}