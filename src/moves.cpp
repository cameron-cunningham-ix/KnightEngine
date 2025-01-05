#include "moves.hpp"
#include "board_utility.hpp"
#include "pext_bitboard.hpp"
#include <bit>
#include <vector>
#include <algorithm>
#include <format>
#include <string>
#include <math.h>

// Forward declarations
// std::vector<Move> generatePsuedoMoves(const ChessBoard &board, const GameState* state);

// // Helper method to check if a move leaves the king in check
// // Might be able to remove this function if all moves generated are legal
// bool MoveGenerator::moveLeavesKingInCheck(ChessBoard board, const DenseMove& move) {
//     // Get side to check
//     Color side = board.currentGameState.sideToMove;
//     // Make the move on board
//     board.makeMove(move, true);
//     // See if side's king is in check - if anything but 0ULL, inCheck is true
//     bool inCheck = board.OppAttacksToSquare(
//         side == WHITE ? board.getWhiteKingSquare() : board.getBlackKingSquare(),
//         side
//     );
//     // Restore board before returning
//     board.unmakeMove(move, false);
//     return inCheck;
// }

// // Validate castling move
// bool MoveGenerator::isValidCastling(const Move& move) {
//     if (!move.isCastle) return false;
    
//     Color color = state->sideToMove;
//     bool isKingside = (move.to % 8 > move.from % 8);
    
//     // Check if castling rights are still available
//     if (color == WHITE) {
//         if (isKingside && !state->canCastleWhiteKingside) return false;
//         if (!isKingside && !state->canCastleWhiteQueenside) return false;
//     } else {
//         if (isKingside && !state->canCastleBlackKingside) return false;
//         if (!isKingside && !state->canCastleBlackQueenside) return false;
//     }
    
//     // Check if squares between king and rook are empty
//     U64 occupancy = board.getAllPieces();
//     if (isKingside) {
//         U64 kingToRook = (color == WHITE) ? 0x60ULL : 0x6000000000000000ULL;
//         if (occupancy & kingToRook) return false;
//     } else {
//         U64 kingToRook = (color == WHITE) ? 0xEULL : 0xE00000000000000ULL;
//         if (occupancy & kingToRook) return false;
//     }
    
//     // Check if king is in check
//     int kingPos = move.from;
//     if (isSquareUnderAttack(kingPos, color == WHITE ? BLACK : WHITE))
//         return false;
        
//     // Check if king passes through check
//     int step = isKingside ? 1 : -1;
//     int passThrough = kingPos + step;
//     if (isSquareUnderAttack(passThrough, color == WHITE ? BLACK : WHITE))
//         return false;
        
//     return true;
// }

// Validate en passant move
// bool MoveGenerator::isValidEnPassant(const Move& move) {
//     if (!move.isEnPassant) return false;
    
//     // Check if en passant square matches
//     // NOTE: expects enPassantSquare to be the square behind the enpassantable pawn
//     if (move.to != state->enPassantSquare) return false;
    
//     // Check if moving piece is a pawn
//     if (move.piece != W_PAWN && move.piece != B_PAWN) return false;
    
//     // Verify correct capture direction
//     int expectedFromRank = (state->sideToMove == WHITE) ? 4 : 3;
//     if (move.from / 8 != expectedFromRank) return false;
    
//     return true;
// }

// // Helper function to find king's position
// int MoveGenerator::findKingSquare(Color color) const {
//     // Get the appropriate king's bitboard
//     U64 kingBB = (color == WHITE) ? board.getWhiteKings() : board.getBlackKings();
//     // Use countr_zero to find the index of the set bit (king's position)
//     return std::countr_zero(kingBB);
// }

// Helper to check if a move is legally possible
// bool MoveGenerator::canMakeMove(Move& move, Color color) {
//     // Create keepsake board
//     ChessBoard keepBoard = board;
    
//     makeMove(board, move);
//     // Check if the move leaves or puts the king in check
//     bool notChecked = !isInCheck(color);    // not checked, so you can make the move
//     board = keepBoard;  // Restore board
//     return notChecked;
// }


// MoveGenerator::MoveGenerator(ChessBoard& b, GameState* s) : board(b), state(s) {}

// // Helper method to check if a square is under attack
// bool MoveGenerator::isSquareUnderAttack(int square, Color attackingColor) {
//     return board.OppAttacksToSquare(square, attackingColor == WHITE ? BLACK : WHITE);
// }

