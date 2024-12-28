#include "utility.hpp"

// Return the corresponding enumPiece for PieceType value
int pieceCode(PieceType ps){
    // Plus 2 to account for white and black values in enumPiece
    return (ps % 6) + 2;
}

// Return the corresponding enumPiece for PieceType value
int colorCode(PieceType ps){
    return ps / 6;
}

void printBitboard(U64 bitb) {
    std::bitset<64> bb (bitb);
    std::cout << "\n";
    for (int j = 7; j >= 0; j--){
        for (int k = 0; k < 8; k++){
            std::cout << bb[j*8 + k];
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

void printBBLine(U64 bitb) {
    std::bitset<64> bb (bitb);
    std::cout << bb << "\n";
}


// Helper function to convert algebraic notation to board index
// e.g., "e4" -> 28
int algebraicToIndex(const std::string& square) {
    if (square.length() != 2) return -1;
    
    int file = tolower(square[0]) - 'a';  // 'a' -> 0, 'b' -> 1, etc.
    int rank = square[1] - '1';           // '1' -> 0, '2' -> 1, etc.
    
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return -1;
    
    return rank * 8 + file;
}

// Helper function to convert board index to algebraic notation
// e.g., 28 -> "e4"
std::string indexToAlgebraic(int index) {
    if (index < 0 || index > 63) return "??";
    
    char file = 'a' + (index % 8);
    char rank = '1' + (index / 8);
    
    return std::string(1, file) + std::string(1, rank);
}



// Setup a chess position from FEN notation
void setupPosition(ChessBoard& board, GameState& state, const std::string& fen) {
    // Create a new empty board
    board = ChessBoard();
    
    // Clear all bitboards
    for (int i = 0; i < 8; i++) {
        board.pieceBB[i] = 0ULL;
    }

    // Clear castling rights
    state.canCastleBlackKingside = false;
    state.canCastleBlackQueenside = false;
    state.canCastleWhiteKingside = false;
    state.canCastleWhiteQueenside = false;
    

    std::stringstream ss(fen);
    std::string position, playerToMove, castlingRights, enPassant, halfTurns, fullTurns;
    ss >> position >> playerToMove >> castlingRights >> enPassant >> halfTurns >> fullTurns;  // Get parts of FEN notation
    
    int square = 56;  // Start from 8th rank (a8 is 56)
    
    // Parse position of FEN
    for (char c : position) {
        // square should only be zero when the FEN is invalid; set to default position
        if (square < 0) {
            setupPosition(board, state, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            return;
        }
        if (c == '/') {
            square -= 16;  // Move down one rank and back to a-file
        } else if (isdigit(c)) {
            square += (c - '0');  // Skip empty squares
        } else if (fenToPiece.count(c)) {
            PieceType piece = fenToPiece.at(c);
            // Set appropriate bits in bitboards
            U64 squareBB = 1ULL << square;
            
            // Set color bitboard
            board.pieceBB[piece <= W_KING ? 0 : 1] |= squareBB;
            
            // Set piece type bitboard
            board.pieceBB[pieceCode(piece)] |= squareBB;
            
            square++;
        }
    }

    // Parse whose turn it is
    state.sideToMove = playerToMove == "w" ? WHITE : BLACK;

    // Parse castling rights
    for (char c : castlingRights) {
        if (c == 'K') state.canCastleWhiteKingside = true;
        if (c == 'Q') state.canCastleWhiteQueenside = true;
        if (c == 'k') state.canCastleBlackKingside = true;
        if (c == 'q') state.canCastleBlackQueenside = true;
    }

    // Parse enpassant
    state.enPassantSquare = enPassant != "-" ? algebraicToIndex(enPassant) : -1;

    // Parse half turns
    std::from_chars(halfTurns.data(), halfTurns.data()+halfTurns.size(), state.halfMoveClock);
    // Parse full move number
    std::from_chars(fullTurns.data(), fullTurns.data()+fullTurns.size(), state.fullMoveNumber);
}



void printFEN(ChessBoard board, GameState state) {
    std::cout << std::format("\n(printFEN) {}\n", getFEN(board, state));
}

std::string getFEN(ChessBoard board, GameState state) {
    std::string fen;
    
    int square = 56;        // Start at a8
    int emptyCount = 0;
    // Loop through all squares for position portion of FEN
    for (square; square >= 0; square++) {
        PieceType current = board.getPieceAt(square);
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

bool isValidFEN(const std::string& fen) {
    
    std::stringstream ss(fen);
    std::string position, playerToMove, castlingRights, enPassant, halfTurns, fullTurns;
    ss >> position >> playerToMove >> castlingRights >> enPassant >> halfTurns >> fullTurns;  // Get parts of FEN notation
    
    int square = 56;  // Start from 8th rank (a8 is 56)
    
    // Check positions of FEN
    for (char c : position) {
        // square should only be zero when the FEN is invalid
        if (square < 0) {
            return false;
        }
        if (c == '/') {
            square -= 16;  // Move down one rank and back to a-file
            continue;
        }
        // Check if piece is valid
        if (!isdigit(c) && fenToPiece.at(c) == -1) return false;
        else if (isdigit(c)) {
            square += (c - '0');  // Skip empty squares
            continue;
        } else if (fenToPiece.count(c)) {
            PieceType piece = fenToPiece.at(c);
            if (piece == -1) return false;
            square++;
        }
    }

    // Check whose turn it is
    if (playerToMove != "w" && playerToMove != "b") return false;

    // Check castling rights
    for (char c : castlingRights) {
        if (c == '-' && castlingRights.size() > 1) return false;
        if (c != 'K' && c != 'Q' && c != 'k' && c != 'q' && c != '-') return false;
    }

    // Check enpassant
    if (enPassant != "-" && (algebraicToIndex(enPassant) < 16 || algebraicToIndex(enPassant) > 47)) return false;

    // Check half turns
    if (halfTurns.find_first_not_of("0123456789") != std::string::npos) return false;
    // Check full turns
    if (fullTurns.find_first_not_of("0123456789") != std::string::npos) return false;

    // FEN is valid
    return true;
}

// Count legal moves in a position
int countLegalMoves(ChessBoard& board, GameState* state) {
    MoveValidator validator(board, state);
    std::vector<Move> moves = generatePsuedoMoves(board, state);
    
    // Filter out illegal moves
    int legalMoveCount = 0;
    for (const Move& move : moves) {
        if (validator.isMoveLegal(move)) {
            legalMoveCount++;
        }
    }
    
    return legalMoveCount;
}

// Verify if position is checkmate
bool isCheckmate(ChessBoard& board, GameState* state) {
    MoveValidator validator(board, state);
    
    // Find king's position
    U64 kingBB = state->sideToMove == WHITE ? board.getWhiteKings() : board.getBlackKings();
    int kingPos = std::countr_zero(kingBB);
    
    // Check if king is in check
    if (!validator.isSquareUnderAttack(kingPos, state->sideToMove == WHITE ? BLACK : WHITE)) {
        return false;  // Not even in check
    }
    
    // Generate all possible moves
    std::vector<Move> moves = generatePsuedoMoves(board, state);
    
    // If any legal move exists, it's not checkmate
    for (const Move& move : moves) {
        if (validator.isMoveLegal(move)) {
            return false;
        }
    }
    
    return true;  // No legal moves and king is in check = checkmate
}

// Verify if position is stalemate
bool isStalemate(ChessBoard& board, GameState* state) {
    MoveValidator validator(board, state);
    
    // Find king's position
    U64 kingBB = state->sideToMove == WHITE ? board.getWhiteKings() : board.getBlackKings();
    int kingPos = std::countr_zero(kingBB);
    
    // If king is in check, it's not stalemate
    if (validator.isSquareUnderAttack(kingPos, state->sideToMove == WHITE ? BLACK : WHITE)) {
        return false;
    }
    
    // Generate all possible moves
    std::vector<Move> moves = generatePsuedoMoves(board, state);
    
    // If any legal move exists, it's not stalemate
    for (const Move& move : moves) {
        if (validator.isMoveLegal(move)) {
            return false;
        }
    }
    
    return true;  // No legal moves but king is not in check = stalemate
}

// Calculates the number of possible positions after a certain number of moves (depth)
// Used for testing move generation accuracy
// @param board: Current chess board position
// @param state: Current game state (castling rights, en passant, etc.)
// @param depth: How many moves deep to calculate
// @return: Number of possible positions at given depth
uint64_t perft(ChessBoard& board, GameState* state, int depth) {
    // Base case - when we reach desired depth, count this position
    if (depth == 0) return 1ULL;
    
    uint64_t nodes = 0;
    
    // Get all possible moves from current position
    std::vector<Move> moves = generatePsuedoMoves(board, state);
    // Create validator for this position
    MoveValidator validator(board, state);
    
    // Try each move
    for (const Move& move : moves) {
        // Skip illegal moves
        if (!validator.isMoveLegal(move)) {
            continue;
        }
        
        // Create temporary board and state for this move
        ChessBoard tempBoard = board;
        GameState tempState = *state;
        
        // Make the move on temporary board
        makeMove(tempBoard, move);
        
        // Update temporary game state for this move
        MoveValidator tempValidator(tempBoard, &tempState);
        tempValidator.updateGameState(move);
        
        // Recurse with temporary position and updated state
        nodes += perft(tempBoard, &tempState, depth - 1);
    }
    
    return nodes;
}

// Calculates the number of possible positions after a certain number of moves (depth)
// Used for testing move generation accuracy
// @param board: Current chess board position
// @param state: Current game state (castling rights, en passant, etc.)
// @param depth: How many moves deep to calculate
// @return: Number of possible positions at given depth
PerftMetrics calcPerftMetrics(ChessBoard& board, GameState* state, int depth) {
    // Base case - when we reach desired depth, count this position
    if (depth == 0) return PerftMetrics(1ULL);
    
    PerftMetrics perftM(0ULL);
    
    // Get all possible moves from current position
    std::vector<Move> moves = generatePsuedoMoves(board, state);
    std::cout << "(perft) # of moves at depth " << depth << ": " << moves.size() << "\n";
    printFEN(board, *state);
    // Create validator for this position
    MoveValidator validator(board, state);
    
    // Try each move
    for (const Move& move : moves) {
        std::cout << "(perft foreach move) depth: " << depth;
        printMove(move);
        // Skip illegal moves
        if (!validator.isMoveLegal(move)) {
            std::cout << "invalid move\n\n";
            continue;
        }
        
        // Create temporary board and state for this move
        ChessBoard tempBoard = board;
        GameState tempState = *state;
        
        // Make the move on temporary board
        makeMove(tempBoard, move);
        
        // Update temporary game state for this move
        MoveValidator tempValidator(tempBoard, &tempState);
        tempValidator.updateGameState(move);

        if (move.isCapture)     perftM.captures = 1;
        if (move.isEnPassant)   perftM.enPassants = 1;
        if (move.isCastle)      perftM.castles = 1;
        if (move.isPromotion)   perftM.promotions = 1;

        if (tempValidator.isInCheck(tempState.sideToMove))     perftM.checks++;
        if (tempValidator.isCheckmate(tempState.sideToMove))   perftM.checkmates++;
        
        std::cout << "\n";
        // Recurse with temporary position and updated state
        perftM += calcPerftMetrics(tempBoard, &tempState, depth - 1);
    }
    
    return perftM;
}

// Helper function to setup common test positions
void setupTestPosition(ChessBoard& board, GameState& state, const std::string& positionName) {
    // Map of common test positions in FEN notation
    const std::map<std::string, std::string> testPositions = {
        {"initial", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"},
        {"kiwipete", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"},
        {"position3", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"},
        {"position4", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"},
        // Add more test positions as needed
    };
    
    if (testPositions.count(positionName)) {
        setupPosition(board, state, testPositions.at(positionName));
        // Parse FEN for state information (castling rights, en passant, etc.)
        // ... (implementation depends on your GameState structure)
    }
}

// Helper to verify attack patterns
bool verifyAttackPattern(const ChessBoard& board, int square, 
                        const std::vector<std::string>& expectedAttacks) {
    U64 attacks;
    if (board.getPieceSet(W_PAWN) & (1ULL << square)) {
        attacks = pawnAttacksWhite[square];
    } else if (board.getPieceSet(W_KNIGHT) & (1ULL << square)) {
        attacks = knightAttacks[square];
    } else if (board.getPieceSet(W_BISHOP) & (1ULL << square)) {
        attacks = bishopAttacks[square];
    } else if (board.getPieceSet(W_ROOK) & (1ULL << square)) {
        attacks = rookAttacks[square];
    } else if (board.getPieceSet(W_QUEEN) & (1ULL << square)) {
        attacks = queenAttacks[square];
    } else if (board.getPieceSet(W_KING) & (1ULL << square)) {
        attacks = kingAttacks[square];
    } else {
        return false;  // No piece on square
    }
    
    // Convert expected attacks to bitboard
    U64 expectedBB = 0ULL;
    for (const std::string& square : expectedAttacks) {
        int index = algebraicToIndex(square);
        if (index != -1) {
            expectedBB |= (1ULL << index);
        }
    }
    
    return attacks == expectedBB;
}

// Convert Standard Algebraic Notation (SAN) to a Move object
// Returns a default Move() if the SAN is invalid or no matching legal move is found
Move sanToMove(const std::string& san, const ChessBoard& board, const GameState& state) {
    // If string is empty, return default move
    if (san.empty()) {
        std::cout << "Empty SAN\n";
        return Move();
    }
    
    // Handle castling moves
    if (san == "O-O") {  // Kingside castle
        int from = state.sideToMove == WHITE ? 4 : 60;   // e1 or e8
        int to = state.sideToMove == WHITE ? 6 : 62;     // g1 or g8
        Move move(state.sideToMove == WHITE ? W_KING : B_KING, from, to);
        move.isCastle = true;
        ChessBoard tempBoard = board;
        GameState tempState = state;
        MoveValidator validator(tempBoard, &tempState);
        if (!validator.isMoveLegal(move)) {
            return Move();  // Return default move if castle is illegal
        }
        return move;
    }
    if (san == "O-O-O") {  // Queenside castle
        int from = state.sideToMove == WHITE ? 4 : 60;   // e1 or e8
        int to = state.sideToMove == WHITE ? 2 : 58;     // c1 or c8
        Move move(state.sideToMove == WHITE ? W_KING : B_KING, from, to);
        move.isCastle = true;
        ChessBoard tempBoard = board;
        GameState tempState = state;
        MoveValidator validator(tempBoard, &tempState);
        if (!validator.isMoveLegal(move)) {
            return Move();  // Return default move if castle is illegal
        }
        return move;
    }

    // Remove check/mate symbols for parsing
    std::string s = san;
    bool isCheck = (s.back() == '+');
    bool isMate = (s.back() == '#');
    if (isCheck || isMate) {
        s.pop_back();
    }

    // Initialize move components
    PieceType pieceType;
    int fromFile = -1, fromRank = -1;
    int toFile = -1, toRank = -1;
    bool isCapture = false;
    bool isPromotion = false;
    PieceType promoteTo = static_cast<PieceType>(-1);        

    // Get piece type from first character if uppercase, otherwise assume pawn
    if (std::isupper(s[0])) {
        char piece = s[0];
        if (fenToPiece.find(piece) == fenToPiece.end()) {
            return Move();  // Invalid piece character
        }
        pieceType = state.sideToMove == WHITE ? 
            fenToPiece.at(piece) : static_cast<PieceType>(fenToPiece.at(piece) + 6);
    } else {
        pieceType = state.sideToMove == WHITE ? W_PAWN : B_PAWN;
        toFile = s[0] - 'a';    // Will be updated if needed by the parsing loop
        if (toFile < 0 || toFile > 7) {
            return Move();  // Invalid file
        }
    }

    // Parse the rest of the move string
    size_t idx = 1;
    while (idx < s.length()) {
        char c = s[idx];
        
        // Parse files (a-h)
        if (std::islower(c) && c >= 'a' && c <= 'h' && c != 'x') {
            if (toFile == -1) {
                toFile = c - 'a';
            } else {
                fromFile = toFile;
                toFile = c - 'a';
            }
        }
        // Parse capture symbol
        else if (c == 'x') {
            isCapture = true;
        }
        // Parse ranks (1-8)
        else if (std::isdigit(c) && c >= '1' && c <= '8') {
            if (toRank == -1) {
                toRank = c - '1';
            } else {
                fromRank = toRank;
                toRank = c - '1';
            }
        }
        // Parse promotion
        else if (c == '=') {
            if (idx + 1 >= s.length()) {
                return Move();  // Missing promotion piece
            }
            isPromotion = true;
            char promPiece = s[idx + 1];
            switch (promPiece) {
                case 'Q':
                    promoteTo = state.sideToMove == WHITE ? W_QUEEN : B_QUEEN;
                    break;
                case 'R':
                    promoteTo = state.sideToMove == WHITE ? W_ROOK : B_ROOK;
                    break;
                case 'B':
                    promoteTo = state.sideToMove == WHITE ? W_BISHOP : B_BISHOP;
                    break;
                case 'N':
                    promoteTo = state.sideToMove == WHITE ? W_KNIGHT : B_KNIGHT;
                    break;
                default:
                    return Move();  // Invalid promotion piece
            }
            idx++;  // Skip the promotion piece character
        }
        idx++;
    }

    // Validate parsed coordinates
    if (toFile == -1 || toRank == -1) {
        return Move();  // Missing destination square
    }

    // Find matching legal move
    std::vector<Move> candidates = generatePsuedoMoves(board, &state);
    ChessBoard tempBoard = board;
    GameState tempState = state;
    MoveValidator validator(tempBoard, &tempState);

    for (Move candidate : candidates) {
        if (!validator.isMoveLegal(candidate)) {
            continue;
        }

        // Check basic move properties
        if (candidate.piece != pieceType || 
            candidate.to != toFile + toRank * 8 ||
            candidate.isCapture != isCapture) {
            continue;
        }

        // Check source square constraints if specified
        int candidateFile = candidate.from % 8;
        int candidateRank = candidate.from / 8;
        if ((fromFile != -1 && candidateFile != fromFile) ||
            (fromRank != -1 && candidateRank != fromRank)) {
            continue;
        }

        // Check promotion
        if (candidate.isPromotion != isPromotion) {
            continue;
        }
        if (isPromotion && promoteTo != candidate.promoteTo) {
            continue;
        }

        return candidate;  // Found matching legal move
    }

    return Move();  // No matching legal move found
}

// Helper function to visualize the chess board in terminal
void printBoard(const ChessBoard& board) {
    // Print column labels
    std::cout << "\n   a b c d e f g h\n";
    std::cout << "   ---------------\n";
    
    // Print rows from top to bottom (8 to 1)
    for (int rank = 7; rank >= 0; rank--) {
        // Print rank number
        std::cout << rank + 1 << "| ";
        
        // Print squares in this rank
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            PieceType piece = board.getPieceAt(square);
            
            // Convert piece type to character
            char pieceChar = ' ';
            switch (piece) {
                case W_PAWN: pieceChar = 'P'; break;
                case W_KNIGHT: pieceChar = 'N'; break;
                case W_BISHOP: pieceChar = 'B'; break;
                case W_ROOK: pieceChar = 'R'; break;
                case W_QUEEN: pieceChar = 'Q'; break;
                case W_KING: pieceChar = 'K'; break;
                case B_PAWN: pieceChar = 'p'; break;
                case B_KNIGHT: pieceChar = 'n'; break;
                case B_BISHOP: pieceChar = 'b'; break;
                case B_ROOK: pieceChar = 'r'; break;
                case B_QUEEN: pieceChar = 'q'; break;
                case B_KING: pieceChar = 'k'; break;
                default: pieceChar = '.'; break;
            }
            std::cout << pieceChar << ' ';
        }
        std::cout << "| " << rank + 1 << "\n";
    }
    std::cout << "   ---------------\n";
    std::cout << "   a b c d e f g h\n\n";
}