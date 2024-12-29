#pragma once

#include "types.hpp"
#include <iostream>
#include <bitset>

// Enumeration for piece types on the board
enum enumPiece
{
    nWhite,     // any white piece
    nBlack,     // any black piece
    nPawn,      // all pawns
    nKnight,    // all knights
    nBishop,    // all bishops
    nRook,      // all rooks
    nQueen,     // all queens
    nKing       // all kings
};


class ChessBoard 
{
private:
    
    U64 pieceBB[8];       // Array of 64-bit bitboards; corresponds to enumPieces
    U64 whiteAttacksBB;   // Bitboard of all squares white attacks
    U64 blackAttacksBB;   // Bitboard of all squares black attacks
    
    // Array of bitboards of all attacks to kings
    // Index 0: Direct attacks
    // Index 1: Pin attacks, i.e. attacks that are blocked by one piece
    //          and would be a check if the blocking piece were removed
    // Used for move generation and move validation
    U64 attacksToWhiteKing[2];
    U64 attacksToBlackKing[2];

public:

    // Default constructor declaration
    ChessBoard();

    // Friend function declarations
    friend void movePiece(ChessBoard& board, int from, int to, PieceType piece);
    friend void removePiece(ChessBoard& board, int square, PieceType piece);
    friend void addPiece(ChessBoard& board, int square, PieceType piece);
    friend void makeMove(ChessBoard& board, const Move& move);
    friend void setupPosition(ChessBoard& board, GameState& state, const std::string& fen);

    // Bitboard access methods
    U64 getPieceSet(PieceType pt) const;

    // Piece position getters
    U64 getWhitePawns() const;
    U64 getWhiteKnights() const;
    U64 getWhiteBishops() const;
    U64 getWhiteRooks() const;
    U64 getWhiteQueens() const;
    U64 getWhiteKings() const;
    U64 getBlackPawns() const;
    U64 getBlackKnights() const;
    U64 getBlackBishops() const;
    U64 getBlackRooks() const;
    U64 getBlackQueens() const;
    U64 getBlackKings() const;
    U64 getWhitePieces() const;
    U64 getBlackPieces() const;
    U64 getAllPieces() const;

    // Piece type getter
    PieceType getPieceAt(int index) const;

    // Board initialization methods
    void initializeWhiteBB();
    void initializeBlackBB();
    void initializePawnsBB();
    void initializeKnightsBB();
    void initializeBishopsBB();
    void initializeRooksBB();
    void initializeQueensBB();
    void initializeKingsBB();

    // Bitboard manipulation methods
    void setBit(U64 &bb, int index);
    void clearBit(U64 &bb, int index);
    bool isBitSet(U64 bb, int index);

    // Attack calculation method
    U64 attacksToSquare(int indexOfSquare, Color colorOfKing) const;

    // Debug printing methods
    void printBB(int i);
    void printBB(U64 bitb);
};