// // Check if a specific color is in check
// bool MoveGenerator::isInCheck(Color color) const {
//     // Find king's square
//     int kingSquare = findKingSquare(color);
    
//     // Check if king's square is under attack by opposite color
//     return board.OppAttacksToSquare(kingSquare, color);
// }

// // Check if a specific color is in checkmate
// bool MoveGenerator::isCheckmate(Color color) {
//     // First check if the player is in check
//     if (!isInCheck(color)) {
//         return false;
//     }
//     // Find king's position
//     U64 kingBB = color == WHITE ? board.getWhiteKings() : board.getBlackKings();
//     int kingPos = std::countr_zero(kingBB);
    
//     // Check if king is in check
//     if (!isSquareUnderAttack(kingPos, color == WHITE ? BLACK : WHITE)) {
//         return false;  // Not even in check
//     }
    
//     // Generate all possible moves
//     std::vector<Move> moves = generatePsuedoMoves(board, state);
    
//     // If any legal move exists, it's not checkmate
//     for (const Move& move : moves) {
//         if (isMoveLegal(move)) {
//             return false;
//         }
//     }
    
//     return true;  // No legal moves and king is in check = checkmate
// }

//// Check if a specific color is in stalemate
// bool MoveGenerator::isStalemate(Color color) {
//     // If in check, not stalemate
//     if (isInCheck(color)) {
//         return false;
//     }

//     // Generate all possible moves for this color
//     std::vector<Move> possibleMoves = generatePsuedoMoves(board, state);
    
//     // Try each move to see if any are legal
//     for (Move& move : possibleMoves) {
//         // Skip moves of the wrong color
//         if ((color == WHITE && move.piece >= B_PAWN) ||
//             (color == BLACK && move.piece <= W_KING)) {
//             continue;
//         }

//         // If any legal move exists, not stalemate
//         if (isMoveLegal(move)) {
//             return false;
//         }
//     }
//     // No legal moves found but not in check, it's stalemate
//     return true;
// }

// // NOTE: This function assumes the move is valid in terms of piece movement
// bool MoveGenerator::isMoveLegal(const Move& move) {
//     // Check if it's the correct side's turn
//     bool isWhitePiece = move.piece <= W_KING;
//     if ((state->sideToMove == WHITE) != isWhitePiece) return false;

//     // Check if the piece is on the square
//     if (getPieceOnSquare(board, move.from) != move.piece) return false;

//     // Check if the destination square is valid
//     if (move.to < 0 || move.to > 63) return false;
    
//     // Handle special moves
//     if (move.isCastle && !isValidCastling(move)) return false;
//     if (move.isEnPassant && !isValidEnPassant(move)) return false;
    
//     // Check promotion validity
//     if (move.isPromotion) {
//         if (move.piece != W_PAWN && move.piece != B_PAWN) return false;
//         int promRank = (move.piece == W_PAWN) ? 7 : 0;
//         if (move.to / 8 != promRank) return false;
//     }
    
//     // Verify the move doesn't leave king in check
//     if (moveLeavesKingInCheck(move)) return false;
    
//     return true;
// }


