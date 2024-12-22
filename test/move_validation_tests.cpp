// test/move_validation_tests.cpp
#include <gtest/gtest.h>
#include "../src/moves.hpp"

// Test fixture for move validation tests
class MoveValidationTest : public ::testing::Test {
protected:
    ChessBoard board;
    GameState state;
    MoveValidator* validator;

    // Constructor initializes members
    MoveValidationTest() : board(), state(), validator(nullptr) {}

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

// Test basic check detection
TEST_F(MoveValidationTest, BasicCheckDetection) {
    // Setup: White king on e1, black rook on e8 (direct check)
    setupPosition(board, state, "8/8/8/8/8/8/8/8 w - - 0 1");
    placePiece(W_KING, 4);  // e1
    placePiece(B_ROOK, 60); // e8
    
    printFEN(board, state);
    setBoard(board, state);

    EXPECT_TRUE(validator->isSquareUnderAttack(4, BLACK));
    EXPECT_TRUE(validator->isInCheck(WHITE));
    
    // Setup: White king on e1, black bishop on h4 (diagonal check)
    setupPosition(board, state, "8/8/8/8/8/8/8/8 w - - 0 1");
    placePiece(W_KING, 4);  // e1
    placePiece(B_BISHOP, 31); // h4
    
    printFEN(board, state);
    setBoard(board, state);

    EXPECT_TRUE(validator->isSquareUnderAttack(4, BLACK));
    EXPECT_TRUE(validator->isInCheck(WHITE));
}

// Test knight check detection
TEST_F(MoveValidationTest, KnightCheckDetection) {
    // Setup: White king on e4, black knight on f6
    setupPosition(board, state, "8/8/8/8/8/8/8/8 w - - 0 1");
    placePiece(W_KING, 28); // e4
    placePiece(B_KNIGHT, 45); // f6

    printFEN(board, state);
    setBoard(board, state);
    
    EXPECT_TRUE(validator->isSquareUnderAttack(28, BLACK));
    EXPECT_TRUE(validator->isInCheck(WHITE));
}

// Test pawn check detection
TEST_F(MoveValidationTest, PawnCheckDetection) {
    // Setup: White king on e4, black pawn on f5
    setupPosition(board, state, "8/8/8/8/8/8/8/8 w - - 0 1");
    placePiece(W_KING, 28); // e4
    placePiece(B_PAWN, 37); // f5

    printFEN(board, state);
    setBoard(board, state);
    
    EXPECT_TRUE(validator->isSquareUnderAttack(28, BLACK));
    EXPECT_TRUE(validator->isInCheck(WHITE));
}

// Test kingside castling validation
TEST_F(MoveValidationTest, KingsideCastling) {
    // Setup: Standard castling position
    setupPosition(board, state, "8/8/8/8/8/8/8/8 w K - 0 1");
    placePiece(W_KING, 4);  // e1
    placePiece(W_ROOK, 7);  // h1
    setBoard(board, state);
    
    Move kingsideCastle{W_KING, 4, 6, false, false};
    kingsideCastle.isCastle = true;

    // Test valid castling
    EXPECT_TRUE(validator->isMoveLegal(kingsideCastle));
    
    // Test with blocked path
    placePiece(W_KNIGHT, 5); // Block with knight on f1
    setBoard(board, state);
    EXPECT_FALSE(validator->isMoveLegal(kingsideCastle));
    
    // Test when in check
    board = ChessBoard();
    setupPosition(board, state, "8/8/8/8/8/8/8/8 w K - 0 1");
    placePiece(W_KING, 4);  // e1
    placePiece(W_ROOK, 7);  // h1
    placePiece(B_ROOK, 60); // e8 - giving check
    setBoard(board, state);
    EXPECT_FALSE(validator->isMoveLegal(kingsideCastle));
}

// Test queenside castling validation
TEST_F(MoveValidationTest, QueensideCastling) {
    // Setup: Standard queenside castling position
    setupPosition(board, state, "8/8/8/8/8/8/8/8 w Q - 0 1");
    placePiece(W_KING, 4);  // e1
    placePiece(W_ROOK, 0);  // a1
    setBoard(board, state);
    
    Move queensideCastle{W_KING, 4, 2, false, false};
    queensideCastle.isCastle = true;
    
    // Test valid castling
    EXPECT_TRUE(validator->isMoveLegal(queensideCastle));
    
    // Test with blocked path
    placePiece(W_BISHOP, 3); // Block with bishop on d1
    setBoard(board, state);
    EXPECT_FALSE(validator->isMoveLegal(queensideCastle));
}

// Test en passant validation
TEST_F(MoveValidationTest, EnPassant) {
    // Setup: En passant position
    setupPosition(board, state, "8/8/8/8/8/8/8/8 w - - 0 1");
    placePiece(W_KING, 4);  // White king on e1
    placePiece(W_PAWN, 36); // White pawn on e5
    placePiece(B_PAWN, 37); // Black pawn on f5 (just moved from f7)
    state.enPassantSquare = 45; // f6 square, square behind enpassantable pawn
    setBoard(board, state);
    
    Move enPassantMove{W_PAWN, 36, 45, true, false}; // e5 to f6
    enPassantMove.isEnPassant = true;
    
    // Test valid en passant
    EXPECT_TRUE(validator->isMoveLegal(enPassantMove));
    
    // Test invalid en passant (wrong turn)
    state.enPassantSquare = -1; // No en passant available
    setBoard(board, state);
    EXPECT_FALSE(validator->isMoveLegal(enPassantMove));
}

// Test rook pin detection from north
TEST_F(MoveValidationTest, RookPinDetectionNorth) {
    // Setup: Pinned rook
    setupPosition(board, state, "8/8/8/8/8/8/8/8 w - - 0 1");
    placePiece(W_KING, 4);   // e1
    placePiece(W_ROOK, 12); // e2
    placePiece(B_ROOK, 60);  // e8
    setBoard(board, state);
    printFEN(board, state);
    
    Move pinnedMove{W_ROOK, 12, 13}; // Try to move rook
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));
    
    // Test legal move along pin line
    Move legalPinnedMove{W_ROOK, 12, 20}; // Move rook up the e-file
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 28;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 36;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 44;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 52;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
}

