#include "board_generation.hpp"
#include "board_utility.hpp"
#include "moves.hpp"
#include "utility.hpp"
#include "pext_bitboard.hpp"
#include <iostream>
#include <string>
#include <stdlib.h>


/// @brief  Default constructor; initialize all bitboards
ChessBoard::ChessBoard() {
    initializeWhiteBB();
    initializeBlackBB();
    initializeEmptyBB();
    initializePawnsBB();
    initializeKnightsBB();
    initializeBishopsBB();
    initializeRooksBB();
    initializeQueensBB();
    initializeKingsBB();
    initializeGameState();
}

/// @brief Set up a board based on the given FEN string.
/// If FEN is invalid, sets up standard position
/// @param fen 
void ChessBoard::setupPositionFromFEN(const std::string& fen) {
    // Clear all attributes
    for (int i = 0; i < 7; i++) {
        pieceBB[i] = 0ULL;
    }
    colorBB[WHITE] = 0ULL;
    colorBB[BLACK] = 0ULL;
    kingSquares[WHITE] = 0;
    kingSquares[BLACK] = 0;
    attacksToKings[WHITE] = 0ULL;
    attacksToKings[BLACK] = 0ULL;
    hasCachedInCheckValue = false;

    // Clear castling rights
    currentGameState.canCastleBlackKingside = false;
    currentGameState.canCastleBlackQueenside = false;
    currentGameState.canCastleWhiteKingside = false;
    currentGameState.canCastleWhiteQueenside = false;
    

    std::stringstream ss(fen);
    std::string position, playerToMove, castlingRights, enPassant, halfTurns, fullTurns;
    ss >> position >> playerToMove >> castlingRights >> enPassant >> halfTurns >> fullTurns;  // Get parts of FEN notation
    
    int square = 56;  // Start from 8th rank (a8 is 56)
    
    // Parse position of FEN
    for (char c : position) {
        // square should only be zero when the FEN is invalid; set to default position
        if (square < 0) {
            setupPositionFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            std::cout << "Invalid FEN\nSet to default position\n";
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
            colorBB[piece <= W_KING ? 0 : 1] |= squareBB;
            
            // Set piece type bitboard
            pieceBB[pieceCode(piece)] |= squareBB;
            // If king, set its square
            if (piece == W_KING) kingSquares[WHITE] = square;
            if (piece == B_KING) kingSquares[BLACK] = square;
            
            square++;
        }
    }
    // Set empty bitboard 
    pieceBB[D_EMPTY] = (~colorBB[WHITE]) & (~colorBB[BLACK]);


    // Parse whose turn it is
    currentGameState.sideToMove = playerToMove == "w" ? WHITE : BLACK;
    currentGameState.oppColor = (Color)!currentGameState.sideToMove;

    // Parse castling rights
    for (char c : castlingRights) {
        if (c == 'K') currentGameState.canCastleWhiteKingside = true;
        if (c == 'Q') currentGameState.canCastleWhiteQueenside = true;
        if (c == 'k') currentGameState.canCastleBlackKingside = true;
        if (c == 'q') currentGameState.canCastleBlackQueenside = true;
    }

    // Parse enpassant
    currentGameState.enPassantSquare = enPassant != "-" ? algebraicToIndex(enPassant) : -1;

    // Parse half turns
    std::from_chars(halfTurns.data(), halfTurns.data()+halfTurns.size(), currentGameState.halfMoveClock);
    // Parse full move number
    std::from_chars(fullTurns.data(), fullTurns.data()+fullTurns.size(), currentGameState.fullMoveNumber);
    // Add initial state to history
    stateHistory.emplace_back(currentGameState);
    currentGameState = stateHistory.back();
}
/// @return String of the current FEN notation of the board 
std::string ChessBoard::getFEN() {
    std::string fen;
    
    int square = 56;        // Start at a8
    int emptyCount = 0;
    // Loop through all squares for position portion of FEN
    for (square; square >= 0; square++) {
        PieceType current = getPieceAt(square);
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
    fen.append(currentGameState.sideToMove == 0 ? "w" : "b");
    // FEN castling rights
    fen.append(" ");
    if (currentGameState.canCastleWhiteKingside) fen.append("K");
    if (currentGameState.canCastleWhiteQueenside) fen.append("Q");
    if (currentGameState.canCastleBlackKingside) fen.append("k");
    if (currentGameState.canCastleBlackQueenside) fen.append("q");
    if (!currentGameState.canCastleWhiteKingside && !currentGameState.canCastleWhiteQueenside &&
        !currentGameState.canCastleBlackKingside && !currentGameState.canCastleBlackQueenside) fen.append("-");
    // FEN En Passant
    fen.append(" ");
    if (currentGameState.enPassantSquare == -1) fen.append("-");
    else fen.append(indexToAlgebraic(currentGameState.enPassantSquare));
    // FEN Half Move clock
    fen.append(" ");
    fen.append(std::to_string(currentGameState.halfMoveClock));
    // FEN Full Move Number
    fen.append(" ");
    fen.append(std::to_string(currentGameState.fullMoveNumber));

    return fen;
}
/// @brief Print FEN notation of current board to std out
void ChessBoard::printFEN() {
    std::cout << std::format("\n(ChessBoard printFEN) {}\n", getFEN());
}
/// @brief Get the bitboard of a specific piece type
/// @param pt 
/// @return If pt == EMPTY, returns Empty pieceBB. If pt == INVALID, returns 0.
/// Else, return right piece set bitboard
U64 ChessBoard::getPieceSet(PieceType pt) const {
    if (pt == EMPTY) return pieceBB[D_EMPTY];
    if (pt == INVALID) return 0ULL;
    return pieceBB[pieceCode(pt)] & colorBB[colorCode(pt)];
}
/// @brief Private helper function to update bitboards for piece movement.
/// Should only be used where move is valid
/// @param from 
/// @param to 
/// @param piece 
void ChessBoard::movePiece(int from, int to, PieceType piece) {
    U64 fromToBB = (1ULL << from) | (1ULL << to);  // BB with both squares set
    // std:: cout << "ChessBoard.movePiece fromToBB:\n";
    // printBB(fromToBB);
    // Get color and piece type
    bool isWhite = piece <= W_KING;
    int pieceType = pieceCode(piece);

    // std::cout << "ChessBoard.movePiece color " << isWhite 
            //   << " piece " << pieceType << "\n";

    // std:: cout << "ChessBoard.movePiece og colorBB:\n";
    // printBB(colorBB[isWhite ? 0 : 1]);

    // Update color bitboard (white or black)
    colorBB[isWhite ? 0 : 1] ^= fromToBB;

    // std:: cout << "ChessBoard.movePiece new colorBB:\n";
    // printBB(colorBB[isWhite ? 0 : 1]);

    // std:: cout << "ChessBoard.movePiece og pieceBB:\n";
    // printBB(pieceBB[pieceType]);

    // Update piece type bitboard (pawn, knight, etc.)
    pieceBB[pieceType] ^= fromToBB;

    // std:: cout << "ChessBoard.movePiece new pieceBB:\n";
    // printBB(pieceBB[pieceType]);

    // Update empty squares bitboard
    pieceBB[D_EMPTY] ^= fromToBB;
    // If king moved, update king square
    if (piece == W_KING) {
        kingSquares[WHITE] = to;
        // std::cout << "ChessBoard.movePiece - kingSquares[WHITE] = " << kingSquares[WHITE] << "\n";
    } 
    else if (piece == B_KING) kingSquares[BLACK] = to;
    // std::cout << "ChessBoard.movePiece - kingSquares[BLACK] = " << kingSquares[BLACK] << "\n";
}
/// @brief Helper function to remove a piece from the board
/// @param square 
/// @param piece 
void ChessBoard::removePiece(int square, PieceType piece) {
    // std::cout << "removing piece " << piece << " at square " << square << "\n";
    // Bitboard with only the square bit 0
    U64 clearSquareBB = ~(1ULL << square);

    // Get color and piece type
    bool isWhite = piece <= W_KING;
    int pieceType = pieceCode(piece);

    // Clear bit in color bitboard
    colorBB[isWhite ? 0 : 1] &= clearSquareBB;

    // Clear bit in piece type bitboard
    pieceBB[pieceType] &= clearSquareBB;
    // Set bit in empty squares bitboard
    pieceBB[D_EMPTY] |= (1ULL << square);
}
/// @brief  Helper function to add a piece to the board
/// @param square 
/// @param piece 
void ChessBoard::addPiece(int square, PieceType piece) {
    U64 squareBB = 1ULL << square;

    // Get color and piece type
    bool isWhite = piece <= W_KING;
    int pieceType = pieceCode(piece);

    // Set bit in color bitboard
    colorBB[isWhite ? 0 : 1] |= squareBB;

    // Set bit in piece type bitboard
    pieceBB[pieceType] |= squareBB;
    // Clear bit in empty squares bitboard
    pieceBB[D_EMPTY] &= ~squareBB;
}
/// @brief Calculates whether current side to move is in check
/// @return True if current side to move is in check, false otherwise
bool ChessBoard::calculateIsInCheck() {
    // Get current side to move
    Color sideToMove = currentGameState.sideToMove;
    // std::cout << "calculateIsInCheck side: " << sideToMove << "\n";
    // Get side's king square
    int kingSquare = kingSquares[sideToMove];
    // std::cout << "calculateIsInCheck kingSquare: " << kingSquare << "\n";

    // Occupancy
    U64 occupancy = getAllPieces();
    // Clear attacksToKings[Color] and checkingCount
    attacksToKings[sideToMove] = 0ULL;
    checkingCount = 0;
    // Get sliding piece bitboards
    U64 oppOrthoSliders = sideToMove == WHITE ? 
        (getBlackRooks() | getBlackQueens()) : (getWhiteRooks() | getWhiteQueens());
    U64 oppDiagSliders = sideToMove == WHITE ?
        (getBlackBishops() | getBlackQueens()) : (getWhiteBishops() | getWhiteQueens());
    U64 attacking;
    // If there's any enemy orthogonal sliding pieces, see if they're checking king
    if (oppOrthoSliders != 0) {
        U64 RQAttacks = PEXT::getRookAttacks(kingSquare, occupancy);
        attacking = RQAttacks & oppOrthoSliders;
        if (attacking) {
            attacksToKings[sideToMove] |= attacking;
        }
    }
    // Any enemy diagonal sliding pieces
    if (oppDiagSliders != 0) {
        U64 BQAttacks = PEXT::getBishopAttacks(kingSquare, occupancy);
        attacking = BQAttacks & oppDiagSliders;
        if (attacking) {
            attacksToKings[sideToMove] |= attacking;
        }
    }
    // Any knights
    U64 oppKnights = sideToMove == WHITE ? getBlackKnights() : getWhiteKnights();
    attacking = ATKMASK_KNIGHT[kingSquare] & oppKnights;
    if (attacking) {
        attacksToKings[sideToMove] |= attacking;
    }
    // Any pawns
    U64 oppPawns = sideToMove == WHITE ? getBlackPawns() : getWhitePawns();
    // Use the mask of the same color that we're checking, this correctly masks any attacking pawns
    U64 pawnAttackMask = sideToMove == WHITE ? ATKMASK_WPAWN[kingSquare] : ATKMASK_BPAWN[kingSquare];
    attacking = pawnAttackMask & oppPawns;
    if (attacking) {
        attacksToKings[sideToMove] |= attacking;
    }
    // No checking for opposite king, since kings can't check each other
    // Count number of bits set in attacks to kings (should be 2 at most)
    checkingCount = popcount(attacksToKings[sideToMove]);
    return checkingCount > 0;
}

// Note: I could use getPieceSet for these getters, but A. these should
// never change and B. doing it this way keeps from unnecessary steps in getPieceSet

/// @return Bitboard of white pawns
U64 ChessBoard::getWhitePawns() const {
    return colorBB[WHITE] & pieceBB[D_PAWN];
}
/// @return Bitboard of white knights 
U64 ChessBoard::getWhiteKnights() const {
    return colorBB[WHITE] & pieceBB[D_KNIGHT];
}
/// @return Bitboard of white bishops
U64 ChessBoard::getWhiteBishops() const {
    return colorBB[WHITE] & pieceBB[D_BISHOP];
}
/// @return Bitboard of white rooks
U64 ChessBoard::getWhiteRooks() const {
    return colorBB[WHITE] & pieceBB[D_ROOK];
}
/// @return Bitboard of white queens
U64 ChessBoard::getWhiteQueens() const {
    return colorBB[WHITE] & pieceBB[D_QUEEN];
}
/// @return Bitboard of white kings
U64 ChessBoard::getWhiteKings() const {
    return colorBB[WHITE] & pieceBB[D_KING];
}
/// @return Bitboard of black pawns
U64 ChessBoard::getBlackPawns() const {
    return colorBB[BLACK] & pieceBB[D_PAWN];
}
/// @return Bitboard of black knights
U64 ChessBoard::getBlackKnights() const {
    return colorBB[BLACK] & pieceBB[D_KNIGHT];
}
/// @return Bitboard of black bishops
U64 ChessBoard::getBlackBishops() const {
    return colorBB[BLACK] & pieceBB[D_BISHOP];
}
/// @return Bitboard of black rooks
U64 ChessBoard::getBlackRooks() const {
    return colorBB[BLACK] & pieceBB[D_ROOK];
}
/// @return Bitboard of black queens
U64 ChessBoard::getBlackQueens() const {
    return colorBB[BLACK] & pieceBB[D_QUEEN];
}
/// @return Bitboard of black kings
U64 ChessBoard::getBlackKings() const {
    return colorBB[BLACK] & pieceBB[D_KING];
}
/// @return Bitboard of white pieces
U64 ChessBoard::getWhitePieces() const {
    return colorBB[WHITE];
}
/// @return Bitboard of black pieces
U64 ChessBoard::getBlackPieces() const {
    return colorBB[BLACK];
}
/// @return Bitboard of all pieces
U64 ChessBoard::getAllPieces() const {
    return colorBB[WHITE] | colorBB[BLACK];
}
/// @return Bitboard of empty squares
U64 ChessBoard::getEmptySquares() const {
    return pieceBB[EMPTY];
}
/// @param side 
/// @return Bitboard of pieces attacking side's king
U64 ChessBoard::getAttacksToKing(Color side) const {
    return attacksToKings[side];
}
/// @param side 
/// @return Bitboard of orthogonal attackers of opposing side
U64 ChessBoard::getOrthogonalOpp(Color side) const {
    return side == WHITE ? getBlackRooks() | getBlackQueens() :
                           getWhiteRooks() | getWhiteQueens();
}
/// @param side 
/// @return Bitboard of diagonal attackers of opposing side
U64 ChessBoard::getDiagonalOpp(Color side) const {
    return side == WHITE ? getBlackBishops() | getBlackQueens() :
                           getWhiteBishops() | getWhiteQueens();
}
/// @return Return the square index of the white king
int ChessBoard::getWhiteKingSquare() const {
    return kingSquares[WHITE];
}
/// @return Return the square index of the black king 
int ChessBoard::getBlackKingSquare() const {
    return kingSquares[BLACK];
}
Color ChessBoard::getSideToMove() const
{
    return currentGameState.sideToMove;
}
Color ChessBoard::getOppSide() const
{
    return currentGameState.oppColor;
}
/// @brief
/// @param index
/// @return PieceType at square index. If index is invalid, returns INVALID
PieceType ChessBoard::getPieceAt(int index) const {
    // Check if index is valid
    if (index < 0 || index > 63) {
        std::cerr << "ChessBoard getPieceAt Error: Invalid index " << index << "\n";
        return PieceType::INVALID;
    }
    // Check if the square is empty
    if (!(getAllPieces() & (1ULL << index))) {
        return PieceType::EMPTY;
    }

    // Check color of piece
    bool isBlack = getBlackPieces() & (1ULL << index);

    // Get piece type by checking each piece bitboard
    if (pieceBB[D_PAWN] & (1ULL << index)) {
        return isBlack ? B_PAWN : W_PAWN;
    }
    if (pieceBB[D_KNIGHT] & (1ULL << index)) {
        return isBlack ? B_KNIGHT : W_KNIGHT;
    }
    if (pieceBB[D_BISHOP] & (1ULL << index)) {
        return isBlack ? B_BISHOP : W_BISHOP;
    }
    if (pieceBB[D_ROOK] & (1ULL << index)) {
        return isBlack ? B_ROOK : W_ROOK;
    }
    if (pieceBB[D_QUEEN] & (1ULL << index)) {
        return isBlack ? B_QUEEN : W_QUEEN;
    }
    if (pieceBB[D_KING] & (1ULL << index)) {
        return isBlack ? B_KING : W_KING;
    }

    // Reaching this point means there is an error in the bitboards,
    // specifically that this index is set in one of the color bitboards 
    // but not in any of the piece bitboards
    std::cerr << "ChessBoard getPieceAt Error: Invalid piece at square " << index << "\n";
    return PieceType::INVALID;
}
/// @brief 
/// @param index 
/// @return DenseType at square index. If index is invalid, returns INVALID
DenseType ChessBoard::getDenseTypeAt(int index) const {
    // Check if index is valid
    if (index < 0 || index > 63) {
        std::cerr << "ChessBoard getPieceAt Error: Invalid index " << index << "\n";
        return DenseType::D_EMPTY;
    }
    // Check if the square is empty
    if (!(getAllPieces() & (1ULL << index))) {
        return DenseType::D_EMPTY;
    }

    // Get piece type by checking each piece bitboard
    if (pieceBB[D_PAWN] & (1ULL << index)) {
        return D_PAWN;
    }
    if (pieceBB[D_KNIGHT] & (1ULL << index)) {
        return D_KNIGHT;
    }
    if (pieceBB[D_BISHOP] & (1ULL << index)) {
        return D_BISHOP;
    }
    if (pieceBB[D_ROOK] & (1ULL << index)) {
        return D_ROOK;
    }
    if (pieceBB[D_QUEEN] & (1ULL << index)) {
        return D_QUEEN;
    }
    if (pieceBB[D_KING] & (1ULL << index)) {
        return D_KING;
    }

    // Reaching this point means there is an error in the bitboards,
    // specifically that this index is set in one of the color bitboards 
    // but not in any of the piece bitboards
    std::cerr << "ChessBoard getPieceAt Error: Invalid piece at square " << index << "\n";
    return DenseType::D_EMPTY;
}
/// @return cachedInCheckValue if it is cached, otherwise calculates check value,
/// caches it, and returns check value
bool ChessBoard::isInCheck() {
    // Value has already been cached, return value
    if (hasCachedInCheckValue) {
        return cachedInCheckValue;
    }
    // No cache, calculate and store
    cachedInCheckValue = calculateIsInCheck();
    hasCachedInCheckValue = true;
    return cachedInCheckValue;
}
/// @brief 
/// @param side 
/// @return 
bool ChessBoard::isSideInCheck(Color side) {
    // std::cout << "ChessBoard.isSideInCheck start\n side " << side << "\n"
    //           << "    kingSquares[side] " << kingSquares[side] << "\n";
    return OppAttacksToSquare(kingSquares[side], side);
}
/// @return Number of pieces checking current side's king
int ChessBoard::getCheckCount() const {
    return checkingCount;
}
/// @brief Initialize normal starting position - white pieces
void ChessBoard::initializeWhiteBB() {
    colorBB[WHITE] = 0xFFFF;
}
/// @brief Initialize normal starting position - black pieces
void ChessBoard::initializeBlackBB() {
    colorBB[BLACK] = 0xFFFF000000000000;
}
/// @brief Initialize normal starting position - empty squares
void ChessBoard::initializeEmptyBB() {
    pieceBB[D_EMPTY] = 0x0000FFFFFFFF0000;
}
/// @brief Initialize normal starting position - pawn pieces
void ChessBoard::initializePawnsBB() {
    pieceBB[D_PAWN] = 0x00FF00000000FF00;
}
/// @brief Initialize normal starting position - knight pieces
void ChessBoard::initializeKnightsBB() {
    pieceBB[D_KNIGHT] = 0x4200000000000042;
}
/// @brief Initialize normal starting position - bishop pieces
void ChessBoard::initializeBishopsBB() {
    pieceBB[D_BISHOP] = 0x2400000000000024;
}
/// @brief Initialize normal starting position - rook pieces
void ChessBoard::initializeRooksBB() {
    pieceBB[D_ROOK] = 0x8100000000000081;
}
/// @brief Initialize normal starting position - queen pieces
void ChessBoard::initializeQueensBB() {
    pieceBB[D_QUEEN] = 0x800000000000008;
}
/// @brief Initialize normal starting position of king related attributes
void ChessBoard::initializeKingsBB() {
    pieceBB[D_KING] = 0x1000000000000010;
    kingSquares[WHITE] = 4;     // White king square index
    kingSquares[BLACK] = 60;    // Black king square index
    attacksToKings[WHITE] = 0ULL;
    attacksToKings[BLACK] = 0ULL;
}
/// @brief Initialize normal starting game state
void ChessBoard::initializeGameState() {
    // New GameState
    currentGameState = GameState();
    // Store initial state onto stateHistory
    stateHistory.emplace_back(currentGameState);
    // Initialize cached check values as false, otherwise they have garbage values
    hasCachedInCheckValue = false;
    cachedInCheckValue = false;
}
/// @brief Find any opponent pieces that are attacking square 'index'
/// @param index Index of square 
/// @param colorOfKing 
/// @return Bitboard of any opposite side pieces attacking this square
U64 ChessBoard::OppAttacksToSquare(int index, Color colorOfKing) const {
    // std::cout << "ChessBoard.OppAttacksToSquare start\n    index "
    //           << index << " color " << colorOfKing << "\n";
    if (index < 0 || index > 63) return 0ULL;
    // Get opposing pieces based on king's color
    U64 opPawns, opKnights, opBQ, opRQ, opK, occupancy;
    if (colorOfKing == WHITE) {
        // Get all black pieces that could attack white king
        opPawns = getBlackPawns();
        opKnights = getBlackKnights();
        opRQ = getBlackRooks() | getBlackQueens();   // Rooks and Queens for orthogonal attacks
        opBQ = getBlackBishops() | getBlackQueens(); // Bishops and Queens for diagonal attacks
        opK = getBlackKings();
    } else {
        // Get all white pieces that could attack black king
        opPawns = getWhitePawns();
        opKnights = getWhiteKnights();
        opRQ = getWhiteRooks() | getWhiteQueens();   // Rooks and Queens for orthogonal attacks
        opBQ = getWhiteBishops() | getWhiteQueens(); // Bishops and Queens for diagonal attacks
        opK = getWhiteKings();
    }

    // Get occupancy of all pieces for blocking calculations
    occupancy = getAllPieces();
    // Return attacking pieces
    return (colorOfKing == WHITE ? ATKMASK_WPAWN[index] : ATKMASK_BPAWN[index]) & opPawns |
            ATKMASK_KNIGHT[index] & opKnights |
            PEXT::getRookAttacks(index, occupancy) & opRQ |
            PEXT::getBishopAttacks(index, occupancy) & opBQ |
            ATKMASK_KING[index] & opK;
}
/// @brief Make a move on the board.
/// Note: This function does not check validity. It should only be used with known
/// legal moves from MoveGeneration.
/// @param move
/// @param searching If searching, move is not added to moveHistory.
void ChessBoard::makeMove(DenseMove move, bool searching) {
    // Get move info
    PieceType movedPiece = move.getPieceType();
    Color movedColor = move.getColor();
    int from = move.getFrom();
    int to = move.getTo();
    PieceType capturedPiece = move.getCaptPiece();
    bool isCastle = move.isCastle();
    bool isEnPass = move.isEnPassant();
    PieceType promoPiece = move.getPromotePiece();
    
    // Handle captures and en passant
    if (capturedPiece != PieceType::EMPTY) {
        // Normal capture
        if (!isEnPass) {
            removePiece(to, capturedPiece);
        }
        // En passant capture
        else {
            // Remove captured pawn
            // En passant square is 'behind' the captured pawn, so we have to calculate it
            int capturedPawnSquare = to + (movedColor == WHITE ? -8 : 8);
            removePiece(capturedPawnSquare, capturedPiece);
        }
    }
    // Handle castling
    else if (isCastle) {
        bool isKingside = (to > from);
        // King gets moved at the end of makeMove, so we only need to move the rook
        if (isKingside) {
            // Move rook from h1/h8 to f1/f8
            int rookFrom = (movedPiece == W_KING) ? 7 : 63;
            int rookTo = (movedPiece == W_KING) ? 5 : 61;
            movePiece(rookFrom, rookTo, (movedPiece == W_KING) ? W_ROOK : B_ROOK);
        } else {
            // Move rook from a1/a8 to d1/d8
            int rookFrom = (movedPiece == W_KING) ? 0 : 56;
            int rookTo = (movedPiece == W_KING) ? 3 : 59;
            movePiece(rookFrom, rookTo, (movedPiece == W_KING) ? W_ROOK : B_ROOK);
        }
    }
    // Handle promotion
    if (promoPiece != PieceType::EMPTY) {
        // std::cout << "makeMove promotion - remove piece " << promoPiece << "\n";
        // Remove pawn from source square
        removePiece(from, movedPiece);
        // Add promoted piece to destination square
        addPiece(to, promoPiece);
    } else {
        // Move piece if not promotion
        movePiece(from, to, movedPiece);
    }

    // Update current game state
    // Castling rights
    if (movedPiece == W_KING) {
        currentGameState.canCastleWhiteKingside = false;
        currentGameState.canCastleWhiteQueenside = false;
    } else if (movedPiece == B_KING) {
        currentGameState.canCastleBlackKingside = false;
        currentGameState.canCastleBlackQueenside = false;
    } else if (from == 0 || to == 0) {
        // Anything moving to or from initial rook squares loses rights
        currentGameState.canCastleWhiteQueenside = false;
    } else if (from == 7 || to == 7) {
        currentGameState.canCastleWhiteKingside = false;
    } else if (from == 56 || to == 56) {
        currentGameState.canCastleBlackQueenside = false;
    } else if (from == 63 || to == 63) {
        currentGameState.canCastleBlackKingside = false;
    }

    // Update en passant
    if (movedPiece != W_PAWN && movedPiece != B_PAWN) {
        currentGameState.enPassantSquare = -1;
    } else {
        int expectedFromRank = movedColor == WHITE ? 1 : 6;
        int expectedToRank = movedColor == WHITE ? 3 : 4;
        if ((from / 8) == expectedFromRank && (to / 8) == expectedToRank) {
            // En Passant square is the square behind the double pushed pawn
            currentGameState.enPassantSquare = (from + to) / 2;
        } else {
            currentGameState.enPassantSquare = -1;
        }
    }

    // Switch side to move
    currentGameState.sideToMove = (Color)!currentGameState.sideToMove;
    currentGameState.oppColor = (Color)!currentGameState.oppColor;


    // Check if half-move clock should be reset or incremented
    if (capturedPiece != PieceType::EMPTY || movedPiece == W_PAWN || movedPiece == B_PAWN) {
        currentGameState.halfMoveClock = 0;
    } else {
        currentGameState.halfMoveClock++;
    }

    // Increment fullmove number if black just moved
    if (currentGameState.sideToMove == WHITE) {
        currentGameState.fullMoveNumber++;
    }
    // Add new state to history
    stateHistory.emplace_back(currentGameState);
    
    // New position check value not cached
    hasCachedInCheckValue = false;
    cachedInCheckValue = false;

    if (!searching) {
        moveHistory.emplace_back(move);
    }
}
/// @brief Umake a move on the board (should be the most recent move made)
/// Note: This function does not check validity. It should only be used with known
/// legal moves from MoveGeneration.
/// @param move
/// @param searching If not searching, latest move is removed from moveHistory
void ChessBoard::unmakeMove(DenseMove move, bool searching) {
    // Pop last state, should be state made from this move
    stateHistory.pop_back();
    // Get previous state
    currentGameState = stateHistory.back();
    // // Switch side to move 
    // currentGameState.sideToMove = (Color)!currentGameState.sideToMove;
    // currentGameState.oppColor = (Color)!currentGameState.oppColor;


    Color undoSide = currentGameState.sideToMove;

    // Get move info
    PieceType movedPiece = move.getPieceType();
    Color movedColor = move.getColor();
    int movedFrom = move.getFrom();
    int movedTo = move.getTo();
    PieceType undoCapturedPiece = move.getCaptPiece();
    bool undoCastle = move.isCastle();
    bool undoEnPass = move.isEnPassant();
    PieceType undoPromoPiece = move.getPromotePiece();

    // Undo captures and en passant
    if (undoCapturedPiece != PieceType::EMPTY) {
        // Normal capture
        if (!undoEnPass) {
            addPiece(movedTo, undoCapturedPiece);
        }
        // En passant capture
        else {
            // Add captured pawn
            // En passant square is 'behind' the captured pawn, so we have to calculate it
            int capturedPawnSquare = movedTo + (movedColor == WHITE ? -8 : 8);
            addPiece(capturedPawnSquare, undoCapturedPiece);
        }
    }
    // Undo castling
    else if (undoCastle) {
        bool isKingside = (movedTo > movedFrom);
        // King gets moved at the end of makeMove, so we only need to move the rook
        if (isKingside) {
            // Move rook from f1/f8 to h1/h8
            int rookFrom = (movedPiece == W_KING) ? BoardUtility::F1 : BoardUtility::F8;
            int rookTo = (movedPiece == W_KING) ? BoardUtility::H1 : BoardUtility::H8;
            movePiece(rookFrom, rookTo, (movedPiece == W_KING) ? W_ROOK : B_ROOK);
        } else {
            // Move rook from a1/a8 to d1/d8
            int rookFrom = (movedPiece == W_KING) ? BoardUtility::A1 : BoardUtility::A8;
            int rookTo = (movedPiece == W_KING) ? BoardUtility::D1 : BoardUtility::D8;
            movePiece(rookFrom, rookTo, (movedPiece == W_KING) ? W_ROOK : B_ROOK);
        }
    }
    // Undo promotion
    if (undoPromoPiece != PieceType::EMPTY) {
        // Add pawn to source square
        addPiece(movedFrom, movedPiece);
        // Remove promoted piece from destination square
        removePiece(movedTo, undoPromoPiece);
    } else {
        // Move piece back
        movePiece(movedTo, movedFrom, movedPiece);
    }
    
    // New position check value not cached
    hasCachedInCheckValue = false;

    if (!searching) {
        moveHistory.emplace_back(move);
    }
}
/// @brief Prints a piece bitboard to the console
/// @param i corresponds to DenseType enum
void ChessBoard::printBB(int i) {
    std::bitset<64> bb (pieceBB[i]);
    std::cout << "\n";
    for (int j = 7; j >= 0; j--){
        for (int k = 0; k < 8; k++){
            std::cout << bb[j*8 + k];
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}
/// @brief Prints bitb parameter to the console
/// @param bitb 
void ChessBoard::printBB(U64 bitb) {
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

void ChessBoard::printBoardInfo(bool fullInfo) {
    std::cout << "Initial\nAll pieces: ";
    printBitboard(getAllPieces());
    std::cout << "White pieces: ";
    printBitboard(getWhitePieces());
    std::cout << "Black pieces: ";
    printBitboard(getBlackPieces());
    std::cout << "Empty squares: ";
    printBitboard(getEmptySquares());
    if (fullInfo) {
        std::cout << "White Pawns: ";
        printBitboard(getWhitePawns());
        std::cout << "Black Pawns: ";
        printBitboard(getBlackPawns());
        std::cout << "White Knights: ";
        printBitboard(getWhiteKnights());
        std::cout << "Black Knights: ";
        printBitboard(getBlackKnights());
        std::cout << "White Bishops: ";
        printBitboard(getWhiteBishops());
        std::cout << "Black Bishops: ";
        printBitboard(getBlackBishops());
        std::cout << "White Rooks: ";
        printBitboard(getWhiteRooks());
        std::cout << "Black Rooks: ";
        printBitboard(getBlackRooks());
        std::cout << "White Queens: ";
        printBitboard(getWhiteQueens());
        std::cout << "Black Queens: ";
        printBitboard(getBlackQueens());
        std::cout << "White Kings: ";
        printBitboard(getWhiteKings());
        std::cout << "Black Kings: ";
        printBitboard(getBlackKings());
    }
}
