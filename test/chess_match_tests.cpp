// test/chess_match_tests.cpp
#include "../src/chess_match.hpp"
#include "../src/human_player.hpp"
#include "../src/pext_bitboard.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <fstream>
#include <iostream>
#include <chrono>

// Mock player class for testing
class MockPlayer : public IPlayer {
private:
    std::string name;
    std::vector<DenseMove> moves;
    size_t moveIndex;
    int timeToThink;
    bool acceptDraw;

public:
    // Constructor takes a name and a sequence of moves to play
    MockPlayer(const std::string& playerName, std::vector<DenseMove> movesToPlay = {}, bool acceptDrawOffer = false, int timeToThink = -1)
        : name(playerName), moves(movesToPlay), moveIndex(0), timeToThink(timeToThink), acceptDraw(acceptDrawOffer) {}
    
    // Get the next move from our predefined sequence
    DenseMove getMove(ChessBoard& board, const ChessClock& clock) override {
        // Debug move print
        if (moveIndex < moves.size()) {
        std::cout << "(MockPlayer getMove) Move " << moveIndex << ": " << moves[moveIndex].getPieceType()
            << " from " << moves[moveIndex].getFrom()  << " to " << moves[moveIndex].getTo() << std::endl;
        }
        // If we have a thinking time, sleep for that duration
        if (timeToThink > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(timeToThink));
        }

        if (moveIndex >= moves.size()) {
            throw std::runtime_error("Player resigned due to move index out of bounds");
        }
        
        return moves[moveIndex++];
    }
    
    void notifyOpponentMove(const DenseMove& move) override {}
    std::string getName() const override { return name; }
    PlayerType getType() const override { return PlayerType::Engine; }
    bool acceptsDraw() const override { return acceptDraw; }
    void onGameEnd() override { moveIndex = 0; }
};

class ChessMatchTest : public ::testing::Test {
protected:
    std::unique_ptr<ChessMatch> match;
    std::unique_ptr<MockPlayer> whitePlayer;
    std::unique_ptr<MockPlayer> blackPlayer;

    void SetUp() override {
        // Initialize PEXT 
        PEXT::initialize();
    }

    // Helper to create a fresh match
    void createMatch(const std::vector<DenseMove>& whiteMoves = {}, 
                    const std::vector<DenseMove>& blackMoves = {}) {
        // Create players
        whitePlayer = std::make_unique<MockPlayer>("White", whiteMoves);
        blackPlayer = std::make_unique<MockPlayer>("Black", blackMoves);
        
        // Create time control
        TimeControl tc(std::chrono::minutes(5), std::chrono::seconds(3));
        
        std::cout << "Creating new match..." << std::endl;
        // Create match with the players
        match = std::make_unique<ChessMatch>(
            std::unique_ptr<IPlayer>(whitePlayer.release()),
            std::unique_ptr<IPlayer>(blackPlayer.release()),
            tc
        );
    }

    void TearDown() override {
        match.reset();  // Ensure clean cleanup
        whitePlayer.reset();
        blackPlayer.reset();
    }
};

// Test match initialization
TEST_F(ChessMatchTest, Initialization) {
    // std::ofstream outfile("CMTInit.txt");
    // // Get original cout buffer
    // std::streambuf* coutBuf = std::cout.rdbuf();
    // if (outfile.is_open()) {
    //     // Redirect
    //     std::cout.rdbuf(outfile.rdbuf());
    // }
    // Verify initial match state
    createMatch();
    EXPECT_TRUE(match->isInProgress());
    EXPECT_EQ(match->getResult(), MatchResult::InProgress);
    EXPECT_EQ(match->getTerminationReason(), TerminationReason::None);
    
    // Verify initial board position
    const ChessBoard& board = match->getBoard();
    EXPECT_EQ(board.getWhitePawns(), 0x000000000000FF00ULL);
    EXPECT_EQ(board.getBlackPawns(), 0x00FF000000000000ULL);
    
    // Verify initial game state
    const GameState& state = match->getState();
    EXPECT_EQ(state.sideToMove, WHITE);
    EXPECT_TRUE(state.canCastleWhiteKingside);
    EXPECT_TRUE(state.canCastleWhiteQueenside);
    EXPECT_TRUE(state.canCastleBlackKingside);
    EXPECT_TRUE(state.canCastleBlackQueenside);

    // // Restore cout
    // std::cout.rdbuf(coutBuf);
    // outfile.close();
}

