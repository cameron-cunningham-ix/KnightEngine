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
    int currCastleRights = board.currentGameState.getCastleRights();

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
                                                      board.getDenseTypeAt(targetSquare),
                                                      false, false, D_QUEEN);
                    moves[moveNum++] = promoteMove;
                    promoteMove.setPromoteTo(D_ROOK);
                    moves[moveNum++] = promoteMove;
                    promoteMove.setPromoteTo(D_BISHOP);
                    moves[moveNum++] = promoteMove;
                    promoteMove.setPromoteTo(D_KNIGHT);
                    moves[moveNum++] = promoteMove;
                }
                else {
                    DenseMove move = DenseMove(W_PAWN, index, targetSquare, 
                                     board.getDenseTypeAt(targetSquare));
                    moves[moveNum++] = move;
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
                                                  targetSquare, D_EMPTY, false,
                                                  false, D_QUEEN);
                moves[moveNum++] = promoteMove;
                promoteMove.setPromoteTo(D_ROOK);
                moves[moveNum++] = promoteMove;
                promoteMove.setPromoteTo(D_BISHOP);
                moves[moveNum++] = promoteMove;
                promoteMove.setPromoteTo(D_KNIGHT);
                moves[moveNum++] = promoteMove;
            } 
            else {
                DenseMove move = DenseMove(W_PAWN, targetSquare - 8, targetSquare);
                moves[moveNum++] = move;
            }
            singlePushes &= (singlePushes - 1);
        }

        while (doublePushes) {
            // Can only move two squares if on starting row
            int targetSquare = std::countr_zero(doublePushes);
            DenseMove move = DenseMove(W_PAWN, targetSquare - 16, targetSquare);
            moves[moveNum++] = move;
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
                                                      board.getDenseTypeAt(targetSquare),
                                                      false, false, D_QUEEN);
                    moves[moveNum++] = promoteMove;
                    promoteMove.setPromoteTo(D_ROOK);
                    moves[moveNum++] = promoteMove;
                    promoteMove.setPromoteTo(D_BISHOP);
                    moves[moveNum++] = promoteMove;
                    promoteMove.setPromoteTo(D_KNIGHT);
                    moves[moveNum++] = promoteMove;
                }
                else {
                    DenseMove move = DenseMove(B_PAWN, index, targetSquare,
                                              board.getDenseTypeAt(targetSquare));
                    moves[moveNum++] = move;
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
                                                      board.getDenseTypeAt(targetSquare),
                                                      false, false, D_QUEEN);
                moves[moveNum++] = promoteMove;
                promoteMove.setPromoteTo(D_ROOK);
                moves[moveNum++] = promoteMove;
                promoteMove.setPromoteTo(D_BISHOP);
                moves[moveNum++] = promoteMove;
                promoteMove.setPromoteTo(D_KNIGHT);
                moves[moveNum++] = promoteMove;
            } 
            else {
                DenseMove move = DenseMove(B_PAWN, targetSquare + 8, targetSquare);
                moves[moveNum++] = move;
            }
            singlePushes &= (singlePushes - 1);
        }

        while (doublePushes) {
            // Can only move two squares if on starting row
            int targetSquare = std::countr_zero(doublePushes);
            DenseMove move = DenseMove(B_PAWN, targetSquare + 16, targetSquare);
            moves[moveNum++] = move;
            doublePushes &= (doublePushes - 1);
        }
    }    
}

