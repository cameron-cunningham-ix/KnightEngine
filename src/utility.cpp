#include "utility.hpp"
#include "types.hpp"

bool IsWithinBoard(int index) {
    return index >= 0 && index < 64;
}



// Print a bitboard to std out in 8x8 grid format
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

// Print a bitboard to std out in one line
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
/// @return Returns string of simple algebraic form of move
std::string DenseMove::toAlgebraic() const {
    std::string result = indexToAlgebraic(getFrom()) + indexToAlgebraic(getTo());
    if (getPromoteDense() != D_EMPTY) {
        switch(getPromoteDense()) {
            case D_KNIGHT:
                result += "n";
                break;
            case D_BISHOP:
                result += "b";
                break;
            case D_ROOK:
                result += "r";
                break;
            case D_QUEEN:
                result += "q";
                break;
            default:
                break;
        }
    }
    return result;
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
int countLegalMoves(ChessBoard& board) {
    int moveNum = 0;
    std::array<DenseMove, MAX_MOVES> moves = MoveGenerator::generateLegalMoves(board, moveNum);
    return moveNum;
}

// Verify if position is checkmate
bool isCheckmate(ChessBoard& board) {
    // std::cout << "isCheckmate start\n";
    // Check if king is in check
    if (!board.isInCheck()) {
        return false;  // Not even in check
    }
    
    // Generate all possible moves
    int moveNum = 0;
    std::array<DenseMove, MAX_MOVES> moves = MoveGenerator::generateLegalMoves(board, moveNum);
    
    // If any legal move exists, it's not checkmate
    if (moves[0] != DenseMove()) {
        return false;
    }
    
    return true;  // No legal moves and king is in check = checkmate
}

// Verify if position is stalemate
bool isStalemate(ChessBoard& board) {
    // std::cout << "isStalemate start\n";
    // Generate all possible moves
    int moveNum = 0;
    std::array<DenseMove, MAX_MOVES> moves = MoveGenerator::generateLegalMoves(board, moveNum);
    // printBoard(board);
    // If any legal move exists, it's not stalemate
    if (moves[0] != DenseMove()) {
        return false;
    }

    // Check if king is in check
    if (board.isInCheck()) {
        return false;  // In check means not stalemate
    }
    
    return true;  // No legal moves and king is not in check = stalemate
}

/// @brief Calculates the number of possible positions after a certain number of moves (depth)
/// Used for testing move generation accuracy
/// @param board: Current chess board position
/// @param state: Current game state (castling rights, en passant, etc.)
/// @param depth: How many moves deep to calculate
/// @return: Number of possible positions at given depth
U64 perft(ChessBoard& board, int maxDepth, int depth, bool displaySubPerft) {
    
    U64 nodes = 0;
    int moveNum = 0;
    if (depth == 0) return 1ULL;
    // Get all possible moves from current position
    std::array<DenseMove, MAX_MOVES> moves = MoveGenerator::generatePsuedoMoves(board, moveNum);
    // Base case - when we reach desired depth, count this position
    
    // Try each move
    for (int i = 0; i < moveNum; i++) {
        U64 sub;
        // Make the move on the board
        board.makeMove(moves[i], true);

        // See if the move left its own side in check
        if (!board.isSideInCheck(board.getOppSide())) {
            // Get perft for new sub position
            sub = perft(board, maxDepth, depth - 1, displaySubPerft);
            nodes += sub;
            if (displaySubPerft && maxDepth == depth) {
                std::cout << moves[i].toAlgebraic() << ": " << sub << "\n";
            }
        }
    
        // Unmake the move
        board.unmakeMove(moves[i], true);
    }
    
    return nodes;
}

// // Calculates the number of possible positions after a certain number of moves (depth)
// // Used for testing move generation accuracy
// // @param board: Current chess board position
// // @param state: Current game state (castling rights, en passant, etc.)
// // @param depth: How many moves deep to calculate
// // @return: Number of possible positions at given depth
// PerftMetrics calcPerftMetrics(ChessBoard& board, GameState* state, int depth) {
//     // Base case - when we reach desired depth, count this position
//     if (depth == 0) return PerftMetrics(1ULL);
    
//     PerftMetrics perftM(0ULL);
    
//     // Get all possible moves from current position
//     std::vector<Move> moves = generatePsuedoMoves(board, state);
//     std::cout << "(perft) # of moves at depth " << depth << ": " << moves.size() << "\n";
//     printFEN(board, *state);
//     // Create validator for this position
//     MoveGenerator validator(board, state);
    
//     // Try each move
//     for (const Move& move : moves) {
//         std::cout << "(perft foreach move) depth: " << depth;
//         printMove(move);
//         // Skip illegal moves
//         if (!validator.isMoveLegal(move)) {
//             std::cout << "invalid move\n\n";
//             continue;
//         }
        
//         // Create temporary board and state for this move
//         ChessBoard tempBoard = board;
//         GameState tempState = *state;
        
//         // Make the move on temporary board
//         makeMove(tempBoard, move);
        
//         // Update temporary game state for this move
//         MoveGenerator tempValidator(tempBoard, &tempState);
//         tempValidator.updateGameState(move);

//         if (move.isCapture)     perftM.captures = 1;
//         if (move.isEnPassant)   perftM.enPassants = 1;
//         if (move.isCastle)      perftM.castles = 1;
//         if (move.isPromotion)   perftM.promotions = 1;

//         if (tempValidator.isInCheck(tempState.sideToMove))     perftM.checks++;
//         if (tempValidator.isCheckmate(tempState.sideToMove))   perftM.checkmates++;
        
//         std::cout << "\n";
//         // Recurse with temporary position and updated state
//         perftM += calcPerftMetrics(tempBoard, &tempState, depth - 1);
//     }
    
//     return perftM;
// }

// Helper function to setup common test positions
void setupTestPosition(ChessBoard& board, const std::string& positionName) {
    // Map of common test positions in FEN notation
    const std::map<std::string, std::string> testPositions = {
        {"initial", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"},
        {"kiwipete", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"},
        {"position3", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"},
        {"position4", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"},
        // Add more test positions as needed
    };

    if (testPositions.count(positionName)) {
        board.setupPositionFromFEN(testPositions.at(positionName));
    }
}

// Helper to verify attack patterns
bool verifyAttackPattern(const ChessBoard& board, int square, 
                        const std::vector<std::string>& expectedAttacks) {
    U64 attacks;
    if (board.getPieceSet(W_PAWN) & (1ULL << square)) {
        attacks = ATKMASK_WPAWN[square];
    } else if (board.getPieceSet(W_KNIGHT) & (1ULL << square)) {
        attacks = ATKMASK_KNIGHT[square];
    } else if (board.getPieceSet(W_BISHOP) & (1ULL << square)) {
        attacks = ATKMASK_BISHOP[square];
    } else if (board.getPieceSet(W_ROOK) & (1ULL << square)) {
        attacks = ATKMASK_ROOK[square];
    } else if (board.getPieceSet(W_QUEEN) & (1ULL << square)) {
        attacks = ATKMASK_QUEEN[square];
    } else if (board.getPieceSet(W_KING) & (1ULL << square)) {
        attacks = ATKMASK_KING[square];
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
DenseMove sanToMove(const std::string& san, ChessBoard& board) {
    // If string is empty, return default move
    if (san.empty()) {
        std::cout << "Empty SAN\n";
        return DenseMove();
    }

    // Initialize move components
    PieceType pieceType;
    int fromFile = -1, fromRank = -1;
    int toFile = -1, toRank = -1;
    bool isCapture = false;
    bool isCastle = false;
    bool isPromotion = false;
    PieceType promoteTo = PieceType::EMPTY;
    // Get state
    Color sideToMove = board.currentGameState.sideToMove;

    DenseMove move = DenseMove();
    
    // Handle castling moves
    if (san == "O-O") {  // Kingside castle
        pieceType = sideToMove == WHITE ? W_KING : B_KING;
        int from = sideToMove == WHITE ? 4 : 60;   // e1 or e8
        int to = sideToMove == WHITE ? 6 : 62;     // g1 or g8
        fromFile = from % 8;
        toFile = to % 8;
        fromRank = from / 8;
        toRank = to / 8;
        isCastle = true;
        move = DenseMove(pieceType, from, to, D_EMPTY, true);
    } else if (san == "O-O-O") {  // Queenside castle
        pieceType = sideToMove == WHITE ? W_KING : B_KING;
        int from = sideToMove == WHITE ? 4 : 60;   // e1 or e8
        int to = sideToMove == WHITE ? 2 : 58;     // c1 or c8
        fromFile = from % 8;
        toFile = to % 8;
        fromRank = from / 8;
        toRank = to / 8;
        isCastle = true;
        move = (pieceType, from, to, D_EMPTY, true);
    } else {
        // Remove check/mate symbols for parsing
        std::string s = san;
        bool isCheck = (s.back() == '+');
        bool isMate = (s.back() == '#');
        if (isCheck || isMate) {
            s.pop_back();
        }


        // Get piece type from first character if uppercase, otherwise assume pawn (ignore castling since it's already set)
        if (std::isupper(s[0]) && s[0] != 'O') {
            char piece = s[0];
            if (fenToPiece.find(piece) == fenToPiece.end()) {
                return DenseMove();  // Invalid piece character
            }
            pieceType = sideToMove == WHITE ? 
                fenToPiece.at(piece) : static_cast<PieceType>(fenToPiece.at(piece) + 8);
        } else if (s[0] != 'O') {
            pieceType = sideToMove == WHITE ? W_PAWN : B_PAWN;
            toFile = s[0] - 'a';    // Will be updated if needed by the parsing loop
            if (toFile < 0 || toFile > 7) {
                return DenseMove();  // Invalid file
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
                    return DenseMove();  // Missing promotion piece
                }
                isPromotion = true;
                char promPiece = s[idx + 1];
                switch (promPiece) {
                    case 'Q':
                        promoteTo = sideToMove == WHITE ? W_QUEEN : B_QUEEN;
                        break;
                    case 'R':
                        promoteTo = sideToMove == WHITE ? W_ROOK : B_ROOK;
                        break;
                    case 'B':
                        promoteTo = sideToMove == WHITE ? W_BISHOP : B_BISHOP;
                        break;
                    case 'N':
                        promoteTo = sideToMove == WHITE ? W_KNIGHT : B_KNIGHT;
                        break;
                    default:
                        return DenseMove();  // Invalid promotion piece
                }
                idx++;  // Skip the promotion piece character
            }
            idx++;
        }
    }


    // Validate parsed coordinates
    if (toFile == -1 || toRank == -1) {
        return DenseMove();  // Missing destination square
    }
    int moveNum = 0;
    // Find matching legal move
    std::array<DenseMove, MAX_MOVES> candidates = MoveGenerator::generateLegalMoves(board, moveNum);

    for (int i = 0; i < moveNum; i++) {
        DenseMove candidate = candidates[i];
        // Check basic move properties
        if (candidate.getPieceType() != pieceType || 
            candidate.getTo() != toFile + toRank * 8 ||
            candidate.isCapture() != isCapture) {
            continue;
        }

        // Check source square constraints if specified
        int candidateFile = candidate.getFrom() % 8;
        int candidateRank = candidate.getFrom() / 8;
        if ((fromFile != -1 && candidateFile != fromFile) ||
            (fromRank != -1 && candidateRank != fromRank)) {
            continue;
        }

        // Check promotion
        if (candidate.isPromotion() != isPromotion) {
            continue;
        }
        if (isPromotion && promoteTo != candidate.getPromotePiece()) {
            continue;
        }
        // Check castling
        if (candidate.isCastle() != isCastle) {
            continue;
        }

        return candidate;  // Found matching legal move
    }

    return DenseMove();  // No matching legal move found
}

// Helper function to visualize the chess board in terminal
void printBoard(const ChessBoard& board) {
    // Print side to move
    std::cout << "Side to move: " << board.getSideToMove() << "\n";
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
/// @brief 
/// @param mask 
/// @param startIndex 
/// @param endIndex 
/// @param direction 0 = E, 1 = SE, 2 = S, 3 = SW, 4 = W, 5 = NW, 6 = N, 7 = NE
/// @return 
U64 setBitsBetween(int startIndex, int endIndex, int direction) {
    //
    switch (direction) {
        case 0:
            return (1 << (endIndex + 1)) - (1 << startIndex);
            break;
        case 1:
            break;
        case 2:
            break;
        case 3:
            break;
        case 4:
            return (1 << (startIndex - 1)) + (1 << endIndex);
        default:
            break;
    }
}