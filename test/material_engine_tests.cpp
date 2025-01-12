// test/material_engine_tests.cpp
#include "../engine/material_engine.hpp"
#include "../src/pext_bitboard.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <fstream>

class MaterialEngineTest : public ::testing::Test {
protected:
    std::unique_ptr<MaterialEngine> engine;
    ChessBoard board;

    void SetUp() override {
        // Initialize PEXT
        PEXT::initialize();
        engine = std::make_unique<MaterialEngine>();
        board = ChessBoard();
    }

    void TearDown() override {
        engine.reset();
    }
};

// Test basic engine initialization
TEST_F(MaterialEngineTest, Initialization) {
    EXPECT_EQ(engine->getName(), "MaterialEngine");
    EXPECT_FALSE(engine->isThinking());
}

// Test material evaluation in starting position
TEST_F(MaterialEngineTest, InitialPositionEvaluation) {
    int eval = engine->evaluatePosition(board);
    EXPECT_EQ(eval, 0); // Starting position should be equal
}

// Test material evaluation with white advantage
TEST_F(MaterialEngineTest, WhiteAdvantageEvaluation) {
    // Setup position where white is up a queen
    board.setupPositionFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    DenseMove queenCapture(W_QUEEN, 3, 59, D_QUEEN);
    board.makeMove(queenCapture, false);
    
    int eval = engine->evaluatePosition(board);
    EXPECT_GT(eval, 800); // White should be up significantly
}

// Test mate in one detection - Scholar's mate position
TEST_F(MaterialEngineTest, MateInOneDetection) {
    testing::internal::CaptureStdout();
    std::ofstream outfile("TestOutput/MaterialEngineTest_MateInOne.txt");
    std::streambuf* coutBuf = std::cout.rdbuf();
    if (outfile.is_open()) {
        outfile << "MaterialEngineTest_MateInOne.txt\n";
        std::cout.rdbuf(outfile.rdbuf());
    }

    // Setup Scholar's mate position
    board.setupPositionFromFEN("rnbqkbnr/pppp1ppp/8/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR w KQkq - 4 4");
    
    // Find best move - should be Qxf7#
    DenseMove bestMove = engine->findBestMove(board);
    
    // Print the board and move for debugging
    printBoard(board);
    std::cout << "Best move found: " << bestMove.toString(false) << "\n";
    
    // The engine should find Qxf7#
    EXPECT_EQ(bestMove.getPieceType(), W_QUEEN);
    EXPECT_EQ(bestMove.getFrom(), BUTIL::H5);
    EXPECT_EQ(bestMove.getTo(), BUTIL::F7);
    EXPECT_TRUE(bestMove.isCapture());
    
    // Verify it's actually mate
    board.makeMove(bestMove, false);
    printBoard(board);
    EXPECT_TRUE(isCheckmate(board));

    std::string output = testing::internal::GetCapturedStdout();
    outfile << output;
    outfile.close();
    std::cout.rdbuf(coutBuf);
}

// Test bishop pair bonus
TEST_F(MaterialEngineTest, BishopPairBonus) {
    // Position with bishop pair vs. no bishop pair
    board.setupPositionFromFEN("rn1qk1nr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    int eval = engine->evaluatePosition(board);
    
    // Difference should include bishop pair bonus
    EXPECT_GT(eval, 0); // Value of bishop pair bonus
}

// Test pawn structure evaluation
TEST_F(MaterialEngineTest, PawnStructureEvaluation) {
    // Position with doubled pawns for white
    board.setupPositionFromFEN("rnbqkbnr/pppppppp/8/8/8/4P3/PPP1PPPP/RNBQKBNR w KQkq - 0 1");
    
    int eval1 = engine->evaluatePosition(board);
    
    // Compare to normal pawn structure
    board.setupPositionFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    int eval2 = engine->evaluatePosition(board);
    
    // Position with doubled pawns should be evaluated worse
    EXPECT_LT(eval1, eval2);
}

// Test search depth adherence
TEST_F(MaterialEngineTest, SearchDepthAdherence) {
    engine->setSearchDepth(2); // Set shallow depth for testing
    
    auto startTime = std::chrono::steady_clock::now();
    DenseMove move = engine->findBestMove(board);
    auto endTime = std::chrono::steady_clock::now();
    
    // With depth 2, search shouldn't take too long
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    EXPECT_LT(duration.count(), 1000); // Should complete within 1 second
}

// Test king safety evaluation
TEST_F(MaterialEngineTest, KingSafetyEvaluation) {
    // Position with exposed king
    board.setupPositionFromFEN("rnbqkbnr/pppp2pp/8/4pp2/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    int eval1 = engine->evaluatePosition(board);
    
    // Compare to protected king
    board.setupPositionFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    int eval2 = engine->evaluatePosition(board);
    
    // Exposed king position should be evaluated worse
    EXPECT_LT(eval1, eval2);
}

// Test that the engine finds mate in one from multiple positions
TEST_F(MaterialEngineTest, MultipleMatePosTest) {
    // King Queen mates
    // White
    board.setupPositionFromFEN("2k5/7Q/2K5/8/8/8/8/8 w - - 0 1");
    DenseMove move1 = engine->findBestMove(board);
    board.makeMove(move1, false);
    EXPECT_TRUE(isCheckmate(board));

    board.setupPositionFromFEN("k7/7Q/2K5/8/8/8/8/8 w - - 0 1");
    DenseMove move2 = engine->findBestMove(board);
    board.makeMove(move2, false);
    EXPECT_TRUE(isCheckmate(board));

    board.setupPositionFromFEN("k7/8/2K5/8/8/8/1Q6/8 w - - 0 1");
    DenseMove move3 = engine->findBestMove(board);
    board.makeMove(move3, false);
    EXPECT_TRUE(isCheckmate(board));

    // Black
    board.setupPositionFromFEN("2K5/7q/2k5/8/8/8/8/8 b - - 0 1");
    DenseMove move4 = engine->findBestMove(board);
    board.makeMove(move1, false);
    EXPECT_TRUE(isCheckmate(board));

    board.setupPositionFromFEN("K7/7q/2k5/8/8/8/8/8 b - - 0 1");
    DenseMove move5 = engine->findBestMove(board);
    board.makeMove(move2, false);
    EXPECT_TRUE(isCheckmate(board));

    board.setupPositionFromFEN("K7/8/2k5/8/8/8/1q6/8 b - - 0 1");
    DenseMove move6 = engine->findBestMove(board);
    board.makeMove(move3, false);
    EXPECT_TRUE(isCheckmate(board));
}