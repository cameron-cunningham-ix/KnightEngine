#include "types.hpp"

std::string colorToString(Color color) {
    switch (color) {
        case WHITE:
            return "White";
        case BLACK:
            return "Black";
        default:
            return "Invalid";
    }
}

std::string denseTypeToString(DenseType type) {
    switch (type) {
        case D_EMPTY:
            return "Empty";
        case D_PAWN:
            return "Pawn";
        case D_KNIGHT:
            return "Knight";
        case D_BISHOP:
            return "Bishop";
        case D_ROOK:
            return "Rook";
        case D_QUEEN:
            return "Queen";
        case D_KING:
            return "King";
        default:
            return "Invalid";
    }
}

std::string pieceTypeToString(PieceType type) {
    switch(type) {
        case PieceType::EMPTY:
            return "Empty";
        case PieceType::INVALID:
            return "Invalid";
        case PieceType::W_PAWN:
            return "White Pawn";
        case PieceType::B_PAWN:
            return "Black Pawn";
        case PieceType::W_KNIGHT:
            return "White Knight";
        case PieceType::B_KNIGHT:
            return "Black Knight";
        case PieceType::W_BISHOP:
            return "White Bishop";
        case PieceType::B_BISHOP:
            return "Black Bishop";
        case PieceType::W_ROOK:
            return "White Rook";
        case PieceType::B_ROOK:
            return "Black Rook";
        case PieceType::W_QUEEN:
            return "White Queen";
        case PieceType::B_QUEEN:
            return "Black Queen";
        case PieceType::W_KING:
            return "White King";
        case PieceType::B_KING:
            return "Black King";
        default:
            return "NULL";
    }
}