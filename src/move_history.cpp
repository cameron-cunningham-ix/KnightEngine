#include "move_history.hpp"
#include "moves.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

// Constructor for standard starting position
MoveHistory::MoveHistory() 
    : startingFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"),
      isStandardStart(true) {
    // Set standard PGN tags
    setTag("Event", "?");
    setTag("Site", "?");
    setTag("Date", "????.??.??");
    setTag("Round", "?");
    setTag("White", "?");
    setTag("Black", "?");
    setTag("Result", "*");
}

// Constructor for custom starting position
MoveHistory::MoveHistory(const std::string& initialFEN)
    : startingFen(initialFEN),
      isStandardStart(initialFEN == "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {
    // Set standard PGN tags
    setTag("Event", "?");
    setTag("Site", "?");
    setTag("Date", "????.??.??");
    setTag("Round", "?");
    setTag("White", "?");
    setTag("Black", "?");
    setTag("Result", "*");
    if (!isStandardStart) {
        setTag("SetUp", "1");
        setTag("FEN", initialFEN);
    }
}

// Add a new move to the history
void MoveHistory::addMove(const DenseMove& move, ChessBoard& board,
                         std::chrono::milliseconds timeStamp) {
    // std::cout << "MoveHistory.addMove start\n";
    // std::cout << "Board side to move: " << board.getSideToMove() << "\n";
    // Generate SAN notation for the move
    std::string san = generateSAN(move, board);
    // std::cout << "MoveHistory.addMove generated SAN: " << san << "\n";
    // std::cout << "Board side to move: " << board.getSideToMove() << "\n";


    // Create and add FEN entry
    // std::cout << "MoveHistory.addMove getting FEN\n";
    std::string fen = getFEN(board);
    // std::cout << "MoveHistory.addMove got FEN: " << fen << "\n";

    fens.emplace_back(fen);
    
    // Create and add history entry
    moves.emplace_back(move, san, fen, timeStamp);
}

// Generate Standard Algebraic Notation for a move
std::string MoveHistory::generateSAN(const DenseMove& move, ChessBoard& board) const {
    std::cout << "generateSAN start, board side: " << board.getSideToMove() << "\n";
    std::cout << move.toString(false) << "\n";
    std::stringstream san;
    // Get move attributes
    PieceType movePiece = move.getPieceType();
    int moveFrom = move.getFrom();
    int moveTo = move.getTo();
    // Handle castling first
    if (move.isCastle()) {
        if (moveTo % 8 > moveFrom % 8) {
            return "O-O";    // Kingside castle
        } else {
            return "O-O-O";  // Queenside castle
        }
    }

    // Add movePiece letter (except for pawns)
    if (movePiece != W_PAWN && movePiece != B_PAWN) {
        auto it = pieceToSAN.find(movePiece);
        if (it != pieceToSAN.end()) {
            san << it->second[0];
        } else {
            throw std::runtime_error("Invalid movePiece type in generateSAN");
        }
    }

    // Check for move ambiguity
    std::vector<DenseMove> legalMoves;
    std::vector<DenseMove> ambiguousMoves;
    
    // Get all legal moves
    legalMoves = MoveGenerator::generateLegalMoves(board);

    // std::cout << "generateSAN genLegalMoves, board side: " << board.getSideToMove() << "\n";

    for (const DenseMove& candidateMove : legalMoves) {
        if (candidateMove.getPieceType() != movePiece
            || candidateMove.getTo() != move.getTo()) {
            continue;
        }
        if (candidateMove.getFrom() == move.getFrom()) {
            continue;
        }
        // Pawn captures are always unambiguous since we add the file later
        if (movePiece == W_PAWN || movePiece == B_PAWN) {
            continue;
        }
        ambiguousMoves.push_back(candidateMove);
    }
    
    // If we found ambiguous moves, determine what disambiguation is needed
    if (!ambiguousMoves.empty()) {
        bool sameFile = false;
        bool sameRank = false;
        
        // Check if any ambiguous moves share the same file or rank
        for (const DenseMove& ambiguousMove : ambiguousMoves) {
            if (ambiguousMove.getFrom() % 8 == move.getFrom() % 8) {
                sameFile = true;
            }
            if (ambiguousMove.getFrom() / 8 == move.getFrom() / 8) {
                sameRank = true;
            }
        }
        
        // Add disambiguation according to standard chess notation rules:
        // 1. Use file if unique
        // 2. Use rank if file is not unique
        // 3. Use both if neither is unique
        if (!sameFile) {
            // File is sufficient for disambiguation
            san << (char)('a' + (move.getFrom() % 8));
        } else if (!sameRank) {
            // Need rank for disambiguation
            san << (char)('1' + (move.getFrom() / 8));
        } else {
            // Need both file and rank
            san << (char)('a' + (move.getFrom() % 8))
                << (char)('1' + (move.getFrom() / 8));
        }
    }

    // Add capture symbol if it's a capture
    if (move.getCaptDense() != D_EMPTY) {
        // Always show file of origin for pawn captures
        if (movePiece == W_PAWN || movePiece == B_PAWN) {
            san << (char)('a' + (moveFrom % 8));
        }
        san << "x";
    }

    // Add destination square
    san << (char)('a' + (moveTo % 8)) << (char)('1' + (moveTo / 8));

    // Add promotion piece
    if (move.getPromoteDense() != D_EMPTY) {
        san << "=" << pieceToSAN.at(move.getPromotePiece());
    }

    // Check for check and checkmate
    // std::cout << "generateSAN before makeMove, board side: " << board.getSideToMove() << "\n";
    board.makeMove(move, true);
    // std::cout << "generateSAN after makeMove, board side: " << board.getSideToMove() << "\n";

    if (board.isInCheck()) {
        if (isCheckmate(board)) {
            san << "#";
        } else {
            san << "+";
        }
    }
    // std::cout << "generateSAN before unmakeMove, board side: " << board.getSideToMove() << "\n";
    // Unmake move
    board.unmakeMove(move, true);
    // std::cout << "generateSAN after unmakeMove, board side: " << board.getSideToMove() << "\n";
    return san.str();
}