// Test custom starting position 1
TEST_F(ChessMatchTest, CustomStartingPosition1) {
    createMatch();
    // Set up a custom position
    std::string customFEN = "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2";
    match->setInitialPosition(customFEN);
    
    const ChessBoard& board = match->getBoard();

    // Verify position is set correctly
    EXPECT_EQ(board.getPieceAt(28), W_PAWN);  // e4
    EXPECT_EQ(board.getPieceAt(34), B_PAWN);  // c5
    EXPECT_EQ(board.getSideToMove(), WHITE);
    EXPECT_TRUE(board.currentGameState.canCastleWhiteKingside);
}

// Test custom starting psotion 2
TEST_F(ChessMatchTest, CustomStartingPosition2) {
    createMatch();
    // Set up a custom position
    std::string customFEN = "k7/8/8/8/8/8/1Q6/K7 w - - 0 1";
    match->setInitialPosition(customFEN);
    
    const ChessBoard& board = match->getBoard();
    const GameState& state = match->getState();
    
    // Verify position is set correctly
    EXPECT_EQ(board.getPieceAt(0), W_KING);  // a1
    EXPECT_EQ(board.getPieceAt(9), W_QUEEN);  // b2
    EXPECT_EQ(board.getPieceAt(56), B_KING);  // a8
    EXPECT_EQ(board.getPieceAt(0), W_KING);
    EXPECT_EQ(board.getPieceAt(9), W_QUEEN);
    EXPECT_EQ(board.getPieceAt(56), B_KING);
    EXPECT_EQ(state.sideToMove, WHITE);
    EXPECT_FALSE(state.canCastleWhiteKingside);
}

TEST_F(ChessMatchTest, StatePersistence) {
    createMatch();

    const ChessBoard& board = match->getBoard();
    std::cout << "Initial state:" << std::endl;
    std::cout << "White pawns: " << std::hex << board.getWhitePawns() << std::dec << std::endl;
    std::cout << "Black pawns: " << std::hex << board.getBlackPawns() << std::dec << std::endl;
    std::cout << "Side to move: " << (board.getSideToMove() == WHITE ? "WHITE" : "BLACK") << std::endl;

    // Make white's move
    DenseMove whiteMove(W_PAWN, 12, 28);  // e2e4
    match->getBoard().makeMove(whiteMove, false);

    std::cout << "\nAfter white move:" << std::endl;
    std::cout << "White pawns: " << std::hex << board.getWhitePawns() << std::dec << std::endl;
    std::cout << "Black pawns: " << std::hex << board.getBlackPawns() << std::dec << std::endl;
    std::cout << "Side to move: " << (board.getSideToMove() == WHITE ? "WHITE" : "BLACK") << std::endl;

    // Verify white's move persisted
    EXPECT_NE(board.getWhitePawns(), 0x000000000000FF00ULL) << "White pawns should have moved";
    EXPECT_EQ(board.getSideToMove(), BLACK) << "Side should have switched to BLACK";

    // Make black's move
    DenseMove blackMove(B_PAWN, 52, 36);  // e7e5
    match->getBoard().makeMove(blackMove, false);

    std::cout << "\nAfter black move:" << std::endl;
    std::cout << "White pawns: " << std::hex << board.getWhitePawns() << std::dec << std::endl;
    std::cout << "Black pawns: " << std::hex << board.getBlackPawns() << std::dec << std::endl;
    std::cout << "Side to move: " << (board.getSideToMove() == WHITE ? "WHITE" : "BLACK") << std::endl;

    // Verify both moves persisted
    EXPECT_NE(board.getWhitePawns(), 0x000000000000FF00ULL) << "White pawns should still show moved";
    EXPECT_NE(board.getBlackPawns(), 0x00FF000000000000ULL) << "Black pawns should have moved";
    EXPECT_EQ(board.getSideToMove(), WHITE) << "Side should have switched back to WHITE";
}