// Test rook pin detection from south
TEST_F(MoveValidationTest, RookPinDetectionSouth) {
    // Setup: Pinned rook
    setupPosition(board, state, "8/8/8/8/8/8/8/8 w - - 0 1");
    placePiece(W_KING, 60);   // e8
    placePiece(W_ROOK, 52); // e7
    placePiece(B_ROOK, 4);  // e1
    setBoard(board, state);
    printFEN(board, state);
    
    Move pinnedMove{W_ROOK, 52, 53}; // Try to move rook
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));
    
    // Test legal move along pin line
    Move legalPinnedMove{W_ROOK, 52, 44}; // Move rook down the e-file
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 36;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 28;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 20;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 12;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
}

// Test rook pin detection from east
TEST_F(MoveValidationTest, RookPinDetectionEast) {
    // Setup: Pinned rook
    setupPosition(board, state, "8/8/8/8/8/8/8/8 w - - 0 1");
    placePiece(W_KING, 24);   // a4
    placePiece(W_ROOK, 25); // b4
    placePiece(B_ROOK, 31);  // h4
    setBoard(board, state);
    printFEN(board, state);
    
    Move pinnedMove{W_ROOK, 25, 33}; // Try to move rook
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));
    
    // Test legal move along pin line
    Move legalPinnedMove{W_ROOK, 25, 26}; // Move rook along 4th rank
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 27;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 28;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 29;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 30;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
}

// Test rook pin detection from west
TEST_F(MoveValidationTest, RookPinDetectionWest) {
    // Setup: Pinned rook
    setupPosition(board, state, "8/8/8/8/8/8/8/8 w - - 0 1");
    placePiece(W_KING, 31);   // h4
    placePiece(W_ROOK, 30); // g4
    placePiece(B_ROOK, 24);  // a4
    setBoard(board, state);
    printFEN(board, state);
    
    Move pinnedMove{W_ROOK, 30, 38}; // Try to move rook
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));
    
    // Test legal move along pin line
    Move legalPinnedMove{W_ROOK, 30, 29}; // Move rook along 4th rank
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 28;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 27;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 26;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 25;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
}

