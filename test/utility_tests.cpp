// test/utility_tests.cpp
#include <gtest/gtest.h>
#include "../src/utility.hpp"

class UtilityTest : public ::testing::Test {
protected:
    ChessBoard board;  // Fresh board for each test
    GameState state;

    void SetUp() override {
        // Called before each test
        board = ChessBoard();
        state = GameState();
    }
};

// Test fenToPiece map
TEST_F(UtilityTest, FENToPieceMap) {
    EXPECT_EQ(fenToPiece.at('P'), W_PAWN);
    EXPECT_EQ(fenToPiece.at('p'), B_PAWN);
    EXPECT_EQ(fenToPiece.at('N'), W_KNIGHT);
    EXPECT_EQ(fenToPiece.at('n'), B_KNIGHT);
    EXPECT_EQ(fenToPiece.at('B'), W_BISHOP);
    EXPECT_EQ(fenToPiece.at('b'), B_BISHOP);
    EXPECT_EQ(fenToPiece.at('R'), W_ROOK);
    EXPECT_EQ(fenToPiece.at('r'), B_ROOK);
    EXPECT_EQ(fenToPiece.at('Q'), W_QUEEN);
    EXPECT_EQ(fenToPiece.at('q'), B_QUEEN);
    EXPECT_EQ(fenToPiece.at('K'), W_KING);
    EXPECT_EQ(fenToPiece.at('k'), B_KING);
}

// Test pieceToFEN map
TEST_F(UtilityTest, PieceToFENMap) {
    EXPECT_EQ(pieceToFEN.at(W_PAWN), "P");
    EXPECT_EQ(pieceToFEN.at(B_PAWN), "p");
    EXPECT_EQ(pieceToFEN.at(W_KNIGHT), "N");
    EXPECT_EQ(pieceToFEN.at(B_KNIGHT), "n");
    EXPECT_EQ(pieceToFEN.at(W_BISHOP), "B");
    EXPECT_EQ(pieceToFEN.at(B_BISHOP), "b");
    EXPECT_EQ(pieceToFEN.at(W_ROOK), "R");
    EXPECT_EQ(pieceToFEN.at(B_ROOK), "r");
    EXPECT_EQ(pieceToFEN.at(W_QUEEN), "Q");
    EXPECT_EQ(pieceToFEN.at(B_QUEEN), "q");
    EXPECT_EQ(pieceToFEN.at(W_KING), "K");
    EXPECT_EQ(pieceToFEN.at(B_KING), "k");
}

// Test pieceCode
TEST_F(UtilityTest, PieceCodes) {
    EXPECT_EQ(pieceCode(W_PAWN), nPawn);
    EXPECT_EQ(pieceCode(B_PAWN), nPawn);
    EXPECT_EQ(pieceCode(W_KNIGHT), nKnight);
    EXPECT_EQ(pieceCode(B_KNIGHT), nKnight);
    EXPECT_EQ(pieceCode(W_BISHOP), nBishop);
    EXPECT_EQ(pieceCode(B_BISHOP), nBishop);
    EXPECT_EQ(pieceCode(W_ROOK), nRook);
    EXPECT_EQ(pieceCode(B_ROOK), nRook);
    EXPECT_EQ(pieceCode(W_QUEEN), nQueen);
    EXPECT_EQ(pieceCode(B_QUEEN), nQueen);
    EXPECT_EQ(pieceCode(W_KING), nKing);
    EXPECT_EQ(pieceCode(B_KING), nKing);
}

// Test colorCode
TEST_F(UtilityTest, ColorCodes) {
    EXPECT_EQ(colorCode(W_PAWN),  WHITE);
    EXPECT_EQ(colorCode(B_PAWN),  BLACK);
    EXPECT_EQ(colorCode(W_KNIGHT), WHITE);
    EXPECT_EQ(colorCode(B_KNIGHT), BLACK);
    EXPECT_EQ(colorCode(W_BISHOP), WHITE);
    EXPECT_EQ(colorCode(B_BISHOP), BLACK);
    EXPECT_EQ(colorCode(W_ROOK),  WHITE);
    EXPECT_EQ(colorCode(B_ROOK),  BLACK);
    EXPECT_EQ(colorCode(W_QUEEN), WHITE);
    EXPECT_EQ(colorCode(B_QUEEN), BLACK);
    EXPECT_EQ(colorCode(W_KING),  WHITE);
    EXPECT_EQ(colorCode(B_KING),  BLACK);
}

