#include "chess_match.hpp"
#include "pext_bitboard.hpp"
#include <chrono>
#include <ctime>

ChessMatch::ChessMatch(std::unique_ptr<IPlayer> white,
                      std::unique_ptr<IPlayer> black,
                      const TimeControl& tc)
    : whitePlayer(std::move(white)),
      blackPlayer(std::move(black)),
      clock(tc),
      board(std::make_unique<ChessBoard>()),
      result(MatchResult::InProgress),
      terminationReason(TerminationReason::None),
      isMatchOver(false),
      isPaused(false) {
    
    // std::cout << "ChessMatch constructor - Board initialized at: " << board.get() << std::endl;
    // std::cout << "Board initialization check: " << std::endl;
    // Verify board was created properly
    if (!board) {
        throw std::runtime_error("Board failed to initialize");
    }
    // Verify board can be accessed
    try {
        auto whitePawns = board->getWhitePawns();
        // std::cout << "Initial white pawns bitboard: " << std::hex << whitePawns << std::dec << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error accessing board: " << e.what() << std::endl;
        throw;
    }
    // Set up history with player names
    history.setTag("White", whitePlayer->getName());
    history.setTag("Black", blackPlayer->getName());
    // std::cout << "ChessMatch constructor complete" << std::endl;
}

void ChessMatch::start() {
    // Initialize PEXT
    PEXT::initialize();
    clock.start();
    
    while (!isMatchOver && !isPaused) {
        // std::cout << "\nTurn start - Board pointer address: " << board.get() << std::endl;
        ChessBoard& boardRef = *board;  // Get a reference to the board
        // std::cout << "Start / before turn:\n";
        // boardRef.printBoardInfo(true);
        // boardRef.printStateHistory();

        IPlayer* currentPlayer = (boardRef.getSideToMove() == WHITE) ?
                                whitePlayer.get() : blackPlayer.get();
        Color prevSide = boardRef.getSideToMove();
        
        try {
            // Pass the reference to the board, not a copy
            DenseMove move = currentPlayer->getMove(boardRef, clock);
            // std::cout << "Before makeMove - Board pointer address: " << board.get() << std::endl;
            // std::cout << "Move: " << move.toString(false) << std::endl;
            // std::cout << "Board before history.addMove:\n";
            // printBoard(boardRef);
            // Add move to history first
            history.addMove(move, boardRef, clock.getWhiteTime());

            // std::cout << "Board after history.addMove (should be same as before):\n";
            // printBoard(boardRef);
            
            // Make the move on our reference
            boardRef.makeMove(move, false);
            
            // std::cout << "After makeMove - Board pointer address: " << board.get() << std::endl;
            // std::cout << "Board after makeMove:\n";
            // printBoard(boardRef);
            

            clock.makeMove();
            
            // Check if side switched using our reference
            if (boardRef.getSideToMove() == prevSide) {
                std::cout << "ERROR: Side to move did not switch properly" << std::endl;
            }
            
            IPlayer* opponent = (boardRef.getSideToMove() == WHITE) ? 
                               whitePlayer.get() : blackPlayer.get();
            opponent->notifyOpponentMove(move);
            
            if (checkForGameEnd()) break;
        } catch (const std::exception& e) {
            std::cout << "Error during move: " << e.what() << std::endl;
            isMatchOver = true;
            result = (boardRef.getSideToMove() == WHITE) ? 
                     MatchResult::BlackWin : MatchResult::WhiteWin;
            terminationReason = (boardRef.getSideToMove() == WHITE) ? 
                               TerminationReason::WhiteResigned :
                               TerminationReason::BlackResigned;
        }
    }
    clock.stop();
}

