#pragma once

#include <stdexcept>
#include <string>
#include <array>
#include <vector>
#include <bitset>
#include <format>
#include <optional>
#include <variant>

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

int pieceCode(PieceType ps);
int colorCode(PieceType ps);

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

const U32 moveMask_CaptType =       0b11100000000000000000000000000000;
const U32 moveMask_DType =          0b00001110000000000000000000000000;
const U32 moveMask_Color =          0b00010000000000000000000000000000;
const U32 moveMask_Piece =          0b00011110000000000000000000000000;
const U32 moveMask_PromoTo =        0b00000001110000000000000000000000;
const U32 moveMask_From =           0b00000000001111110000000000000000;
const U32 moveMask_To =             0b00000000000000001111110000000000;
const U32 moveMask_IsCastle =       0b00000000000000000000001000000000;
const U32 moveMask_IsEnPass =       0b00000000000000000000000100000000;

// Move struct using one unsigned 32-bit integer
struct DenseMove {
    // U32 structure [24 relevant bits]:
    // [3 bits]    [3 bits] [1 bit] [3 bits]  [6 bits] [6 bits] [1 bit]    [1 bit]    [8 bits]
    // [Capt Type] [DType]  [Color] [PromoTo] [From]   [To]     [IsCastle] [isEnPass] [Unused]
    // This way important moves (checks, captures and high value pieces) can be
    // more easily sorted by highest to lowest value
    U32 data;

    /// @brief Default constructor - 0, will correspond with EMPTY PieceType
    DenseMove() : data(0U) {}

    /// @brief Constructor with all fields - DenseTpe
    DenseMove(DenseType piece, Color color, int from, int to,
              DenseType captureType = DenseType::D_EMPTY, 
              bool isCastle = false, bool isEnPassant = false,
              DenseType promoteTo = DenseType::D_EMPTY) {
        if (from < 0 || from > 63 || to < 0 || to > 63) {
            throw std::invalid_argument("Invalid square index 1");
        }
        data = 0;
        data |= (isEnPassant << 8);
        data |= (isCastle << 9);
        data |= (to << 10);
        data |= (from << 16);
        data |= (promoteTo << 22);
        data |= (piece << 25);
        data |= (color << 28);
        data |= (captureType << 29);
    }
    
    /// @brief Constructor with all fields - PieceType
    DenseMove(PieceType piece, int from, int to,
              DenseType captureType = DenseType::D_EMPTY, 
              bool isCastle = false, bool isEnPassant = false,
              DenseType promoteTo = DenseType::D_EMPTY) {
        if (from < 0 || from > 63 || to < 0 || to > 63) {
            throw std::invalid_argument("Invalid square index 2");
        }
        data = 0;
        data |= (isEnPassant << 8);
        data |= (isCastle << 9);
        data |= (to << 10);
        data |= (from << 16);
        data |= (promoteTo << 22);
        data |= (piece << 25);
        data |= (captureType << 29);
    }


    /// @brief Constructor from unsigned 32-bit integer
    DenseMove(U32 d) : data(d) {}

    // Equality operator
    bool operator==(const DenseMove& other) const {
        return data == other.data;
    }

    // Getters

