#include "../src/move_history.hpp"
#include "../src/utility.hpp"
#include "../src/pext_bitboard.hpp"
#include <gtest/gtest.h>

class SANGenerationTest : public ::testing::Test {
protected:
    MoveHistory history;
    ChessBoard board;

    void SetUp() override {
        // Initialize PEXT
        PEXT::initialize();
        history = MoveHistory();
        board = ChessBoard();
    }
};

// Test basic pawn and piece moves
TEST_F(SANGenerationTest, BasicMoves) {
    board.setupPositionFromFEN( "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Pawn moves
    EXPECT_EQ("e4", history.generateSAN(DenseMove(W_PAWN, 12, 28), board));
    EXPECT_EQ("d3", history.generateSAN(DenseMove(W_PAWN, 11, 19), board));
    
    // Piece moves
    EXPECT_EQ("Nf3", history.generateSAN(DenseMove(W_KNIGHT, 6, 21), board));
    EXPECT_EQ("Bc4", history.generateSAN(DenseMove(W_BISHOP, 5, 26), board));
    EXPECT_EQ("Qd3", history.generateSAN(DenseMove(W_QUEEN, 3, 19), board));
    EXPECT_EQ("Ke2", history.generateSAN(DenseMove(W_KING, 4, 12), board));
}

// Test captures
TEST_F(SANGenerationTest, Captures) {
    board.setupPositionFromFEN( "rnbqkbnr/ppp1p1pp/8/3p1p2/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1");
    
    // Pawn captures
    EXPECT_EQ("exd5", history.generateSAN(DenseMove(W_PAWN, 28, 35, D_PAWN), board));
    EXPECT_EQ("exf5", history.generateSAN(DenseMove(W_PAWN, 28, 37, D_PAWN), board));
    
    board.setupPositionFromFEN( "rnbqkbnr/ppp1pppp/8/8/3pP3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    // En passant capture
    DenseMove enPassant(B_PAWN, 27, 20, D_PAWN, false, true);
    EXPECT_EQ("dxe3", history.generateSAN(enPassant, board));
}

// Test castling
TEST_F(SANGenerationTest, Castling) {
    board.setupPositionFromFEN( "r3k2r/pppqpppp/8/8/8/8/PPPQPPPP/R3K2R w KQkq - 0 1");
    
    DenseMove kingside(W_KING, 4, 6, D_EMPTY, true);
    EXPECT_EQ("O-O", history.generateSAN(kingside, board));
    
    DenseMove queenside(W_KING, 4, 2, D_EMPTY, true);
    EXPECT_EQ("O-O-O", history.generateSAN(queenside, board));
}

// Test check and checkmate
TEST_F(SANGenerationTest, CheckAndMate) {
    board.setupPositionFromFEN( "rnbqkbnr/pppp2pp/5p2/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1");
    
    // Move that gives check
    EXPECT_EQ("Qh5+", history.generateSAN(DenseMove(W_QUEEN, 3, 39), board));
    
    board.setupPositionFromFEN( "rnbqkbnr/pppp1ppp/8/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR w KQkq - 0 1");
    
    // Scholar's mate position
    EXPECT_EQ("Qxf7#", history.generateSAN(DenseMove(W_QUEEN, 39, 53, D_PAWN), board));
}

// Test pawn promotion
TEST_F(SANGenerationTest, Promotion) {
    board.setupPositionFromFEN( "7k/4P3/8/8/8/8/8/4K3 w - - 0 1");
    
    DenseMove promotion(W_PAWN, 52, 60);
    promotion.setPromoteTo(D_QUEEN);
    EXPECT_EQ("e8=Q+", history.generateSAN(promotion, board));
    
    board.setupPositionFromFEN( "3r4/4P2k/8/8/8/8/8/4K3 w - - 0 1");
    
    DenseMove promotionCapture(W_PAWN, 52, 59);
    promotion.setPromoteTo(D_QUEEN);
    EXPECT_EQ("exd8=Q", history.generateSAN(promotionCapture, board));
}

// Test move disambiguation
TEST_F(SANGenerationTest, Disambiguation) {
    // Test file disambiguation
    board.setupPositionFromFEN("5k1K/8/8/8/8/2N5/8/2N5 w - - 0 1");
    EXPECT_EQ("N3e2", history.generateSAN(DenseMove(W_KNIGHT, 18, 12), board));
    EXPECT_EQ("N1e2", history.generateSAN(DenseMove(W_KNIGHT, 2, 12), board));
    
    // Test rank disambiguation
    board.setupPositionFromFEN("5k1K/8/8/8/8/1N6/N7/8 w - - 0 1");
    EXPECT_EQ("Nbc1", history.generateSAN(DenseMove(W_KNIGHT, 17, 2), board));
    EXPECT_EQ("Nac1", history.generateSAN(DenseMove(W_KNIGHT, 8, 2), board));
    
    // Test full disambiguation (both file and rank needed)
    board.setupPositionFromFEN( "5k1K/8/8/8/8/N3N3/8/N3N3 w - - 0 1");
    EXPECT_EQ("Na1c2", history.generateSAN(DenseMove(W_KNIGHT, 0, 10), board));
    EXPECT_EQ("Ne1c2", history.generateSAN(DenseMove(W_KNIGHT, 4, 10), board));
    EXPECT_EQ("Na3c2", history.generateSAN(DenseMove(W_KNIGHT, 16, 10), board));
    EXPECT_EQ("Ne3c2", history.generateSAN(DenseMove(W_KNIGHT, 20, 10), board));
}

// Test edge cases
TEST_F(SANGenerationTest, EdgeCases) {
    // Multiple pieces that could move to same square but some are pinned
    board.setupPositionFromFEN( "8/8/8/3k4/8/2R5/2R5/4K3 w - - 1 1");
    EXPECT_EQ("Rc4", history.generateSAN(DenseMove(W_ROOK, 18, 26), board));
    
    // Capture that gives check
    board.setupPositionFromFEN("5k2/8/8/8/8/5n2/8/4KR2 w - - 0 1");
    EXPECT_EQ("Rxf3+", history.generateSAN(DenseMove(W_ROOK, 5, 21, D_KNIGHT), board));
    
    // Promotion that gives check
    board.setupPositionFromFEN( "6k1/4P3/8/8/8/8/8/4K3 w - - 0 1");
    DenseMove promCheck(W_PAWN, 52, 60);
    promCheck.setPromoteTo(D_QUEEN);
    EXPECT_EQ("e8=Q+", history.generateSAN(promCheck, board));
}