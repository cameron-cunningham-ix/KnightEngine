// src/utility.hpp
#ifndef UTILITY_H
#define UTILITY_H

#include "types.hpp"
#include "board_generation.hpp"
#include "moves.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <format>
#include <map>
#include <charconv>
#include <cstdint>
#include <vector>
#include <bit>

struct PerftMetrics {
    uint64_t nodes;      // Number of 'leaf' nodes
    uint64_t captures;
    uint64_t enPassants;
    uint64_t castles;
    uint64_t promotions;
    uint64_t checks;
    uint64_t checkmates;

    PerftMetrics(uint64_t node = 0, uint64_t capture = 0, uint64_t enPassant = 0,
                 uint64_t castle = 0, uint64_t promotion = 0, uint64_t check = 0,
                 uint64_t checkmate = 0) : 
                 nodes(node), captures(capture), enPassants(enPassant), castles(castle),
                 promotions(promotion), checks(check), checkmates(checkmate) {};

    

    //bool operator==(const PerftMetrics& other) const = default;

    PerftMetrics& operator+=(const PerftMetrics& other) {
        nodes += other.nodes;
        captures += other.captures;
        enPassants += other.enPassants;
        castles += other.castles;
        promotions += other.promotions;
        checks += other.checks;
        checkmates += other.checkmates;
        return *this;
    }
};

// Map to convert FEN piece characters to PieceType
const std::map<char, PieceType> fenToPiece = {
    {'P', W_PAWN}, {'N', W_KNIGHT}, {'B', W_BISHOP},
    {'R', W_ROOK}, {'Q', W_QUEEN}, {'K', W_KING},
    {'p', B_PAWN}, {'n', B_KNIGHT}, {'b', B_BISHOP},
    {'r', B_ROOK}, {'q', B_QUEEN}, {'k', B_KING}
};

// Map to convert PieceType to FEN piece character strings
const std::map<PieceType, std::string> pieceToFEN = {
    {W_PAWN, "P"}, {W_KNIGHT, "N"}, {W_BISHOP, "B"},
    {W_ROOK, "R"}, {W_QUEEN, "Q"}, {W_KING, "K"},
    {B_PAWN, "p"}, {B_KNIGHT, "n"}, {B_BISHOP, "b"},
    {B_ROOK, "r"}, {B_QUEEN, "q"}, {B_KING, "k"},
};

// Map to convert PieceType to SAN piece character strings
const std::map<PieceType, std::string> pieceToSAN = {
    {W_KNIGHT, "N"}, {W_BISHOP, "B"},
    {W_ROOK, "R"}, {W_QUEEN, "Q"}, {W_KING, "K"},
    {B_KNIGHT, "N"}, {B_BISHOP, "B"},
    {B_ROOK, "R"}, {B_QUEEN, "Q"}, {B_KING, "K"},
};

int pieceCode(PieceType ps);
int colorCode(PieceType ps);
void printBitboard(U64 bitb);
void printBBLine(U64 bitb);
int algebraicToIndex(const std::string& square);
std::string indexToAlgebraic(int index);
void setupPosition(ChessBoard& board, GameState& state, const std::string& fen);
void printFEN(ChessBoard board, GameState state);
bool isValidFEN(const std::string& fen);
int countLegalMoves(ChessBoard& board, GameState* state);
bool isCheckmate(ChessBoard& board, GameState* state);
bool isStalemate(ChessBoard& board, GameState* state);
uint64_t perft(ChessBoard& board, GameState* state, int depth);
PerftMetrics calcPerftMetrics(ChessBoard& board, GameState* state, int depth);
void setupTestPosition(ChessBoard& board, GameState& state, const std::string& positionName);
bool verifyAttackPattern(const ChessBoard& board, int square, 
                        const std::vector<std::string>& expectedAttacks);


#endif