#include "../src/moves.hpp"
#include "../src/pext_bitboard.hpp"
#include <gtest/gtest.h>
#include <chrono>

// Test fixture for execution time tests
class ExecutionTimeTest : public ::testing::Test {
protected:
    ChessBoard board;

    // Constructor initializes members
    ExecutionTimeTest() : board() {}

    // Setup runs before each test
    void SetUp() override {
        // Initialize PEXT
        PEXT::initialize();
        board = ChessBoard();
    }

    // Cleanup after each test
    void TearDown() override {
        
    }

};

// Test execution time of getRookAttacks
TEST_F(ExecutionTimeTest, GetRookAttacksSingle) {
    setupTestPosition(board, "kiwipete");
    
    auto start = std::chrono::high_resolution_clock::now();
    
    U64 attacks = PEXT::getRookAttacks(28, board.getAllPieces());

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "\nGetRookAttacksSingle Elapsed time: " << duration.count() << " nanos\n\n";
}

// Test execution time of getRookAttacks
TEST_F(ExecutionTimeTest, GetRookAttacksMulti) {
    setupTestPosition(board, "kiwipete");
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 100; i++) {
        U64 attacks = PEXT::getRookAttacks(i%64, board.getAllPieces());
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "\nGetRookAttacksSingle Elapsed time: " << duration.count() << " micros\n\n";
}

// Test execution time of getBishopAttacks
TEST_F(ExecutionTimeTest, GetBishopAttacksSingle) {
    setupTestPosition(board, "kiwipete");
    
    auto start = std::chrono::high_resolution_clock::now();
    
    U64 attacks = PEXT::getBishopAttacks(28, board.getAllPieces());

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "\nGetBishopAttacksSingle Elapsed time: " << duration.count() << " nanos\n\n";
}

// Test execution time of getBishopAttacks
TEST_F(ExecutionTimeTest, GetBishopAttacksMulti) {
    setupTestPosition(board, "kiwipete");
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 100; i++) {
        U64 attacks = PEXT::getBishopAttacks(28, board.getAllPieces());
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "\nGetBishopAttacksSingle Elapsed time: " << duration.count() << " micros\n\n";
}

// Test execution time of getQueenAttacks
TEST_F(ExecutionTimeTest, GetQueenAttacksSingle) {
    setupTestPosition(board, "kiwipete");
    
    auto start = std::chrono::high_resolution_clock::now();
    
    U64 attacks = PEXT::getRookAttacks(28, board.getAllPieces()) | PEXT::getBishopAttacks(28, board.getAllPieces());

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "\nGetQueenAttacksSingle Elapsed time: " << duration.count() << " nanos\n\n";
}

// Test execution time of getQueenAttacks
TEST_F(ExecutionTimeTest, GetQueenAttacksMulti) {
    setupTestPosition(board, "kiwipete");
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 100; i++) {
        U64 attacks = PEXT::getRookAttacks(28, board.getAllPieces()) | PEXT::getBishopAttacks(28, board.getAllPieces());
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "\nGetQueenAttacksSingle Elapsed time: " << duration.count() << " micros\n\n";
}

// Test execution time of ChessBoard::OppAttacksToSquare
TEST_F(ExecutionTimeTest, ChessBoardAttacksToSquareSingle) {
    setupTestPosition(board, "kiwipete");
    
    auto start = std::chrono::high_resolution_clock::now();
    
    U64 attacks = board.OppAttacksToSquare(28, WHITE);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "\nChessBoardAttacksToSquareSingle Elapsed time: " << duration.count() << " nanos\n\n";
}

// Test execution time of ChessBoard::OppAttacksToSquare
TEST_F(ExecutionTimeTest, ChessBoardAttacksToSquareMulti) {
    setupTestPosition(board, "kiwipete");
    
    auto start = std::chrono::high_resolution_clock::now();
    // 100 calls
    for (int i = 0; i < 100; i++) {
        U64 attacks = board.OppAttacksToSquare(i%64, WHITE);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "\nChessBoardAttacksToSquareMulti Elapsed time: " << duration.count() << " micros\n\n";
}

// Test execution time of makeMove and unmakeMove
TEST_F(ExecutionTimeTest, MakeMoveD1) {
    setupTestPosition(board, "kiwipete");
    int moveNum = 0;
    std::array<DenseMove, MAX_MOVES> moves = MoveGenerator::generateLegalMoves(board, moveNum);
    ChessBoard tempBoard = board;

    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < moveNum; i++) {
        board.makeMove(moves[i], true);
        board.unmakeMove(moves[i], true);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "\nMakeMoveD1 Elapsed time: " << duration.count() << " micros\n\n";
}
