#pragma once

#include <array>
#include <bitset>

typedef unsigned long long U64;

enum PieceType{
    W_PAWN,     // 0
    W_KNIGHT,   // 1
    W_BISHOP,   // 2
    W_ROOK,     // 3
    W_QUEEN,    // 4
    W_KING,     // 5
    B_PAWN,     // 6     
    B_KNIGHT,   // 7
    B_BISHOP,   // 8
    B_ROOK,     // 9
    B_QUEEN,    // 10
    B_KING,     // 11
};

enum Color{
    WHITE,      // 0
    BLACK       // 1
};

struct Move {
    PieceType piece;
    int from;
    int to;
    bool isCapture;
    bool isPromotion;
    bool isCastle;
    bool isEnPassant;
    PieceType promoteTo;
    
    Move(PieceType p, int f, int t, bool cap = false, bool prom = false) :
        piece(p), from(f), to(t), isCapture(cap), isPromotion(prom),
        isCastle(false), isEnPassant(false), promoteTo(W_QUEEN) {}

    // Default constructor
    Move() : piece(W_PAWN), from(-1), to(-1), isCapture(false), isPromotion(false),
             isCastle(false), isEnPassant(false), promoteTo(W_QUEEN) {}

    bool operator==(const Move& other) const {
        return piece == other.piece && from == other.from && to == other.to &&
               isCapture == other.isCapture && isPromotion == other.isPromotion &&
               isCastle == other.isCastle && isEnPassant == other.isEnPassant &&
               promoteTo == other.promoteTo;
    }
};

struct GameState {
    // Castling rights for each color and side
    bool canCastleWhiteKingside;   // White king's side (h1)
    bool canCastleWhiteQueenside;  // White queen's side (a1)
    bool canCastleBlackKingside;   // Black king's side (h8)
    bool canCastleBlackQueenside;  // Black queen's side (a8)
    
    // Track en passant possibility
    int enPassantSquare;           // Square where en passant capture is possible (-1 if none)
    
    // Current turn
    Color sideToMove;              // Whose turn it is
    
    int halfMoveClock;             // How long its been since the last pawn push or piece capture (useful for determining 50-move draw rule)
    int fullMoveNumber;            // Number of the move currently being played (starts at 1, incremented when Black plays a move)

    // Constructor with default values
    GameState() : 
        canCastleWhiteKingside(true),
        canCastleWhiteQueenside(true),
        canCastleBlackKingside(true),
        canCastleBlackQueenside(true),
        enPassantSquare(-1),
        sideToMove(WHITE),
        halfMoveClock(0),
        fullMoveNumber(1) {}
};