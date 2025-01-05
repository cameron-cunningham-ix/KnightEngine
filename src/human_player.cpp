#include "human_player.hpp"
#include "moves.hpp"
#include <sstream>

DenseMove HumanPlayer::getMove(ChessBoard& board,
                               const ChessClock& clock) {
    // Print current game state
    std::cout << "\nIt's " << name << "'s turn.\n";
    std::cout << "Time remaining: " 
              << (board.currentGameState.sideToMove == WHITE ? 
                  clock.getWhiteTime().count() : 
                  clock.getBlackTime().count()) 
              << "ms\n";

    while (true) {
        std::string input;
        std::cout << "Enter move (e.g. 'e4' or 'Nf3') or command ('resign', 'draw'): ";
        std::getline(std::cin, input);

        // Handle special commands
        if (input == "resign") {
            resign();
            throw std::runtime_error("Player resigned");
        }
        if (input == "draw") {
            acceptedDraw = true;
            continue;
        }

        // sanToMove checks if the move is legal
        DenseMove chosenMove = sanToMove(input, board);

        if (chosenMove == DenseMove()) {
            std::cout << "Invalid move. Please use Standard Algebraic Notation (e.g. 'e4' or 'Nf3')\n";
            continue;
        } else {
            return chosenMove;
        }
    }
}

void HumanPlayer::notifyOpponentMove(const DenseMove& move) {
    // Human players don't need to process opponent moves
    // But we could print them here if desired
    std::cout << "Opponent played: " << move.getPieceType()
              << " from " << move.getFrom()
              << " to " << move.getTo() << std::endl;
}

void HumanPlayer::onGameEnd() {
    // Reset state
    isResigned = false;
    acceptedDraw = false;
}

void HumanPlayer::offerDraw() {
    std::cout << "Your opponent offers a draw. Accept? (y/n): ";
    std::string response;
    std::getline(std::cin, response);
    acceptedDraw = (response == "y" || response == "Y");
}

void HumanPlayer::resign() {
    isResigned = true;
}