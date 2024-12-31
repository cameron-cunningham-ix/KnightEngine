#pragma once

#include <stdexcept>
#include <array>
#include <bitset>

typedef unsigned long long U64;     // Used for bitboards
typedef unsigned int U32;           // Used for dense move representation

// Enum for piece types on the board + empty
// 3 LSB are used to represent the piece type, 4th bit is used to represent the color
enum PieceType : int {
    EMPTY = 0,      // 0  - 0000
    INVALID = 8,    // 8  - 1000
    W_PAWN = 1,     // 1  - 0001 
    W_KNIGHT = 2,   // 2  - 0010 
    W_BISHOP = 3,   // 3  - 0011 
    W_ROOK = 4,     // 4  - 0100 
    W_QUEEN = 5,    // 5  - 0101 
    W_KING = 6,     // 6  - 0110 
    B_PAWN = 9,     // 9  - 1001 
    B_KNIGHT = 10,  // 10 - 1010 
    B_BISHOP = 11,  // 11 - 1011 
    B_ROOK = 12,    // 12 - 1100 
    B_QUEEN = 13,   // 13 - 1101
    B_KING = 14     // 14 - 1110
};

enum Color : bool {
    WHITE = 0,      // 0
    BLACK = 1       // 1
};

// Type for use with board bitboards and DenseMove - compare 3 LSB with DenseMove 3 LSB
enum DenseType : int {
    D_EMPTY = 0,      // 0 - 000
    D_PAWN = 1,       // 1 - 001
    D_KNIGHT = 2,     // 2 - 010
    D_BISHOP = 3,     // 3 - 011
    D_ROOK = 4,       // 4 - 100
    D_QUEEN = 5,      // 5 - 101
    D_KING = 6        // 6 - 110
};

// Masks for extracting fields from dense move representation
const U32 moveMask_DType =     0b00000000000000000000000111;
const U32 moveMask_Color =     0b00000000000000000000001000;
const U32 moveMask_Piece =     0b00000000000000000000001111;
const U32 moveMask_From =      0b00000000000000001111110000;
const U32 moveMask_To =        0b00000000001111110000000000;
const U32 moveMask_IsCapture = 0b00000000010000000000000000;
const U32 moveMask_CaptType =  0b00000011100000000000000000;
const U32 moveMask_IsCastle =  0b00000100000000000000000000;
const U32 moveMask_IsEnPass =  0b00001000000000000000000000;
const U32 moveMask_IsPromo =   0b00010000000000000000000000;
const U32 moveMask_PromoTo =   0b11100000000000000000000000;

// Move struct using one unsigned 32-bit integer
// Structure:
// [3b]      [1b]      [1b]       [1b]       [3b]            [1b]     [6b] [6b]   [1b]    [3b]
// [PromoTo] [isPromo] [isEnPass] [isCastle] [CaptDenseType] [isCapt] [To] [From] [Color] [DenseType]
struct DenseMove {
    U32 data;

    // Default constructor
    DenseMove() : data(0U) {}

    // Constructor with all fields
    DenseMove(DenseType piece, Color color, int from, int to, bool isCapture = false,
              DenseType captureType = DenseType::D_EMPTY, bool isCastle = false, bool isEnPassant = false,
              bool isPromotion = false, DenseType promoteTo = DenseType::D_EMPTY) {
        data = 0;
        data |= piece;
        data |= (color << 3);
        if (from < 0 || from > 63 || to < 0 || to > 63) {
            throw std::invalid_argument("Invalid square index");
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
    
    // Constructor from unsigned 32-bit integer
    DenseMove(U32 d) : data(d) {}

    // Equality operator
    bool operator==(const DenseMove& other) const {
        return data == other.data;
    }


    // Getters
    DenseType getDenseType() const { return static_cast<DenseType>(data & moveMask_DType); }
    PieceType getPieceType() const { return static_cast<PieceType>(data & moveMask_Piece); }
    Color getColor() const { return static_cast<Color>((data & moveMask_Color) >> 3); }
    int getFrom() const { return (data & moveMask_From) >> 4; }
    int getTo() const { return (data & moveMask_To) >> 10; }
    bool isCapture() const { return (data & moveMask_IsCapture) >> 16; }
    DenseType getCaptDense() const { return static_cast<DenseType>((data & moveMask_CaptType) >> 17); }
    PieceType getCaptPiece() const {
        if (getColor() == WHITE) {
            return static_cast<PieceType>(getCaptDense());
        } else {
            return static_cast<PieceType>(getCaptDense() + 8);
        }
    }
    bool isCastle() const { return (data & moveMask_IsCastle) >> 20; }
    bool isEnPassant() const { return (data & moveMask_IsEnPass) >> 21; }
    bool isPromotion() const { return (data & moveMask_IsPromo) >> 22; }
    DenseType getPromoteDense() const { return static_cast<DenseType>((data & moveMask_PromoTo) >> 23); }
    PieceType getPromotePiece() const {
        if (getColor() == WHITE) {
            return static_cast<PieceType>(getPromoteDense());
        } else {
            return static_cast<PieceType>(getPromoteDense() + 8);
        }
    }

    // Setter for promotion type; useful for pawn promotions
    void setPromoteTo(DenseType promoteTo) {
        data &= ~moveMask_PromoTo;
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
        isCastle(false), isEnPassant(false), promoteTo(PieceType::W_QUEEN) {}

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