bool ChessMatch::checkForGameEnd() {
    // std::cout << "ChessMatch checkForGameEnd start\n";
    // Check for checkmate
    if (isCheckmate(*board)) {
        // std::cout << "    isCheckmate true\n";
        isMatchOver = true;
        result = (board->currentGameState.sideToMove == WHITE) ? 
                 MatchResult::BlackWin : MatchResult::WhiteWin;
        terminationReason = TerminationReason::Checkmate;
        return true;
    }
    // std::cout << "    isCheckmate false\n";
    // printBoard(*board);
    
    // Check for stalemate
    if (isStalemate(*board)) {
        // std::cout << "    isStalemate true\n";
        isMatchOver = true;
        result = MatchResult::Draw;
        terminationReason = TerminationReason::Stalemate;
        return true;
    }
    // std::cout << "    isStalemate false\n";
    // printBoard(*board);

    
    // Check for threefold repetition
    if (isThreefoldRepetition()) {
        // std::cout << "    isThreefoldRep true\n";
        isMatchOver = true;
        result = MatchResult::Draw;
        terminationReason = TerminationReason::ThreefoldRepetition;
        return true;
    }
    // std::cout << "    isThreefoldRep false\n";

    
    // Check 50-move rule
    if (board->currentGameState.halfMoveClock >= 100) {
        isMatchOver = true;
        result = MatchResult::Draw;
        terminationReason = TerminationReason::FiftyMoveRule;
        return true;
    }
    
    // Check for insufficient material
    if (hasInsufficientMaterial()) {
        isMatchOver = true;
        result = MatchResult::Draw;
        terminationReason = TerminationReason::InsufficientMaterial;
        return true;
    }
    
    // Check for time forfeit
    if (clock.isWhiteTimeUp()) {
        isMatchOver = true;
        result = MatchResult::BlackWin;
        terminationReason = TerminationReason::WhiteTimeout;
        return true;
    }
    if (clock.isBlackTimeUp()) {
        isMatchOver = true;
        result = MatchResult::WhiteWin;
        terminationReason = TerminationReason::BlackTimeout;
        return true;
    }
    
    return false;
}

void ChessMatch::pause() {
    // Pause the ongoing match and stop the clock
    if (!isMatchOver && !isPaused) {
        clock.pause();
        isPaused = true;
    }
}

void ChessMatch::resume() {
    // Resume a paused match and restart the clock
    if (!isMatchOver && isPaused) {
        clock.resume();
        isPaused = false;
        start();
    }
}

void ChessMatch::abort() {
    // Immediately end the match without a winner
    if (!isMatchOver) {
        clock.stop();
        isMatchOver = true;
        result = MatchResult::Aborted;
        terminationReason = TerminationReason::Aborted;
        
        // Notify players of aborted game
        whitePlayer->onGameEnd();
        blackPlayer->onGameEnd();
    }
}

void ChessMatch::setTimeControl(const TimeControl& tc) {
    // Update time control settings - only allowed before match starts
    if (!clock.isClockRunning()) {
        clock = ChessClock(tc);
    }
}

void ChessMatch::setInitialPosition(const std::string& fen) {
    // Set up a custom starting position - only allowed before match starts
    if (!clock.isClockRunning()) {
        board = std::make_unique<ChessBoard>(); // Create new board
        board->setupPositionFromFEN(fen);
        history = MoveHistory(fen);  // Reset history with new starting position
        
        // Update player names in history
        history.setTag("White", whitePlayer->getName());
        history.setTag("Black", blackPlayer->getName());
    }
}

bool ChessMatch::isThreefoldRepetition() const {
    // Get all positions from match history
    const auto& moves = history.getMoves();
    if (moves.empty()) return false;

    // Count occurrences of the current position
    std::string currentFEN = moves.back().fen;
    
    // Strip move counters and en passant from FEN since they don't affect position
    auto stripExtraInfo = [](const std::string& fen) -> std::string {
        std::stringstream ss(fen);
        std::string position, turn, castling;
        ss >> position >> turn >> castling;
        return position + " " + turn + " " + castling;
    };
    
    std::string strippedCurrentFEN = stripExtraInfo(currentFEN);
    int repetitionCount = 0;

    // Check all previous positions
    for (const auto& entry : moves) {
        if (stripExtraInfo(entry.fen) == strippedCurrentFEN) {
            repetitionCount++;
            if (repetitionCount >= 3) {
                return true;
            }
        }
    }
    
    return false;
}

std::string ChessMatch::getPGN() const {
    // First update the game result tag based on match outcome
    std::string resultStr;
    switch (result) {
        case MatchResult::WhiteWin: resultStr = "1-0"; break;
        case MatchResult::BlackWin: resultStr = "0-1"; break;
        case MatchResult::Draw: resultStr = "1/2-1/2"; break;
        default: resultStr = "*"; break;  // Game still in progress or aborted
    }
    
    // Create a copy of history to modify
    MoveHistory tempHistory = history;
    
    // Update required PGN tags
    tempHistory.setTag("Event", "Chess Match");
    tempHistory.setTag("Site", "Local");
    tempHistory.setTag("Date", getCurrentDate());  // Need to implement this helper
    tempHistory.setTag("Round", "1");
    tempHistory.setTag("Result", resultStr);
    
    // Add custom tags for match information
    tempHistory.setTag("TimeControl", clock.getTimeControl().toString());  // Need to implement this
    
    // Add termination reason as a comment on the last move if game is over
    if (isMatchOver && !history.getMoves().empty()) {
        std::string reason;
        switch (terminationReason) {
            case TerminationReason::Checkmate: reason = "Checkmate"; break;
            case TerminationReason::Stalemate: reason = "Stalemate"; break;
            case TerminationReason::ThreefoldRepetition: reason = "Threefold repetition"; break;
            case TerminationReason::FiftyMoveRule: reason = "Fifty-move rule"; break;
            case TerminationReason::InsufficientMaterial: reason = "Insufficient material"; break;
            case TerminationReason::MutualAgreement: reason = "Draw by mutual agreement"; break;
            case TerminationReason::WhiteResigned: reason = "White resigned"; break;
            case TerminationReason::BlackResigned: reason = "Black resigned"; break;
            case TerminationReason::WhiteTimeout: reason = "White lost on time"; break;
            case TerminationReason::BlackTimeout: reason = "Black lost on time"; break;
            case TerminationReason::Aborted: reason = "Game aborted"; break;
            default: reason = "Unknown reason"; break;
        }
        tempHistory.addComment(reason, tempHistory.length() - 1);
    }
    
    return tempHistory.toPGN();
}