// Test algebraic notation conversion
TEST_F(UtilityTest, AlgebraicNotationConversion) {
    // Test valid conversions
    EXPECT_EQ(algebraicToIndex("a1"), 0);
    EXPECT_EQ(algebraicToIndex("h1"), 7);
    EXPECT_EQ(algebraicToIndex("a8"), 56);
    EXPECT_EQ(algebraicToIndex("h8"), 63);
    EXPECT_EQ(algebraicToIndex("e4"), 28);
    
    // Test invalid inputs
    EXPECT_EQ(algebraicToIndex("i1"), -1);  // Invalid file
    EXPECT_EQ(algebraicToIndex("a9"), -1);  // Invalid rank
    EXPECT_EQ(algebraicToIndex(""), -1);    // Empty string
    EXPECT_EQ(algebraicToIndex("a"), -1);   // Incomplete
    
    // Test reverse conversion
    EXPECT_EQ(indexToAlgebraic(0), "a1");
    EXPECT_EQ(indexToAlgebraic(7), "h1");
    EXPECT_EQ(indexToAlgebraic(56), "a8");
    EXPECT_EQ(indexToAlgebraic(63), "h8");
    EXPECT_EQ(indexToAlgebraic(28), "e4");
    
    // Test invalid indices
    EXPECT_EQ(indexToAlgebraic(-1), "??");
    EXPECT_EQ(indexToAlgebraic(64), "??");
}

// Test FEN position setup and printing
TEST_F(UtilityTest, FENOperations) {
    // Test initial position
    std::string initialFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    setupPosition(board, state, initialFEN);
    
    // Verify piece positions
    EXPECT_EQ(board.getWhitePawns(), 0x000000000000FF00ULL);
    EXPECT_EQ(board.getBlackPawns(), 0x00FF000000000000ULL);
    EXPECT_EQ(board.getWhiteKings(), 0x0000000000000010ULL);
    EXPECT_EQ(board.getBlackKings(), 0x1000000000000000ULL);
    
    // Verify game state
    EXPECT_EQ(state.sideToMove, WHITE);
    EXPECT_TRUE(state.canCastleWhiteKingside);
    EXPECT_TRUE(state.canCastleWhiteQueenside);
    EXPECT_TRUE(state.canCastleBlackKingside);
    EXPECT_TRUE(state.canCastleBlackQueenside);
    EXPECT_EQ(state.enPassantSquare, -1);
    
    // Test complex position
    std::string complexFEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    setupPosition(board, state, complexFEN);
    
    // Test printing FEN (should match input)
    testing::internal::CaptureStdout();
    printFEN(board, state);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find(complexFEN) != std::string::npos);
}

