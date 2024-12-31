#include "board_generation.hpp"
#include "types.hpp"
#include "moves.hpp"
#include "utility.hpp"
#include <iostream>
#include <string>
#include <stdlib.h>


// Default constructor; initialize all bitboards
ChessBoard::ChessBoard(){
    initializeWhiteBB();
    initializeBlackBB();
    initializePawnsBB();
    initializeKnightsBB();
    initializeBishopsBB();
    initializeRooksBB();
    initializeQueensBB();
    initializeKingsBB();
}

// Get the bitboard of a specific piece type
U64 ChessBoard::getPieceSet(PieceType pt) const {
    if (pt == EMPTY) return pieceBB[D_EMPTY];
    if (pt == INVALID) return 0ULL;
    return pieceBB[pieceCode(pt)] & colorBB[colorCode(pt)];
}

// Private helper function to update bitboards for piece movement
// Should only be used in functions that ensure the move is valid
void ChessBoard::movePiece(int from, int to, PieceType piece) {
    U64 fromToBB = (1ULL << from) | (1ULL << to);  // BB with both squares set

    // Get color and piece type
    bool isWhite = piece <= W_KING;
    int pieceType = pieceCode(piece);

    // Update color bitboard (white or black)
    colorBB[isWhite ? 0 : 1] ^= fromToBB;

    // Update piece type bitboard (pawn, knight, etc.)
    pieceBB[pieceType] ^= fromToBB;
    // Update empty squares bitboard
    pieceBB[D_EMPTY] ^= fromToBB;
}