/// @brief Generates all possible pawn moves for current side of 'board' and pushes them onto 'moves' vector
/// @param board 
/// @param moves 
void MoveGenerator::generatePawnMoves(const ChessBoard& board, std::vector<DenseMove>& moves) {
    // Get occupancy and empty squares on board
    U64 occupancy = board.getAllPieces();
    U64 emptySquares = board.getEmptySquares();
    Color sideToMove = board.currentGameState.sideToMove;
    // If checkingCount is 1, we need to either block attack or capture piece
    // Double check should already be accounted for and so not necessary to check
    // if (board.getCheckCount() == 1) {
    //     U64 attackMask = board.getAttacksToKing(sideToMove);
    //     U64 orthoAtt = attackMask & board.getOrthogonalOpp(sideToMove);
    // }

    if (sideToMove == WHITE) {
        U64 whitePawns = board.getWhitePawns();
        // Calculate all capturing moves for white pawns
        while (whitePawns) {
            int index = std::countr_zero(whitePawns);   // Get index of the lowest set bit
            // Use the precomputed attack mask
            U64 attackMask = ATKMASK_WPAWN[index] & board.getBlackPieces();

            // Loop through each bit in the attack mask
            while (attackMask) {
                int targetSquare = std::countr_zero(attackMask);     // Get index of the lowest set bit
                // If 7th rank, move to 8th rank and promote
                if (targetSquare >= 56 && targetSquare < 64) {
                    DenseMove promoteMove = DenseMove(W_PAWN, index, targetSquare, 
                                                      board.getDenseTypeAt(targetSquare));
                    promoteMove.setPromoteTo(D_KNIGHT);
                    moves.push_back(promoteMove);
                    promoteMove.setPromoteTo(D_BISHOP);
                    moves.push_back(promoteMove);
                    promoteMove.setPromoteTo(D_ROOK);
                    moves.push_back(promoteMove);
                    promoteMove.setPromoteTo(D_QUEEN);
                    moves.push_back(promoteMove);
                }
                else {
                    moves.push_back(DenseMove(W_PAWN, index, targetSquare, 
                                     board.getDenseTypeAt(targetSquare)));
                }
                attackMask &= (attackMask - 1);     // Clear the least significant bit
            }
            whitePawns &= (whitePawns - 1);     // Clear the least significant bit
        }
        // Reset whitePawns bitboard
        whitePawns = board.getWhitePawns();
        // One square forward
        U64 singlePushes = (whitePawns << 8) & emptySquares;

        // Two squares forward
        // Hex value: each hex digit is 4 bits; 0x0000000000FF corresponds to bits in 3rd rank
        U64 doublePushes = ((singlePushes & 0x0000000000FF0000) << 8) & emptySquares;

        while (singlePushes) {
            int targetSquare = std::countr_zero(singlePushes);
            // If on 7th rank, move to 8th and promote
            if (targetSquare >= 56 && targetSquare < 64) {
                DenseMove promoteMove = DenseMove(W_PAWN, targetSquare - 8, 
                                                  targetSquare);
                promoteMove.setPromoteTo(D_KNIGHT);
                moves.push_back(promoteMove);
                promoteMove.setPromoteTo(D_BISHOP);
                moves.push_back(promoteMove);
                promoteMove.setPromoteTo(D_ROOK);
                moves.push_back(promoteMove);
                promoteMove.setPromoteTo(D_QUEEN);
                moves.push_back(promoteMove);
            } 
            else {
                moves.push_back(DenseMove(W_PAWN, targetSquare - 8, targetSquare));
            }
            singlePushes &= (singlePushes - 1);
        }

        while (doublePushes) {
            // Can only move two squares if on starting row
            int targetSquare = std::countr_zero(doublePushes);
            moves.push_back(DenseMove(W_PAWN, targetSquare - 16, targetSquare));
            doublePushes &= (doublePushes - 1);
        }
    }
    
    else {
        U64 blackPawns = board.getBlackPawns();
        // Calculate all capturing moves for black pawns
        while (blackPawns) {
            int index = std::countr_zero(blackPawns);        // Get index of the lowest set bit
            // Use the precomputed attack mask
            U64 attackMask = ATKMASK_BPAWN[index] & board.getWhitePieces();

            // Loop through each bit in the attack mask
            while (attackMask) {
                int targetSquare = std::countr_zero(attackMask);     // Get index of the lowest set bit
                // If 2nd rank, move to 1st rank and promote
                if (targetSquare >= 0 && targetSquare < 8) {
                    DenseMove promoteMove = DenseMove(B_PAWN, index, targetSquare, 
                                                      board.getDenseTypeAt(targetSquare));
                    promoteMove.setPromoteTo(D_KNIGHT);
                    moves.push_back(promoteMove);
                    promoteMove.setPromoteTo(D_BISHOP);
                    moves.push_back(promoteMove);
                    promoteMove.setPromoteTo(D_ROOK);
                    moves.push_back(promoteMove);
                    promoteMove.setPromoteTo(D_QUEEN);
                    moves.push_back(promoteMove);
                }
                else {
                    moves.push_back(DenseMove(B_PAWN, index, targetSquare,
                                              board.getDenseTypeAt(targetSquare)));
                }
                attackMask &= (attackMask - 1);     // Clear the least significant bit
            }
            blackPawns &= (blackPawns - 1);     // Clear the least significant bit
        }
        // Reset blackPawns bitboard
        blackPawns = board.getBlackPawns();
        // One square forward
        U64 singlePushes = (blackPawns >> 8) & emptySquares;

        // Two squares forward
        // Hex value: each hex digit is 4 bits; 0x0000000000FF corresponds to bits in 6th rank
        U64 doublePushes = ((singlePushes & 0x0000FF0000000000) >> 8) & emptySquares;

        while (singlePushes) {
            int targetSquare = std::countr_zero(singlePushes);
            // If on 2nd rank, move to 1st and promote
            if (targetSquare >= 0 && targetSquare < 8) {
                DenseMove promoteMove = DenseMove(B_PAWN, targetSquare + 8, targetSquare, 
                                                      board.getDenseTypeAt(targetSquare));
                promoteMove.setPromoteTo(D_KNIGHT);
                moves.push_back(promoteMove);
                promoteMove.setPromoteTo(D_BISHOP);
                moves.push_back(promoteMove);
                promoteMove.setPromoteTo(D_ROOK);
                moves.push_back(promoteMove);
                promoteMove.setPromoteTo(D_QUEEN);
                moves.push_back(promoteMove);
            } 
            else {
                moves.push_back(DenseMove(B_PAWN, targetSquare + 8, targetSquare));
            }
            singlePushes &= (singlePushes - 1);
        }

        while (doublePushes) {
            // Can only move two squares if on starting row
            int targetSquare = std::countr_zero(doublePushes);
            moves.push_back(DenseMove(B_PAWN, targetSquare + 16, targetSquare));
            doublePushes &= (doublePushes - 1);
        }
    }    
}

