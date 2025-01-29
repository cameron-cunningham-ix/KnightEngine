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

// Test multiple position evaluations
TEST_F(MaterialEngineTest, MultPositionEval) {
    int eval = engine->evaluatePosition(board);
    EXPECT_EQ(eval, 0); // Starting position should be equal

    // Scholars mate before move
    board.setupPositionFromFEN("r1bqkbnr/1ppp1ppp/p1n5/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR w KQkq - 0 4");
    eval = engine->evaluatePosition(board);
    EXPECT_NEAR(eval, 0, 100);
    // Scholars mate after move
    board.setupPositionFromFEN("r1bqkbnr/1ppp1Qpp/p1n5/4p3/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 0 4");
    eval = engine->evaluatePosition(board);
    EXPECT_EQ(eval, 100000);
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
    std::ofstream outfile("TestOutput/MatEngTest_MateInOne.txt");
    std::streambuf* coutBuf = std::cout.rdbuf();
    if (outfile.is_open()) {
        outfile << "MatEngTest_MateInOne.txt\n";
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
TEST_F(MaterialEngineTest, MultMate1PosTest) {
    testing::internal::CaptureStdout();
    std::ofstream outfile("TestOutput/MatEngTest_MultMate1PosTest.txt");
    std::streambuf* coutBuf = std::cout.rdbuf();
    if (outfile.is_open()) {
        outfile << "MatEngTest_MateInOne.txt\n";
        std::cout.rdbuf(outfile.rdbuf());
    }
    // King Queen mates
    DenseMove move;
    // White
    board.setupPositionFromFEN("2k5/7Q/2K5/8/8/8/8/8 w - - 0 1");
    move = engine->findBestMove(board);
    std::cout << std::format("1st Test board FEN: {} found bestMove {}\n", board.getFEN(), move.toAlgebraic());
    board.makeMove(move, false);
    EXPECT_TRUE(isCheckmate(board));

    board.setupPositionFromFEN("k7/7Q/2K5/8/8/8/8/8 w - - 0 1");
    std::cout << std::format("2nd Test board FEN initial: {}\n",  board.getFEN());
    move = engine->findBestMove(board);
    std::cout << std::format("2nd Test board FEN after FBM: {} found bestMove {}\n", board.getFEN(), move.toAlgebraic());
    board.makeMove(move, false);
    std::cout << std::format("2nd Test board FEN after making move: {} found bestMove {}\n", board.getFEN(), move.toAlgebraic());
    EXPECT_TRUE(isCheckmate(board));

    board.setupPositionFromFEN("k7/8/2K5/8/8/8/1Q6/8 w - - 0 1");
    move = engine->findBestMove(board);
    std::cout << std::format("3rd Test board FEN: {} found bestMove {}\n", board.getFEN(), move.toAlgebraic());
    board.makeMove(move, false);
    EXPECT_TRUE(isCheckmate(board));    // Problem
    
    board.setupPositionFromFEN("8/1Q6/8/8/8/2K5/8/k7 w - - 0 1");
    move = engine->findBestMove(board);
    std::cout << std::format("4th Test board FEN: {} found bestMove {}\n", board.getFEN(), move.toAlgebraic());
    board.makeMove(move, false);
    EXPECT_TRUE(isCheckmate(board));

    board.setupPositionFromFEN("8/1Q6/8/8/8/2K5/k7/8 w - - 0 1");
    move = engine->findBestMove(board);
    std::cout << std::format("5th Testboard FEN: {} found bestMove {}\n", board.getFEN(), move.toAlgebraic());
    board.makeMove(move, false);
    EXPECT_TRUE(isCheckmate(board));

    // Black
    board.setupPositionFromFEN("2K5/7q/2k5/8/8/8/8/8 b - - 0 1");
    move = engine->findBestMove(board);
    std::cout << std::format("6th Test board FEN: {} found bestMove {}\n", board.getFEN(), move.toAlgebraic());
    board.makeMove(move, false);
    EXPECT_TRUE(isCheckmate(board));

    board.setupPositionFromFEN("K7/7q/2k5/8/8/8/8/8 b - - 0 1");
    move = engine->findBestMove(board);
    std::cout << std::format("7th Test board FEN: {} found bestMove {}\n", board.getFEN(), move.toAlgebraic());
    board.makeMove(move, false);
    EXPECT_TRUE(isCheckmate(board));

    board.setupPositionFromFEN("K7/8/2k5/8/8/8/1q6/8 b - - 0 1");
    move = engine->findBestMove(board);
    std::cout << std::format("8th Test board FEN: {} found bestMove {}\n", board.getFEN(), move.toAlgebraic());
    board.makeMove(move, false);
    EXPECT_TRUE(isCheckmate(board));    // Problem

    board.setupPositionFromFEN("8/1q6/8/8/8/2k5/8/K7 b - - 0 1");
    move = engine->findBestMove(board);
    std::cout << std::format("9th Test board FEN: {} found bestMove {}\n", board.getFEN(), move.toAlgebraic());
    board.makeMove(move, false);
    EXPECT_TRUE(isCheckmate(board));

    board.setupPositionFromFEN("8/1q6/8/8/8/2k5/K7/8 b - - 0 1");
    move = engine->findBestMove(board);
    std::cout << std::format("10th Test board FEN: {} found bestMove {}\n", board.getFEN(), move.toAlgebraic());
    board.makeMove(move, false);
    EXPECT_TRUE(isCheckmate(board));


    std::string output = testing::internal::GetCapturedStdout();
    outfile << output;
    outfile.close();
    std::cout.rdbuf(coutBuf);
}



// Test search depth time
TEST_F(MaterialEngineTest, SearchDepth2) {
    engine->setSearchDepth(2); // Set shallow depth for testing
    
    auto startTime = std::chrono::steady_clock::now();
    DenseMove move = engine->findBestMove(board);
    auto endTime = std::chrono::steady_clock::now();
    
    // With depth 2, search shouldn't take too long
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    EXPECT_LT(duration.count(), 1000); // Should complete within 1 second
}

// Test search depth time
TEST_F(MaterialEngineTest, SearchDepth3) {
    engine->setSearchDepth(3);

    auto startTime = std::chrono::steady_clock::now();
    DenseMove move = engine->findBestMove(board);
    auto endTime = std::chrono::steady_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    EXPECT_LT(duration.count(), 1000); // Should complete within 1 second
}

// Test search depth time
TEST_F(MaterialEngineTest, SearchDepth4) {
    engine->setSearchDepth(4);

    auto startTime = std::chrono::steady_clock::now();
    DenseMove move = engine->findBestMove(board);
    auto endTime = std::chrono::steady_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    EXPECT_LT(duration.count(), 10000); // Should complete within 10 seconds
}

// Test search depth time
TEST_F(MaterialEngineTest, SearchDepth5) {
    engine->setSearchDepth(5);

    auto startTime = std::chrono::steady_clock::now();
    DenseMove move = engine->findBestMove(board);
    auto endTime = std::chrono::steady_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    EXPECT_LT(duration.count(), 15000); // Should complete within 15 seconds
}

// Test search depth time
TEST_F(MaterialEngineTest, SearchDepth6) {
    engine->setSearchDepth(6);

    auto startTime = std::chrono::steady_clock::now();
    DenseMove move = engine->findBestMove(board);
    auto endTime = std::chrono::steady_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    EXPECT_LT(duration.count(), 15000); // Should complete within 15 seconds
}

// Test search depth time
TEST_F(MaterialEngineTest, SearchDepth7) {
    engine->setSearchDepth(7);

    auto startTime = std::chrono::steady_clock::now();
    DenseMove move = engine->findBestMove(board);
    auto endTime = std::chrono::steady_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    EXPECT_LT(duration.count(), 15000); // Should complete within 15 seconds
}

// Test search depth time
TEST_F(MaterialEngineTest, SearchDepth8) {
    engine->setSearchDepth(8);

    auto startTime = std::chrono::steady_clock::now();
    DenseMove move = engine->findBestMove(board);
    auto endTime = std::chrono::steady_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    EXPECT_LT(duration.count(), 15000); // Should complete within 15 seconds
}

// // Test search depth time
// TEST_F(MaterialEngineTest, SearchDepth9) {
//     engine->setSearchDepth(9);

//     auto startTime = std::chrono::steady_clock::now();
//     DenseMove move = engine->findBestMove(board);
//     auto endTime = std::chrono::steady_clock::now();
    
//     auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
//     EXPECT_LT(duration.count(), 15000); // Should complete within 15 seconds
// }

// // Test search depth time
// TEST_F(MaterialEngineTest, SearchDepth10) {
//     engine->setSearchDepth(10);

//     auto startTime = std::chrono::steady_clock::now();
//     DenseMove move = engine->findBestMove(board);
//     auto endTime = std::chrono::steady_clock::now();
    
//     auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
//     EXPECT_LT(duration.count(), 15000); // Should complete within 15 seconds
// }

// Test move picking
TEST_F(MaterialEngineTest, GoodMovePicking1) {
    testing::internal::CaptureStdout();
    std::ofstream outfile("TestOutput/MatEngTest_GMP1.txt");
    std::streambuf* coutBuf = std::cout.rdbuf();
    if (outfile.is_open()) {
        outfile << "MatEngTest_GMP1.txt\n";
        std::cout.rdbuf(outfile.rdbuf());
    }

    // Setup position
    board.setupPositionFromFEN("r1bqkbnr/pppppppp/8/8/3nP3/8/PPP2PPP/RNBQKBNR w KQkq - 0 3");
    
    // Find best move - should be Qxd4
    DenseMove bestMove = engine->findBestMove(board);
    
    // Print the board and move for debugging
    printBoard(board);
    std::cout << "Best move found: " << bestMove.toString(false) << "\n";
    
    // The engine should find Qxd4
    EXPECT_EQ(bestMove.getPieceType(), W_QUEEN);
    EXPECT_EQ(bestMove.getFrom(), BUTIL::D1);
    EXPECT_EQ(bestMove.getTo(), BUTIL::D4);
    EXPECT_TRUE(bestMove.isCapture());
    EXPECT_EQ(bestMove.getCaptPiece(), B_KNIGHT);
    
    // Verify it's the correct move
    board.makeMove(bestMove, false);
    printBoard(board);

    std::string output = testing::internal::GetCapturedStdout();
    outfile << output;
    outfile.close();
    std::cout.rdbuf(coutBuf);
}

// Test move picking
TEST_F(MaterialEngineTest, GoodMovePicking2) {
    testing::internal::CaptureStdout();
    std::ofstream outfile("TestOutput/MatEngTest_GMP2.txt");
    std::streambuf* coutBuf = std::cout.rdbuf();
    if (outfile.is_open()) {
        outfile << "MatEngTest_GMP2.txt\n";
        std::cout.rdbuf(outfile.rdbuf());
    }

    // Setup position
    board.setupPositionFromFEN("r1bqkbnr/pppppppp/8/8/3nP1Q1/8/PPP2PPP/RNB1KBNR b KQkq - 1 3");
    
    // Find best move - should be Nxc2
    DenseMove bestMove = engine->findBestMove(board);
    
    // Print the board and move for debugging
    printBoard(board);
    std::cout << "Best move found: " << bestMove.toString(false) << "\n";
    
    // The engine should find Nxc2
    EXPECT_EQ(bestMove.getPieceType(), B_KNIGHT);
    EXPECT_EQ(bestMove.getFrom(), BUTIL::D4);
    EXPECT_EQ(bestMove.getTo(), BUTIL::C2);
    EXPECT_TRUE(bestMove.isCapture());
    EXPECT_EQ(bestMove.getCaptPiece(), W_PAWN);
    
    // Verify it's the correct move
    board.makeMove(bestMove, false);
    printBoard(board);

    std::string output = testing::internal::GetCapturedStdout();
    outfile << output;
    outfile.close();
    std::cout.rdbuf(coutBuf);
}

// Test move picking
TEST_F(MaterialEngineTest, GoodMovePicking3) {
    testing::internal::CaptureStdout();
    std::ofstream outfile("TestOutput/MatEngTest_GMP3.txt");
    std::streambuf* coutBuf = std::cout.rdbuf();
    if (outfile.is_open()) {
        outfile << "MatEngTest_GMP3.txt\n";
        std::cout.rdbuf(outfile.rdbuf());
    }

    // Setup position
    board.setupPositionFromFEN("r1bqkbnr/pppppppp/8/4n3/8/2N2N2/PPPPPPPP/R1BQKB1R w KQkq - 4 3");
    
    // Find best move - should be Nxe5
    DenseMove bestMove = engine->findBestMove(board);
    
    // Print the board and move for debugging
    printBoard(board);
    std::cout << "Best move found: " << bestMove.toString(false) << "\n";
    
    // The engine should find Nxe5
    EXPECT_EQ(bestMove.getPieceType(), W_KNIGHT);
    EXPECT_EQ(bestMove.getFrom(), BUTIL::F3);
    EXPECT_EQ(bestMove.getTo(), BUTIL::E5);
    EXPECT_TRUE(bestMove.isCapture());
    EXPECT_EQ(bestMove.getCaptPiece(), B_KNIGHT);
    
    // Verify it's the correct move
    board.makeMove(bestMove, false);
    printBoard(board);

    std::string output = testing::internal::GetCapturedStdout();
    outfile << output;
    outfile.close();
    std::cout.rdbuf(coutBuf);
}

// Test move picking
TEST_F(MaterialEngineTest, GoodMovePicking4) {
    testing::internal::CaptureStdout();
    std::ofstream outfile("TestOutput/MatEngTest_GMP4.txt");
    std::streambuf* coutBuf = std::cout.rdbuf();
    if (outfile.is_open()) {
        outfile << "MatEngTest_GMP4.txt\n";
        std::cout.rdbuf(outfile.rdbuf());
    }

    // Setup position
    board.setupPositionFromFEN("k7/8/8/7p/6P1/8/8/7K w - - 0 1");
    
    // Find best move - should be gxh5
    DenseMove bestMove = engine->findBestMove(board);
    
    // Print the board and move for debugging
    printBoard(board);
    std::cout << "Best move found: " << bestMove.toString(false) << "\n";
    
    // The engine should find Nxe5
    EXPECT_EQ(bestMove.getPieceType(), W_PAWN);
    EXPECT_EQ(bestMove.getFrom(), BUTIL::G4);
    EXPECT_EQ(bestMove.getTo(), BUTIL::H5);
    EXPECT_TRUE(bestMove.isCapture());
    EXPECT_EQ(bestMove.getCaptPiece(), B_PAWN);
    
    // Verify it's the correct move
    board.makeMove(bestMove, false);
    printBoard(board);

    std::string output = testing::internal::GetCapturedStdout();
    outfile << output;
    outfile.close();
    std::cout.rdbuf(coutBuf);
}

// Test move picking
TEST_F(MaterialEngineTest, GoodMovePicking5) {
    testing::internal::CaptureStdout();
    std::ofstream outfile("TestOutput/MatEngTest_GMP5.txt");
    std::streambuf* coutBuf = std::cout.rdbuf();
    if (outfile.is_open()) {
        outfile << "MatEngTest_GMP5.txt\n";
        std::cout.rdbuf(outfile.rdbuf());
    }

    // Setup position
    board.setupPositionFromFEN("k7/8/8/p7/1P6/8/8/7K b - - 0 1");
    
    // Find best move - should be gxh5
    DenseMove bestMove = engine->findBestMove(board);
    
    // Print the board and move for debugging
    printBoard(board);
    std::cout << "Best move found: " << bestMove.toString(false) << "\n";
    
    // The engine should find Nxe5
    EXPECT_EQ(bestMove.getPieceType(), B_PAWN);
    EXPECT_EQ(bestMove.getFrom(), BUTIL::A5);
    EXPECT_EQ(bestMove.getTo(), BUTIL::B4);
    EXPECT_TRUE(bestMove.isCapture());
    EXPECT_EQ(bestMove.getCaptPiece(), W_PAWN);
    
    // Verify it's the correct move
    board.makeMove(bestMove, false);
    printBoard(board);

    std::string output = testing::internal::GetCapturedStdout();
    outfile << output;
    outfile.close();
    std::cout.rdbuf(coutBuf);
}

// Test move picking
TEST_F(MaterialEngineTest, GoodMovePicking6) {
    testing::internal::CaptureStdout();
    std::ofstream outfile("TestOutput/MatEngTest_GMP6.txt");
    std::streambuf* coutBuf = std::cout.rdbuf();
    if (outfile.is_open()) {
        outfile << "MatEngTest_GMP6.txt\n";
        std::cout.rdbuf(outfile.rdbuf());
    }

    // Setup position
    board.setupPositionFromFEN("k7/8/8/p7/1P6/8/8/7K b - - 0 1");
    
    // Find best move - should be gxh5
    DenseMove bestMove = engine->findBestMove(board);
    
    // Print the board and move for debugging
    printBoard(board);
    std::cout << "Best move found: " << bestMove.toString(false) << "\n";
    
    // The engine should find Nxe5
    EXPECT_EQ(bestMove.getPieceType(), B_PAWN);
    EXPECT_EQ(bestMove.getFrom(), BUTIL::A5);
    EXPECT_EQ(bestMove.getTo(), BUTIL::B4);
    EXPECT_TRUE(bestMove.isCapture());
    EXPECT_EQ(bestMove.getCaptPiece(), W_PAWN);
    
    // Verify it's the correct move
    board.makeMove(bestMove, false);
    printBoard(board);

    std::string output = testing::internal::GetCapturedStdout();
    outfile << output;
    outfile.close();
    std::cout.rdbuf(coutBuf);
}

// Test positions from matein2.txt file
TEST_F(MaterialEngineTest, MateIn2Positions) {
    testing::internal::CaptureStdout();
    std::ofstream outfile("TestOutput/MatEngTest_MateIn2.txt");
    std::streambuf* coutBuf = std::cout.rdbuf();
    if (outfile.is_open()) {
        outfile << "MatEngTest_MateIn2.txt\n";
        std::cout.rdbuf(outfile.rdbuf());
    }

    // Read the FEN positions from file
    std::ifstream inFile("matein2.txt");
    std::string fen;

    if (!inFile.is_open()) {
        std::cout << "Failed to open matein2.txt" << std::endl;
        FAIL();
    }

    int positionNumber = 1;
    while (std::getline(inFile, fen)) {
        if (fen.empty()) continue;  // Skip empty lines

        std::cout << "\nTesting position " << positionNumber << ":\n";
        std::cout << "FEN: " << fen << "\n";

        // Set up the position
        board.setupPositionFromFEN(fen);
        
        // Print initial position
        printBoard(board);
        
        // Find best move
        DenseMove bestMove = engine->findBestMove(board);
        std::cout << "Best move found: " << bestMove.toString(false) << "\n";
        
        // Make the move
        board.makeMove(bestMove, false);
        printBoard(board);
        
        // Verify it's mate in 1 for opponent
        EXPECT_TRUE(isCheckmate(board)) 
            << "Position " << positionNumber << " is not mate after move " 
            << bestMove.toString(false);

        positionNumber++;
    }

    std::string output = testing::internal::GetCapturedStdout();
    outfile << output;
    outfile.close();
    std::cout.rdbuf(coutBuf);
}