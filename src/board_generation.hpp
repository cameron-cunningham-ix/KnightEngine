#pragma once

#include "types.hpp"
#include <iostream>
#include <bitset>
#include <vector>

/// @brief Class representing a current state of a chess board, including game state and
/// move history
class ChessBoard 
{
public:
    /// @brief Default constructor, sets standard position
    ChessBoard();

    void setupPositionFromFEN(const std::string &fen);
    std::string getFEN();
    void printFEN();
    // Current board state
    GameState currentGameState;
    // History of game states
    std::vector<GameState> stateHistory;
    // History of moves
    std::vector<DenseMove> moveHistory;

    // Bitboard getters
    U64 getPieceSet(PieceType pt) const;
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
    U64 getAttacksToKing(Color side) const;
    U64 getOrthogonalOpp(Color side) const;
    U64 getDiagonalOpp(Color side) const;
    // King square getters
    int getWhiteKingSquare() const;
    int getBlackKingSquare() const;
    // State getters
    Color getSideToMove() const;
    Color getOppSide() const;
    
    // Piece type getter
    PieceType getPieceAt(int index) const;
    DenseType getDenseTypeAt(int index) const;

    // Whether current side to move is in check
    bool isInCheck();
    // Whether 'side' is in check
    bool isSideInCheck(Color side);
    // How many pieces are checking current side's king
    int getCheckCount() const;

    // Attack calculation methods
    U64 OppAttacksToSquare(int indexOfSquare, Color colorOfKing) const;

    // Public board alteration methods
    void makeMove(DenseMove move, bool searching);
    void unmakeMove(DenseMove move, bool searching);

    // Debug printing methods
    void printBB(int i);
    void printBB(U64 bitb);
    void printBoardInfo(bool fullInfo = true);

private:
    U64 pieceBB[7];         // Bitboards for each piece type corresponding to the DenseType enum
    U64 colorBB[2];         // Bitboards of all white pieces and all black pieces
    U64 attacksToKings[2];  // Bitboards for the current attack masks to the kings
    /* Board keeps a cached check value rather than just calculating it per move and storing 
     * that value since there are instances where we 
     *  
     */
    bool hasCachedInCheckValue; // Whether there is a cached inCheck value for this position
    bool cachedInCheckValue;    // In check value for current side to move
    // King squares - 0 = WHITE, 1 - BLACK for easy calculating
    int kingSquares[2];
    int checkingCount;          // The number of pieces currently checking the current side's king

    

    // Normal board initialization methods
    void initializeWhiteBB();
    void initializeBlackBB();
    void initializeEmptyBB();
    void initializePawnsBB();
    void initializeKnightsBB();
    void initializeBishopsBB();
    void initializeRooksBB();
    void initializeQueensBB();
    void initializeKingsBB();
    void initializeGameState();


    // Private board alteration functions
    void movePiece(int from, int to, PieceType piece);
    void removePiece(int square, PieceType piece);
    void addPiece(int square, PieceType piece);

    bool calculateIsInCheck();
};