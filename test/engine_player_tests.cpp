#include "../src/engine/random_engine.hpp"
#include "../src/engine/syrinx_engine.hpp"
#include "../src/engine_player.hpp"
#include "../src/pext_bitboard.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <chrono>

// Mock chess engine for testing
class MockEngine : public ChessEngineBase {
public:
    MockEngine() : ChessEngineBase("MockEngine", "1.0", "Test Author"), moveToReturn() {}
    
    DenseMove findBestMove(ChessBoard& board,
                           ChessClock& clock,
                           int maxDepth) override {
        isSearching = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(50*maxDepth));  // Simulate thinking
        isSearching = false;
        return moveToReturn;
    }
    
    int evaluatePosition(const ChessBoard& board) override {
        return 0;
    }
    
    void setMoveToReturn(const DenseMove& move) {
        moveToReturn = move;
    }

private:
    DenseMove moveToReturn;
};

class EnginePlayerTest : public ::testing::Test {
protected:
    std::unique_ptr<MockEngine> mockEngine;
    std::unique_ptr<EnginePlayer> player;
    ChessBoard board;
    GameState state;
    TimeControl tc;
    ChessClock clock;

    EnginePlayerTest() 
        : tc(std::chrono::minutes(5), std::chrono::seconds(3)),
          clock(tc),
          board(),
          state() {}

    void SetUp() override {
        // Initialize PEXT
        PEXT::initialize();
        mockEngine = std::make_unique<MockEngine>();
        player = std::make_unique<EnginePlayer>(std::move(mockEngine));
    }

    void TearDown() override {
        player.reset();
        mockEngine.reset();
    }
};

// Test basic initialization
TEST_F(EnginePlayerTest, Initialization) {
    EXPECT_FALSE(player->isThinking());
    EXPECT_EQ(player->getType(), PlayerType::Engine);
    EXPECT_FALSE(player->acceptsDraw());
}

// Test getting a move
TEST_F(EnginePlayerTest, GetMove) {
    // Setup expected move
    DenseMove expectedMove(W_PAWN, 12, 28);  // e2e4
    
    // Create new engine with the move we want
    auto newMockEngine = std::make_unique<MockEngine>();
    auto* mockPtr = dynamic_cast<MockEngine*>(newMockEngine.get());
    mockPtr->setMoveToReturn(expectedMove);
    
    // Create new player with this engine
    EnginePlayer testPlayer(std::move(newMockEngine));
    
    // Get move from player
    DenseMove actualMove = testPlayer.getMove(board, clock);
    
    EXPECT_EQ(actualMove.getFrom(), expectedMove.getFrom());
    EXPECT_EQ(actualMove.getTo(), expectedMove.getTo());
    EXPECT_EQ(actualMove.getPieceType(), expectedMove.getPieceType());
}

// Test time management
TEST_F(EnginePlayerTest, TimeManagement) {
    testing::internal::CaptureStdout();
    std::ofstream outfile("TestOutput/EnginePlayerTest_TimeManagement.txt");
    if (outfile.is_open()) {
        outfile << "EnginePlayerTest_TimeManagement.txt:\n";
    }
    // Create new engine with specific time controls
    auto newMockEngine = std::make_unique<MockEngine>();
    EnginePlayer timeControlledPlayer(
        std::move(newMockEngine),
        false
    );
    
    // Verify player respects minimum time
    auto startTime = std::chrono::steady_clock::now();
    timeControlledPlayer.getMove(board, clock);
    auto duration = std::chrono::steady_clock::now() - startTime;
    
    EXPECT_GE(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count(), 100);
    std::string output = testing::internal::GetCapturedStdout();
    outfile << output;
    outfile.close();
}

// Test UCI protocol commands
TEST_F(EnginePlayerTest, UCICommands) {
    // Test UCI initialization
    player->uci();
    EXPECT_TRUE(player->waitForInitialization());
    
    // Test option setting
    player->setOption("Hash", "32");
    
    // Test position command
    std::string moves = "e2e4";
    player->position("", moves);  // Empty FEN means use startpos
    
    // Test isready command
    player->isReady();
    
    // Test stop command
    player->stop();
    EXPECT_FALSE(player->isThinking());
}

// Test engine with real search engine
TEST_F(EnginePlayerTest, SyrinxIntegration) {
    // Create player with Syrinx
    auto syrinxEngine = std::make_unique<Syrinx>();
    auto materialPlayer = std::make_unique<EnginePlayer>(std::move(syrinxEngine));
    
    // Test getting a move
    DenseMove move = materialPlayer->getMove(board, clock);
    
    // EXPECT_TRUE(validator.isMoveLegal(move));
}

