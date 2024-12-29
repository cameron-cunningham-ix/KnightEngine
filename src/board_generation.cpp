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

// Get the bitboard of a specific piece type, i.e. white pawns, black knights, etc. 
U64 ChessBoard::getPieceSet(PieceType pt) const {
    return pieceBB[pieceCode(pt)] & pieceBB[colorCode(pt)];
}

// Get the bitboard of white pawns
// Note: I could use getPieceSet(W_PAWN), but A. these should never change and B. doing it this way 
U64 ChessBoard::getWhitePawns() const {
    return pieceBB[nWhite] & pieceBB[nPawn];
}

U64 ChessBoard::getWhiteKnights() const {
    return pieceBB[nWhite] & pieceBB[nKnight];
}

U64 ChessBoard::getWhiteBishops() const {
    return pieceBB[nWhite] & pieceBB[nBishop];
}

U64 ChessBoard::getWhiteRooks() const {
    return pieceBB[nWhite] & pieceBB[nRook];
}

U64 ChessBoard::getWhiteQueens() const {
    return pieceBB[nWhite] & pieceBB[nQueen];
}

U64 ChessBoard::getWhiteKings() const {
    return pieceBB[nWhite] & pieceBB[nKing];
}

U64 ChessBoard::getBlackPawns() const {
    return pieceBB[nBlack] & pieceBB[nPawn];
}

U64 ChessBoard::getBlackKnights() const {
    return pieceBB[nBlack] & pieceBB[nKnight];
}

U64 ChessBoard::getBlackBishops() const {
    return pieceBB[nBlack] & pieceBB[nBishop];
}

U64 ChessBoard::getBlackRooks() const {
    return pieceBB[nBlack] & pieceBB[nRook];
}

U64 ChessBoard::getBlackQueens() const {
    return pieceBB[nBlack] & pieceBB[nQueen];
}

U64 ChessBoard::getBlackKings() const {
    return pieceBB[nBlack] & pieceBB[nKing];
}

U64 ChessBoard::getWhitePieces() const {
    return pieceBB[nWhite];
}

U64 ChessBoard::getBlackPieces() const {
    return pieceBB[nBlack];
}

U64 ChessBoard::getAllPieces() const {
    return pieceBB[nWhite] | pieceBB[nBlack];
}

PieceType ChessBoard::getPieceAt(int index) const {
    // First check if the square is empty
    if (!(getAllPieces() & (1ULL << index))) {
        return PieceType(-1);
    }

    // Check color of piece
    bool isBlack = getBlackPieces() & (1ULL << index);

    // Get piece type by checking each piece bitboard
    if (pieceBB[nPawn] & (1ULL << index)) {
        return isBlack ? B_PAWN : W_PAWN;
    }
    if (pieceBB[nKnight] & (1ULL << index)) {
        return isBlack ? B_KNIGHT : W_KNIGHT;
    }
    if (pieceBB[nBishop] & (1ULL << index)) {
        return isBlack ? B_BISHOP : W_BISHOP;
    }
    if (pieceBB[nRook] & (1ULL << index)) {
        return isBlack ? B_ROOK : W_ROOK;
    }
    if (pieceBB[nQueen] & (1ULL << index)) {
        return isBlack ? B_QUEEN : W_QUEEN;
    }
    if (pieceBB[nKing] & (1ULL << index)) {
        return isBlack ? B_KING : W_KING;
    }

    // Should never reach here if bitboards are valid
    return PieceType(-1);
}

void ChessBoard::initializeWhiteBB() {
    pieceBB[nWhite] = 0xFFFF;
}

void ChessBoard::initializeBlackBB() {
    pieceBB[nBlack] = 0xFFFF000000000000;
}

void ChessBoard::initializePawnsBB() {
    pieceBB[nPawn] = 0x00FF00000000FF00;
}

void ChessBoard::initializeKnightsBB() {
    pieceBB[nKnight] = 0x4200000000000042;
}

void ChessBoard::initializeBishopsBB() {
    pieceBB[nBishop] = 0x2400000000000024;
}

void ChessBoard::initializeRooksBB() {
    pieceBB[nRook] = 0x8100000000000081;
}

void ChessBoard::initializeQueensBB() {
    pieceBB[nQueen] = 0x800000000000008;
}

void ChessBoard::initializeKingsBB() {
    pieceBB[nKing] = 0x1000000000000010;
}

// Set a bit at a given index
void ChessBoard::setBit(U64 &bb, int index) {
    // bitboard ORed with 1, left shifted by 'index' number of bits
    if (index >= 0 && index < 64)
        bb |= (1ULL << index);
}

// Clear a bit at a given index
void ChessBoard::clearBit(U64 &bb, int index) {
    // bitboard ANDed with the negation of 1 left shifted by 'index' number of bits
    // ~(1ULL << index) gives you all 1's except for 0 bit at 'index'
    if (index >= 0 && index < 64)
        bb &= ~(1ULL << index);
}

// Check if a bit is set at a given index
bool ChessBoard::isBitSet(U64 bb, int index) {
    // bitboard ANDed with single 1 bit at position 'index'
    return bb & (1ULL << index);
}


U64 ChessBoard::attacksToSquare(int indexOfSquare, Color colorOfKing) const {
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

    return (colorOfKing == WHITE ? pawnAttacksWhite[indexOfSquare] : pawnAttacksBlack[indexOfSquare]) & opPawns |
            knightAttacks[indexOfSquare] & opKnights |
            getRookAttacks(indexOfSquare, occupancy) & opR |
            getBishopAttacks(indexOfSquare, occupancy) & opB |
            getQueenAttacks(indexOfSquare, occupancy) & opQ |
            kingAttacks[indexOfSquare] & opK;
}

// Prints the bitboard to the console 
// Bottom left: a1, top right: h8
void ChessBoard::printBB(int i){
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