// Test legal moves counting
TEST_F(UtilityTest, LegalMovesCount) {
    // Test initial position
    setupPosition(board, state, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1");
    EXPECT_EQ(countLegalMoves(board, &state), 20);  // Initial position has 20 legal moves
    
    // Test scholar's mate position
    setupPosition(board, state, "r1bqkbnr/pppp1Qpp/8/n3p3/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 0 1");
    EXPECT_EQ(countLegalMoves(board, &state), 0);  // No legal moves, checkmate
}

// Test checkmate and stalemate detection
TEST_F(UtilityTest, GameEndDetection) {
    // Test scholar's mate checkmate
    setupPosition(board, state, "rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 0 1");
    EXPECT_TRUE(isCheckmate(board, &state));
    EXPECT_FALSE(isStalemate(board, &state));
    
    // Test basic stalemate
    setupPosition(board, state, "k7/8/1Q6/8/8/8/8/K7 b - - 0 1");
    EXPECT_FALSE(isCheckmate(board, &state));
    EXPECT_TRUE(isStalemate(board, &state));
    
    // Test normal position
    setupPosition(board, state, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1");
    EXPECT_FALSE(isCheckmate(board, &state));
    EXPECT_FALSE(isStalemate(board, &state));
}

// Test position setup for common test positions
TEST_F(UtilityTest, TestPositionSetup) {
    setupTestPosition(board, state, "initial");
    EXPECT_EQ(board.getWhitePawns(), 0x000000000000FF00ULL);
    
    setupTestPosition(board, state, "kiwipete");
    EXPECT_NE(board.getWhitePawns(), 0x000000000000FF00ULL);  // Should be different from initial
    
    setupTestPosition(board, state, "nonexistent");
    // Should handle invalid position name gracefully
}

// Test attack pattern verification
TEST_F(UtilityTest, AttackPatternVerification) {
    // Test knight attack pattern
    setupPosition(board, state, "8/8/8/8/4N3/8/8/8 w - - 0 1");
    std::vector<std::string> expectedKnightAttacks = {
        "f6", "g5", "g3", "f2", "d2", "c3", "c5", "d6"
    };
    EXPECT_TRUE(verifyAttackPattern(board, 28, expectedKnightAttacks));
    
    // Test bishop attack pattern
    setupPosition(board, state, "8/8/8/8/4B3/8/8/8 w - - 0 1");
    std::vector<std::string> expectedBishopAttacks = {
        "h7", "g6", "f5", "d3", "c2", "b1",
        "a8", "b7", "c6", "d5", "f3", "g2", "h1"
    };
    EXPECT_TRUE(verifyAttackPattern(board, 28, expectedBishopAttacks));

    // Test queen attack pattern
    setupPosition(board, state, "8/8/8/8/4Q3/8/8/8 w - - 0 1");
    std::vector<std::string> expectedQueenAttacks = {
        "h7", "g6", "f5", "d3", "c2", "b1",
        "a8", "b7", "c6", "d5", "f3", "g2", "h1",
        "e1", "e2", "e3", "e5", "e6", "e7", "e8",
        "a4", "b4", "c4", "d4", "f4", "g4", "h4"
    };
    EXPECT_TRUE(verifyAttackPattern(board, 28, expectedQueenAttacks));
}

// Test perft function for initial board
TEST_F(UtilityTest, PerftD1CalculationInitial) {
    // Test initial position at different depths
    setupPosition(board, state, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1");
    EXPECT_EQ(perft(board, &state, 1), 20ULL);          // Depth 1
}

// Test perft function for initial board
TEST_F(UtilityTest, PerftD2CalculationInitial) {
    // Test initial position at different depths
    setupPosition(board, state, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1");
    EXPECT_EQ(perft(board, &state, 2), 400ULL);         // Depth 2
}

// Test perft function for initial board
TEST_F(UtilityTest, PerftD3CalculationInitial) {
    // Test initial position at different depths
    setupPosition(board, state, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1");
    EXPECT_EQ(perft(board, &state, 3), 8902ULL);        // Depth 3
}

// Test perft function for initial board
TEST_F(UtilityTest, PerftD4CalculationInitial) {
    // Test initial position at different depths
    setupPosition(board, state, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1");
    EXPECT_EQ(perft(board, &state, 4), 197281ULL);      // Depth 4
}

// // Test perft function for initial board
// TEST_F(UtilityTest, PerftD5CalculationInitial) {
//     // Test initial position at different depths
//     setupPosition(board, state, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1");
//     EXPECT_EQ(perft(board, &state, 5), 4865609ULL);     // Depth 5
// }

// // Test perft function for initial board
// TEST_F(UtilityTest, PerftD6CalculationInitial) {
//     // Test initial position at different depths
//     setupPosition(board, state, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1");
//     EXPECT_EQ(perft(board, &state, 6), 119060324ULL);   // Depth 6
// }

// // Test perft function for initial board
// TEST_F(UtilityTest, PerftD7CalculationInitial) {
//     // Test initial position at different depths
//     setupPosition(board, state, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1");
//     EXPECT_EQ(perft(board, &state, 7), 3195901860ULL);   // Depth 7
// }

// TEST_F(UtilityTest, PerftD8CalculationInitial) {
//     // Test initial position at different depths
//     setupPosition(board, state, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1");
//     EXPECT_EQ(perft(board, &state, 8), 84998978956ULL);   // Depth 8
// }

//Test perft function for kiwipete board
TEST_F(UtilityTest, PerftD1CalculationKiwiPete) {
    setupPosition(board, state, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    EXPECT_EQ(perft(board, &state, 1), 48ULL);     // Depth 1
}

// Test perft function for kiwipete board
TEST_F(UtilityTest, PerftD2CalculationKiwiPete) {
    setupPosition(board, state, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    EXPECT_EQ(perft(board, &state, 2), 2039ULL);   // Depth 2
}

// Test perft function for kiwipete board
TEST_F(UtilityTest, PerftD3CalculationKiwiPete) {
    setupPosition(board, state, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    EXPECT_EQ(perft(board, &state, 3), 97862ULL);   // Depth 3
}

// // Test perft function for kiwipete board
// TEST_F(UtilityTest, PerftD4CalculationKiwiPete) {
//     setupPosition(board, state, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
//     EXPECT_EQ(perft(board, &state, 4), 4085603ULL);   // Depth 4
// }

// // Test perft function for "Position 3" board (from chessprogramming.org)
// TEST_F(UtilityTest, PerftD1CalculationPos3) {
//     setupPosition(board, state, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");
//     EXPECT_EQ(perft(board, &state, 1), 14ULL);   // Depth 1
// }

// // Test perft function for "Position 3" board (from chessprogramming.org)
// TEST_F(UtilityTest, PerftMetricsD1CalculationPos3) {
//     setupPosition(board, state, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
//     PerftMetrics metrics = calcPerftMetrics(board, &state, 1);  // Depth 1
//     EXPECT_EQ(metrics.nodes, 14);
//     EXPECT_EQ(metrics.captures, 1);
//     EXPECT_EQ(metrics.enPassants, 0);
//     EXPECT_EQ(metrics.castles, 0);
//     EXPECT_EQ(metrics.promotions, 0);
//     EXPECT_EQ(metrics.checks, 2);
//     EXPECT_EQ(metrics.checkmates, 0);
// }

// // Test perft function for "Position 3" board (from chessprogramming.org)
// TEST_F(UtilityTest, PerftMetricsD2CalculationPos3) {
//     setupPosition(board, state, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
//     PerftMetrics metrics = calcPerftMetrics(board, &state, 2);  // Depth 2
//     EXPECT_EQ(metrics.nodes, 191);
//     EXPECT_EQ(metrics.captures, 14);
//     EXPECT_EQ(metrics.enPassants, 0);
//     EXPECT_EQ(metrics.castles, 0);
//     EXPECT_EQ(metrics.promotions, 0);
//     EXPECT_EQ(metrics.checks, 10);
//     EXPECT_EQ(metrics.checkmates, 0);
// }

// // Test perft function for "Position 3_1" board (from chessprogramming.org)
// TEST_F(UtilityTest, PerftMetricsD1CalculationPos3_1) {
//     setupPosition(board, state, "8/2p5/3p4/KP5r/5R1k/8/4P1P1/8 b - - 0 1");
//     PerftMetrics metrics = calcPerftMetrics(board, &state, 1);  // Depth 1
//     EXPECT_EQ(metrics.nodes, 2);
//     EXPECT_EQ(metrics.captures, 0);
//     EXPECT_EQ(metrics.enPassants, 0);
//     EXPECT_EQ(metrics.castles, 0);
//     EXPECT_EQ(metrics.promotions, 0);
//     EXPECT_EQ(metrics.checks, 0);
//     EXPECT_EQ(metrics.checkmates, 0);
// }

// // Test perft function for "Position 3" board (from chessprogramming.org)
// TEST_F(UtilityTest, PerftD3CalculationPos3) {
//     setupPosition(board, state, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
//     EXPECT_EQ(perft(board, &state, 3), 2812ULL);   // Depth 3
// }

// Test perft function for "Position 4" board (from chessprogramming.org)
TEST_F(UtilityTest, PerftD1CalculationPos4) {
    setupPosition(board, state, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    EXPECT_EQ(perft(board, &state, 1), 6ULL);   // Depth 1
}

// Test perft function for "Position 4" board (from chessprogramming.org)
TEST_F(UtilityTest, PerftD2CalculationPos4) {
    setupPosition(board, state, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    EXPECT_EQ(perft(board, &state, 2), 264ULL);   // Depth 2
}

// Test perft function for "Position 4" board (from chessprogramming.org)
TEST_F(UtilityTest, PerftD3CalculationPos4) {
    setupPosition(board, state, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    EXPECT_EQ(perft(board, &state, 3), 9467ULL);   // Depth 3
}

// Test perft function for "Position 4" board (from chessprogramming.org)
TEST_F(UtilityTest, PerftD4CalculationPos4) {
    setupPosition(board, state, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    EXPECT_EQ(perft(board, &state, 4), 422333ULL);   // Depth 4
}

// Test perft function for "Position 4" mirrored board (from chessprogramming.org)
TEST_F(UtilityTest, PerftD1CalculationPos4Mirror) {
    setupPosition(board, state, "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 ");
    EXPECT_EQ(perft(board, &state, 1), 6ULL);   // Depth 1
}

// Test perft function for "Position 4" board (from chessprogramming.org)
TEST_F(UtilityTest, PerftD2CalculationPos4Mirror) {
    setupPosition(board, state, "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 ");
    EXPECT_EQ(perft(board, &state, 2), 264ULL);   // Depth 2
}

// Test perft function for "Position 4" board (from chessprogramming.org)
TEST_F(UtilityTest, PerftD3CalculationPos4Mirror) {
    setupPosition(board, state, "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 ");
    EXPECT_EQ(perft(board, &state, 3), 9467ULL);   // Depth 3
}

// Test perft function for "Position 4" board (from chessprogramming.org)
TEST_F(UtilityTest, PerftD4CalculationPos4Mirror) {
    setupPosition(board, state, "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 ");
    EXPECT_EQ(perft(board, &state, 4), 422333ULL);   // Depth 4
}

// // Test perft function for "Position 4" board (from chessprogramming.org)
// TEST_F(UtilityTest, PerftD5CalculationPos4Mirror) {
//     setupPosition(board, state, "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 ");
//     EXPECT_EQ(perft(board, &state, 5), 15833292ULL);   // Depth 5
// }

// Test perft function for "Position 5" board (from chessprogramming.org)
TEST_F(UtilityTest, PerftD1CalculationPos5) {
    setupPosition(board, state, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
    EXPECT_EQ(perft(board, &state, 1), 44ULL);   // Depth 1
}

// Test perft function for "Position 5" board (from chessprogramming.org)
TEST_F(UtilityTest, PerftD2CalculationPos5) {
    setupPosition(board, state, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
    EXPECT_EQ(perft(board, &state, 2), 1486ULL);   // Depth 2
}

// Test perft function for "Position 5" board (from chessprogramming.org)
TEST_F(UtilityTest, PerftD3CalculationPos5) {
    setupPosition(board, state, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
    EXPECT_EQ(perft(board, &state, 3), 62379ULL);   // Depth 3
}

// Test perft function for "Position 5" board (from chessprogramming.org)
TEST_F(UtilityTest, PerftD4CalculationPos5) {
    setupPosition(board, state, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
    EXPECT_EQ(perft(board, &state, 4), 2103487ULL);   // Depth 4
}

// // Test perft function for "Position 5" board (from chessprogramming.org)
// TEST_F(UtilityTest, PerftD5CalculationPos5) {
//     setupPosition(board, state, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
//     EXPECT_EQ(perft(board, &state, 5), 89941194ULL);   // Depth 5
// }

// Test perft function for "Position 6" board (from chessprogramming.org)
TEST_F(UtilityTest, PerftD1CalculationPos6) {
    setupPosition(board, state, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
    EXPECT_EQ(perft(board, &state, 1), 46ULL);   // Depth 1
}

// Test perft function for "Position 6" board (from chessprogramming.org)
TEST_F(UtilityTest, PerftD2CalculationPos6) {
    setupPosition(board, state, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
    EXPECT_EQ(perft(board, &state, 2), 2079ULL);   // Depth 2
}

// Test perft function for "Position 6" board (from chessprogramming.org)
TEST_F(UtilityTest, PerftD3CalculationPos6) {
    setupPosition(board, state, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
    EXPECT_EQ(perft(board, &state, 3), 89890ULL);   // Depth 3
}

// // Test perft function for "Position 6" board (from chessprogramming.org)
// TEST_F(UtilityTest, PerftD4CalculationPos6) {
//     setupPosition(board, state, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
//     EXPECT_EQ(perft(board, &state, 4), 3894594ULL);   // Depth 4
// }

// Test perft function for 3rd board (from https://github.com/AndyGrant/Ethereal/blob/master/src/perft/standard.epd)
TEST_F(UtilityTest, PerftD1CalculationStandardPos3) {
    setupPosition(board, state, "4k3/8/8/8/8/8/8/4K2R w K - 0 1");
    EXPECT_EQ(perft(board, &state, 1), 15ULL);   // Depth 1
}

// Test perft function for 3rd board (from https://github.com/AndyGrant/Ethereal/blob/master/src/perft/standard.epd)
TEST_F(UtilityTest, PerftD2CalculationStandardPos3) {
    setupPosition(board, state, "4k3/8/8/8/8/8/8/4K2R w K - 0 1");
    EXPECT_EQ(perft(board, &state, 2), 66ULL);   // Depth 2
}

// Test perft function for 3rd board (from https://github.com/AndyGrant/Ethereal/blob/master/src/perft/standard.epd)
TEST_F(UtilityTest, PerftD3CalculationStandardPos3) {
    setupPosition(board, state, "4k3/8/8/8/8/8/8/4K2R w K - 0 1");
    EXPECT_EQ(perft(board, &state, 3), 1197ULL);   // Depth 1
}

// Test perft function for 3rd board (from https://github.com/AndyGrant/Ethereal/blob/master/src/perft/standard.epd)
TEST_F(UtilityTest, PerftD4CalculationStandardPos3) {
    setupPosition(board, state, "4k3/8/8/8/8/8/8/4K2R w K - 0 1");
    EXPECT_EQ(perft(board, &state, 4), 7059ULL);   // Depth 4
}

// Test perft function for 3rd board (from https://github.com/AndyGrant/Ethereal/blob/master/src/perft/standard.epd)
TEST_F(UtilityTest, PerftD5CalculationStandardPos3) {
    setupPosition(board, state, "4k3/8/8/8/8/8/8/4K2R w K - 0 1");
    EXPECT_EQ(perft(board, &state, 5), 133987ULL);   // Depth 5
}

// Test perft function for 4th board
TEST_F(UtilityTest, PerftD1CalculationStandardPos4) {
    setupPosition(board, state, "4k3/8/8/8/8/8/8/R3K3 w Q - 0 1");
    EXPECT_EQ(perft(board, &state, 1), 16ULL);   // Depth 1
}

// Test perft function for 4th board
TEST_F(UtilityTest, PerftD2CalculationStandardPos4) {
    setupPosition(board, state, "4k3/8/8/8/8/8/8/R3K3 w Q - 0 1");
    EXPECT_EQ(perft(board, &state, 2), 71ULL);   // Depth 2
}

// Test perft function for 4th board
TEST_F(UtilityTest, PerftD3CalculationStandardPos4) {
    setupPosition(board, state, "4k3/8/8/8/8/8/8/R3K3 w Q - 0 1");
    EXPECT_EQ(perft(board, &state, 3), 1287ULL);   // Depth 3
}

// Test perft function for 4th board
TEST_F(UtilityTest, PerftD4CalculationStandardPos4) {
    setupPosition(board, state, "4k3/8/8/8/8/8/8/R3K3 w Q - 0 1");
    EXPECT_EQ(perft(board, &state, 4), 7626ULL);   // Depth 4
}

// Test perft function for 4th board
TEST_F(UtilityTest, PerftD5CalculationStandardPos4) {
    setupPosition(board, state, "4k3/8/8/8/8/8/8/R3K3 w Q - 0 1");
    EXPECT_EQ(perft(board, &state, 5), 145232ULL);   // Depth 5
}

// Test perft function for 5th board
TEST_F(UtilityTest, PerftD1CalculationStandardPos5) {
    setupPosition(board, state, "4k2r/8/8/8/8/8/8/4K3 w k - 0 1");
    EXPECT_EQ(perft(board, &state, 1), 5ULL);   // Depth 1
}

// Test perft function for 5th board
TEST_F(UtilityTest, PerftD2CalculationStandardPos5) {
    setupPosition(board, state, "4k2r/8/8/8/8/8/8/4K3 w k - 0 1");
    EXPECT_EQ(perft(board, &state, 2), 75ULL);   // Depth 2
}

// Test perft function for 5th board
TEST_F(UtilityTest, PerftD3CalculationStandardPos5) {
    setupPosition(board, state, "4k2r/8/8/8/8/8/8/4K3 w k - 0 1");
    EXPECT_EQ(perft(board, &state, 3), 459ULL);   // Depth 3
}

// Test perft function for 5th board
TEST_F(UtilityTest, PerftD4CalculationStandardPos5) {
    setupPosition(board, state, "4k2r/8/8/8/8/8/8/4K3 w k - 0 1");
    EXPECT_EQ(perft(board, &state, 4), 8290ULL);   // Depth 4
}

// Test perft function for 5th board
TEST_F(UtilityTest, PerftD5CalculationStandardPos5) {
    setupPosition(board, state, "4k2r/8/8/8/8/8/8/4K3 w k - 0 1");
    EXPECT_EQ(perft(board, &state, 5), 47635ULL);   // Depth 5
}

// Test perft function for 6th board
TEST_F(UtilityTest, PerftD1CalculationStandardPos6) {
    setupPosition(board, state, "r3k3/8/8/8/8/8/8/4K3 w q - 0 1");
    EXPECT_EQ(perft(board, &state, 1), 5ULL);   // Depth 1
}

// Test perft function for 6th board
TEST_F(UtilityTest, PerftD2CalculationStandardPos6) {
    setupPosition(board, state, "r3k3/8/8/8/8/8/8/4K3 w q - 0 1");
    EXPECT_EQ(perft(board, &state, 2), 80ULL);   // Depth 2
}

// Test perft function for 6th board
TEST_F(UtilityTest, PerftD3CalculationStandardPos6) {
    setupPosition(board, state, "r3k3/8/8/8/8/8/8/4K3 w q - 0 1");
    EXPECT_EQ(perft(board, &state, 3), 493ULL);   // Depth 3
}

// Test perft function for 6th board
TEST_F(UtilityTest, PerftD4CalculationStandardPos6) {
    setupPosition(board, state, "r3k3/8/8/8/8/8/8/4K3 w q - 0 1");
    EXPECT_EQ(perft(board, &state, 4), 8897ULL);   // Depth 4
}

// Test perft function for 6th board
TEST_F(UtilityTest, PerftD5CalculationStandardPos6) {
    setupPosition(board, state, "r3k3/8/8/8/8/8/8/4K3 w q - 0 1");
    EXPECT_EQ(perft(board, &state, 5), 52710ULL);   // Depth 5
}