// Test engine with random engine
TEST_F(EnginePlayerTest, RandomEngineIntegration) {
    // Create player with RandomEngine
    auto randomEngine = std::make_unique<RandomEngine>();
    auto randomPlayer = std::make_unique<EnginePlayer>(std::move(randomEngine));
    
    // Test getting multiple moves
    std::vector<DenseMove> moves;
    for(int i = 0; i < 5; i++) {
        moves.push_back(randomPlayer->getMove(board, clock));
    }
    
    // Verify all moves are legal and potentially different
    bool hasDifferentMoves = false;
    for(const DenseMove& move : moves) {
        if(move.getFrom() != moves[0].getFrom() || move.getTo() != moves[0].getTo()) {
            hasDifferentMoves = true;
        }
    }
    EXPECT_TRUE(hasDifferentMoves) << "Random engine should generate different moves";
}

// Test game end notification
TEST_F(EnginePlayerTest, GameEnd) {
    // Start thinking
    std::thread searchThread([this]() {
        player->getMove(board, clock);
    });
    
    // Wait for engine to start thinking
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    // Notify game end
    player->onGameEnd();
    
    // Engine should stop thinking
    if(searchThread.joinable()) {
        searchThread.join();
    }
    EXPECT_FALSE(player->isThinking());
}

// Test opponent move notification
TEST_F(EnginePlayerTest, OpponentMoveNotification) {
    DenseMove opponentMove(B_PAWN, 52, 36);  // e7e5
    player->notifyOpponentMove(opponentMove);
    
    // Engine should update its internal state
    // This is mostly tested through the integration tests
    EXPECT_FALSE(player->isThinking());
}

// Test concurrent UCI command handling
TEST_F(EnginePlayerTest, ConcurrentCommands) {
    // Start a search in a separate thread
    std::thread searchThread([this]() {
        player->getMove(board, clock);
    });
    
    // Send UCI commands while engine is searching
    player->isReady();  // Should respond immediately even while searching
    player->stop();     // Should stop the search
    
    if(searchThread.joinable()) {
        searchThread.join();
    }
    
    EXPECT_FALSE(player->isThinking());
}

// Test error handling
TEST_F(EnginePlayerTest, ErrorHandling1) {
    // Test invalid option
    player->setOption("InvalidOption", "value");  // Should not crash
    
    // Test invalid position
    player->position("invalid fen", {});  // Should handle gracefully
    
    // Test invalid moves
    std::string invalidMoves = "a1a1";  // Invalid move
    player->position("", invalidMoves);  // Should handle gracefully
}

TEST_F(EnginePlayerTest, SyrinxUCI1) {
    testing::internal::CaptureStdout();
    std::ofstream outfile("TestOutput/EnginePlayerTest_SyrinxUCI1.txt");
    if (outfile.is_open()) {
        outfile << "EnginePlayerTest_SyrinxUCI1.txt:\n";
    }
    // Create player with Syrinx
    auto syrinxEngine = std::make_unique<Syrinx>();
    auto syrinx = std::make_unique<EnginePlayer>(std::move(syrinxEngine));
 
    // Test UCI initialization
    syrinx->uci();
    EXPECT_TRUE(syrinx->waitForInitialization());
    
    // Test position command
    std::string moves = "";
    syrinx->position("r1bqk2r/p1pnn1b1/1p1pp1pp/8/2PPp3/2N1BN2/PP1QBPPP/R4RK1 w kq - 0 1", moves);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    syrinx->printBoard();

    std::string output = testing::internal::GetCapturedStdout();
    outfile << output;
    outfile.close();
}

TEST_F(EnginePlayerTest, SyrinxUCI2) {
    testing::internal::CaptureStdout();
    std::ofstream outfile("TestOutput/EnginePlayerTest_SyrinxUCI2.txt");
    if (outfile.is_open()) {
        outfile << "EnginePlayerTest_SyrinxUCI2.txt:\n";
    }
    // Create player with Syrinx
    auto syrinxEngine = std::make_unique<Syrinx>();
    auto syrinx = std::make_unique<EnginePlayer>(std::move(syrinxEngine));
 
    // Test UCI initialization
    syrinx->uci();
    EXPECT_TRUE(syrinx->waitForInitialization());
    
    // Test position command
    std::string moves = "f3h4 e6e5 h4g6 e7g6 d4e5 g7e5 e2h5 d8h4 h5g6 e8e7 c3d5 e7f8 f2f4 e4f3 f1f3 f8g7 g6e4 d7c5 e3c5 d6c5 d5e3 h8d8 d2e1 h4h5";
    syrinx->position("r1bqk2r/p1pnn1b1/1p1pp1pp/8/2PPp3/2N1BN2/PP1QBPPP/R4RK1 w kq - 0 1", moves);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    syrinx->printBoard();

    std::string output = testing::internal::GetCapturedStdout();
    outfile << output;
    outfile.close();
}