    /// @return DenseType of move
    DenseType getDenseType() const { return static_cast<DenseType>((data & moveMask_DType) >> 25); }
    /// @return PieceType of move
    PieceType getPieceType() const { return static_cast<PieceType>((data & moveMask_Piece) >> 25); }
    /// @return Color of move
    Color getColor() const { return static_cast<Color>((data & moveMask_Color) >> 28); }
    /// @return Starting square of move
    int getFrom() const { return (data & moveMask_From) >> 16; }
    /// @return Ending square of move
    int getTo() const { return (data & moveMask_To) >> 10; }
    /// @return DenseType of captured piece
    DenseType getCaptDense() const { return static_cast<DenseType>((data & moveMask_CaptType) >> 29); }
    /// @return PieceType of captured piece
    PieceType getCaptPiece() const {
        int piece = (data & moveMask_CaptType) >> 29;
        if (piece == 0) return PieceType::EMPTY;
        piece |= getColor() == WHITE ? 0b1000 : 0b0;    // If moving piece is white, captured piece is black and vice versa
        return static_cast<PieceType>(piece);
    }
    /// @return True if capture is not empty
    bool isCapture() const { return getCaptDense() != D_EMPTY; }
    /// @return True if this is a castling move, false otherwise
    bool isCastle() const { return (data & moveMask_IsCastle) >> 9; }
    /// @return True if this is an en passant move, false otherwise
    bool isEnPassant() const { return (data & moveMask_IsEnPass) >> 8; }
    /// @return DenseType of piece to promote to
    DenseType getPromoteDense() const { return static_cast<DenseType>((data & moveMask_PromoTo) >> 22); }
    /// @return PieceType of piece to promote to
    PieceType getPromotePiece() const {
        int piece = (data & moveMask_PromoTo) >> 22;
        if (piece == 0) return PieceType::EMPTY;
        piece |= (data & moveMask_Color) >> 25;
        return static_cast<PieceType>(piece);
    }
    /// @return True if promotion type is not empty
    bool isPromotion() const { return data & moveMask_PromoTo; }
    /// @brief Setter for promotion type; useful for pawn promotions
    /// @param promoteTo 
    void setPromoteTo(DenseType promoteTo) {
        data &= ~moveMask_PromoTo;
        data |= (promoteTo << 22);
    }
    /// @brief 
    /// @param castle 
    void setCastle(bool castle) {
        data &= ~moveMask_IsCastle;
        data |= (castle << 9);
    }
    /// @brief 
    /// @param enpass 
    void setEnPass(bool enpass) {
        data &= ~moveMask_IsEnPass;
        data |= (enpass << 8);
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

    // Current turn
    Color sideToMove;              // Whose turn it is

    // Castling rights for each color and side
    bool canCastleWhiteKingside;   // White king's side (h1)
    bool canCastleWhiteQueenside;  // White queen's side (a1)
    bool canCastleBlackKingside;   // Black king's side (h8)
    bool canCastleBlackQueenside;  // Black queen's side (a8)
    
    // Track en passant possibility
    int enPassantSquare;           // Square where en passant capture is possible (-1 if none)
    
    
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
        halfMoveClock(0),
        fullMoveNumber(1) {}
    
    /// @brief 
    /// @return Current castling rights
    int getCastleRights() const {
        int rights = 0;
        rights  += (canCastleWhiteKingside * 8)
                + (canCastleWhiteQueenside * 4)
                + (canCastleBlackKingside * 2)
                + (canCastleBlackQueenside);
        return rights;
    }
    /// @brief 
    /// @return 
    int getEnPassantFileIndex() const {
        return enPassantSquare % 8;
    }
    std::string toString() const {
        std::string rights;
        if (canCastleWhiteKingside) rights.append("K");
        if (canCastleWhiteQueenside) rights.append("Q");
        if (canCastleBlackKingside) rights.append("k");
        if (canCastleBlackQueenside) rights.append("q");
        if (!canCastleWhiteKingside & !canCastleWhiteQueenside &
            !canCastleBlackKingside & !canCastleBlackQueenside) {
                rights = "-";
        }
        std::string result = std::format("side: {} {} {} {} {}",
        colorToString(sideToMove), rights,
        enPassantSquare, halfMoveClock, fullMoveNumber);

        return result;
    }
};

// Represents a single engine option
struct EngineOption {
    enum class Type {
        Check,  // Boolean option
        Spin,   // Integer within a range
        String  // Text option
    };

    std::string name;
    Type type;
    std::variant<bool, int, std::string> defaultValue;
    std::variant<bool, int, std::string> currentValue;
    std::optional<int> minValue;  // Only for Spin
    std::optional<int> maxValue;  // Only for Spin

    // Construct a Check option
    static EngineOption createCheck(const std::string& name, bool defaultVal) {
        return EngineOption{name, Type::Check, defaultVal, defaultVal, std::nullopt, std::nullopt};
    }

    // Construct a Spin option
    static EngineOption createSpin(const std::string& name, int defaultVal, int min, int max) {
        return EngineOption{name, Type::Spin, defaultVal, defaultVal, min, max};
    }

    // Construct a String option  
    static EngineOption createString(const std::string& name, const std::string& defaultVal) {
        return EngineOption{name, Type::String, defaultVal, defaultVal, std::nullopt, std::nullopt};
    }

    // Convert to UCI option string
    std::string toUCIString() const {
        std::string result = "option name " + name + " type ";
        switch (type) {
            case Type::Check:
                result += "check default " + std::to_string(std::get<bool>(defaultValue));
                break;
            case Type::Spin:
                result += "spin default " + std::to_string(std::get<int>(defaultValue)) + 
                         " min " + std::to_string(*minValue) + 
                         " max " + std::to_string(*maxValue);
                break;
            case Type::String:
                result += "string default " + std::get<std::string>(defaultValue);
                break;
        }
        return result;
    }

    // Update option value from string
    bool setValue(const std::string& value) {
        try {
            switch (type) {
                case Type::Check:
                    currentValue = (value == "true" || value == "1");
                    break;
                case Type::Spin: {
                    int intVal = std::stoi(value);
                    if (intVal < *minValue || intVal > *maxValue) return false;
                    currentValue = intVal;
                    break;
                }
                case Type::String:
                    currentValue = value;
                    break;
            }
            return true;
        } catch (...) {
            return false;
        }
    }

    // Get current value as string
    std::string getCurrentValueString() const {
        switch (type) {
            case Type::Check:
                return std::get<bool>(currentValue) ? "true" : "false";
            case Type::Spin:
                return std::to_string(std::get<int>(currentValue));
            case Type::String:
                return std::get<std::string>(currentValue);
        }
        return "";
    }
};