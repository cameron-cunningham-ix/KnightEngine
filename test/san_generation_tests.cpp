#include <gtest/gtest.h>
#include "../src/move_history.hpp"

class SANGenerationTest : public ::testing::Test {
protected:
    MoveHistory history;
    ChessBoard board;
    GameState state;

    void SetUp() override {
        history = MoveHistory();
        board = ChessBoard();
        state = GameState();
    }
};

// Test basic pawn and piece moves
TEST_F(SANGenerationTest, BasicMoves) {
    setupPosition(board, state, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Pawn moves
    EXPECT_EQ("e4", history.generateSAN(Move(W_PAWN, 12, 28), board, state));
    EXPECT_EQ("d3", history.generateSAN(Move(W_PAWN, 11, 19), board, state));
    
    // Piece moves
    EXPECT_EQ("Nf3", history.generateSAN(Move(W_KNIGHT, 6, 21), board, state));
    EXPECT_EQ("Bc4", history.generateSAN(Move(W_BISHOP, 5, 26), board, state));
    EXPECT_EQ("Qd3", history.generateSAN(Move(W_QUEEN, 3, 19), board, state));
    EXPECT_EQ("Ke2", history.generateSAN(Move(W_KING, 4, 12), board, state));
}

// Test captures
TEST_F(SANGenerationTest, Captures) {
    setupPosition(board, state, "rnbqkbnr/ppp1p1pp/8/3p1p2/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1");
    
    // Pawn captures
    EXPECT_EQ("exd5", history.generateSAN(Move(W_PAWN, 28, 35, true), board, state));
    EXPECT_EQ("exf5", history.generateSAN(Move(W_PAWN, 28, 37, true), board, state));
    
    setupPosition(board, state, "rnbqkbnr/ppp1pppp/8/8/3pP3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    // En passant capture
    Move enPassant(B_PAWN, 27, 20, true);
    enPassant.isEnPassant = true;
    EXPECT_EQ("dxe3", history.generateSAN(enPassant, board, state));
}

// Test castling
TEST_F(SANGenerationTest, Castling) {
    setupPosition(board, state, "r3k2r/pppqpppp/8/8/8/8/PPPQPPPP/R3K2R w KQkq - 0 1");
    
    Move kingside(W_KING, 4, 6);
    kingside.isCastle = true;
    EXPECT_EQ("O-O", history.generateSAN(kingside, board, state));
    
    Move queenside(W_KING, 4, 2);
    queenside.isCastle = true;
    EXPECT_EQ("O-O-O", history.generateSAN(queenside, board, state));
}

// Test check and checkmate
TEST_F(SANGenerationTest, CheckAndMate) {
    setupPosition(board, state, "rnbqkbnr/pppp2pp/5p2/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1");
    
    // Move that gives check
    EXPECT_EQ("Qh5+", history.generateSAN(Move(W_QUEEN, 3, 39), board, state));
    
    setupPosition(board, state, "rnbqkbnr/pppp1ppp/8/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR w KQkq - 0 1");
    
    // Scholar's mate position
    EXPECT_EQ("Qxf7#", history.generateSAN(Move(W_QUEEN, 39, 53, true), board, state));
}

// Test pawn promotion
TEST_F(SANGenerationTest, Promotion) {
    setupPosition(board, state, "8/4P3/8/8/8/8/8/4K3 w - - 0 1");
    
    Move promotion(W_PAWN, 52, 60);
    promotion.isPromotion = true;
    promotion.promoteTo = W_QUEEN;
    EXPECT_EQ("e8=Q", history.generateSAN(promotion, board, state));
    
    setupPosition(board, state, "3r4/4P3/8/8/8/8/8/8 w - - 0 1");
    
    Move promotionCapture(W_PAWN, 52, 59, true);
    promotionCapture.isPromotion = true;
    promotionCapture.promoteTo = W_QUEEN;
    EXPECT_EQ("exd8=Q", history.generateSAN(promotionCapture, board, state));
}

// Test move disambiguation
TEST_F(SANGenerationTest, Disambiguation) {
    // Test file disambiguation
    setupPosition(board, state, "8/8/8/8/8/2N5/8/2N5 w - - 0 1");
    EXPECT_EQ("N3d2", history.generateSAN(Move(W_KNIGHT, 18, 19), board, state));
    EXPECT_EQ("N1d2", history.generateSAN(Move(W_KNIGHT, 2, 19), board, state));
    
    // Test rank disambiguation
    setupPosition(board, state, "8/8/8/8/8/1N6/8/N7 w - - 0 1");
    EXPECT_EQ("Nbc3", history.generateSAN(Move(W_KNIGHT, 17, 18), board, state));
    EXPECT_EQ("Nac3", history.generateSAN(Move(W_KNIGHT, 0, 18), board, state));
    
    // Test full disambiguation (both file and rank needed)
    setupPosition(board, state, "8/8/8/8/8/N7/8/N7 w - - 0 1");
    EXPECT_EQ("N1b2", history.generateSAN(Move(W_KNIGHT, 0, 9), board, state));
    EXPECT_EQ("N3b2", history.generateSAN(Move(W_KNIGHT, 16, 9), board, state));
}

// Test edge cases
TEST_F(SANGenerationTest, EdgeCases) {
    // Multiple pieces that could move to same square but some are pinned
    setupPosition(board, state, "8/8/8/8/3k4/2R5/2R5/4K3 w - - 0 1");
    EXPECT_EQ("R2c4", history.generateSAN(Move(W_ROOK, 10, 26), board, state));
    
    // Capture that gives check
    setupPosition(board, state, "4k3/8/8/8/8/5n2/8/4K2R w - - 0 1");
    EXPECT_EQ("Rxf3+", history.generateSAN(Move(W_ROOK, 7, 21, true), board, state));
    
    // Promotion that gives check
    setupPosition(board, state, "4k3/4P3/8/8/8/8/8/4K3 w - - 0 1");
    Move promCheck(W_PAWN, 52, 60);
    promCheck.isPromotion = true;
    promCheck.promoteTo = W_QUEEN;
    EXPECT_EQ("e8=Q+", history.generateSAN(promCheck, board, state));
}