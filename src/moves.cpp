#include "moves.hpp"
#include "board_utility.hpp"
#include "pext_bitboard.hpp"
#include <bit>
#include <vector>
#include <algorithm>
#include <format>
#include <string>
#include <math.h>

/// @brief Generates all possible pawn moves for current side of 'board' and 
/// @param board 
/// @param moves 
void MoveGenerator::generatePawnMoves(const ChessBoard& board, std::array<DenseMove, MAX_MOVES>& moves, int& moveNum) {
    // Get occupancy and empty squares on board
    U64 occupancy = board.getAllPieces();
    U64 emptySquares = board.getEmptySquares();
    Color sideToMove = board.getSideToMove();
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
                    moves[moveNum++] = promoteMove;
                    promoteMove.setPromoteTo(D_BISHOP);
                    moves[moveNum++] = promoteMove;
                    promoteMove.setPromoteTo(D_ROOK);
                    moves[moveNum++] = promoteMove;
                    promoteMove.setPromoteTo(D_QUEEN);
                    moves[moveNum++] = promoteMove;
                }
                else {
                    moves[moveNum++] = (DenseMove(W_PAWN, index, targetSquare, 
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
                moves[moveNum++] = promoteMove;
                promoteMove.setPromoteTo(D_BISHOP);
                moves[moveNum++] = promoteMove;
                promoteMove.setPromoteTo(D_ROOK);
                moves[moveNum++] = promoteMove;
                promoteMove.setPromoteTo(D_QUEEN);
                moves[moveNum++] = promoteMove;
            } 
            else {
                moves[moveNum++] = (DenseMove(W_PAWN, targetSquare - 8, targetSquare));
            }
            singlePushes &= (singlePushes - 1);
        }

        while (doublePushes) {
            // Can only move two squares if on starting row
            int targetSquare = std::countr_zero(doublePushes);
            moves[moveNum++] = (DenseMove(W_PAWN, targetSquare - 16, targetSquare));
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
                    moves[moveNum++] = promoteMove;
                    promoteMove.setPromoteTo(D_BISHOP);
                    moves[moveNum++] = promoteMove;
                    promoteMove.setPromoteTo(D_ROOK);
                    moves[moveNum++] = promoteMove;
                    promoteMove.setPromoteTo(D_QUEEN);
                    moves[moveNum++] = promoteMove;
                }
                else {
                    moves[moveNum++] = (DenseMove(B_PAWN, index, targetSquare,
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
                moves[moveNum++] = promoteMove;
                promoteMove.setPromoteTo(D_BISHOP);
                moves[moveNum++] = promoteMove;
                promoteMove.setPromoteTo(D_ROOK);
                moves[moveNum++] = promoteMove;
                promoteMove.setPromoteTo(D_QUEEN);
                moves[moveNum++] = promoteMove;
            } 
            else {
                moves[moveNum++] = (DenseMove(B_PAWN, targetSquare + 8, targetSquare));
            }
            singlePushes &= (singlePushes - 1);
        }

        while (doublePushes) {
            // Can only move two squares if on starting row
            int targetSquare = std::countr_zero(doublePushes);
            moves[moveNum++] = (DenseMove(B_PAWN, targetSquare + 16, targetSquare));
            doublePushes &= (doublePushes - 1);
        }
    }    
}

/// @brief Generates all possible en passant captures for 'board' and pushes them onto 'moves' vector
/// @param board 
/// @param moves 
void MoveGenerator::generateEnPassantMoves(const ChessBoard &board, std::array<DenseMove, MAX_MOVES>& moves, int& moveNum) {
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
            moves[moveNum++] = move;
            epCaptors &= (epCaptors - 1);  // Clear least significant bit
        }
    } else {
        // Get black pawns that can capture the white pawn via en passant
        U64 epCaptors = ATKMASK_WPAWN[enPassSquare] & board.getBlackPawns();
        
        // For each black pawn that can make the capture
        while (epCaptors) {
            int from = std::countr_zero(epCaptors);  // Get index of capturing pawn
            DenseMove move(B_PAWN, from, enPassSquare, D_PAWN, false, true);
            moves[moveNum++] = move;
            epCaptors &= (epCaptors - 1);  // Clear least significant bit
        }
    }
}
/// @brief Generates all possible castling moves from 'board' and pushes them onto 'moves' vector
/// @param board 
/// @param state 
/// @param moves 
void MoveGenerator::generateCastlingMoves(const ChessBoard& board, std::array<DenseMove, MAX_MOVES>& moves, int& moveNum) {
    // Get current side color
    Color sideToMove = board.getSideToMove();
    // Get board occupancy
    U64 occupancy = board.getAllPieces();
    // Get enemy attacks to starting 
    U64 enemyAttacks = board.OppAttacksToSquare(sideToMove == WHITE ? 4 : 60, sideToMove);
    // Kings cannot castle out of check
    if (enemyAttacks) return;

    if (sideToMove == WHITE) {
        // If white still has kingside castle rights and there's no pieces or attacks in between
        if (board.currentGameState.canCastleWhiteKingside &&
            (occupancy & BUTIL::W_ShortCastleMask) == 0 &&
            board.OppAttacksToSquare(5, WHITE) == 0) {
            DenseMove move(W_KING, 4, 6, D_EMPTY, true);
            moves[moveNum++] = move;
        }
        // If white still has queenside castle rights and there's no pieces or attacks in between
        if (board.currentGameState.canCastleWhiteQueenside &&
            (occupancy & BUTIL::W_LongCastleMask) == 0 &&
            board.OppAttacksToSquare(3, WHITE) == 0) {
            DenseMove move(W_KING, 4, 2, D_EMPTY, true);
            moves[moveNum++] = move;
        }
    } else {
        // If black still has kingside castle rights and there's no pieces or attacks in between
        if (board.currentGameState.canCastleBlackKingside &&
            (occupancy & BUTIL::B_ShortCastleMask) == 0 &&
            board.OppAttacksToSquare(61, BLACK) == 0) {
            DenseMove move(B_KING, 60, 62, D_EMPTY, true);
            moves[moveNum++] = move;
        }
        // If black still has queenside castle rights and there's no pieces or attacks in between
        if (board.currentGameState.canCastleBlackQueenside &&
            (occupancy & BUTIL::B_LongCastleMask) == 0 &&
            board.OppAttacksToSquare(59, BLACK) == 0) {
            DenseMove move(B_KING, 60, 58, D_EMPTY, true);
            moves[moveNum++] = move;
        }
    }
}

