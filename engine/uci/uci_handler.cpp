#include "uci_handler.hpp"
#include <iostream>
#include <sstream>
#include <string>

std::vector<DenseMove> parseUCIMoves(const std::string& fen, std::istringstream& iss) {
    std::vector<DenseMove> moves;
    ChessBoard board;
    
    if (fen.empty()) {
        board.setupPositionFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    } else {
        board.setupPositionFromFEN(fen);
    }
    
    std::string move_str;
    while (iss >> move_str) {
        // Convert UCI move string (e.g., "e2e4") to DenseMove
        if (move_str.length() >= 4) {
            int from = algebraicToIndex(move_str.substr(0, 2));
            int to = algebraicToIndex(move_str.substr(2, 2));
            if (from != -1 && to != -1) {
                PieceType piece = board.getPieceAt(from);
                DenseType capturedPiece = board.getDenseTypeAt(to);

                DenseMove move(piece, from, to, capturedPiece);

                // Handle promotion
                if (move_str.length() > 4) {
                    char promo = move_str[4];
                    DenseType promoteTo;
                    switch (promo) {
                        case 'q': promoteTo = D_QUEEN; break;
                        case 'r': promoteTo = D_ROOK; break;
                        case 'b': promoteTo = D_BISHOP; break;
                        case 'n': promoteTo = D_KNIGHT; break;
                        default: continue;  // Invalid promotion
                    }
                    move.setPromoteTo(promoteTo);
                }

                moves.push_back(move);
                board.makeMove(move, true);
            }
        }
    }
    
    return moves;
}

void uciLoop(std::unique_ptr<EnginePlayer>& player) {
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "quit") {
            break;
        }
        else if (cmd == "uci") {
            player->uci();
        }
        else if (cmd == "isready") {
            player->isReady();
        }
        else if (cmd == "ucinewgame") {
            player->uciNewGame();
        }
        else if (cmd == "position") {
            std::string pos_type;
            iss >> pos_type;

            std::string fen;
            std::string moves;

            if (pos_type == "startpos") {
                // Empty fen indicates startpos
                std::string token;
                if (iss >> token && token == "moves") {
                    // Get rest of line as moves
                    std::getline(iss, moves);
                    // Remove leading whitespace if any
                    moves = moves.substr(moves.find_first_not_of(" \t"));
                }
            }
            else if (pos_type == "fen") {
                // Read the FEN string
                std::string part;
                while (iss >> part && part != "moves") {
                    fen += part + " ";
                }
                
                if (part == "moves") {
                    // Get rest of line as moves
                    std::getline(iss, moves);
                    // Remove leading whitespace if any
                    moves = moves.substr(moves.find_first_not_of(" \t"));
                }
            }

            player->position(fen, moves);
        }
        else if (cmd == "go") {
            // Parse go parameters
            std::map<std::string, std::string> params;
            std::string param, value;
            while (iss >> param) {
                if (iss >> value) {
                    params[param] = value;
                }
            }
            player->go(params);
        }
        else if (cmd == "stop") {
            player->stop();
        }
        else if (cmd == "setoption") {
            std::string name, value;
            std::string token;
            iss >> token; // Skip "name"
            iss >> name;
            iss >> token; // Skip "value"
            iss >> value;
            player->setOption(name, value);
        }
    }
}