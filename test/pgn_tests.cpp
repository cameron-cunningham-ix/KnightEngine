#include <gtest/gtest.h>
#include "../src/move_history.hpp"

class PGNTest : public ::testing::Test {
protected:
    MoveHistory history;
    ChessBoard board;
    GameState state;

    void SetUp() override {
        history = MoveHistory();
        board = ChessBoard();
        state = GameState();
    }

    // Helper to verify move count
    void verifyMoveCount(const std::string& pgn, size_t expectedMoves) {
        ASSERT_TRUE(history.fromPGN(pgn)) << "Failed to parse PGN";
        EXPECT_EQ(history.length(), expectedMoves) << "Incorrect number of moves";
    }

    // Helper to verify specific moves
    void verifyMove(const std::string& pgn, size_t moveIndex, 
                   const std::string& expectedSAN) {
        ASSERT_TRUE(history.fromPGN(pgn)) << "Failed to parse PGN";
        ASSERT_LT(moveIndex, history.length()) << "Move index out of range";
        EXPECT_EQ(history.getMove(moveIndex).san, expectedSAN)
            << "Incorrect SAN at move " << moveIndex;
    }
};

// Test generateSAN
TEST_F(PGNTest, GenerateSAN) {
    setupPosition(board, state, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    Move e4 = Move(W_PAWN, 12, 28);
    Move na3 = Move(W_KNIGHT, 1, 16);
    EXPECT_EQ("e4", history.generateSAN(e4, board, state));
    EXPECT_EQ("Na3", history.generateSAN(na3, board, state));
}

// Test basic move parsing
TEST_F(PGNTest, BasicMoves) {
    const char* pgn = "1. e4 e5 2. Nf3 Nc6 3. Bb5 a6";
    verifyMoveCount(pgn, 6);
    verifyMove(pgn, 0, "e4");
    verifyMove(pgn, 1, "e5");
    verifyMove(pgn, 2, "Nf3");
    verifyMove(pgn, 3, "Nc6");
    verifyMove(pgn, 4, "Bb5");
    verifyMove(pgn, 5, "a6");
}

// Test castling moves
TEST_F(PGNTest, Castling) {
    const char* pgn = "1. e4 e5 2. Nf3 Nf6 3. Bc4 Bc5 4. O-O O-O";
    verifyMoveCount(pgn, 8);
    verifyMove(pgn, 6, "O-O");
    verifyMove(pgn, 7, "O-O");

    pgn = "1. d4 d5 2. c4 e6 3. Nc3 Nf6 4. Bg5 Be7 5. e3 O-O 6. Nf3 h6 "
          "7. Bh4 b6 8. Be2 Bb7 9. O-O Ne4 10. Bxe7 Qxe7 11. Rc1 Nd7 12. cxd5 exd5 "
          "13. Qa4 a5 14. O-O-O";
    verifyMoveCount(pgn, 27);
    verifyMove(pgn, 26, "O-O-O");
}

// Test captures
TEST_F(PGNTest, Captures) {
    const char* pgn = "1. e4 e5 2. Nf3 Nc6 3. Bb5 a6 4. Bxc6 dxc6";
    verifyMoveCount(pgn, 8);
    verifyMove(pgn, 6, "Bxc6");
    verifyMove(pgn, 7, "dxc6");
}

// Test pawn promotions
TEST_F(PGNTest, Promotions) {
    const char* pgn = 
        "1. e4 d5 2. exd5 c6 3. dxc6 Nf6 4. cxb7 Bg4 5. bxa8=Q";
    verifyMoveCount(pgn, 9);
    verifyMove(pgn, 8, "bxa8=Q");

    // Test all promotion pieces
    pgn = "1. e4 d5 2. e5 d4 3. e6 d3 4. exf7+ Kd7 5. fxg8=N+ Ke6 "
          "6. h4 a5 7. h5 a4 8. h6 a3 9. h7 a2 10. h8=B a1=R";
    verifyMoveCount(pgn, 20);
    verifyMove(pgn, 18, "h8=B");
    verifyMove(pgn, 19, "a1=R");
}

// Test check and checkmate notation
TEST_F(PGNTest, CheckAndMate) {
    const char* pgn = "1. e4 e5 2. Bc4 Nc6 3. Qh5 Nf6 4. Qxf7#";
    verifyMoveCount(pgn, 7);
    verifyMove(pgn, 6, "Qxf7#");

    pgn = "1. e4 e5 2. Nf3 Nc6 3. Bc4 Bc5 4. b4+ Bxb4";
    verifyMoveCount(pgn, 7);
    verifyMove(pgn, 5, "b4+");
}

// Test move disambiguation
TEST_F(PGNTest, Disambiguation) {
    // File disambiguation
    const char* pgn = "1. d4 d5 2. Nf3 Nf6 3. Nc3 Nc6 4. Nbd2";
    verifyMoveCount(pgn, 7);
    verifyMove(pgn, 6, "Nbd2");

    // Rank disambiguation
    pgn = "1. d4 d5 2. Nc3 Nc6 3. Nf3 Nf6 4. e4 e6 5. N3e2";
    verifyMoveCount(pgn, 9);
    verifyMove(pgn, 8, "N3e2");
}

// Test PGN tags
TEST_F(PGNTest, Tags) {
    const char* pgn = 
        "[Event \"World Championship Match\"]\n"
        "[Site \"London, England\"]\n"
        "[Date \"2000.01.01\"]\n"
        "[Round \"1\"]\n"
        "[White \"Player, White\"]\n"
        "[Black \"Player, Black\"]\n"
        "[Result \"1-0\"]\n"
        "\n"
        "1. e4 e5 2. Nf3 Nc6 3. Bb5 1-0";

    ASSERT_TRUE(history.fromPGN(pgn));
    EXPECT_EQ(history.getTag("Event"), "World Championship Match");
    EXPECT_EQ(history.getTag("Site"), "London, England");
    EXPECT_EQ(history.getTag("Date"), "2000.01.01");
    EXPECT_EQ(history.getTag("Result"), "1-0");
}

// Test custom starting positions
TEST_F(PGNTest, CustomStartingPosition) {
    const char* pgn = 
        "[SetUp \"1\"]\n"
        "[FEN \"r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 1\"]\n"
        "\n"
        "1. Bb5 a6 2. Ba4 b5";

    ASSERT_TRUE(history.fromPGN(pgn));
    EXPECT_FALSE(history.isStartStandard());
    EXPECT_EQ(history.getStartingFEN(), 
              "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 1");
    verifyMoveCount(pgn, 4);
}

// Test comments and annotations
TEST_F(PGNTest, CommentsAndAnnotations) {
    const char* pgn = 
        "1. e4 {Best by test} e5 2. Nf3 Nc6 {The Ruy Lopez is coming} "
        "3. Bb5 $1 a6 $2 {Too passive}";

    ASSERT_TRUE(history.fromPGN(pgn));
    verifyMoveCount(pgn, 6);
    EXPECT_EQ(history.getMove(0).comment, "Best by test");
    EXPECT_EQ(history.getMove(3).comment, "The Ruy Lopez is coming");
    EXPECT_EQ(history.getMove(5).comment, "Too passive");

    const auto& move4 = history.getMove(4);
    EXPECT_FALSE(move4.nags.empty());
    EXPECT_EQ(move4.nags[0], "1");

    const auto& move5 = history.getMove(5);
    EXPECT_FALSE(move5.nags.empty());
    EXPECT_EQ(move5.nags[0], "2");
}

// Test variations
TEST_F(PGNTest, Variations) {
    const char* pgn = 
        "1. e4 e5 2. Nf3 (2. f4 exf4 3. Bc4 Qh4+) 2... Nc6 3. Bb5";

    ASSERT_TRUE(history.fromPGN(pgn));
    verifyMoveCount(pgn, 5);  // Should only count mainline moves
    verifyMove(pgn, 2, "Nf3");  // Mainline move should be Nf3, not f4
}

// Test error cases
TEST_F(PGNTest, ErrorCases) {
    // Invalid castling
    EXPECT_FALSE(history.fromPGN("1. e4 e5 2. O-O"));  // Can't castle yet

    // Invalid moves
    EXPECT_FALSE(history.fromPGN("1. e5"));  // Pawn can't move two squares diagonally
    EXPECT_FALSE(history.fromPGN("1. e4 e5 2. Nf3 Nc6 3. Bb5 Bb4"));  // Bishop can't move through pieces

    // Malformed PGN
    EXPECT_FALSE(history.fromPGN("[Event \"Test\" 1. e4"));  // Missing closing quote
    EXPECT_FALSE(history.fromPGN("1. e4 e5 2. Nf3 {Unclosed comment"));  // Unclosed comment

    // Invalid FEN
    EXPECT_FALSE(history.fromPGN(
        "[SetUp \"1\"]\n"
        "[FEN \"invalid fen\"]\n"
        "1. e4 e5"
    ));
}

// Test UTF-8 support
TEST_F(PGNTest, UTF8Support) {
    const char* pgn = 
        "[Event \"Künstlicher Wettkampf\"]\n"
        "[White \"Spieler, Weiß\"]\n"
        "[Black \"Spieler, Schwarz\"]\n"
        "1. e4 e5 2. Nf3 Nc6";

    ASSERT_TRUE(history.fromPGN(pgn));
    EXPECT_EQ(history.getTag("Event"), "Künstlicher Wettkampf");
    EXPECT_EQ(history.getTag("White"), "Spieler, Weiß");
}

// Test move numbering variants
TEST_F(PGNTest, MoveNumbering) {
    // Standard numbering
    EXPECT_TRUE(history.fromPGN("1. e4 e5 2. Nf3 Nc6"));

    // Black first with ellipsis
    EXPECT_TRUE(history.fromPGN("1... e5 2. Nf3 Nc6"));

    // Sparse numbering
    EXPECT_TRUE(history.fromPGN("1. e4 e5 Nf3 Nc6 Bb5 a6"));
}