// Add a comment to a specific move
void MoveHistory::addComment(const std::string& comment, size_t moveIndex) {
    if (moveIndex < moves.size()) {
        moves[moveIndex].comment = comment;
    }
}

// Add a Numeric Annotation Glyph to a move
void MoveHistory::addNAG(const std::string& nag, size_t moveIndex) {
    if (moveIndex < moves.size()) {
        moves[moveIndex].nags.push_back(nag);
    }
}

// Set a PGN tag
void MoveHistory::setTag(const std::string& name, const std::string& value) {
    auto it = std::find_if(tags.begin(), tags.end(),
                          [&name](const auto& pair) { return pair.first == name; });
    if (it != tags.end()) {
        it->second = value;
    } else {
        tags.emplace_back(name, value);
    }
}

// Get a PGN tag value
std::string MoveHistory::getTag(const std::string& name) const {
    auto it = std::find_if(tags.begin(), tags.end(),
                          [&name](const auto& pair) { return pair.first == name; });
    return it != tags.end() ? it->second : "";
}

// Check if a tag exists
bool MoveHistory::hasTag(const std::string& name) const {
    return std::find_if(tags.begin(), tags.end(),
                       [&name](const auto& pair) { return pair.first == name; }) != tags.end();
}

// Convert move history to PGN format
std::string MoveHistory::toPGN() const {
    std::stringstream pgn;

    // Write tags
    for (const auto& [name, value] : tags) {
        pgn << "[" << name << " \"" << value << "\"]\n";
    }
    pgn << "\n";

    // Write moves
    for (int i = 0; i < (int)moves.size(); ++i) {
        if (i % 2 == 0) {
            pgn << (i/2 + 1) << ". ";
        }
        
        pgn << moveToPGN(moves[i], i/2 + 1, i % 2 == 0);
        
        if (!moves[i].comment.empty()) {
            pgn << " {" << moves[i].comment << "}";
        }
        
        for (const auto& nag : moves[i].nags) {
            pgn << " $" << nag;
        }
        
        pgn << " ";
    }

    // Add result
    pgn << getTag("Result");
    
    return pgn.str();
}

// Convert a single move to PGN format
std::string MoveHistory::moveToPGN(const HistoryEntry& entry, int moveNumber, bool isWhite) const {
    std::stringstream pgn;
    pgn << entry.san;
    return pgn.str();
}

