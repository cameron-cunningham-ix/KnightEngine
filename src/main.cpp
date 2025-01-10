#include "chess_match.hpp"
#include "human_player.hpp"
#include "engine_player.hpp"
#include "random_engine.hpp"
#include "material_engine.hpp"
#include <iostream>
#include <string>
#include <thread>



// Helper function to print current game status
void printGameStatus(ChessMatch& match) {
    const ChessClock& clock = match.getClock();
    
    // Print time remaining
    auto whiteTime = std::chrono::duration_cast<std::chrono::seconds>(clock.getWhiteTime());
    auto blackTime = std::chrono::duration_cast<std::chrono::seconds>(clock.getBlackTime());
    
    printBoard(match.getBoard());

    std::cout << "Time remaining:\n";
    std::cout << "White: " << whiteTime.count() << "s\n";
    std::cout << "Black: " << blackTime.count() << "s\n\n";
    
    // Print whose turn it is
    std::cout << (match.getState().sideToMove == WHITE ? "White" : "Black") 
              << " to move\n\n";
    
    // If in check, print warning
    if (match.getBoard().getCheckCount()) {
        std::cout << "CHECK!\n\n";
    }
}

int main() {
    // Create a 5-minute game with 3-second increment
    TimeControl tc(std::chrono::minutes(5),    // Initial time
                  std::chrono::seconds(3),     // Increment
                  std::chrono::seconds(0));    // No delay
    
    // Ask user for player type
    std::cout << "Select player type for White:\n";
    std::cout << "1. Human Player\n";
    std::cout << "2. Random Engine\n";
    std::cout << "3. Simple Material Engine\n";
    std::cout << "Choice: ";
    int whiteChoice;
    std::cin >> whiteChoice;
    std::cin.ignore();  // Clear newline

    std::cout << "\nSelect player type for Black:\n";
    std::cout << "1. Human Player\n";
    std::cout << "2. Random Engine\n";
    std::cout << "3. Simple Material Engine\n";
    std::cout << "Choice: ";
    int blackChoice;
    std::cin >> blackChoice;
    std::cin.ignore();  // Clear newline

    // Create white player
    std::unique_ptr<IPlayer> whitePlayer;
    if (whiteChoice == 1) {
        std::cout << "Enter name for White player: ";
        std::string whiteName;
        std::getline(std::cin, whiteName);
        whitePlayer = std::make_unique<HumanPlayer>(whiteName);
    } else if (whiteChoice == 2) {
        auto randomEngine = std::make_unique<RandomEngine>();
        whitePlayer = std::make_unique<EnginePlayer>(std::move(randomEngine));
    } else {
        auto materialEngine = std::make_unique<MaterialEngine>();
        whitePlayer = std::make_unique<EnginePlayer>(std::move(materialEngine));
    }

    // Create black player
    std::unique_ptr<IPlayer> blackPlayer;
    if (blackChoice == 1) {
        std::cout << "Enter name for Black player: ";
        std::string blackName;
        std::getline(std::cin, blackName);
        blackPlayer = std::make_unique<HumanPlayer>(blackName);
    } else if (blackChoice == 2) {
        auto randomEngine = std::make_unique<RandomEngine>();
        blackPlayer = std::make_unique<EnginePlayer>(std::move(randomEngine));
    } else {
        auto materialEngine = std::make_unique<MaterialEngine>();
        blackPlayer = std::make_unique<EnginePlayer>(std::move(materialEngine));
    }
    
    // Create and start the match
    ChessMatch match(std::move(whitePlayer), std::move(blackPlayer), tc);
    
    std::cout << "Welcome to Chess!\n";
    std::cout << "Enter moves in the format 'e2e4' (from-square to-square)\n";
    std::cout << "For pawn promotion, add the piece letter: 'e7e8q' for queen\n";
    std::cout << "Commands: 'resign' to resign, 'draw' to offer/accept draw\n\n";
    
    // Print initial position
    printBoard(match.getBoard());
    printGameStatus(match);

    // Start match in a separate thread
    std::thread matchThread([&match]() {
        match.start();
    });

    // Track when we should update display
    GameState lastState = match.getState();
    ChessBoard lastBoard = match.getBoard();

    // Main update loop
    while (match.isInProgress()) {
        const GameState& currentState = match.getState();
        const ChessBoard& currentBoard = match.getBoard();
        
        // Only update display if something has changed
        if (currentState.sideToMove != lastState.sideToMove ||
            currentBoard.getAllPieces() != lastBoard.getAllPieces()) {
            printGameStatus(match);
            
            // Update our tracked state
            lastState = currentState;
            lastBoard = currentBoard;
        }
        
        // Small sleep to prevent excessive CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Wait for match thread to complete
    if (matchThread.joinable()) {
        matchThread.join();
    }

    // Print final position and results
    printBoard(match.getBoard());
    
    // Print game result
    std::cout << "Game Over!\n";
    switch (match.getResult()) {
        case MatchResult::WhiteWin:
            std::cout << "White wins";
            break;
        case MatchResult::BlackWin:
            std::cout << "Black wins";
            break;
        case MatchResult::Draw:
            std::cout << "Game drawn";
            break;
        case MatchResult::Aborted:
            std::cout << "Game aborted";
            break;
        default:
            std::cout << "Unknown result";
            break;
    }
    
    // Print termination reason
    std::cout << " by ";
    switch (match.getTerminationReason()) {
        case TerminationReason::Checkmate:
            std::cout << "checkmate";
            break;
        case TerminationReason::Stalemate:
            std::cout << "stalemate";
            break;
        case TerminationReason::ThreefoldRepetition:
            std::cout << "threefold repetition";
            break;
        case TerminationReason::FiftyMoveRule:
            std::cout << "fifty-move rule";
            break;
        case TerminationReason::InsufficientMaterial:
            std::cout << "insufficient material";
            break;
        case TerminationReason::MutualAgreement:
            std::cout << "mutual agreement";
            break;
        case TerminationReason::WhiteResigned:
        case TerminationReason::BlackResigned:
            std::cout << "resignation";
            break;
        case TerminationReason::WhiteTimeout:
        case TerminationReason::BlackTimeout:
            std::cout << "timeout";
            break;
        case TerminationReason::Aborted:
            std::cout << "abortion";
            break;
        default:
            std::cout << "unknown reason";
            break;
    }
    std::cout << "\n\n";
    
    // Print PGN of the game
    std::cout << "Game PGN:\n";
    std::cout << match.getPGN() << "\n";
    
    return 0;
}