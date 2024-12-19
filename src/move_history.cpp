// src/move_history.cpp
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
void MoveHistory::addMove(const Move& move, const ChessBoard& board, const GameState& state,
                         std::chrono::milliseconds timeStamp) {
    // Generate SAN notation for the move
    std::string san = generateSAN(move, board, state);
    
    // Get FEN of position after move
    ChessBoard newBoard = board;
    GameState newState = state;
    makeMove(newBoard, move);
    MoveValidator validator(newBoard, &newState);
    validator.updateGameState(move);
    
    std::string fen = getFEN(newBoard, newState); // TODO: Implement getFEN() function
    
    // Create and add history entry
    moves.emplace_back(move, san, fen, timeStamp);
}

// Generate Standard Algebraic Notation for a move
std::string MoveHistory::generateSAN(const Move& move, const ChessBoard& board, const GameState& state) const {
    std::stringstream san;
    
    // Debugging: Print move details
    std::cout << "Move: from " << move.from << " to " << move.to << std::endl;
    std::cout << "Piece: " << move.piece << std::endl;
    std::cout << "Is Capture: " << move.isCapture << std::endl;
    std::cout << "Is Promotion: " << move.isPromotion << std::endl;
    std::cout << "Promote To: " << move.promoteTo << std::endl;
    std::cout << "Is Castle: " << move.isCastle << std::endl;

    // Handle castling first
    if (move.isCastle) {
        if (move.to % 8 > move.from % 8) {
            return "O-O";    // Kingside castle
        } else {
            return "O-O-O";  // Queenside castle
        }
    }

    std::cout << "Move is not a castle" << std::endl;

    // Add piece letter (except for pawns)
    if (move.piece != W_PAWN && move.piece != B_PAWN) {
        auto it = pieceToSAN.find(move.piece);
        if (it != pieceToSAN.end()) {
            san << it->second[0];
        } else {
            throw std::runtime_error("Invalid piece type in generateSAN");
        }
    }

    std::cout << "Piece letter added" << std::endl;

    // Check for move ambiguity
    std::vector<Move> legalMoves;
    std::vector<Move> ambiguousMoves;
    
    // Get all pseudo-legal moves
    auto pseudoMoves = generatePsuedoMoves(board, &state);
    ChessBoard tempBoard = board;
    GameState tempState = state;
    MoveValidator validator(tempBoard, &tempState);

    std::cout << "Generated pseudo moves" << std::endl;
    
    // Filter to only legal moves of the same piece type going to the same square
    for (const Move& candidateMove : pseudoMoves) {
        if (candidateMove.piece != move.piece || candidateMove.to != move.to) {
            continue;
        }
        if (!validator.isMoveLegal(candidateMove)) {
            continue;
        }
        if (candidateMove.from == move.from) {
            continue;
        }
        ambiguousMoves.push_back(candidateMove);
    }

    std::cout << "Filtered ambiguous moves" << std::endl;
    
    // If we found ambiguous moves, determine what disambiguation is needed
    if (!ambiguousMoves.empty()) {
        bool sameFile = false;
        bool sameRank = false;
        
        // Check if any ambiguous moves share the same file or rank
        for (const Move& ambiguousMove : ambiguousMoves) {
            if (ambiguousMove.from % 8 == move.from % 8) {
                sameFile = true;
            }
            if (ambiguousMove.from / 8 == move.from / 8) {
                sameRank = true;
            }
        }
        
        // Add disambiguation according to standard chess notation rules:
        // 1. Use file if unique
        // 2. Use rank if file is not unique
        // 3. Use both if neither is unique
        if (!sameFile) {
            // File is sufficient for disambiguation
            san << (char)('a' + (move.from % 8));
        } else if (!sameRank) {
            // Need rank for disambiguation
            san << (char)('1' + (move.from / 8));
        } else {
            // Need both file and rank
            san << (char)('a' + (move.from % 8))
                << (char)('1' + (move.from / 8));
        }
    }

    std::cout << "Disambiguation added" << std::endl;

    // Add capture symbol if it's a capture
    if (move.isCapture) {
        // Always show file of origin for pawn captures
        if (move.piece == W_PAWN || move.piece == B_PAWN) {
            san << (char)('a' + (move.from % 8));
        }
        san << "x";
    }

    std::cout << "Capture symbol added" << std::endl;

    // Add destination square
    san << (char)('a' + (move.to % 8)) << (char)('1' + (move.to / 8));

    // Add promotion piece
    if (move.isPromotion) {
        san << "=" << pieceToFEN.at(move.promoteTo)[0];
    }

    std::cout << "Destination square and promotion piece added" << std::endl;

    // Check for check and checkmate
    makeMove(tempBoard, move);
    MoveValidator tempValidator(tempBoard, &tempState);
    tempValidator.updateGameState(move);
    
    std::cout << "Made move" << std::endl;

    if (tempValidator.isCheckmate(tempState.sideToMove)) {
        std::cout << "Checkmate" << std::endl;
        san << "#";
    } else if (tempValidator.isInCheck(tempState.sideToMove)) {
        std::cout << "Check" << std::endl;
        san << "+";
    }

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
    for (size_t i = 0; i < moves.size(); ++i) {
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

// Convert Standard Algebraic Notation (SAN) to a Move object
Move MoveHistory::sanToMove(const std::string& san, const ChessBoard& board, const GameState& state) {
    try {
        // Handle castling moves
        if (san == "O-O") {  // Kingside castle
            int from = state.sideToMove == WHITE ? 4 : 60;   // e1 or e8
            int to = state.sideToMove == WHITE ? 6 : 62;     // g1 or g8
            Move move(state.sideToMove == WHITE ? W_KING : B_KING, from, to);
            move.isCastle = true;
            return move;
        }
        if (san == "O-O-O") {  // Queenside castle
            int from = state.sideToMove == WHITE ? 4 : 60;   // e1 or e8
            int to = state.sideToMove == WHITE ? 2 : 58;     // c1 or c8
            Move move(state.sideToMove == WHITE ? W_KING : B_KING, from, to);
            move.isCastle = true;
            return move;
        }

        // Remove check/mate symbols for parsing
        std::string s = san;
        bool isCheck = (s.back() == '+');
        bool isMate = (s.back() == '#');
        if (isCheck || isMate) {
            s.pop_back();
        }

        // Parse move components
        PieceType pieceType;
        int fromFile = -1, fromRank = -1;
        bool isCapture = false;
        int toFile = -1, toRank = -1;
        PieceType promoteTo = static_cast<PieceType>(-1);

        size_t idx = 0;

        // Get piece type
        if (std::isupper(s[idx])) {
            char piece = s[idx++];
            pieceType = state.sideToMove == WHITE ? 
                fenToPiece.at(piece) : static_cast<PieceType>(fenToPiece.at(piece) + 6);
        } else {
            pieceType = state.sideToMove == WHITE ? W_PAWN : B_PAWN;
        }

        // Parse disambiguation and/or source square
        while (idx < s.length() && (std::isdigit(s[idx]) || std::islower(s[idx]))) {
            if (std::isdigit(s[idx])) {
                fromRank = s[idx] - '1';
            } else if (std::islower(s[idx])) {
                // Could be source file or destination file for pawn capture
                if (idx + 1 < s.length() && s[idx + 1] == 'x') {
                    fromFile = s[idx] - 'a';
                    isCapture = true;
                    idx++;  // Skip 'x'
                } else {
                    fromFile = s[idx] - 'a';
                }
            }
            idx++;
        }

        // Handle capture symbol if present
        if (idx < s.length() && s[idx] == 'x') {
            isCapture = true;
            idx++;
        }

        // Get destination square
        if (idx + 1 >= s.length() || !std::islower(s[idx]) || !std::isdigit(s[idx + 1])) {
            throw std::runtime_error("Invalid destination square in SAN: " + san);
        }
        toFile = s[idx] - 'a';
        toRank = s[idx + 1] - '1';
        idx += 2;

        // Handle promotion if present
        if (idx < s.length() && s[idx] == '=') {
            if (++idx >= s.length() || !std::isupper(s[idx])) {
                throw std::runtime_error("Invalid promotion piece in SAN: " + san);
            }
            char promoPiece = s[idx];
            promoteTo = state.sideToMove == WHITE ? 
                fenToPiece.at(promoPiece) : static_cast<PieceType>(fenToPiece.at(promoPiece) + 6);
        }

        // Find matching legal move
        std::vector<Move> candidates = generatePsuedoMoves(board, &state);
        ChessBoard tempBoard = board;
        GameState tempState = state;
        MoveValidator validator(tempBoard, &tempState);

        for (Move candidate : candidates) {
            if (!validator.isMoveLegal(candidate)) continue;

            // Check basic move properties
            if (candidate.piece != pieceType || 
                candidate.to != toFile + toRank * 8 ||
                candidate.isCapture != isCapture) {
                continue;
            }

            // Check source square constraints
            int candidateFile = candidate.from % 8;
            int candidateRank = candidate.from / 8;
            if ((fromFile != -1 && candidateFile != fromFile) ||
                (fromRank != -1 && candidateRank != fromRank)) {
                continue;
            }

            // Handle promotion
            if (candidate.isPromotion) {
                if (promoteTo == static_cast<PieceType>(-1)) continue;
                candidate.promoteTo = promoteTo;
            }

            return candidate;
        }

        throw std::runtime_error("No matching legal move found for SAN: " + san);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Error parsing SAN: ") + san + " - " + e.what());
    }
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
        ChessBoard board;
        GameState state;
        if (!isStandardStart) {
            setupPosition(board, state, startingFen);
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
                            Move move = sanToMove(currentMove, board, state);
                            
                            // Verify move is legal
                            MoveValidator validator(board, &state);
                            if (!validator.isMoveLegal(move)) {
                                throw std::runtime_error("Illegal move: " + currentMove);
                            }
                            
                            // Add move to history
                            addMove(move, board, state);
                            
                            // Update board position
                            makeMove(board, move);
                            validator.updateGameState(move);
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
                Move move = sanToMove(currentMove, board, state);
                MoveValidator validator(board, &state);
                if (validator.isMoveLegal(move)) {
                    addMove(move, board, state);
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
        std::cerr << "PGN parsing error: " << e.what() << std::endl;
        return false;
    }
}

// Helper method to process a single move token
void MoveHistory::processMoveToken(const std::string& token, ChessBoard& board, GameState& state) {
    // Skip move numbers
    if (token.find('.') != std::string::npos) return;
    
    // Handle game termination markers
    if (token == "1-0" || token == "0-1" || token == "1/2-1/2" || token == "*") {
        setTag("Result", token);
        return;
    }
    
    // Handle NAGs (Numeric Annotation Glyphs)
    if (token[0] == '$') {
        if (!moves.empty()) {
            moves.back().nags.push_back(token.substr(1));
        }
        return;
    }
    
    try {
        // Convert SAN to move
        Move move = sanToMove(token, board, state);
        
        // Verify move is legal
        MoveValidator validator(board, &state);
        if (!validator.isMoveLegal(move)) {
            throw std::runtime_error("Illegal move: " + token);
        }
        
        // Add move to history
        addMove(move, board, state);
        
        // Update board position
        makeMove(board, move);
        validator.updateGameState(move);
        
    } catch (const std::exception& e) {
        // Log error for debugging
        std::cerr << "Error processing move: " << token << " - " << e.what() << std::endl;
        throw;  // Re-throw to handle in fromPGN
    }
}

std::string MoveHistory::getFEN(ChessBoard board, GameState state) {
    std::string fen;
    
    int square = 56;        // Start at a8
    int emptyCount = 0;
    // Loop through all squares for position portion of FEN
    for (square; square >= 0; square++) {
        PieceType current = board.getPieceAt(square);
        //std::cout << "square: " << square << " piece: " << current << std::endl;
        // Empty square
        if (current == -1) {
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
            std::string fenPiece = pieceToFEN.at(current);
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
    fen.append(state.sideToMove == 0 ? "w" : "b");
    // FEN castling rights
    fen.append(" ");
    if (state.canCastleWhiteKingside) fen.append("K");
    if (state.canCastleWhiteQueenside) fen.append("Q");
    if (state.canCastleBlackKingside) fen.append("k");
    if (state.canCastleBlackQueenside) fen.append("q");
    if (!state.canCastleWhiteKingside && !state.canCastleWhiteQueenside &&
        !state.canCastleBlackKingside && !state.canCastleBlackQueenside) fen.append("-");
    // FEN En Passant
    fen.append(" ");
    if (state.enPassantSquare == -1) fen.append("-");
    else fen.append(indexToAlgebraic(state.enPassantSquare));
    // FEN Half Move clock
    fen.append(" ");
    fen.append(std::to_string(state.halfMoveClock));
    // FEN Full Move Number
    fen.append(" ");
    fen.append(std::to_string(state.fullMoveNumber));

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