/// @brief Generates all possible en passant captures for 'board' and pushes them onto 'moves' vector
/// @param board 
/// @param moves 
void MoveGenerator::generateEnPassantMoves(const ChessBoard &board, std::array<DenseMove, MAX_MOVES>& moves, int& moveNum) {
    int enPassSquare = board.currentGameState.enPassantSquare;
    // If no en passant square is available, return
    if (enPassSquare == -1) return;
    // Get current state
    Color sideToMove = board.currentGameState.sideToMove;

    // Get pawns that could potentially make en passant capture
    if (sideToMove == WHITE) {
        // Get white pawns that can capture the black pawn via en passant
        U64 epCaptors = ATKMASK_BPAWN[enPassSquare] & board.getWhitePawns();
        
        // For each white pawn that can make the capture
        while (epCaptors) {
            int from = std::countr_zero(epCaptors);  // Get index of capturing pawn
            DenseMove move(W_PAWN, from, enPassSquare, D_PAWN,
                         false, true, D_EMPTY);
            
            moves[moveNum++] = move;
            epCaptors &= (epCaptors - 1);  // Clear least significant bit
        }
    } else {
        // Get black pawns that can capture the white pawn via en passant
        U64 epCaptors = ATKMASK_WPAWN[enPassSquare] & board.getBlackPawns();
        
        // For each black pawn that can make the capture
        while (epCaptors) {
            int from = std::countr_zero(epCaptors);  // Get index of capturing pawn
            DenseMove move(B_PAWN, from, enPassSquare, D_PAWN,
                         false, true, D_EMPTY);
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
    Color sideToMove = board.getSideToMove();
    // Get enemy attacks to starting square
    U64 enemyAttacks = board.OppAttacksToSquare(sideToMove == WHITE ? 4 : 60, sideToMove);
    // Kings cannot castle out of check
    if (enemyAttacks) return;

    // Castling right info in moves is for rebuilding state in unmakeMove;
    // therefore moves take current castling rights unchanged
    int currCastleRights = board.currentGameState.getCastleRights();
    // Get board occupancy
    U64 occupancy = board.getAllPieces();

    if (sideToMove == WHITE) {
        // If white still has kingside castle rights and there's no pieces
        // or attacks in between
        if (board.currentGameState.canCastleWhiteKingside &&
            (occupancy & BUTIL::W_ShortCastleMask) == 0 &&
            board.OppAttacksToSquare(5, WHITE) == 0) {
            DenseMove move(W_KING, 4, 6, D_EMPTY, true, false, D_EMPTY);
            moves[moveNum++] = move;
        }
        // If white still has queenside castle rights
        if (board.currentGameState.canCastleWhiteQueenside &&
            (occupancy & BUTIL::W_LongCastleMask) == 0 &&
            board.OppAttacksToSquare(3, WHITE) == 0) {
            DenseMove move(W_KING, 4, 2, D_EMPTY, true, false, D_EMPTY);
            moves[moveNum++] = move;
        }
    } else {
        // If black still has kingside castle rights
        if (board.currentGameState.canCastleBlackKingside &&
            (occupancy & BUTIL::B_ShortCastleMask) == 0 &&
            board.OppAttacksToSquare(61, BLACK) == 0) {
            DenseMove move(B_KING, 60, 62, D_EMPTY, true, false, D_EMPTY);
            moves[moveNum++] = move;
        }
        // If black still has queenside castle rights
        if (board.currentGameState.canCastleBlackQueenside &&
            (occupancy & BUTIL::B_LongCastleMask) == 0 &&
            board.OppAttacksToSquare(59, BLACK) == 0) {
            DenseMove move(B_KING, 60, 58, D_EMPTY, true, false, D_EMPTY);
            moves[moveNum++] = move;
        }
    }
}

/// @brief 
/// @param board 
/// @param moves 
/// @param piece 
void MoveGenerator::generatePieceMoves(const ChessBoard &board, std::array<DenseMove, MAX_MOVES>& moves, int& moveNum) {
    U64 pieceBB;
    Color sideToMove = board.getSideToMove();
    U64 occupancy = board.getAllPieces();
    U64 emptySquares = board.getEmptySquares();
    int currCastleRights = board.currentGameState.getCastleRights();

    if (sideToMove == WHITE) {
        U64 opposition = board.getBlackPieces();
        // Mask of attacks from index
        U64 attackMask;
        // Actual attacks from index
        U64 attacks;
        // Free spaces from index
        U64 freeSpace;

        // Queens
        pieceBB = board.getWhiteQueens();
        while (pieceBB) {
            int index = std::countr_zero(pieceBB);
            attackMask = ATKMASK_QUEEN[index];
            attacks = attackMask & opposition;
            freeSpace = attackMask & emptySquares;

            // Lines of attack for queen
            U64 blocked = PEXT::getBishopAttacks(index, occupancy) | PEXT::getRookAttacks(index, occupancy);
            attacks &= blocked;
            freeSpace &= blocked;

            while (attacks) {
                int targetSquare = std::countr_zero(attacks);
                moves[moveNum++] = DenseMove(W_QUEEN, index, targetSquare, 
                                board.getDenseTypeAt(targetSquare), false, false,
                                D_EMPTY);
                attacks &= (attacks - 1);
            }
            while (freeSpace) {
                int targetSquare = std::countr_zero(freeSpace);
                moves[moveNum++] = DenseMove(W_QUEEN, index, targetSquare, 
                                D_EMPTY, false, false, D_EMPTY);
                freeSpace &= (freeSpace - 1);
            }
            pieceBB &= (pieceBB - 1);
        }
        // Rooks
        pieceBB = board.getWhiteRooks();
        while (pieceBB) {
            int index = std::countr_zero(pieceBB);
            attackMask = ATKMASK_ROOK[index];
            attacks = attackMask & opposition;
            freeSpace = attackMask & emptySquares;

            // Lines of attack for rook
            U64 blocked = PEXT::getRookAttacks(index, occupancy);
            attacks &= blocked;
            freeSpace &= blocked;

            while (attacks) {
                int targetSquare = std::countr_zero(attacks);
                moves[moveNum++] = DenseMove(W_ROOK, index, targetSquare, 
                                board.getDenseTypeAt(targetSquare), false, false,
                                D_EMPTY);
                attacks &= (attacks - 1);
            }
            while (freeSpace) {
                int targetSquare = std::countr_zero(freeSpace);
                moves[moveNum++] = DenseMove(W_ROOK, index, targetSquare, 
                                D_EMPTY, false, false, D_EMPTY);
                freeSpace &= (freeSpace - 1);
            }
            pieceBB &= (pieceBB - 1);
        }
        // Knights
        pieceBB = board.getWhiteKnights();
        while (pieceBB) {
            int index = std::countr_zero(pieceBB);
            attackMask = ATKMASK_KNIGHT[index];
            attacks = attackMask & opposition;
            freeSpace = attackMask & emptySquares;

            while (attacks) {
                int targetSquare = std::countr_zero(attacks);
                moves[moveNum++] = DenseMove(W_KNIGHT, index, targetSquare, 
                                board.getDenseTypeAt(targetSquare), false, false,
                                D_EMPTY);
                attacks &= (attacks - 1);
            }
            while (freeSpace) {
                int targetSquare = std::countr_zero(freeSpace);
                moves[moveNum++] = DenseMove(W_KNIGHT, index, targetSquare, 
                                D_EMPTY, false, false, D_EMPTY);
                freeSpace &= (freeSpace - 1);
            }
            pieceBB &= (pieceBB - 1);
        }
        // Bishops
        pieceBB = board.getWhiteBishops();
        while (pieceBB) {
            int index = std::countr_zero(pieceBB);
            attackMask = ATKMASK_BISHOP[index];
            attacks = attackMask & opposition;
            freeSpace = attackMask & emptySquares;

            // Lines of attack for bishop
            U64 blocked = PEXT::getBishopAttacks(index, occupancy);
            attacks &= blocked;
            freeSpace &= blocked;

            while (attacks) {
                int targetSquare = std::countr_zero(attacks);
                moves[moveNum++] = DenseMove(W_BISHOP, index, targetSquare, 
                                board.getDenseTypeAt(targetSquare), false, false,
                                D_EMPTY);
                attacks &= (attacks - 1);
            }
            while (freeSpace) {
                int targetSquare = std::countr_zero(freeSpace);
                moves[moveNum++] = DenseMove(W_BISHOP, index, targetSquare, 
                                D_EMPTY, false, false, D_EMPTY);
                freeSpace &= (freeSpace - 1);
            }
            pieceBB &= (pieceBB - 1);
        }
        // King
        pieceBB = board.getWhiteKings();
        while (pieceBB) {
            int index = std::countr_zero(pieceBB);
            attackMask = ATKMASK_KING[index];
            attacks = attackMask & opposition;
            freeSpace = attackMask & emptySquares;

            while (attacks) {
                int targetSquare = std::countr_zero(attacks);
                moves[moveNum++] = DenseMove(W_KING, index, targetSquare, 
                                board.getDenseTypeAt(targetSquare), false, false,
                                D_EMPTY);
                attacks &= (attacks - 1);
            }
            while (freeSpace) {
                int targetSquare = std::countr_zero(freeSpace);
                moves[moveNum++] = DenseMove(W_KING, index, targetSquare, 
                                D_EMPTY, false, false, D_EMPTY);
                freeSpace &= (freeSpace - 1);
            }
            pieceBB &= (pieceBB - 1);
        }
    } else {
        U64 opposition = board.getWhitePieces();
        // Mask of attacks from index
        U64 attackMask;
        // Actual attacks from index
        U64 attacks;
        // Free spaces from index
        U64 freeSpace;

        // Queens
        pieceBB = board.getBlackQueens();
        while (pieceBB) {
            int index = std::countr_zero(pieceBB);
            attackMask = ATKMASK_QUEEN[index];
            attacks = attackMask & opposition;
            freeSpace = attackMask & emptySquares;

            // Lines of attack for queen
            U64 blocked = PEXT::getBishopAttacks(index, occupancy) | PEXT::getRookAttacks(index, occupancy);
            attacks &= blocked;
            freeSpace &= blocked;

            while (attacks) {
                int targetSquare = std::countr_zero(attacks);
                moves[moveNum++] = DenseMove(B_QUEEN, index, targetSquare, 
                                board.getDenseTypeAt(targetSquare), false, false,
                                D_EMPTY);
                attacks &= (attacks - 1);
            }
            while (freeSpace) {
                int targetSquare = std::countr_zero(freeSpace);
                moves[moveNum++] = DenseMove(B_QUEEN, index, targetSquare, 
                                D_EMPTY, false, false, D_EMPTY);
                freeSpace &= (freeSpace - 1);
            }
            pieceBB &= (pieceBB - 1);
        }
        // Rooks
        pieceBB = board.getBlackRooks();
        while (pieceBB) {
            int index = std::countr_zero(pieceBB);
            attackMask = ATKMASK_ROOK[index];
            attacks = attackMask & opposition;
            freeSpace = attackMask & emptySquares;

            // Lines of attack for rook
            U64 blocked = PEXT::getRookAttacks(index, occupancy);
            attacks &= blocked;
            freeSpace &= blocked;

            while (attacks) {
                int targetSquare = std::countr_zero(attacks);
                moves[moveNum++] = DenseMove(B_ROOK, index, targetSquare, 
                                board.getDenseTypeAt(targetSquare), false, false,
                                D_EMPTY);
                attacks &= (attacks - 1);
            }
            while (freeSpace) {
                int targetSquare = std::countr_zero(freeSpace);
                moves[moveNum++] = DenseMove(B_ROOK, index, targetSquare, 
                                D_EMPTY, false, false, D_EMPTY);
                freeSpace &= (freeSpace - 1);
            }
            pieceBB &= (pieceBB - 1);
        }
        // Knights
        pieceBB = board.getBlackKnights();
        while (pieceBB) {
            int index = std::countr_zero(pieceBB);
            attackMask = ATKMASK_KNIGHT[index];
            attacks = attackMask & opposition;
            freeSpace = attackMask & emptySquares;

            while (attacks) {
                int targetSquare = std::countr_zero(attacks);
                moves[moveNum++] = DenseMove(B_KNIGHT, index, targetSquare, 
                                board.getDenseTypeAt(targetSquare), false, false,
                                D_EMPTY);
                attacks &= (attacks - 1);
            }
            while (freeSpace) {
                int targetSquare = std::countr_zero(freeSpace);
                moves[moveNum++] = DenseMove(B_KNIGHT, index, targetSquare, 
                                D_EMPTY, false, false, D_EMPTY);
                freeSpace &= (freeSpace - 1);
            }
            pieceBB &= (pieceBB - 1);
        }
        // Bishops
        pieceBB = board.getBlackBishops();
        while (pieceBB) {
            int index = std::countr_zero(pieceBB);
            attackMask = ATKMASK_BISHOP[index];
            attacks = attackMask & opposition;
            freeSpace = attackMask & emptySquares;

            // Lines of attack for bishop
            U64 blocked = PEXT::getBishopAttacks(index, occupancy);
            attacks &= blocked;
            freeSpace &= blocked;

            while (attacks) {
                int targetSquare = std::countr_zero(attacks);
                moves[moveNum++] = DenseMove(B_BISHOP, index, targetSquare, 
                                board.getDenseTypeAt(targetSquare), false, false,
                                D_EMPTY);
                attacks &= (attacks - 1);
            }
            while (freeSpace) {
                int targetSquare = std::countr_zero(freeSpace);
                moves[moveNum++] = DenseMove(B_BISHOP, index, targetSquare, 
                                D_EMPTY, false, false, D_EMPTY);
                freeSpace &= (freeSpace - 1);
            }
            pieceBB &= (pieceBB - 1);
        }
        // King
        pieceBB = board.getBlackKings();
        while (pieceBB) {
            int index = std::countr_zero(pieceBB);
            attackMask = ATKMASK_KING[index];
            attacks = attackMask & opposition;
            freeSpace = attackMask & emptySquares;

            while (attacks) {
                int targetSquare = std::countr_zero(attacks);
                moves[moveNum++] = DenseMove(B_KING, index, targetSquare, 
                                board.getDenseTypeAt(targetSquare), false, false,
                                D_EMPTY);
                attacks &= (attacks - 1);
            }
            while (freeSpace) {
                int targetSquare = std::countr_zero(freeSpace);
                moves[moveNum++] = DenseMove(B_KING, index, targetSquare, 
                                D_EMPTY, false, false, D_EMPTY);
                freeSpace &= (freeSpace - 1);
            }
            pieceBB &= (pieceBB - 1);
        }
    }
}



// Generate the list of moves that can possibly be made by the current side to move,
// not taking into account other legality
std::array<DenseMove, MAX_MOVES> MoveGenerator::generatePsuedoMoves(const ChessBoard &board, int& moveNum) {
    // Move list to be returned
    std::array<DenseMove, MAX_MOVES> move_list = {};
    
    MoveGenerator::generatePieceMoves(board, move_list, moveNum);
    MoveGenerator::generatePawnMoves(board, move_list, moveNum);
    MoveGenerator::generateEnPassantMoves(board, move_list, moveNum);
    MoveGenerator::generateCastlingMoves(board, move_list, moveNum);
    
    return move_list;
}

std::array<DenseMove, MAX_MOVES> MoveGenerator::generateLegalMoves(ChessBoard& board, int& moveNum) {
    if (board.getFEN() == "k7/7Q/2K5/8/8/8/8/8 w - - 0 1" ||
        board.getFEN() == "k7/7Q/2K5/8/8/8/8/8 b - - 1 1") {
        std::cout << std::format("2nd Test board FEN inside genLegal1: {}\n", board.getFEN());
    }
    
    // std::cout << "MoveGenerator.generateLegalMoves start\n";
    // Reset moveNum to 0, since moveNum should always indicate number of moves in array
    moveNum = 0;
    // generatePsuedoMoves will set moveNum to correct number of moves to check
    std::array<DenseMove, MAX_MOVES> legal = generatePsuedoMoves(board, moveNum);

    if (board.getFEN() == "k7/7Q/2K5/8/8/8/8/8 w - - 0 1" ||
        board.getFEN() == "k7/7Q/2K5/8/8/8/8/8 b - - 1 1") {
        std::cout << std::format("2nd Test board FEN inside genLegal2: {}\n", board.getFEN());
    }

    Color sideToMove = board.getSideToMove();
    if (board.getFEN() == "k7/7Q/2K5/8/8/8/8/8 w - - 0 1" ||
        board.getFEN() == "k7/7Q/2K5/8/8/8/8/8 b - - 1 1") {
        std::cout << std::format("2nd Test board FEN inside genLegal3: {}\n", board.getFEN());
    }

    std::string prevFEN = board.getFEN();
    std::string newFEN;
    // std::vector<DenseMove> legal;
    // For each psuedo legal move generated
    for (int i = 0; i < moveNum; i) {
        // std::cout << "psuedo move: " << legal[i].toString(false) << "\n";
        // Make the move on the board
        board.makeMove(legal[i], true);
        // std::cout << "    made move\n";
        // If the move did not leave its own side in check,
        // its a legal move, add to list
        if (board.isSideInCheck(sideToMove)) {
            // Unmake the move to return board to prev state
            board.unmakeMove(legal[i], true);
            legal[i] = legal[--moveNum];
            newFEN = board.getFEN();
            if (newFEN != prevFEN) {
                std::cout << std::format("i {} move 'illegal' {}\nPrevFEN {} NewFen {}\n", i, legal[i].toAlgebraic(), prevFEN, newFEN);
            }
            // moveNum--;
            continue;
        }
        board.unmakeMove(legal[i], true);
        i++;
        newFEN = board.getFEN();
        if (newFEN != prevFEN) {
            std::cout << std::format("i {} move 'legal' {}\nPrevFEN {} NewFen {}\n", i, legal[i].toAlgebraic(), prevFEN, newFEN);
        }
    }

    if (board.getFEN() == "k7/7Q/2K5/8/8/8/8/8 w - - 0 1" ||
        board.getFEN() == "k7/7Q/2K5/8/8/8/8/8 b - - 1 1") {
        std::cout << std::format("2nd Test board FEN inside genLegal4: {}\nstateHistory:", board.getFEN());
        board.printStateHistory();
    }

    // No legal moves, ensure indices are empty move
    if (moveNum == 0) legal.fill(DenseMove());
    // std::cout << "    generateLegalMoves returning, size " << moveNum << "\n";
    return legal;
}