/// @brief Generates all possible en passant captures for 'board' and pushes them onto 'moves' vector
/// @param board 
/// @param moves 
void MoveGenerator::generateEnPassantMoves(const ChessBoard &board, std::vector<DenseMove> &moves) {
    // Get current state
    Color sideToMove = board.currentGameState.sideToMove;
    int enPassSquare = board.currentGameState.enPassantSquare;
    // If no en passant square is available, return
    if (enPassSquare == -1) return;

    // Get pawns that could potentially make en passant capture
    if (sideToMove == WHITE) {
        // Get white pawns that can capture the black pawn via en passant
        U64 epCaptors = ATKMASK_BPAWN[enPassSquare] & board.getWhitePawns();
        
        // For each white pawn that can make the capture
        while (epCaptors) {
            int from = std::countr_zero(epCaptors);  // Get index of capturing pawn
            DenseMove move(W_PAWN, from, enPassSquare, D_PAWN, false, true);
            moves.push_back(move);
            epCaptors &= (epCaptors - 1);  // Clear least significant bit
        }
    } else {
        // Get black pawns that can capture the white pawn via en passant
        U64 epCaptors = ATKMASK_WPAWN[enPassSquare] & board.getBlackPawns();
        
        // For each black pawn that can make the capture
        while (epCaptors) {
            int from = std::countr_zero(epCaptors);  // Get index of capturing pawn
            DenseMove move(B_PAWN, from, enPassSquare, D_PAWN, false, true);
            moves.push_back(move);
            epCaptors &= (epCaptors - 1);  // Clear least significant bit
        }
    }
}
/// @brief Generates all possible castling moves from 'board' and pushes them onto 'moves' vector
/// @param board 
/// @param state 
/// @param moves 
void MoveGenerator::generateCastlingMoves(const ChessBoard& board, std::vector<DenseMove>& moves) {
    // Get board occupancy
    U64 occupancy = board.getAllPieces();
    if (board.currentGameState.sideToMove == WHITE) {
        // If white still has kingside castle rights and there's no pieces in between
        if (board.currentGameState.canCastleWhiteKingside &&
            (occupancy & BoardUtility::W_ShortCastleMask) == 0) {
            DenseMove move(W_KING, 4, 6, D_EMPTY, true);
            moves.push_back(move);
        }
        // If white still has queenside castle rights and there's no pieces in between
        if (board.currentGameState.canCastleWhiteQueenside &&
            (occupancy & BoardUtility::W_LongCastleMask) == 0) {
            DenseMove move(W_KING, 4, 2, D_EMPTY, true);
            moves.push_back(move);
        }
    } else {
        // If black still has kingside castle rights and there's no pieces in between
        if (board.currentGameState.canCastleBlackKingside &&
            (occupancy & BoardUtility::B_ShortCastleMask) == 0) {
            DenseMove move(B_KING, 60, 62, D_EMPTY, true);
            moves.push_back(move);
        }
        // If black still has queenside castle rights and there's no pieces in between
        if (board.currentGameState.canCastleBlackQueenside &&
            (occupancy & BoardUtility::B_LongCastleMask) == 0) {
            DenseMove move(B_KING, 60, 58, D_EMPTY, true);
            moves.push_back(move);
        }
    }
}