// Test scholars mate sequence
TEST_F(ChessMatchTest, ScholarsMate) {
    std::ofstream outfile("TestOutput/ScholarsMate.txt");
    // Get original cout buffer
    std::streambuf* coutBuf = std::cout.rdbuf();
    if (outfile.is_open()) {
        // Redirect
        std::cout.rdbuf(outfile.rdbuf());
    }

    std::vector<DenseMove> whiteMoves = {
        DenseMove(W_PAWN, 12, 28),      // 1. e4
        DenseMove(W_BISHOP, 5, 26),     // 2. Bc4
        DenseMove(W_QUEEN, 3, 39),      // 3. Qh5
        DenseMove(W_QUEEN, 39, 53, D_PAWN) // 4. Qxf7#
    };
    
    std::vector<DenseMove> blackMoves = {
        DenseMove(B_PAWN, 52, 36),      // 1... e5
        DenseMove(B_KNIGHT, 62, 45),    // 2... Nc6
        DenseMove(B_PAWN, 55, 47)       // 3... h6
    };
    
    // Create match with the moves
    createMatch(whiteMoves, blackMoves);

    // Verify initial state
    ASSERT_TRUE(match != nullptr);
    std::cout << "Initial board state:" << std::endl;
    printBoard(match->getBoard());
    
    match->start();

    // Verify final state
    std::cout << "Final board state:" << std::endl;
    printBoard(match->getBoard());
    EXPECT_FALSE(match->isInProgress());
    EXPECT_EQ(match->getResult(), MatchResult::WhiteWin);
    EXPECT_EQ(match->getTerminationReason(), TerminationReason::Checkmate);
    
    // Restore cout
    std::cout.rdbuf(coutBuf);
    outfile.close();
}

// Test resignation
TEST_F(ChessMatchTest, Resignation) {
    // Setup where black resigns after white's first move
    std::vector<DenseMove> whiteMoves = {
        DenseMove(W_PAWN, 12, 28)  // 1. e4
    };
    std::vector<DenseMove> blackMoves = {};  // Empty moves vector will trigger resignation
    
    createMatch(whiteMoves, blackMoves);
    match->start();
    
    EXPECT_FALSE(match->isInProgress());
    EXPECT_EQ(match->getResult(), MatchResult::WhiteWin);
    EXPECT_EQ(match->getTerminationReason(), TerminationReason::BlackResigned);
}

// Test stalemate
TEST_F(ChessMatchTest, Stalemate) {
    std::vector<DenseMove> whiteMoves = {
        DenseMove(W_QUEEN, 9, 41)  // Stalemate move
    };
    std::vector<DenseMove> blackMoves = {};
    
    createMatch(whiteMoves, blackMoves);
    // Setup a basic stalemate position
    match->setInitialPosition("k7/8/8/8/8/8/1Q6/K7 w - - 0 1");
    match->start();
    
    EXPECT_FALSE(match->isInProgress());
    EXPECT_EQ(match->getResult(), MatchResult::Draw);
    EXPECT_EQ(match->getTerminationReason(), TerminationReason::Stalemate);
}

// Test time forfeit
TEST_F(ChessMatchTest, TimeForfeit) {
    // Create very short time control
    TimeControl tc(std::chrono::seconds(1), std::chrono::seconds(0));
    
    // Create players that will take too long
    whitePlayer = std::make_unique<MockPlayer>("White", std::vector<DenseMove> {DenseMove(W_PAWN, 8, 16)}, false, 2100);
    blackPlayer = std::make_unique<MockPlayer>("Black");
    
    match = std::make_unique<ChessMatch>(
        std::unique_ptr<IPlayer>(whitePlayer.release()),
        std::unique_ptr<IPlayer>(blackPlayer.release()),
        tc
    );
    
    match->start();
    std::cout << "White pawns: " << std::endl;
    printBBLine(match->getBoard().getWhitePawns());
    // // Wait for time to run out
    // std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    
    
    EXPECT_FALSE(match->isInProgress());
    EXPECT_EQ(match->getResult(), MatchResult::BlackWin);
    EXPECT_EQ(match->getTerminationReason(), TerminationReason::WhiteTimeout);
}

