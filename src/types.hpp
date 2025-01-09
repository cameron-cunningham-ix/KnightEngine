#pragma once

#include <stdexcept>
#include <array>
#include <bitset>
#include <format>

const int MAX_PLY = 512;
const int MAX_MOVES = 218;

typedef unsigned long long U64;     // Used for bitboards
typedef unsigned int U32;           // Used for dense move representation

/// @brief Enum for piece types on the board + empty and invalid
/// 3 LSB are used to represent the piece type, 4th bit is used to represent the color
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
/// @brief Enum for piece color
enum Color : bool {
    WHITE,      // 0
    BLACK       // 1
};
/// @brief Enum for use with board bitboards and DenseMove - compare 3 LSB with DenseMove 3 LSB
enum DenseType : int {
    D_EMPTY = 0,      // 0 - 000
    D_PAWN = 1,       // 1 - 001
    D_KNIGHT = 2,     // 2 - 010
    D_BISHOP = 3,     // 3 - 011
    D_ROOK = 4,       // 4 - 100
    D_QUEEN = 5,      // 5 - 101
    D_KING = 6        // 6 - 110
};

std::string colorToString(Color color);
std::string denseTypeToString(DenseType type);
std::string pieceTypeToString(PieceType type);



// Masks for extracting fields from dense move representation

const U32 moveMask_DType =     0b000000000000000000000111;
const U32 moveMask_Color =     0b000000000000000000001000;
const U32 moveMask_Piece =     0b000000000000000000001111;
const U32 moveMask_From =      0b000000000000001111110000;
const U32 moveMask_To =        0b000000001111110000000000;
const U32 moveMask_CaptType =  0b000001110000000000000000;
const U32 moveMask_IsCastle =  0b000010000000000000000000;
const U32 moveMask_IsEnPass =  0b000100000000000000000000;
const U32 moveMask_PromoTo =   0b111000000000000000000000;

// Move struct using one unsigned 32-bit integer
struct DenseMove {
    // Int structure:
    // [3b]      [1b]       [1b]       [3b]            [6b] [6b]   [1b]    [3b]
    // [PromoTo] [isEnPass] [isCastle] [CaptDenseType] [To] [From] [Color] [DenseType]
    U32 data;

    /// @brief Default constructor - 0, will correspond with EMPTY PieceType
    DenseMove() : data(0U) {}

    /// @brief Constructor with all fields - DenseTpe
    DenseMove(DenseType piece, Color color, int from, int to,
              DenseType captureType = DenseType::D_EMPTY, 
              bool isCastle = false, bool isEnPassant = false,
              DenseType promoteTo = DenseType::D_EMPTY) {
        data = 0;
        data |= piece;
        data |= (color << 3);
        if (from < 0 || from > 63 || to < 0 || to > 63) {
            throw std::invalid_argument("Invalid square index");
        }
        data |= (from << 4);
        data |= (to << 10);
        data |= (captureType << 16);
        data |= (isCastle << 19);
        data |= (isEnPassant << 20);
        data |= (promoteTo << 21);
    }
    
    /// @brief Constructor with all fields - PieceType
    DenseMove(PieceType piece, int from, int to,
              DenseType captureType = DenseType::D_EMPTY, 
              bool isCastle = false, bool isEnPassant = false,
              DenseType promoteTo = DenseType::D_EMPTY) {
        data = 0;
        data |= piece;
        if (from < 0 || from > 63 || to < 0 || to > 63) {
            throw std::invalid_argument("Invalid square index");
        }
        data |= (from << 4);
        data |= (to << 10);
        data |= (captureType << 16);
        data |= (isCastle << 19);
        data |= (isEnPassant << 20);
        data |= (promoteTo << 21);
    }


    /// @brief Constructor from unsigned 32-bit integer
    DenseMove(U32 d) : data(d) {}

    // Equality operator
    bool operator==(const DenseMove& other) const {
        return data == other.data;
    }

    // Getters

