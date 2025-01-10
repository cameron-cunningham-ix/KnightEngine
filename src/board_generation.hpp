#pragma once

#include "types.hpp"
#include <iostream>
#include <bitset>
#include <vector>

/// @brief Class representing a current state of a chess board, including game state and
/// previous played moves. 
/// 
class ChessBoard 
{
public:
    /// @brief Default constructor, sets standard position
    ChessBoard();
    /// @brief Sets board and state to position defined by fen string
    /// @param fen 
    /// @note If FEN string is invalid, sets board to standard position
    void setupPositionFromFEN(const std::string &fen);
    /// @return Returns FEN string of the current position of the board
    std::string getFEN();
    /// @brief Prints FEN string of the current position to std::cout
    void printFEN();
    // Current board state
    GameState currentGameState;
    // History of game states
    std::array<GameState, MAX_PLY> stateHistory; /// STACK
    // History of moves
    std::array<DenseMove, MAX_PLY> moveHistory;

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
    /// @param side 
    /// @return 
    U64 getAttacksToKing(Color side) const;
    U64 getAttacksForSide(Color side) const;
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
    bool isSideInCheck(Color side) const;
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
    void printStateHistory();

private:
    U64 pieceBB[7];         // Bitboards for each piece type corresponding to the DenseType enum
    U64 colorBB[2];         // Bitboards of all white pieces and all black pieces
    U64 attacksToKings[2];  // Bitboards for the current attack masks to the kings
    U64 attacksBB[2];       // Bitboards for the current attack masks of each side
    /* Board keeps a cached check value rather than just calculating it per move and storing 
     * that value since there are instances where we 
     *  
     */
    bool hasCachedInCheckValue; // Whether there is a cached inCheck value for this position
    bool cachedInCheckValue;    // In check value for current side to move
    // King squares - 0 = WHITE, 1 - BLACK for easy calculating
    int kingSquares[2];
    int checkingCount;          // The number of pieces currently checking the current side's king
    int plyIndex;               // Current ply; used to index into stateHistory and moveHistory

    

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
    void calculateAttacksForSide(Color side);
};