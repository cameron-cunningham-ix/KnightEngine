// src/board_generation.hpp
#ifndef BOARD_GENERATION_H
#define BOARD_GENERATION_H

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

// Main ChessBoard class declaration
class ChessBoard 
{
private:
    // Array of 64-bit bitboards; corresponds to enumPiece
    U64 pieceBB[8];

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
    U64& getPieceBitboard(PieceType pt);
    U64& getColorBitboard(PieceType pt);
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

#endif // BOARD_GENERATION_H