// Parse a PGN (Portable Game Notation) string into the move history
bool MoveHistory::fromPGN(const std::string& pgn) {
    try {
        std::istringstream stream(pgn);
        std::string line;
        
        // Clear existing history
        clear();
        
        // First pass: Parse PGN tags section
        // Tags look like: [TagName "TagValue"]
        while (std::getline(stream, line)) {
            // Skip empty lines and whitespace
            if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos) {
                continue;
            }
            
            // Check if we've reached the end of tags section
            if (line[0] != '[') {
                break;
            }
            
            // Parse tag
            size_t firstSpace = line.find(' ');
            size_t firstQuote = line.find('"');
            size_t lastQuote = line.find_last_of('"');
            
            // Validate tag format
            if (firstSpace == std::string::npos || firstQuote == std::string::npos || 
                lastQuote == std::string::npos || lastQuote == firstQuote) {
                continue;  // Skip malformed tag
            }
            
            // Extract tag name and value
            std::string tagName = line.substr(1, firstSpace - 1);
            std::string tagValue = line.substr(firstQuote + 1, lastQuote - firstQuote - 1);
            setTag(tagName, tagValue);

            // Special handling for FEN tag
            if (tagName == "FEN") {
                startingFen = tagValue;
                isStandardStart = false;
            }
        }
        
        // Initialize board with starting position
        ChessBoard board = ChessBoard();
        if (!isStandardStart) {
            if (isValidFEN(startingFen)) {
                board.setupPositionFromFEN(startingFen);
            }
            else 
                return false;
        }
        
        // Second pass: Parse moves section
        // Collect all remaining text for move parsing
        std::string moveText = line;  // Start with the first non-tag line
        std::string nextLine;
        while (std::getline(stream, nextLine)) {
            if (!nextLine.empty()) {
                moveText += " " + nextLine;
            }
        }
        
        // State tracking for parsing
        bool inComment = false;
        bool inVariation = false;
        int bracketDepth = 0;
        int parenthesisDepth = 0;
        std::string currentComment;
        std::string currentMove;
        
        // Process each character
        for (size_t i = 0; i < moveText.length(); ++i) {
            char c = moveText[i];
            
            // Handle comments
            if (c == '{') {
                inComment = true;
                bracketDepth++;
                currentComment.clear();
                continue;
            }
            if (c == '}') {
                bracketDepth--;
                if (bracketDepth == 0) {
                    inComment = false;
                    if (!moves.empty()) {
                        moves.back().comment = currentComment;
                    }
                }
                continue;
            }
            if (inComment) {
                currentComment += c;
                continue;
            }
            
            // Handle variations
            if (c == '(') {
                inVariation = true;
                parenthesisDepth++;
                continue;
            }
            if (c == ')') {
                parenthesisDepth--;
                if (parenthesisDepth == 0) {
                    inVariation = false;
                }
                continue;
            }
            if (inVariation) {
                continue;
            }
            
            // Handle actual moves and other tokens
            if (std::isspace(c)) {
                if (!currentMove.empty()) {
                    try {
                        // Handle game termination markers
                        if (currentMove == "1-0" || currentMove == "0-1" || 
                            currentMove == "1/2-1/2" || currentMove == "*") {
                            setTag("Result", currentMove);
                        }
                        // Handle NAGs (Numeric Annotation Glyphs)
                        else if (currentMove[0] == '$') {
                            if (!moves.empty()) {
                                moves.back().nags.push_back(currentMove.substr(1));
                            }
                        }
                        // Skip move numbers
                        else if (currentMove.find('.') == std::string::npos) {
                            // Process as actual move
                            DenseMove move = sanToMove(currentMove, board);
                            std::cout << "fromPGN move from sanToMove - move: " << move.toString(false) << "\n";
                            
                            // Add move to history
                            addMove(move, board);

                            // Update board position
                            board.makeMove(move, true);
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "Error processing move: " << currentMove 
                                << " - " << e.what() << std::endl;
                        return false;
                    }
                    currentMove.clear();
                }
            } else {
                currentMove += c;
            }
        }
        
        // Handle last move if any
        if (!currentMove.empty()) {
            try {
                // Handle game termination markers
                if (currentMove == "1-0" || currentMove == "0-1" || 
                    currentMove == "1/2-1/2" || currentMove == "*") {
                    setTag("Result", currentMove);
                }
                else {
                    DenseMove move = sanToMove(currentMove, board);

                    // Add move to history
                    addMove(move, board);

                    // Update board position
                    board.makeMove(move, true);
                }
            } catch (const std::exception& e) {
                return false;
            }
        }
        
        // Verify no unclosed comments or variations
        if (bracketDepth != 0 || parenthesisDepth != 0) {
            return false;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        std::cout << "PGN parsing error: " << e.what() << std::endl;
        return false;
    }
}

// // Helper method to process a single move token
// void MoveHistory::processMoveToken(const std::string& token, ChessBoard& board) {
//     // Skip move numbers
//     if (token.find('.') != std::string::npos) return;
    
//     // Handle game termination markers
//     if (token == "1-0" || token == "0-1" || token == "1/2-1/2" || token == "*") {
//         setTag("Result", token);
//         return;
//     }
    
//     // Handle NAGs (Numeric Annotation Glyphs)
//     if (token[0] == '$') {
//         if (!moves.empty()) {
//             moves.back().nags.push_back(token.substr(1));
//         }
//         return;
//     }
    
//     try {
//         // Convert SAN to move
//         DenseMove move = sanToMove(token, board);
        
//         // Add move to history
//         addMove(move, board);
        
//         // Update board position
//         board.makeMove(move, true);
//     } catch (const std::exception& e) {
//         // Log error for debugging
//         std::cerr << "Error processing move: " << token << " - " << e.what() << std::endl;
//         throw;  // Re-throw to handle in fromPGN
//     }
// }

std::string MoveHistory::getFEN(const ChessBoard& board) {
    // std::cout << "MoveHistory.getFEN start\n";
    std::string fen;
    
    int square = 56;        // Start at a8
    int emptyCount = 0;
    // Loop through all squares for position portion of FEN
    for (square; square >= 0; square++) {
        PieceType current = board.getPieceAt(square);
        // std::cout << "  square " << square << " piece " << current << "\n";
        // Empty square
        if (current == PieceType::EMPTY) {
            emptyCount++;
            // Entire rank empty or end of the rank
            if (emptyCount >= 8 || square % 8 == 7) {
                fen.append(std::to_string(emptyCount));
                if (square != 7) { fen.append("/"); }
                square -= 16;
                emptyCount = 0;
                continue;
            }
        }
        // Piece on current square
        else {
            // std::cout << "  try get pieceToFEN\n";
            std::string fenPiece = pieceToFEN.at(current);
            // std::cout << "  " << fenPiece << "\n";
            // Append any previous empty squares
            if (emptyCount != 0) {
                fen.append(std::to_string(emptyCount));
                emptyCount = 0;
            }
            fen.append(fenPiece);

            // Last square in the row; ignore last square in row a
            if (square % 8 == 7) {
                if (square != 7) { fen.append("/"); }
                square -= 16;
            }
        }
    }

    // FEN player turn
    fen.append(" ");
    fen.append(board.currentGameState.sideToMove == 0 ? "w" : "b");
    // FEN castling rights
    fen.append(" ");
    if (board.currentGameState.canCastleWhiteKingside) fen.append("K");
    if (board.currentGameState.canCastleWhiteQueenside) fen.append("Q");
    if (board.currentGameState.canCastleBlackKingside) fen.append("k");
    if (board.currentGameState.canCastleBlackQueenside) fen.append("q");
    if (!board.currentGameState.canCastleWhiteKingside && 
        !board.currentGameState.canCastleWhiteQueenside &&
        !board.currentGameState.canCastleBlackKingside && 
        !board.currentGameState.canCastleBlackQueenside) fen.append("-");
    // FEN En Passant
    fen.append(" ");
    if (board.currentGameState.enPassantSquare == -1) fen.append("-");
    else fen.append(indexToAlgebraic(board.currentGameState.enPassantSquare));
    // FEN Half Move clock
    fen.append(" ");
    fen.append(std::to_string(board.currentGameState.halfMoveClock));
    // FEN Full Move Number
    fen.append(" ");
    fen.append(std::to_string(board.currentGameState.fullMoveNumber));

    return fen;
}

// Get a specific move from history
const HistoryEntry& MoveHistory::getMove(size_t index) const {
    if (index >= moves.size()) {
        throw std::out_of_range("Move index out of range");
    }
    return moves[index];
}

// Get the last move made
const HistoryEntry& MoveHistory::getLastMove() const {
    if (moves.empty()) {
        throw std::runtime_error("No moves in history");
    }
    return moves.back();
}

// Clear all moves and reset to initial state
void MoveHistory::clear() {
    moves.clear();
    setTag("Result", "*");
}