    /// @return DenseType of move
    DenseType getDenseType() const { return static_cast<DenseType>(data & moveMask_DType); }
    /// @return PieceType of move
    PieceType getPieceType() const { return static_cast<PieceType>(data & moveMask_Piece); }
    /// @return Color of move
    Color getColor() const { return static_cast<Color>((data & moveMask_Color) >> 3); }
    /// @return Starting square of move
    int getFrom() const { return (data & moveMask_From) >> 4; }
    /// @return Ending square of move
    int getTo() const { return (data & moveMask_To) >> 10; }
    /// @return DenseType of captured piece
    DenseType getCaptDense() const { return static_cast<DenseType>((data & moveMask_CaptType) >> 16); }
    /// @return PieceType of captured piece
    PieceType getCaptPiece() const {
        int piece = (data & moveMask_CaptType) >> 16;
        if (piece == 0) return PieceType::EMPTY;
        piece |= getColor() == WHITE ? 0b1000 : 0b0;    // If moving piece is white, captured piece is black and vice versa
        return static_cast<PieceType>(piece);
    }
    /// @return True if capture is not empty
    bool isCapture() const { return getCaptDense() != D_EMPTY; }
    /// @return True if this is a castling move, false otherwise
    bool isCastle() const { return (data & moveMask_IsCastle) >> 19; }
    /// @return True if this is an en passant move, false otherwise
    bool isEnPassant() const { return (data & moveMask_IsEnPass) >> 20; }
    /// @return DenseType of piece to promote to
    DenseType getPromoteDense() const { return static_cast<DenseType>((data & moveMask_PromoTo) >> 21); }
    /// @return PieceType of piece to promote to
    PieceType getPromotePiece() const {
        // if (getColor() == WHITE) {
        //     return static_cast<PieceType>(getPromoteDense());
        // } else {
        //     return static_cast<PieceType>(getPromoteDense() + 8);
        // }
        int piece = (data & moveMask_PromoTo) >> 21;
        if (piece == 0) return PieceType::EMPTY;
        piece |= (data & moveMask_Color);
        return static_cast<PieceType>(piece);
    }
    /// @return True if promotion type is not empty
    bool isPromotion() const { return getPromoteDense() != D_EMPTY; }
    /// @brief Setter for promotion type; useful for pawn promotions
    /// @param promoteTo 
    void setPromoteTo(DenseType promoteTo) {
        data &= ~moveMask_PromoTo;
        data |= (promoteTo << 21);
    }
    /// @param isBrief If true, returns a shortened version. If false, returns all values
    /// @return String representation of the move
    std::string toString(bool isBrief) const {
        std::string result = std::format("{} from {} to {}\n", 
                                         pieceTypeToString(getPieceType()),
                                         getFrom(), getTo());

        if (isBrief) return result;
        else {
            std::string add = std::format(" - isCapture: {}" 
                                          " captured: {} isCastle: {}" 
                                          " isEnPass: {} isPromo: {}" 
                                          " Promo Piece: {}\n",
                                          isCapture(), pieceTypeToString(getCaptPiece()),
                                          isCastle(), isEnPassant(),
                                          isPromotion(), pieceTypeToString(getPromotePiece()));
            result.append(add);
        }
        return result;
    }
    std::string toAlgebraic() const;
};

/// @brief Struct representing the current state of the game.
/// Contains information about castling rights, en passant square, side to move, etc.
/// Used in ChessBoard class.
struct GameState {
public:
    // Castling rights for each color and side
    bool canCastleWhiteKingside;   // White king's side (h1)
    bool canCastleWhiteQueenside;  // White queen's side (a1)
    bool canCastleBlackKingside;   // Black king's side (h8)
    bool canCastleBlackQueenside;  // Black queen's side (a8)
    
    // Track en passant possibility
    int enPassantSquare;           // Square where en passant capture is possible (-1 if none)
    
    // Current turn
    Color sideToMove;              // Whose turn it is
    Color oppColor;                // Opposite color
    
    // How long its been since the last pawn push or piece capture 
    // (used for determining 50-move draw rule)
    int halfMoveClock;
    // Number of the move currently being played (starts at 1,
    // incremented when Black plays a move)
    int fullMoveNumber;

    // Constructor with default values
    GameState() : 
        canCastleWhiteKingside(true),
        canCastleWhiteQueenside(true),
        canCastleBlackKingside(true),
        canCastleBlackQueenside(true),
        enPassantSquare(-1),
        sideToMove(WHITE),
        oppColor(BLACK),
        halfMoveClock(0),
        fullMoveNumber(1) {}
    
    std::string toString() {
        std::string rights;
        if (canCastleWhiteKingside) rights.append("K");
        if (canCastleWhiteQueenside) rights.append("Q");
        if (canCastleBlackKingside) rights.append("k");
        if (canCastleBlackQueenside) rights.append("q");
        if (!canCastleWhiteKingside & !canCastleWhiteQueenside &
            !canCastleBlackKingside & !canCastleBlackQueenside) {
                rights = "-";
        }
        std::string result = std::format("side: {} opp: {} {} {} {} {}",
        colorToString(sideToMove), colorToString(oppColor), rights,
        enPassantSquare, halfMoveClock, fullMoveNumber);

        return result;
    }
};