/// @brief 
/// @param board 
/// @param moves 
/// @param piece 
void MoveGenerator::generatePieceMoves(const ChessBoard &board, std::vector<DenseMove> &moves, PieceType piece) {
    // std::cout << "MoveGen.genPieceMoves start\n    piece " << piece << "\n";
    U64 pieceBB;
    U64 opposition = board.getSideToMove() == WHITE ? board.getBlackPieces() : 
                                                      board.getWhitePieces();
    U64 friendlyOccupancy = board.getSideToMove() == WHITE ? board.getWhitePieces() : 
                                                             board.getBlackPieces();
    U64 occupancy = board.getAllPieces();
    U64 emptySquares = board.getEmptySquares();

    // std::cout << "    bitboards\nopposition\n";
    // printBitboard(opposition);
    // std::cout << "    friendlyOccupancy\n";
    // printBitboard(friendlyOccupancy);
    // std::cout << "    occupancy\n";
    // printBitboard(occupancy);
    // std::cout << "    emptySquares\n";
    // printBitboard(emptySquares);


    switch (piece) {
        case W_PAWN:
            // Pawns have their own function
            return;
            break;
        case W_KNIGHT:
            pieceBB = board.getWhiteKnights();
            break;
        case W_BISHOP:
            pieceBB = board.getWhiteBishops();
            break;
        case W_ROOK:
            pieceBB = board.getWhiteRooks();
            break;
        case W_QUEEN:
            pieceBB = board.getWhiteQueens();
            break;
        case W_KING:
            pieceBB = board.getWhiteKings();
            break;
        case B_PAWN:
            // Pawns have their own function
            return;
            break;
        case B_KNIGHT:
            pieceBB = board.getBlackKnights();
            break;
        case B_BISHOP:
            pieceBB = board.getBlackBishops();
            break;
        case B_ROOK:
            pieceBB = board.getBlackRooks();
            break;
        case B_QUEEN:
            pieceBB = board.getBlackQueens();
            break;
        case B_KING:
            pieceBB = board.getBlackKings();
            break;
        default:
            // Handle invalid piece type or error case
            break;
    }


    while (pieceBB) {
        int index = std::countr_zero(pieceBB);     // Get index of the lowest set bit
        // Use precomputed attack mask for bishops
        
        U64 attackMask;
        switch (piece) {
            case W_KNIGHT:
            case B_KNIGHT:
                attackMask = ATKMASK_KNIGHT[index];
                break;
            case W_BISHOP:
            case B_BISHOP:
                attackMask = ATKMASK_BISHOP[index];
                break;
            case W_ROOK:
            case B_ROOK:
                attackMask = ATKMASK_ROOK[index];
                break;
            case W_QUEEN:
            case B_QUEEN:
                attackMask = ATKMASK_QUEEN[index];
                break;
            case W_KING:
            case B_KING:
                attackMask = ATKMASK_KING[index];
                break;
            default:
                // Handle invalid piece type or error case
                break;
        }
        // Bitboard of all possible attacks
        U64 attacks = attackMask & opposition;
        // Bitboard of all non-capturing moves
        U64 freeSpace = attackMask & emptySquares;
        
        if (piece == W_ROOK || piece == B_ROOK) {
            U64 blocked = PEXT::getRookAttacks(index, occupancy);
            attacks &= blocked;
            freeSpace &= blocked;
        }
        
        if (piece == W_BISHOP || piece == B_BISHOP) {
            U64 blocked = PEXT::getBishopAttacks(index, occupancy);
            attacks &= blocked;
            freeSpace &= blocked;
        }

        if (piece == W_QUEEN || piece == B_QUEEN){
            U64 blocked = PEXT::getRookAttacks(index, occupancy) | PEXT::getBishopAttacks(index, occupancy);
            attacks &= blocked;
            freeSpace &= blocked;
        }
        
        while (attacks) {
            int targetSquare = std::countr_zero(attacks);       // Get index of the lowest set bit
            moves.push_back(DenseMove(piece, index, targetSquare, board.getDenseTypeAt(targetSquare)));
            attacks &= (attacks - 1);
        }

        while (freeSpace) {
            int targetSquare = std::countr_zero(freeSpace);     // Get index of the lowest set bit
            moves.push_back(DenseMove(piece, index, targetSquare));
            freeSpace &= (freeSpace - 1);
        }
        pieceBB &= (pieceBB - 1);
    }
}