// Test basic PGN export
TEST_F(ChessMatchTest, PGNExport1) {
    // Play a few moves
    std::vector<DenseMove> whiteMoves = {
        DenseMove(W_PAWN, 12, 28),      // 1. e4
        DenseMove(W_KNIGHT, 6, 21)      // 2. Nf3
    };
    std::vector<DenseMove> blackMoves = {
        DenseMove(B_PAWN, 52, 36),      // 1... e5
        DenseMove(B_KNIGHT, 62, 45)     // 2... Nf6
    };
    
    createMatch(whiteMoves, blackMoves);
    match->start();
    
    std::string pgn = match->getPGN();
    std::cout << "PGN:\n" << pgn << std::endl;
    
    // Verify PGN contains essential elements
    EXPECT_TRUE(pgn.find("[Event \"Chess Match\"]") != std::string::npos);
    EXPECT_TRUE(pgn.find("[White \"White\"]") != std::string::npos);
    EXPECT_TRUE(pgn.find("[Black \"Black\"]") != std::string::npos);
    EXPECT_TRUE(pgn.find("1. e4 e5 2. Nf3 Nf6") != std::string::npos);
}

// Test PGN export with game result
TEST_F(ChessMatchTest, PGNExport2) {
    testing::internal::CaptureStdout();

    std::ofstream outfile("TestOutput/PGNExport2.txt");
    if (outfile.is_open()) {
        outfile << "PGNExport2 test: " << std::endl;
    }
    // Setup a match that ends in checkmate
    std::vector<DenseMove> whiteMoves = {
        DenseMove(W_PAWN, 12, 28),      // 1. e4
        DenseMove(W_KNIGHT, 6, 21),     // 2. Nf3
        DenseMove(W_KNIGHT, 21, 36, D_PAWN),    // 3. Nxe5
        DenseMove(W_KNIGHT, 36, 42, D_PAWN),     // 4. Nxc6
        DenseMove(W_KNIGHT, 42, 48, D_PAWN),     // 5. Nxa7
        DenseMove(W_KNIGHT, 48, 58, D_BISHOP),     // 6. Nxc8
        DenseMove(W_KNIGHT, 58, 43),           // 7. Nd6+
        DenseMove(W_KNIGHT, 43, 58),           // 8. Nc8+
        DenseMove(W_BISHOP, 5, 26),           // 9. Bc4+
        DenseMove(W_PAWN, 11, 27),           // 10. d4+
        DenseMove(W_QUEEN, 3, 19),           // 11. Qd3#
    };
    std::vector<DenseMove> blackMoves = {
        DenseMove(B_PAWN, 52, 36),      // 1... e5
        DenseMove(B_KNIGHT, 62, 45),    // 2... Nf6
        DenseMove(B_PAWN, 50, 42),      // 3... c6 
        DenseMove(B_ROOK, 63, 62),      // 4... Rg8
        DenseMove(B_ROOK, 62, 63),      // 5... Rh8
        DenseMove(B_ROOK, 63, 62),       // 6... Rg8
        DenseMove(B_KING, 60, 52),       // 7... Ke7
        DenseMove(B_KING, 52, 44),        // 8... Ke6
        DenseMove(B_KING, 44, 36),        // 9... Ke5
        DenseMove(B_KING, 36, 28, D_PAWN)        // 10... Kxe4
    };
    
    createMatch(whiteMoves, blackMoves);
    match->start();
    
    std::string pgn = match->getPGN();
    std::cout << "PGN:\n" << pgn << std::endl;
    
    // Verify PGN contains essential elements
    EXPECT_TRUE(pgn.find("[Result \"1-0\"]") != std::string::npos);
    EXPECT_TRUE(pgn.find("1. e4 e5 2. Nf3 Nf6 3. Nxe5 c6 4. Nxc6 Rg8 5. Nxa7 Rh8 6. Nxc8 Rg8 7. Nd6+ Ke7 "
                         "8. Nc8+ Ke6 9. Bc4+ Ke5 10. d4+ Kxe4 11. Qd3#") != std::string::npos);
    std::string output = testing::internal::GetCapturedStdout();
    outfile << output;
    outfile.close();
}


