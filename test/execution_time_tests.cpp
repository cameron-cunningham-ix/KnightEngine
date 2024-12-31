// test/execution_time_tests.cpp
#include <gtest/gtest.h>
#include <chrono>
#include "../src/moves.hpp"

// Test fixture for execution time tests
class ExecutionTimeTest : public ::testing::Test {
protected:
    ChessBoard board;
    GameState state;
    MoveValidator* validator;

    // Constructor initializes members
    ExecutionTimeTest() : board(), state(), validator(nullptr) {}

    // Setup runs before each test
    void SetUp() override {
        board = ChessBoard();
        state = GameState();
        validator = new MoveValidator(board, &state);
    }

    // Cleanup after each test
    void TearDown() override {
        delete validator;
        validator = nullptr;
    }

    // Helper method to set up a piece on the board
    void placePiece(PieceType piece, int square) {
        addPiece(board, square, piece);
    }

    void setBoard(ChessBoard newBoard, GameState newState) {
        board = newBoard;
        state = newState;
        validator = new MoveValidator(board, &state);
    }
};

// Test execution time of getRookAttacks
TEST_F(ExecutionTimeTest, GetRookAttacksSingle) {
    setupTestPosition(board, state, "kiwipete");
    
    auto start = std::chrono::high_resolution_clock::now();
    
    U64 attacks = getRookAttacks(28, board.getAllPieces());

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "\nGetRookAttacksSingle Elapsed time: " << duration.count() << " nanos\n\n";
}

// Test execution time of getRookAttacks
TEST_F(ExecutionTimeTest, GetRookAttacksMulti) {
    setupTestPosition(board, state, "kiwipete");
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 100; i++) {
        U64 attacks = getRookAttacks(i%64, board.getAllPieces());
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "\nGetRookAttacksSingle Elapsed time: " << duration.count() << " micros\n\n";
}

// Test execution time of getBishopAttacks
TEST_F(ExecutionTimeTest, GetBishopAttacksSingle) {
    setupTestPosition(board, state, "kiwipete");
    
    auto start = std::chrono::high_resolution_clock::now();
    
    U64 attacks = getBishopAttacks(28, board.getAllPieces());

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "\nGetBishopAttacksSingle Elapsed time: " << duration.count() << " nanos\n\n";
}

// Test execution time of getBishopAttacks
TEST_F(ExecutionTimeTest, GetBishopAttacksMulti) {
    setupTestPosition(board, state, "kiwipete");
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 100; i++) {
        U64 attacks = getBishopAttacks(i%64, board.getAllPieces());
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "\nGetBishopAttacksSingle Elapsed time: " << duration.count() << " micros\n\n";
}

// Test execution time of getQueenAttacks
TEST_F(ExecutionTimeTest, GetQueenAttacksSingle) {
    setupTestPosition(board, state, "kiwipete");
    
    auto start = std::chrono::high_resolution_clock::now();
    
    U64 attacks = getQueenAttacks(28, board.getAllPieces());

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "\nGetQueenAttacksSingle Elapsed time: " << duration.count() << " nanos\n\n";
}

// Test execution time of getQueenAttacks
TEST_F(ExecutionTimeTest, GetQueenAttacksMulti) {
    setupTestPosition(board, state, "kiwipete");
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 100; i++) {
        U64 attacks = getQueenAttacks(i%64, board.getAllPieces());
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "\nGetQueenAttacksSingle Elapsed time: " << duration.count() << " micros\n\n";
}

// Test execution time of ChessBoard::OppAttacksToSquare
TEST_F(ExecutionTimeTest, ChessBoardAttacksToSquareSingle) {
    setupTestPosition(board, state, "kiwipete");
    
    auto start = std::chrono::high_resolution_clock::now();
    
    U64 attacks = board.OppAttacksToSquare(28, WHITE);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "\nChessBoardAttacksToSquareSingle Elapsed time: " << duration.count() << " nanos\n\n";
}

// Test execution time of ChessBoard::OppAttacksToSquare
TEST_F(ExecutionTimeTest, ChessBoardAttacksToSquareMulti) {
    setupTestPosition(board, state, "kiwipete");
    
    auto start = std::chrono::high_resolution_clock::now();
    // 100 calls
    for (int i = 0; i < 100; i++) {
        U64 attacks = board.OppAttacksToSquare(i%64, WHITE);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "\nChessBoardAttacksToSquareMulti Elapsed time: " << duration.count() << " micros\n\n";
}

// Test execution time of makeMove
TEST_F(ExecutionTimeTest, MakeMoveD1) {
    setupTestPosition(board, state, "kiwipete");
    std::vector<Move> moves = generatePsuedoMoves(board, &state);
    ChessBoard tempBoard = board;

    auto start = std::chrono::high_resolution_clock::now();
    for(const Move& move : moves) {
        makeMove(tempBoard, move);
        tempBoard = board;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "\nMakeMoveD1 Elapsed time: " << duration.count() << " micros\n\n";
}

// Test execution time of makeMove
TEST_F(ExecutionTimeTest, IsMoveLegalD1) {
    setupTestPosition(board, state, "kiwipete");
    setBoard(board, state);
    std::vector<Move> moves = generatePsuedoMoves(board, &state);

    auto start = std::chrono::high_resolution_clock::now();
    for(const Move& move : moves) {
        bool legal = validator->isMoveLegal(move);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "\nIsMoveLegalD1 Elapsed time: " << duration.count() << " micros\n\n";
}