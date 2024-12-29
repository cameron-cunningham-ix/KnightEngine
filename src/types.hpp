#pragma once

#include <array>
#include <bitset>

typedef unsigned long long U64;     // Used for bitboards
typedef unsigned int U32;           // Used for dense move representation

enum PieceType : unsigned short {
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

enum Color : bool {
    WHITE,      // 0
    BLACK       // 1
};

// Type for use with DenseMove - compare 3 LSB with DenseMove 3 LSB
enum DenseType : unsigned short {
    EMPTY,      // 0 - 000
    PAWN,       // 1 - 001
    KNIGHT,     // 2 - 010
    BISHOP,     // 3 - 011
    ROOK,       // 4 - 100
    QUEEN,      // 5 - 101
    KING        // 6 - 110
};

// Masks for extracting fields from dense move representation
const U32 move_PieceMask =          0b00000000000000000000000111;
const U32 move_ColorMask =          0b00000000000000000000001000;
const U32 move_FromMask =           0b00000000000000001111110000;
const U32 move_ToMask =             0b00000000001111110000000000;
const U32 move_IsCaptureMask =      0b00000000010000000000000000;
const U32 move_CaptureTypeMask =    0b00000011100000000000000000;
const U32 move_IsCastleMask =       0b00000100000000000000000000;
const U32 move_IsEnPassantMask =    0b00001000000000000000000000;
const U32 move_IsPromotionMask =    0b00010000000000000000000000;
const U32 move_PromoteToMask =      0b11100000000000000000000000;

// Move struct using one unsigned 32-bit integer
// Structure:
// [3b]      [1b]      [1b]       [1b]       [3b]            [1b]     [6b] [6b]   [1b]    [3b]
// [PromoTo] [isPromo] [isEnPass] [isCastle] [CaptDenseType] [isCapt] [To] [From] [Color] [DenseType]
struct DenseMove {
    U32 data;

    // Default constructor
    DenseMove() : data(0) {}

    // Constructor with all fields
    DenseMove(DenseType piece, Color color, int from, int to, bool isCapture = false,
              DenseType captureType = EMPTY, bool isCastle = false, bool isEnPassant = false,
              bool isPromotion = false, DenseType promoteTo = EMPTY) {
        data = 0;
        data |= piece;
        data |= (color << 3);
        if (from < 0 || from > 63 || to < 0 || to > 63) {
            throw std::invalid_argument("Invalid square index to DenseMove constructor");
        }
        data |= (from << 4);
        data |= (to << 10);
        data |= (isCapture << 16);
        data |= (captureType << 17);
        data |= (isCastle << 20);
        data |= (isEnPassant << 21);
        data |= (isPromotion << 22);
        data |= (promoteTo << 23);
    }

    // Equality operator
    bool operator==(const DenseMove& other) const {
        return data == other.data;
    }

    // Constructor from unsigned 32-bit integer
    DenseMove(U32 d) : data(d) {}

    // Getters
    DenseType getPiece() const { return static_cast<DenseType>(data & move_PieceMask); }
    Color getColor() const { return static_cast<Color>((data & move_ColorMask) >> 3); }
    int getFrom() const { return (data & move_FromMask) >> 4; }
    int getTo() const { return (data & move_ToMask) >> 10; }
    bool isCapture() const { return (data & move_IsCaptureMask) >> 16; }
    DenseType getCaptureType() const { return static_cast<DenseType>((data & move_CaptureTypeMask) >> 17); }
    bool isCastle() const { return (data & move_IsCastleMask) >> 20; }
    bool isEnPassant() const { return (data & move_IsEnPassantMask) >> 21; }
    bool isPromotion() const { return (data & move_IsPromotionMask) >> 22; }
    DenseType getPromoteTo() const { return static_cast<DenseType>((data & move_PromoteToMask) >> 23); }

    // Setter for promotion type; useful for pawn promotions
    void setPromoteTo(DenseType promoteTo) {
        data &= ~move_PromoteToMask;
        data |= (promoteTo << 23);
    }
};


// Move struct using normal attributes
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

// Struct to hold game state information
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