// Generate the list of moves that can possibly be made by the current side to move,
// not taking into account other legality
std::vector<DenseMove> MoveGenerator::generatePsuedoMoves(const ChessBoard &board) {
    // Move list to be returned
    std::vector<DenseMove> move_list;

    // If current side's king is in double check, only the king can move 
    // and we can return early
    if (board.getCheckCount() == 2) {
        MoveGenerator::generatePieceMoves(board, move_list, 
            board.currentGameState.sideToMove == WHITE ? W_KING : B_KING);
        return move_list;
    }

    MoveGenerator::generatePawnMoves(board, move_list);
    MoveGenerator::generateEnPassantMoves(board, move_list);
    MoveGenerator::generateCastlingMoves(board, move_list);
    if (board.currentGameState.sideToMove == WHITE) {
        MoveGenerator::generatePieceMoves(board, move_list, W_KNIGHT);
        MoveGenerator::generatePieceMoves(board, move_list, W_BISHOP);
        MoveGenerator::generatePieceMoves(board, move_list, W_ROOK);
        MoveGenerator::generatePieceMoves(board, move_list, W_QUEEN);
        MoveGenerator::generatePieceMoves(board, move_list, W_KING);
    }
    else {
        MoveGenerator::generatePieceMoves(board, move_list, B_KNIGHT);
        MoveGenerator::generatePieceMoves(board, move_list, B_BISHOP);
        MoveGenerator::generatePieceMoves(board, move_list, B_ROOK);
        MoveGenerator::generatePieceMoves(board, move_list, B_QUEEN);
        MoveGenerator::generatePieceMoves(board, move_list, B_KING);
    }
    return move_list;
}

std::vector<DenseMove> MoveGenerator::generateLegalMoves(ChessBoard& board) {
    // std::cout << "MoveGenerator.generateLegalMoves start\n";
    std::vector<DenseMove> psuedo = generatePsuedoMoves(board);
    // std::cout << "    Psuedo legal moves generated\n";

    std::vector<DenseMove> legal;
    // For each psuedo legal move generated
    for (const DenseMove& move : psuedo) {
        // std::cout << "    move: " << std::bitset<32>(move.data) << "\n";
        // Make the move on the board
        board.makeMove(move, true);
        // std::cout << "    made move\n";
        // If the move did not leave its own side in check,
        // its a legal move, add to list
        if (!board.isSideInCheck(move.getColor())) {
            // std::cout << "    legal: " << move.getPieceType() << " from "
            //     << move.getFrom() << " to " << move.getTo() << " capture " << move.getCaptPiece()
            //     << "\n";
            legal.emplace_back(move);
        }
        // std::cout << "    unmaking move\n";
        // Unmake the move to return board to prev state
        board.unmakeMove(move, true);
    }
    // std::cout << "    generateLegalMoves returning, size " << legal.size() << "\n";
    return legal;
}

void printMove(const Move &move) {
    std::string piece;
    switch (move.piece) {
    case W_PAWN:
        piece = "W_PAWN";
        break;
    case W_KNIGHT:
        piece = "W_KNIGHT";
        break;
    case W_BISHOP:
        piece = "W_BISHOP";
        break;
    case W_ROOK:
        piece = "W_ROOK";
        break;
    case W_QUEEN:
        piece = "W_QUEEN";
        break;
    case W_KING:
        piece = "W_KING";
        break;
    case B_PAWN:
        piece = "B_PAWN";
        break;
    case B_KNIGHT:
        piece = "B_KNIGHT";
        break;
    case B_BISHOP:
        piece = "B_BISHOP";
        break;
    case B_ROOK:
        piece = "B_ROOK";
        break;
    case B_QUEEN:
        piece = "B_QUEEN";
        break;
    case B_KING:
        piece = "B_KING";
        break;
    default:
        piece = "INVALID";
        break;
    }

    std::cout << std::format("\n(printMove) Move: {}, From: {}, To: {}, Capture?: {}, Promote?: {}\n",
        piece, indexToAlgebraic(move.from), indexToAlgebraic(move.to), move.isCapture, move.isPromotion);
}