/// @brief 
/// @param board 
/// @param moves 
/// @param piece 
void MoveGenerator::generatePieceMoves(const ChessBoard &board, std::array<DenseMove, MAX_MOVES>& moves, int& moveNum, PieceType piece) {
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
            moves[moveNum++] = DenseMove(piece, index, targetSquare, board.getDenseTypeAt(targetSquare));
            attacks &= (attacks - 1);
        }

        while (freeSpace) {
            int targetSquare = std::countr_zero(freeSpace);     // Get index of the lowest set bit
            moves[moveNum++] = DenseMove(piece, index, targetSquare);
            freeSpace &= (freeSpace - 1);
        }
        pieceBB &= (pieceBB - 1);
    }
}



// Generate the list of moves that can possibly be made by the current side to move,
// not taking into account other legality
std::array<DenseMove, MAX_MOVES> MoveGenerator::generatePsuedoMoves(const ChessBoard &board, int& moveNum) {
    // Move list to be returned
    std::array<DenseMove, MAX_MOVES> move_list = {};
    // Current side's color
    Color sideToMove = board.getSideToMove();
    // If current side's king is in double check, only the king can move 
    if (board.getCheckCount() == 2) {
        // std::cout << "genPsuedoMoves getCheckCount == 2\n";
        MoveGenerator::generatePieceMoves(board, move_list, moveNum,
                                          sideToMove == WHITE ? W_KING : B_KING);
    }
    MoveGenerator::generatePawnMoves(board, move_list, moveNum);
    MoveGenerator::generateEnPassantMoves(board, move_list, moveNum);
    MoveGenerator::generateCastlingMoves(board, move_list, moveNum);
    if (board.currentGameState.sideToMove == WHITE) {
        MoveGenerator::generatePieceMoves(board, move_list, moveNum, W_KNIGHT);
        MoveGenerator::generatePieceMoves(board, move_list, moveNum, W_BISHOP);
        MoveGenerator::generatePieceMoves(board, move_list, moveNum, W_ROOK);
        MoveGenerator::generatePieceMoves(board, move_list, moveNum, W_QUEEN);
        MoveGenerator::generatePieceMoves(board, move_list, moveNum, W_KING);
    }
    else {
        MoveGenerator::generatePieceMoves(board, move_list, moveNum, B_KNIGHT);
        MoveGenerator::generatePieceMoves(board, move_list, moveNum, B_BISHOP);
        MoveGenerator::generatePieceMoves(board, move_list, moveNum, B_ROOK);
        MoveGenerator::generatePieceMoves(board, move_list, moveNum, B_QUEEN);
        MoveGenerator::generatePieceMoves(board, move_list, moveNum, B_KING);
    }
    
    return move_list;
}

std::array<DenseMove, MAX_MOVES> MoveGenerator::generateLegalMoves(ChessBoard& board, int& moveNum) {
    // std::cout << "MoveGenerator.generateLegalMoves start\n";
    // Reset moveNum to 0, since moveNum should always indicate number of moves in array
    moveNum = 0;
    // generatePsuedoMoves will set moveNum to correct number of moves to check
    std::array<DenseMove, MAX_MOVES> legal = generatePsuedoMoves(board, moveNum);

    Color sideToMove = board.getSideToMove();
    // std::vector<DenseMove> legal;
    // For each psuedo legal move generated
    for (int i = 0; i < moveNum; i) {
        // std::cout << "psuedo move: " << move.toString(false) << "\n";
        // Make the move on the board
        board.makeMove(legal[i], true);
        // std::cout << "    made move\n";
        // If the move did not leave its own side in check,
        // its a legal move, add to list
        if (board.isSideInCheck(sideToMove)) {
            // Unmake the move to return board to prev state
            board.unmakeMove(legal[i], true);
            legal[i] = legal[--moveNum];
            // moveNum--;
            continue;
        }
        board.unmakeMove(legal[i], true);
        i++;
    }
    // No legal moves, ensure indices are empty move
    if (moveNum == 0) legal.fill(DenseMove());
    // std::cout << "    generateLegalMoves returning, size " << legal.size() << "\n";
    return legal;
}