// // Test pause and resume
// TEST_F(ChessMatchTest, PauseResume) {
//     TimeControl tc(std::chrono::seconds(10), std::chrono::seconds(10));
//     // Create players that will wait
//     whitePlayer = std::make_unique<MockPlayer>("White", std::vector<DenseMove> {DenseMove(W_PAWN, 8, 16)}, false, 2100);
//     blackPlayer = std::make_unique<MockPlayer>("Black", std::vector<DenseMove> {DenseMove(B_PAWN, 48, 40)}, false, 2100);
    
//     match = std::make_unique<ChessMatch>(
//         std::unique_ptr<IPlayer>(whitePlayer.release()),
//         std::unique_ptr<IPlayer>(blackPlayer.release()),
//         tc
//     );
//     match->start();
//     match->pause();
    
//     EXPECT_FALSE(match->getClock().isClockRunning());
    
//     match->resume();
//     EXPECT_TRUE(match->getClock().isClockRunning());
// }

// Test threefold repetition
TEST_F(ChessMatchTest, ThreefoldRepetition) {
    // Setup moves that repeat a position three times
    std::vector<DenseMove> whiteMoves = {
        DenseMove(W_KNIGHT, 6, 21),     // 1. Nf3
        DenseMove(W_KNIGHT, 21, 6),     // 2. Ng1
        DenseMove(W_KNIGHT, 6, 21),     // 3. Nf3
        DenseMove(W_KNIGHT, 21, 6),     // 4. Ng1
        DenseMove(W_KNIGHT, 6, 21)      // 5. Nf3
    };
    
    std::vector<DenseMove> blackMoves = {
        DenseMove(B_KNIGHT, 62, 45),    // 1... Nc6
        DenseMove(B_KNIGHT, 45, 62),    // 2... Ng8
        DenseMove(B_KNIGHT, 62, 45),    // 3... Nc6
        DenseMove(B_KNIGHT, 45, 62),    // 4... Ng8
    };
    
    createMatch(whiteMoves, blackMoves);
    match->start();
    
    EXPECT_FALSE(match->isInProgress());
    EXPECT_EQ(match->getResult(), MatchResult::Draw);
    EXPECT_EQ(match->getTerminationReason(), TerminationReason::ThreefoldRepetition);
}

// Test fifty move rule
TEST_F(ChessMatchTest, FiftyMoveRule) {
    // Set up a position close to fifty moves without captures or pawn moves
    GameState state;
    state.halfMoveClock = 99;  // One move away from fifty move rule
    
    
    std::vector<DenseMove> whiteMoves = {
        DenseMove(W_ROOK, 2, 3)  // DenseMove that triggers fifty move rule
    };
    std::vector<DenseMove> blackMoves = {
        DenseMove(B_KING, 40, 41)
    };
    
    createMatch(whiteMoves, blackMoves);
    match->setInitialPosition("8/8/k7/8/8/8/8/K1R5 w - - 99 75");
    match->start();
    
    EXPECT_FALSE(match->isInProgress());
    EXPECT_EQ(match->getResult(), MatchResult::Draw);
    EXPECT_EQ(match->getTerminationReason(), TerminationReason::FiftyMoveRule);
}

// Test insufficient material
TEST_F(ChessMatchTest, InsufficientMaterial) {
    
    std::vector<DenseMove> whiteMoves = {
        DenseMove(W_KING, 4, 12)  // Any legal move
    };
    std::vector<DenseMove> blackMoves = {};
    
    createMatch(whiteMoves, blackMoves);
    // Set up a king vs king position
    match->setInitialPosition("4k3/8/8/8/8/8/8/4K3 w - - 0 1");
    match->start();
    
    EXPECT_FALSE(match->isInProgress());
    EXPECT_EQ(match->getResult(), MatchResult::Draw);
    EXPECT_EQ(match->getTerminationReason(), TerminationReason::InsufficientMaterial);
}