// Test rook pin detection from multiple directions
TEST_F(MoveValidationTest, RookPinDetectionMulti) {
    // Setup: Pinned rook
    setupPosition(board, state, "4r3/8/8/4R3/r2RKR1r/4R3/8/4r3 w - - 0 1");
    setBoard(board, state);
    printFEN(board, state);
    
    Move pinnedMove{W_ROOK, 20, 19}; // Try to move each rook out of the way
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));
    pinnedMove.to = 21;
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));
    pinnedMove.from = 29;
    pinnedMove.to = 21;
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));
    pinnedMove.from = 29;
    pinnedMove.to = 37;
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));
    pinnedMove.from = 27;
    pinnedMove.to = 19;
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));
    pinnedMove.from = 27;
    pinnedMove.to = 35;
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));
    pinnedMove.from = 36;
    pinnedMove.to = 37;
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));
    pinnedMove.from = 36;
    pinnedMove.to = 35;
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));

    // Test legal move along pin line
    Move legalPinnedMove{W_ROOK, 20, 12}; // Move rooks on the e-file
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.from = 36;
    legalPinnedMove.to = 44;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 52;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.from = 29;
    legalPinnedMove.to = 30;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.from = 27;
    legalPinnedMove.to = 26;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 25;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
}

// Test bishop pin detection from northeast
TEST_F(MoveValidationTest, BishopPinDetectionNE) {
    // Setup: Pinned bishop
    setupPosition(board, state, "7b/8/8/8/8/8/1B6/K7 w - - 0 1");
    setBoard(board, state);
    printFEN(board, state);
    
    Move pinnedMove{W_BISHOP, 9, 16}; // Try to move bishop
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));
    pinnedMove.to = 2;
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));
    
    // Test legal move along pin line
    Move legalPinnedMove{W_BISHOP, 9, 18}; // Move bishop along NE diagonal
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 27;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 36;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 45;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 54;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
}

// Test bishop pin detection from southeast
TEST_F(MoveValidationTest, BishopPinDetectionSE) {
    // Setup: Pinned bishop
    setupPosition(board, state, "K7/1B6/8/8/8/8/8/7b w - - 0 1");
    setBoard(board, state);
    printFEN(board, state);
    
    Move pinnedMove{W_BISHOP, 49, 58}; // Try to move bishop
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));
    pinnedMove.to = 40;
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));
    
    // Test legal move along pin line
    Move legalPinnedMove{W_BISHOP, 49, 42}; // Move bishop along SE diagonal
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 35;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 28;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 21;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 14;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
}

// Test bishop pin detection from southwest
TEST_F(MoveValidationTest, BishopPinDetectionSW) {
    // Setup: Pinned bishop
    setupPosition(board, state, "7K/6B1/8/8/8/8/8/b7 w - - 0 1");
    setBoard(board, state);
    printFEN(board, state);
    
    Move pinnedMove{W_BISHOP, 54, 61}; // Try to move bishop
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));
    pinnedMove.to = 47;
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));
    
    // Test legal move along pin line
    Move legalPinnedMove{W_BISHOP, 54, 45}; // Move bishop along SW diagonal
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 36;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 27;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 18;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 9;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
}

// Test bishop pin detection from northwest
TEST_F(MoveValidationTest, BishopPinDetectionNW) {
    // Setup: Pinned bishop
    setupPosition(board, state, "b7/8/8/8/8/8/6B1/7K w - - 0 1");
    setBoard(board, state);
    printFEN(board, state);
    
    Move pinnedMove{W_BISHOP, 14, 5}; // Try to move bishop
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));
    pinnedMove.to = 23;
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));
    
    // Test legal move along pin line
    Move legalPinnedMove{W_BISHOP, 14, 21}; // Move bishop along SW diagonal
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 28;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 35;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 42;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 49;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
}

