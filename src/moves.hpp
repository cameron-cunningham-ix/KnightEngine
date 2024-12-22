// src/moves.hpp
#ifndef MOVES_H
#define MOVES_H

#include "types.hpp"
#include "board_generation.hpp"
#include "utility.hpp"
#include <vector>
#include <array>
#include <string>



// Precomputed attack masks for each piece at every square
// on the board
extern std::array<U64, 64> pawnAttacksWhite;
extern std::array<U64, 64> pawnAttacksBlack;
extern std::array<U64, 64> knightAttacks;
extern std::array<U64, 64> bishopAttacks;
extern std::array<U64, 64> rookAttacks;
extern std::array<U64, 64> queenAttacks;
extern std::array<U64, 64> kingAttacks;

// Move validator class declaration
class MoveValidator {
private:
    ChessBoard board;
    GameState* state;
    
    bool moveLeavesKingInCheck(const Move& move);
    bool isValidCastling(const Move& move);
    bool isValidEnPassant(const Move& move);
    int findKingSquare(Color color) const;
    bool canMakeMove(Move& move, Color color);
    
public:
    MoveValidator(ChessBoard& b, GameState* s);
    bool isSquareUnderAttack(int square, Color attackingColor);
    bool isInCheck(Color color) const;
    bool isCheckmate(Color color);
    bool isStalemate(Color color);
    bool isMoveLegal(const Move& move);
    void updateGameState(const Move& move);
};

// Move generation function declarations
void generatePawnMoves(const ChessBoard& cboard, const GameState* state, std::vector<Move>& moves);
void generateEnPassantMoves(const ChessBoard &cboard, const GameState* state, std::vector<Move> &moves);
void generateCastlingMoves(const ChessBoard& cboard, const GameState* state, std::vector<Move>& moves);
void generatePieceMoves(const ChessBoard &cboard, std::vector<Move> &moves, PieceType piece);
std::vector<Move> generatePsuedoMoves(const ChessBoard &cboard, const GameState* state);

// Move execution function declarations
void movePiece(ChessBoard& board, int from, int to, PieceType piece);
void removePiece(ChessBoard& board, int square, PieceType piece);
void addPiece(ChessBoard& board, int square, PieceType piece);
void makeMove(ChessBoard& board, const Move& move);
PieceType getPieceOnSquare(const ChessBoard& board, int square);

// Attack computation function declarations
U64 getRookAttacks(int square, U64 blockers);
U64 getBishopAttacks(int square, U64 blockers);
U64 getQueenAttacks(int square, U64 blockers);

// Board position helper function declarations
bool isWithinBoard(int index);
void printMove(const Move &move);

// Attack table initialization functions
void initializePawnAttacksWhite();
void initializePawnAttacksBlack();
void initializeKnightAttacks();
void initializeBishopAttacks();
void initializeRookAttacks();
void initializeQueenAttacks();
void initializeKingAttacks();
void initializeAllAttacks();

#endif