// Helper function to remove a piece from the board
// Should only be used in functions that ensure the move is valid
void ChessBoard::removePiece(int square, PieceType piece) {
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

// Helper function to add a piece to the board
// Should only be used in functions that ensure the move is valid
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

/// @brief Update the attack bitboards (whiteAttacksBB, blackAttacksBB and attacksToWhiteKing, attacksToBlackKing)
void ChessBoard::updateAttacksBitBoards()
{
    
}

// Note: I could use getPieceSet(W_PAWN) for these getters, but A. these should
// never change and B. doing it this way keeps from unnecessary steps in getPieceSet

// Get the bitboard of white pawns
U64 ChessBoard::getWhitePawns() const {
    return colorBB[WHITE] & pieceBB[D_PAWN];
}
// Get the bitboard of white knights
U64 ChessBoard::getWhiteKnights() const {
    return colorBB[WHITE] & pieceBB[D_KNIGHT];
}
// Get the bitboard of white bishops
U64 ChessBoard::getWhiteBishops() const {
    return colorBB[WHITE] & pieceBB[D_BISHOP];
}
// Get the bitboard of white rooks
U64 ChessBoard::getWhiteRooks() const {
    return colorBB[WHITE] & pieceBB[D_ROOK];
}
// Get the bitboard of white queens
U64 ChessBoard::getWhiteQueens() const {
    return colorBB[WHITE] & pieceBB[D_QUEEN];
}
// Get the bitboard of white kings
U64 ChessBoard::getWhiteKings() const {
    return colorBB[WHITE] & pieceBB[D_KING];
}
// Get the bitboard of black pawns
U64 ChessBoard::getBlackPawns() const {
    return colorBB[BLACK] & pieceBB[D_PAWN];
}
// Get the bitboard of black knights
U64 ChessBoard::getBlackKnights() const {
    return colorBB[BLACK] & pieceBB[D_KNIGHT];
}
// Get the bitboard of black bishops
U64 ChessBoard::getBlackBishops() const {
    return colorBB[BLACK] & pieceBB[D_BISHOP];
}
// Get the bitboard of black rooks
U64 ChessBoard::getBlackRooks() const {
    return colorBB[BLACK] & pieceBB[D_ROOK];
}
// Get the bitboard of black queens
U64 ChessBoard::getBlackQueens() const {
    return colorBB[BLACK] & pieceBB[D_QUEEN];
}
// Get the bitboard of black kings
U64 ChessBoard::getBlackKings() const {
    return colorBB[BLACK] & pieceBB[D_KING];
}
// Get the bitboard of all white pieces
U64 ChessBoard::getWhitePieces() const {
    return colorBB[WHITE];
}
// Get the bitboard of all black pieces
U64 ChessBoard::getBlackPieces() const {
    return colorBB[BLACK];
}
// Get the bitboard of all pieces
U64 ChessBoard::getAllPieces() const {
    return colorBB[WHITE] | colorBB[BLACK];
}
// Get the bitboard of all empty squares
U64 ChessBoard::getEmptySquares() const {
    return pieceBB[EMPTY];
}

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

void ChessBoard::initializeWhiteBB() {
    colorBB[WHITE] = 0xFFFF;
}

void ChessBoard::initializeBlackBB() {
    colorBB[BLACK] = 0xFFFF000000000000;
}

void ChessBoard::initializeEmptyBB() {
    pieceBB[D_EMPTY] = 0x0000FFFFFFFF0000;
}

void ChessBoard::initializePawnsBB() {
    pieceBB[D_PAWN] = 0x00FF00000000FF00;
}

void ChessBoard::initializeKnightsBB() {
    pieceBB[D_KNIGHT] = 0x4200000000000042;
}

void ChessBoard::initializeBishopsBB() {
    pieceBB[D_BISHOP] = 0x2400000000000024;
}

void ChessBoard::initializeRooksBB() {
    pieceBB[D_ROOK] = 0x8100000000000081;
}

void ChessBoard::initializeQueensBB() {
    pieceBB[D_QUEEN] = 0x800000000000008;
}

void ChessBoard::initializeKingsBB() {
    pieceBB[D_KING] = 0x1000000000000010;
}

// Set a bit at a given index
void ChessBoard::setBit(U64 &bb, int index) {
    // bitboard ORed with 1, left shifted by 'index' number of bits
    if (index >= 0 && index < 64)
        bb |= (1ULL << index);
    else {
        std::cerr << "ChessBoard setBit Error: Invalid index " << index << "\n";
    }
}

// Clear a bit at a given index
void ChessBoard::clearBit(U64 &bb, int index) {
    // bitboard ANDed with the negation of 1 left shifted by 'index' number of bits
    // ~(1ULL << index) gives you all 1's except for 0 bit at 'index'
    if (index >= 0 && index < 64)
        bb &= ~(1ULL << index);
    else {
        std::cerr << "ChessBoard clearBit Error: Invalid index " << index << "\n";
    }
}

// Check if a bit is set at a given index
bool ChessBoard::isBitSet(U64 bb, int index) {
    // bitboard ANDed with single 1 bit at position 'index'
    if (index < 0 || index > 63) {
        std::cerr << "ChessBoard isBitSet Error: Invalid index " << index << "\n";
        return false;
    }
    return bb & (1ULL << index);
}


U64 ChessBoard::OppAttacksToSquare(int index, Color colorOfKing) const {
    // Get opposing pieces based on king's color
    U64 opPawns, opKnights, opQ, opB, opR, opK, occupancy;
    if (colorOfKing == WHITE) {
        // Get all black pieces that could attack white king
        opPawns = getBlackPawns();
        opKnights = getBlackKnights();
        opQ = getBlackQueens();    // Queens can move like rooks and bishops
        opR = getBlackRooks();           // Add rooks to rook+queen attack mask
        opB = getBlackBishops();         // Add bishops to bishop+queen attack mask
        opK = getBlackKings();
    } else {
        // Get all white pieces that could attack black king
        opPawns = getWhitePawns();
        opKnights = getWhiteKnights();
        opQ = getWhiteQueens();    // Queens can move like rooks and bishops
        opR = getWhiteRooks();           // Add rooks to rook+queen attack mask
        opB = getWhiteBishops();         // Add bishops to bishop+queen attack mask
        opK = getWhiteKings();
    }

    // Get occupancy of all pieces for blocking calculations
    occupancy = getAllPieces();

    return (colorOfKing == WHITE ? ATKMASK_WPAWN[index] : ATKMASK_BPAWN[index]) & opPawns |
            ATKMASK_KNIGHT[index] & opKnights |
            getRookAttacks(index, occupancy) & opR |
            getBishopAttacks(index, occupancy) & opB |
            getQueenAttacks(index, occupancy) & opQ |
            ATKMASK_KING[index] & opK;
}

/// @brief Alter bitboards to reflect a move. Returns true if move is valid, false otherwise.
/// Note: This function checks validity based on board position only, not game state such as
/// castling rights, en passant, etc.
/// @param move
/// @param safeMode if true, will check if move is valid before making it
/// @return true if move is made, false otherwise. Move is always made if safeMode is false.
bool ChessBoard::makeMove(DenseMove move, bool safeMode = true) {
    PieceType movedPiece = move.getPieceType();
    Color movedColor = move.getColor();
    int from = move.getFrom();
    int to = move.getTo();

    // Handle invalid or empty move
    if (safeMode) {
        if (movedPiece == PieceType::EMPTY || 
            movedPiece == PieceType::INVALID) {
            std::cerr << "ChessBoard makeMove Error: Empty or invalid move\n";
            return false;
        }
        // Handle out of bounds move
        if (from < 0 || from > 63 || 
            to < 0 || to > 63) {
            std::cerr << "ChessBoard makeMove Error: Out of bounds move\n";
            return false;
        }
    }
    // Handle captures and en passant
    if (move.isCapture()) {
        PieceType capturedPiece = getPieceAt(to);
        // Handle invalid captures
        if (safeMode) {
            if (capturedPiece == PieceType::EMPTY || 
                capturedPiece == PieceType::INVALID) {
                std::cerr << "ChessBoard makeMove Error: Empty or invalid capture\n";
                return false;
            } else if (capturedPiece != move.getCaptPiece()) {
                std::cerr << "ChessBoard makeMove Error: Wrong capture type\n";
                return false;
            } else if (colorCode(capturedPiece) == movedColor) {
                std::cerr << "ChessBoard makeMove Error: Invalid friendly capture\n";
                return false;
            }
        }
        // Normal capture
        if (!move.isEnPassant()) {
            removePiece(to, capturedPiece);
        }
        // En passant capture
        else {
            // Can only capture en passant if both are pawns
            if (safeMode &&
                ((movedPiece != W_PAWN && movedPiece != B_PAWN) ||
                (capturedPiece != W_PAWN && capturedPiece != B_PAWN))) {
                std::cerr << "ChessBoard makeMove Error: Invalid en passant capture\n";
                return false;
            }
            // Remove captured pawn
            // En passant square is 'behind' the captured pawn, so we have to calculate it
            int capturedPawnSquare = to + (movedColor == WHITE ? -8 : 8);
            removePiece(capturedPawnSquare, capturedPiece);
        }
    }
    // Handle castling
    else if (move.isCastle()) {
        bool isKingside = (to > from);
        if (safeMode) {
            // Verify king position
            if (movedPiece == W_KING) {
                U64 kingBB = getWhiteKings();
                if (kingBB != (1ULL << 4)) {
                    std::cerr << "ChessBoard makeMove Error: Invalid white king position\n";
                    return false;
                }
                if (isKingside) {
                    // Verify rook position
                    U64 rookBB = getWhiteRooks();
                    if (rookBB != (1ULL << 7)) {
                        std::cerr << "ChessBoard makeMove Error: Invalid short white rook position\n";
                        return false;
                    }
                    // Ensure no pieces between king and rook
                    if (getAllPieces() & W_KINGSIDECASTLEMASK) {
                        std::cerr << "ChessBoard makeMove Error: Pieces between king and rook\n";
                        return false;
                    }
                } else {
                    // Verify rook position
                    U64 rookBB = getWhiteRooks();
                    if (rookBB != 1ULL) {
                        std::cerr << "ChessBoard makeMove Error: Invalid long white rook position\n";
                        return false;
                    }
                    // Ensure no pieces between king and rook
                    if (getAllPieces() & W_QUEENSIDECASTLEMASK) {
                        std::cerr << "ChessBoard makeMove Error: Pieces between king and rook\n";
                        return false;
                    }
                }
            } else if (movedPiece == B_KING) {
                U64 kingBB = getBlackKings();
                if (kingBB != (1ULL << 60)) {
                    std::cerr << "ChessBoard makeMove Error: Invalid black king position\n";
                    return false;
                }
                if (isKingside) {
                    // Verify rook position
                    U64 rookBB = getBlackRooks();
                    if (rookBB != (1ULL << 63)) {
                        std::cerr << "ChessBoard makeMove Error: Invalid short black rook position\n";
                        return false;
                    }
                    // Ensure no pieces between king and rook
                    if (getAllPieces() & B_KINGSIDECASTLEMASK) {
                        std::cerr << "ChessBoard makeMove Error: Pieces between king and rook\n";
                        return false;
                    }
                } else {
                    // Verify rook position
                    U64 rookBB = getBlackRooks();
                    if (rookBB != (1ULL << 56)) {
                        std::cerr << "ChessBoard makeMove Error: Invalid long black rook position\n";
                        return false;
                    }
                    // Ensure no pieces between king and rook
                    if (getAllPieces() & B_QUEENSIDECASTLEMASK) {
                        std::cerr << "ChessBoard makeMove Error: Pieces between king and rook\n";
                        return false;
                    }
                }
            } else {
                std::cerr << "ChessBoard makeMove Error: Invalid king piece type\n";
                return false;
            }
        }

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
    if (move.isPromotion()) {
        PieceType promoteTo = move.getPromotePiece();
        if (safeMode) {
            // Verify promotion piece
            if (promoteTo == PieceType::EMPTY || 
                promoteTo == PieceType::INVALID ||
                promoteTo == W_KING || promoteTo == B_KING ||
                promoteTo == W_PAWN || promoteTo == B_PAWN) {
                std::cerr << "ChessBoard makeMove Error: Invalid promotion piece\n";
                return false;
            }
            // Verify promotion rank
            if (movedPiece == W_PAWN) {
                if (to / 8 != 7) {
                    std::cerr << "ChessBoard makeMove Error: Invalid white pawn promotion rank\n";
                    return false;
                }
            } else if (movedPiece == B_PAWN) {
                if (to / 8 != 0) {
                    std::cerr << "ChessBoard makeMove Error: Invalid black pawn promotion rank\n";
                    return false;
                }
            } else {
                std::cerr << "ChessBoard makeMove Error: Invalid promotion piece type\n";
                return false;
            }
        }
        // Remove pawn from source square
        removePiece(from, movedPiece);
        // Add promoted piece to destination square
        addPiece(to, promoteTo);
        return true;
    }
    // Move piece
    movePiece(from, to, movedPiece);
    return true;
}

// Prints the bitboard to the console 
// Bottom left: a1, top right: h8
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

// Prints the bitboard to the console 
// Bottom left: a1, top right: h8
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