// Helper function to get current date in PGN format (YYYY.MM.DD)
std::string ChessMatch::getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y.%m.%d");
    return ss.str();
}

bool ChessMatch::hasInsufficientMaterial() const {
    // Check for pieces that always guarantee sufficient material
    std::string lastMoveFEN = history.getLastMove().fen;
    if (lastMoveFEN.find('P') != std::string::npos ||   // Any pawn is sufficient
        lastMoveFEN.find('p') != std::string::npos ||
        lastMoveFEN.find('Q') != std::string::npos ||   // Any queen is sufficient
        lastMoveFEN.find('q') != std::string::npos ||   
        lastMoveFEN.find('R') != std::string::npos ||   // Any rook is sufficient
        lastMoveFEN.find('r') != std::string::npos) {   
        return false;
    }

    // Count pieces for each side
    int whiteBishops = (int)std::count(lastMoveFEN.begin(), lastMoveFEN.end(), 'B');
    int blackBishops = (int)std::count(lastMoveFEN.begin(), lastMoveFEN.end(), 'b');
    int whiteKnights = (int)std::count(lastMoveFEN.begin(), lastMoveFEN.end(), 'N');
    int blackKnights = (int)std::count(lastMoveFEN.begin(), lastMoveFEN.end(), 'n');

    // If either side has both a knight and a bishop, there's sufficient material
    if ((whiteBishops > 0 && whiteKnights > 0) || 
        (blackBishops > 0 && blackKnights > 0)) {
        return false;
    }

    // If either side has two or more knights, there's sufficient material
    if (whiteKnights > 1 || blackKnights > 1) {
        return false;
    }

    // If there are bishops, check if they're on the same colored squares
    if (whiteBishops > 1 || blackBishops > 1) {
        // Get positions of all bishops
        std::vector<int> whiteBishopSquares;
        std::vector<int> blackBishopSquares;
        
        // Scan the board for bishops
        for (int i = 0; i < 64; i++) {
            if (board->getPieceAt(i) == W_BISHOP) {
                whiteBishopSquares.push_back(i);
            } else if (board->getPieceAt(i) == B_BISHOP) {
                blackBishopSquares.push_back(i);
            }
        }

        // Check if bishops are on same colored squares
        auto squareColor = [](int square) {
            return ((square / 8) + (square % 8)) % 2;  // 0 for light, 1 for dark
        };

        // If any side has bishops on different colored squares, material is sufficient
        if (whiteBishopSquares.size() > 1) {
            bool sameDiagonal = true;
            int firstColor = squareColor(whiteBishopSquares[0]);
            for (size_t i = 1; i < whiteBishopSquares.size(); i++) {
                if (squareColor(whiteBishopSquares[i]) != firstColor) {
                    return false;  // Different colored bishops = sufficient material
                }
            }
        }

        if (blackBishopSquares.size() > 1) {
            bool sameDiagonal = true;
            int firstColor = squareColor(blackBishopSquares[0]);
            for (size_t i = 1; i < blackBishopSquares.size(); i++) {
                if (squareColor(blackBishopSquares[i]) != firstColor) {
                    return false;  // Different colored bishops = sufficient material
                }
            }
        }
        
        // If both sides have a bishop, check if they're on same colored squares
        if (whiteBishopSquares.size() == 1 && blackBishopSquares.size() == 1) {
            if (squareColor(whiteBishopSquares[0]) != 
                squareColor(blackBishopSquares[0])) {
                return false;  // Opposite colored bishops = sufficient material
            }
        }
    }

    // If we get here, all remaining material combinations are insufficient
    return true;
}