// Test bishop pin detection from multiple directions
TEST_F(MoveValidationTest, BishopPinDetectionMulti) {
    // Setup: Pinned bishop
    setupPosition(board, state, "1b5b/8/3B1B2/4K3/3B1B2/8/7b/b7 w - - 0 1");
    setBoard(board, state);
    printFEN(board, state);
    
    Move pinnedMove{W_BISHOP, 45, 38}; // Try to move each bishop
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));
    pinnedMove.from = 29;
    pinnedMove.to = 20;
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));
    pinnedMove.from = 27;
    pinnedMove.to = 20;
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));
    pinnedMove.from = 43;
    pinnedMove.to = 52;
    EXPECT_FALSE(validator->isMoveLegal(pinnedMove));
    
    // Test legal move along each pin line
    Move legalPinnedMove{W_BISHOP, 45, 54};
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.from = 29;
    legalPinnedMove.to = 22;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.from = 27;
    legalPinnedMove.to = 18;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.to = 9;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
    legalPinnedMove.from = 43;
    legalPinnedMove.to = 50;
    EXPECT_TRUE(validator->isMoveLegal(legalPinnedMove));
}


// Test pawn promotion validation
TEST_F(MoveValidationTest, PawnPromotion) {
    // Setup: Pawn about to promote
    setupPosition(board, state, "8/7P/8/8/8/8/8/4K3 w - - 0 1");
    setBoard(board, state);
    
    Move promotionMove{W_PAWN, 55, 63, false, true}; // h7 to h8
    EXPECT_TRUE(validator->isMoveLegal(promotionMove));
    
    // Test promotion with capture
    placePiece(B_ROOK, 62); // Black rook on g8
    setBoard(board, state);
    Move promotionCapture{W_PAWN, 55, 62, true, true};
    EXPECT_TRUE(validator->isMoveLegal(promotionCapture));
}

// Test checkmate detection
TEST_F(MoveValidationTest, CheckmateDetection) {
    // Setup: Scholar's mate position
    setupPosition(board, state, "rnbqkbnr/1ppp1Qpp/p7/4p3/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 0 1");
    setBoard(board, state);
    
    EXPECT_TRUE(validator->isInCheck(BLACK));
    EXPECT_TRUE(validator->isCheckmate(BLACK));
}

// Test stalemate detection
TEST_F(MoveValidationTest, StalemateDetection) {
    // Setup: Basic stalemate position
    setupPosition(board, state, "k1K5/8/1Q6/8/8/8/8/8 b - - 0 1");
    setBoard(board, state);
    
    EXPECT_FALSE(validator->isInCheck(BLACK));
    EXPECT_TRUE(validator->isStalemate(BLACK));
}

// // Test piece movement through other pieces
// TEST_F(MoveValidationTest, MoveThroughPieces) {
//     // Setup: Blocked bishop
//     placePiece(W_BISHOP, 2); // c1
//     placePiece(W_PAWN, 11);  // c2
    
//     Move illegalMove{W_BISHOP, 2, 20}; // Try to move bishop through pawn
//     EXPECT_FALSE(validator->isMoveLegal(illegalMove));
// }

// // Test capture validation
// TEST_F(MoveValidationTest, CaptureValidation) {
//     // Setup: Simple capture position
//     placePiece(W_BISHOP, 28); // e4
//     placePiece(B_PAWN, 37);   // f5
    
//     Move captureMove{W_BISHOP, 28, 37, true, false};
//     EXPECT_TRUE(validator->isMoveLegal(captureMove));
    
//     // Test illegal capture of own piece
//     board = ChessBoard();
//     placePiece(W_BISHOP, 28); // e4
//     placePiece(W_PAWN, 37);   // f5
    
//     Move illegalCapture{W_BISHOP, 28, 37, true, false};
//     EXPECT_FALSE(validator->isMoveLegal(illegalCapture));
// }

// Run all tests
int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}