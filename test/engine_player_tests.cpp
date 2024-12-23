// test/engine_player_tests.cpp
#include <gtest/gtest.h>
#include "../src/engine_player.hpp"
#include "../src/material_engine.hpp"
#include "../src/random_engine.hpp"
#include <chrono>

// Mock chess engine for testing
class MockEngine : public ChessEngineBase {
public:
    MockEngine() : ChessEngineBase("MockEngine", "1.0", "Test Author"), moveToReturn() {}
    
    Move findBestMove(const ChessBoard& board, const GameState& state, int maxDepth = -1) override {
        isSearching = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));  // Simulate thinking
        isSearching = false;
        return moveToReturn;
    }
    
    int evaluatePosition(const ChessBoard& board, const GameState& state) override {
        return 0;
    }
    
    void setMoveToReturn(const Move& move) {
        moveToReturn = move;
    }

private:
    Move moveToReturn;
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
        mockEngine = std::make_unique<MockEngine>();
        player = std::make_unique<EnginePlayer>(std::move(mockEngine));
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
    Move expectedMove(W_PAWN, 12, 28);  // e2e4
    
    // Create new engine with the move we want
    auto newMockEngine = std::make_unique<MockEngine>();
    auto* mockPtr = dynamic_cast<MockEngine*>(newMockEngine.get());
    mockPtr->setMoveToReturn(expectedMove);
    
    // Create new player with this engine
    EnginePlayer testPlayer(std::move(newMockEngine));
    
    // Get move from player
    Move actualMove = testPlayer.getMove(board, state, clock);
    
    EXPECT_EQ(actualMove.from, expectedMove.from);
    EXPECT_EQ(actualMove.to, expectedMove.to);
    EXPECT_EQ(actualMove.piece, expectedMove.piece);
}

// Test time management
TEST_F(EnginePlayerTest, TimeManagement) {
    // Create new engine with specific time controls
    auto newMockEngine = std::make_unique<MockEngine>();
    EnginePlayer timeControlledPlayer(
        std::move(newMockEngine),
        false,
        std::chrono::milliseconds(100),   // Min time
        std::chrono::milliseconds(1000)   // Max time
    );
    
    // Verify player respects minimum time
    auto startTime = std::chrono::steady_clock::now();
    timeControlledPlayer.getMove(board, state, clock);
    auto duration = std::chrono::steady_clock::now() - startTime;
    
    EXPECT_GE(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count(), 100);
}

// Test UCI protocol commands
TEST_F(EnginePlayerTest, UCICommands) {
    // Test UCI initialization
    player->uci();
    EXPECT_TRUE(player->isInitialized());
    
    // Test option setting
    player->setOption("Hash", "32");
    
    // Test position command
    std::vector<Move> moves = {Move(W_PAWN, 12, 28)};  // e2e4
    player->position("", moves);  // Empty FEN means use startpos
    
    // Test isready command
    player->isReady();
    
    // Test stop command
    player->stop();
    EXPECT_FALSE(player->isThinking());
}

// Test engine with real search engine
TEST_F(EnginePlayerTest, MaterialEngineIntegration) {
    // Create player with MaterialEngine
    auto materialEngine = std::make_unique<MaterialEngine>();
    auto materialPlayer = std::make_unique<EnginePlayer>(std::move(materialEngine));
    
    // Test getting a move
    Move move = materialPlayer->getMove(board, state, clock);
    
    // Verify move is legal
    MoveValidator validator(board, &state);
    EXPECT_TRUE(validator.isMoveLegal(move));
}

// Test engine with random engine
TEST_F(EnginePlayerTest, RandomEngineIntegration) {
    // Create player with RandomEngine
    auto randomEngine = std::make_unique<RandomEngine>();
    auto randomPlayer = std::make_unique<EnginePlayer>(std::move(randomEngine));
    
    // Test getting multiple moves
    std::vector<Move> moves;
    for(int i = 0; i < 5; i++) {
        moves.push_back(randomPlayer->getMove(board, state, clock));
    }
    
    // Verify all moves are legal and potentially different
    MoveValidator validator(board, &state);
    bool hasDifferentMoves = false;
    for(const Move& move : moves) {
        EXPECT_TRUE(validator.isMoveLegal(move));
        if(move.from != moves[0].from || move.to != moves[0].to) {
            hasDifferentMoves = true;
        }
    }
    EXPECT_TRUE(hasDifferentMoves) << "Random engine should generate different moves";
}

// Test game end notification
TEST_F(EnginePlayerTest, GameEnd) {
    // Start thinking
    std::thread searchThread([this]() {
        player->getMove(board, state, clock);
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
    Move opponentMove(B_PAWN, 52, 36);  // e7e5
    player->notifyOpponentMove(opponentMove);
    
    // Engine should update its internal state
    // This is mostly tested through the integration tests
    EXPECT_FALSE(player->isThinking());
}

// Test concurrent UCI command handling
TEST_F(EnginePlayerTest, ConcurrentCommands) {
    // Start a search in a separate thread
    std::thread searchThread([this]() {
        player->getMove(board, state, clock);
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
    std::vector<Move> invalidMoves = {Move()};  // Invalid move
    player->position("", invalidMoves);  // Should handle gracefully
}