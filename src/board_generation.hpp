#pragma once

#include "types.hpp"
#include <iostream>
#include <bitset>

/// @brief Class representing a chess board. Includes GameState information.
class ChessBoard 
{
private:
    U64 pieceBB[7];       // Bitboard for each piece type corresponding to the DenseType enum
    U64 colorBB[2];       // Bitboard of all white pieces and all black pieces
    U64 whiteAttacksBB;   // Bitboard of all squares white attacks
    U64 blackAttacksBB;   // Bitboard of all squares black attacks
    
    // Array of bitboards of all attacks to kings
    // Index 0: Direct attacks
    // Index 1: Pin attacks, i.e. attacks that are blocked by one piece
    //          and would be a check if the blocking piece were removed
    // Used for move generation and move validation
    U64 attacksToWhiteKing[2];
    U64 attacksToBlackKing[2];

    // Board utility
    static const U64 W_KINGSIDECASTLEMASK =  0x0000000000000060; // 
    static const U64 W_QUEENSIDECASTLEMASK = 0x0000000000000006; //
    static const U64 B_KINGSIDECASTLEMASK =  0x6000000000000000; //
    static const U64 B_QUEENSIDECASTLEMASK = 0x0600000000000000; //

    
    // Bitboard manipulation methods
    void setBit(U64 &bb, int index);
    void clearBit(U64 &bb, int index);
    bool isBitSet(U64 bb, int index);

    // Private board alteration functions
    void movePiece(int from, int to, PieceType piece);
    void removePiece(int square, PieceType piece);
    void addPiece(int square, PieceType piece);
    void updateAttacksBitBoards();

public:

    // Default constructor declaration
    ChessBoard();

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
    U64 getEmptySquares() const;

    // Piece type getter
    PieceType getPieceAt(int index) const;

    // Board initialization methods
    void initializeWhiteBB();
    void initializeBlackBB();
    void initializeEmptyBB();
    void initializePawnsBB();
    void initializeKnightsBB();
    void initializeBishopsBB();
    void initializeRooksBB();
    void initializeQueensBB();
    void initializeKingsBB();


    // Attack calculation methods
    U64 OppAttacksToSquare(int indexOfSquare, Color colorOfKing) const;

    // Public board alteration methods
    bool makeMove(DenseMove move, bool safeMode = true);
    void unmakeMove(DenseMove move);

    // Debug printing methods
    void printBB(int i);
    void printBB(U64 bitb);
};