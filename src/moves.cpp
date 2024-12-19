// src/moves.cpp
#include "moves.hpp"
#include <bit>
#include <vector>
#include <algorithm>
#include <format>
#include <string>
#include <math.h>



// Forward declarations
std::vector<Move> generatePsuedoMoves(const ChessBoard &cboard, const GameState* state);

// Helper method to check if a move leaves the king in check
bool MoveValidator::moveLeavesKingInCheck(const Move& move) {
    // Create a backup board to hold original 
    ChessBoard keepBoard = board;
    // GameState state = GameState();
    //printFEN(board, *state);
    makeMove(board, move);
    //printFEN(board, *state);
    // Find king's position
    int kingPos = std::countr_zero(state->sideToMove == WHITE ? 
                    board.getWhiteKings() : board.getBlackKings());
    
    // Check if king is under attack after move
    bool attacked = isSquareUnderAttack(kingPos, state->sideToMove == WHITE ? BLACK : WHITE);
    // Restore board before returning
    board = keepBoard;
    //std::cout << "attacked: " << attacked << "\n";
    return attacked;
}

// Validate castling move
bool MoveValidator::isValidCastling(const Move& move) {
    if (!move.isCastle) return false;
    
    Color color = state->sideToMove;
    bool isKingside = (move.to % 8 > move.from % 8);
    
    // Check if castling rights are still available
    if (color == WHITE) {
        if (isKingside && !state->canCastleWhiteKingside) return false;
        if (!isKingside && !state->canCastleWhiteQueenside) return false;
    } else {
        if (isKingside && !state->canCastleBlackKingside) return false;
        if (!isKingside && !state->canCastleBlackQueenside) return false;
    }
    
    // Check if squares between king and rook are empty
    U64 occupancy = board.getAllPieces();
    if (isKingside) {
        U64 kingToRook = (color == WHITE) ? 0x60ULL : 0x6000000000000000ULL;
        if (occupancy & kingToRook) return false;
    } else {
        U64 kingToRook = (color == WHITE) ? 0xEULL : 0xE00000000000000ULL;
        if (occupancy & kingToRook) return false;
    }
    
    // Check if king is in check
    int kingPos = move.from;
    if (isSquareUnderAttack(kingPos, color == WHITE ? BLACK : WHITE))
        return false;
        
    // Check if king passes through check
    int step = isKingside ? 1 : -1;
    int passThrough = kingPos + step;
    if (isSquareUnderAttack(passThrough, color == WHITE ? BLACK : WHITE))
        return false;
        
    return true;
}

// Validate en passant move
bool MoveValidator::isValidEnPassant(const Move& move) {
    if (!move.isEnPassant) return false;
    
    // Check if en passant square matches
    // NOTE: expects enPassantSquare to be the square behind the enpassantable pawn
    if (move.to != state->enPassantSquare) return false;
    
    // Check if moving piece is a pawn
    if (move.piece != W_PAWN && move.piece != B_PAWN) return false;
    
    // Verify correct capture direction
    int expectedFromRank = (state->sideToMove == WHITE) ? 4 : 3;
    if (move.from / 8 != expectedFromRank) return false;
    
    return true;
}

// Helper function to find king's position
int MoveValidator::findKingSquare(Color color) const {
    // Get the appropriate king's bitboard
    U64 kingBB = (color == WHITE) ? board.getWhiteKings() : board.getBlackKings();
    // Use countr_zero to find the index of the set bit (king's position)
    return std::countr_zero(kingBB);
}

// Helper to check if a move is legally possible
bool MoveValidator::canMakeMove(Move& move, Color color) {
    // Create keepsake board
    ChessBoard keepBoard = board;
    
    makeMove(board, move);
    // Check if the move leaves or puts the king in check
    bool notChecked = !isInCheck(color);    // not checked, so you can make the move
    board = keepBoard;  // Restore board
    return notChecked;
}


MoveValidator::MoveValidator(ChessBoard& b, GameState* s) : board(b), state(s) {}

// Helper method to check if a square is under attack
bool MoveValidator::isSquareUnderAttack(int square, Color attackingColor) {
    return board.attacksToSquare(square, attackingColor == WHITE ? BLACK : WHITE);
}

// Check if a specific color is in check
bool MoveValidator::isInCheck(Color color) const {
    // Find king's square
    std::cout << "checking if color in check\n";
    int kingSquare = findKingSquare(color);
    std::cout << "king square: " << kingSquare << "\n";
    
    // Check if king's square is under attack by opposite color
    return board.attacksToSquare(kingSquare, color);
}

// Check if a specific color is in checkmate
bool MoveValidator::isCheckmate(Color color) {
    // First check if the player is in check
    if (!isInCheck(color)) {
        std::cout << "not in check\n";
        return false;
    }
    std::cout << "in check, checking mate\n";
    // Generate all possible moves for this color
    std::vector<Move> possibleMoves = generatePsuedoMoves(board, state);
    std::cout << "possible moves: " << possibleMoves.size() << "\n";
    
    // Try each move to see if it gets out of check
    for (Move& move : possibleMoves) {
        std::cout << "move: " << move.from << " " << move.to << " " << move.piece << "\n";
        // Skip moves of the wrong color
        if ((color == WHITE && move.piece >= B_PAWN) ||
            (color == BLACK && move.piece <= W_KING)) {
            std::cout << "skipping\n";
            continue;
        }

        std::cout << "not skipping\n";

        // If any legal move exists, not checkmate
        if (canMakeMove(move, color)) {
            return false;
        }
        std::cout << "can't make move\n";
    }

    // No legal moves found, it's checkmate
    return true;
}

// Check if a specific color is in stalemate
bool MoveValidator::isStalemate(Color color) {
    // If in check, not stalemate
    if (isInCheck(color)) {
        return false;
    }

    // Generate all possible moves for this color
    std::vector<Move> possibleMoves = generatePsuedoMoves(board, state);
    
    // Try each move to see if any are legal
    for (Move& move : possibleMoves) {
        // Skip moves of the wrong color
        if ((color == WHITE && move.piece >= B_PAWN) ||
            (color == BLACK && move.piece <= W_KING)) {
            continue;
        }

        // If any legal move exists, not stalemate
        if (canMakeMove(move, color)) {
            std::cout << "can make move: " << move.from << " " << move.to << " " << move.piece << "\n";
            return false;
        }
    }

    // No legal moves found but not in check, it's stalemate
    return true;
}

bool MoveValidator::isMoveLegal(const Move& move) {
    // Check if it's the correct side's turn
    bool isWhitePiece = move.piece <= W_KING;
    if ((state->sideToMove == WHITE) != isWhitePiece) return false;
    
    // Handle special moves
    if (move.isCastle && !isValidCastling(move)) return false;
    if (move.isEnPassant && !isValidEnPassant(move)) return false;
    
    // Check promotion validity
    if (move.isPromotion) {
        if (move.piece != W_PAWN && move.piece != B_PAWN) return false;
        int promRank = (move.piece == W_PAWN) ? 7 : 0;
        if (move.to / 8 != promRank) return false;
    }
    
    // Verify the move doesn't leave king in check
    if (moveLeavesKingInCheck(move)) return false;
    
    return true;
}

// Update game state after a move
void MoveValidator::updateGameState(const Move& move) {
    // Update castling rights
    if (move.piece == W_KING) {
        state->canCastleWhiteKingside = false;
        state->canCastleWhiteQueenside = false;
        std::cout << "white king moved\n";
    } else if (move.piece == B_KING) {
        state->canCastleBlackKingside = false;
        state->canCastleBlackQueenside = false;
        std::cout << "black king moved\n";
    }
    
    // Check if rooks moved or were captured
    if (move.from == 0 || move.to == 0) state->canCastleWhiteQueenside = false;
    if (move.from == 7 || move.to == 7) state->canCastleWhiteKingside = false;
    if (move.from == 56 || move.to == 56) state->canCastleBlackQueenside = false;
    if (move.from == 63 || move.to == 63) state->canCastleBlackKingside = false;
    
    // Update en passant square
    if ((move.piece == W_PAWN || move.piece == B_PAWN) &&
        abs(move.to - move.from) == 16) {
        state->enPassantSquare = (move.from + move.to) / 2;
    } else {
        state->enPassantSquare = -1;
    }
    
    // Switch side to move
    state->sideToMove = (state->sideToMove == WHITE) ? BLACK : WHITE;
    std::cout << "side to move: " << state->sideToMove << "\n";
}

// Helper function to update bitboards for a piece movement
void movePiece(ChessBoard& board, int from, int to, PieceType piece) {
    // Clear bit at 'from' square and set bit at 'to' square for the piece's bitboard
    std::cout << "moving piece\n";
    U64 fromToBB = (1ULL << from) | (1ULL << to);  // Bitboard with both squares set
    printBBLine(fromToBB);

    // Clear bit at 'from' square and set bit at 'to' square for the piece's bitboard
    // Get color and piece type
    bool isWhite = piece <= W_KING;
    std::cout << "piece: " << piece << "\n";
    int pieceType = pieceCode(piece);
    std::cout << "piece type: " << pieceType << "\n";

    // Update color bitboard (white or black)
    std::cout << "before\n";
    printBBLine(board.pieceBB[isWhite ? 0 : 1]);
    board.pieceBB[isWhite ? 0 : 1] ^= fromToBB;
    std::cout << "after\n";
    printBBLine(board.pieceBB[isWhite ? 0 : 1]);

    // Update piece type bitboard (pawn, knight, etc.)
    board.pieceBB[pieceType] ^= fromToBB;
    printBBLine(board.pieceBB[pieceType]);
}

// Helper function to remove a piece from the board
void removePiece(ChessBoard& board, int square, PieceType piece) {
    U64 squareBB = 1ULL << square;
    std::cout << "removing piece\n";
    printBBLine(squareBB);

    // Get color and piece type
    bool isWhite = piece <= W_KING;
    std::cout << "piece color to remove: " << piece << "\n";
    int pieceType = pieceCode(piece);
    std::cout << "piece type to remove: " << pieceType << "\n";

    // Clear bit in color bitboard
    std::cout << "before removal (color)\n";
    printBBLine(board.pieceBB[isWhite ? 0 : 1]);
    board.pieceBB[isWhite ? 0 : 1] &= ~squareBB;
    std::cout << "after removal (color)\n";
    printBBLine(board.pieceBB[isWhite ? 0 : 1]);


    // Clear bit in piece type bitboard
    std::cout << "before removal (piece type)\n";
    printBBLine(board.pieceBB[pieceType]);
    board.pieceBB[pieceType] &= ~squareBB;
    std::cout << "after removal (piece type)\n";
    printBBLine(board.pieceBB[pieceType]);
}

// Helper function to add a piece to the board
void addPiece(ChessBoard& board, int square, PieceType piece) {
    U64 squareBB = 1ULL << square;

    // Get color and piece type
    bool isWhite = piece <= W_KING;
    int pieceType = pieceCode(piece);

    // Set bit in color bitboard
    board.pieceBB[isWhite ? 0 : 1] |= squareBB;

    // Set bit in piece type bitboard
    board.pieceBB[pieceType] |= squareBB;
}

// Function to determine what piece type is on a given square
PieceType getPieceOnSquare(const ChessBoard& board, int square) {
    return board.getPieceAt(square);
}

void makeMove(ChessBoard& board, const Move& move) {
    // Handle captures first (except en passant)
    //std::cout << "making move\n";
    
    if (move.isCapture && !move.isEnPassant) {
        // Remove captured piece
        PieceType capturedPiece = board.getPieceAt(move.to);
        std::cout << "piece captured: " << capturedPiece << "\n";
        removePiece(board, move.to, capturedPiece);
        std::cout << "piece removed\n";
    }

    if (move.isCastle) {
        // Move king
        movePiece(board, move.from, move.to, move.piece);
        
        // Move rook based on castle type
        bool isKingside = (move.to % 8 > move.from % 8);
        bool isWhite = move.piece == W_KING;
        
        if (isKingside) {
            // Move rook from h1/h8 to f1/f8
            int rookFrom = isWhite ? 7 : 63;
            int rookTo = isWhite ? 5 : 61;
            movePiece(board, rookFrom, rookTo, isWhite ? W_ROOK : B_ROOK);
        } else {
            // Move rook from a1/a8 to d1/d8
            int rookFrom = isWhite ? 0 : 56;
            int rookTo = isWhite ? 3 : 59;
            movePiece(board, rookFrom, rookTo, isWhite ? W_ROOK : B_ROOK);
        }
    } else if (move.isEnPassant) {
        // Move capturing pawn
        movePiece(board, move.from, move.to, move.piece);
        
        // Remove captured pawn
        int capturedPawnSquare = move.to + (move.piece <= W_KING ? -8 : 8);
        removePiece(board, capturedPawnSquare, 
                    move.piece <= W_KING ? B_PAWN : W_PAWN);
    } else if (move.isPromotion) {
        // Remove pawn from source square
        removePiece(board, move.from, move.piece);
        
        // Add promoted piece to destination square
        addPiece(board, move.to, move.promoteTo);
    } else {
        // Regular move
        movePiece(board, move.from, move.to, move.piece);
    }
}


void initializePawnAttacksWhite() {
    //std::cout << "init pawn attacks...\n";
    for (int i = 0; i < 64; i++) {
        U64 attacks = 0ULL;
        if (i % 8 != 0) attacks |= (1ULL << (i + 7));   // Left diagonal
        if (i % 8 != 7) attacks |= (1ULL << (i + 9));   // Right diagonal
        pawnAttacksWhite[i] = attacks;
    }
}

void initializePawnAttacksBlack() {
    //std::cout << "init pawn attacks...\n";
    for (int i = 0; i < 64; i++) {
        U64 attacks = 0ULL;
        if (i % 8 != 0) attacks |= (1ULL << (i - 9));   // Left diagonal
        if (i % 8 != 7) attacks |= (1ULL << (i - 7));   // Right diagonal
        pawnAttacksBlack[i] = attacks;
    }
}

void initializeKnightAttacks() {
    //std::cout << "init knight attacks...\n";
    for (int i = 0; i < 64; i++) {
        U64 attacks = 0ULL;
        if (i % 8 != 0) {    // center left
            if (i - 17 >= 0) attacks |= (1ULL << (i - 17));     // bottom square
            if (i + 15 < 64) attacks |= (1ULL << (i + 15));     // top square
        }
        if (i % 8 != 7) {    // center right
            if (i - 15 >= 0) attacks |= (1ULL << (i - 15));     // bottom square
            if (i + 17 < 64) attacks |= (1ULL << (i + 17));     // top square
        }
        if (i % 8 > 1) {     // middle left
            if (i - 10 >= 0) attacks |= (1ULL << (i - 10));     // low square
            if (i + 6 < 64) attacks |= (1ULL << (i + 6));       // high square
        }
        if (i % 8 < 6) {     // middle right
            if (i - 6 >= 0) attacks |= (1ULL << (i - 6));       // low square
            if (i + 10 < 64) attacks |= (1ULL << (i + 10));     // high square
        }
        knightAttacks[i] = attacks; 
    }
}

void initializeBishopAttacks() {
    //std::cout << "init bishop attacks...\n";
    for (int i = 0; i < 64; i++) {
        U64 attacks = 0ULL;
        // Top right diagonal
        int tr = 9;
        while ((i + tr) < 64 && ((i + tr) % 8 != 0)) {
            attacks |= (1ULL << (i + tr));
            tr += 9;
        }
        // Bottom right diagonal
        int br = 7;
        while ((i - br) >= 0 && ((i - br) % 8 != 0)) {
            attacks |= (1ULL << (i - br));
            br += 7;
        }
        // Bottom left diagonal
        int bl = 9;
        while ((i - bl) >= 0 && ((i - bl) % 8 != 7)) {
            attacks |= (1ULL << (i - bl));
            bl += 9;
        }
        // Top left diagonal
        int tl = 7;
        while ((i + tl) < 64 && ((i + tl) % 8 != 7)) {
            attacks |= (1ULL << (i + tl));
            tl += 7;
        }
        bishopAttacks[i] = attacks;
    }
}

void initializeRookAttacks() {
    //std::cout << "init rook attacks...\n";
    for (int i = 0; i < 64; i++) {
        U64 attacks = 0ULL;
        // Rank mask: all squares on the same rank (horizontal)
        attacks |= 0x00000000000000FFULL << (8*(i / 8));
        
        // File mask: all squares on the same file (vertical)
        attacks |= 0x0101010101010101ULL << (i % 8);

        // Remove the rook's current position from the attack mask
        attacks ^= (1ULL << i);
        rookAttacks[i] = attacks;
    }
}

void initializeQueenAttacks() {
    //std::cout << "init queen attacks...\n";
    for (int i = 0; i < 64; i++) {
        queenAttacks[i] = bishopAttacks[i] | rookAttacks[i];
    }
}

void initializeKingAttacks() {
    //std::cout << "init king attacks...\n";
    for (int i = 0; i < 64; i++) {
        U64 attacks = 0ULL;
        if (i % 8 != 7) {
            attacks |= (1ULL << (i + 1));       // Right square
            if (i - 7 > 0) 
                attacks |= (1ULL << (i - 7));   // Bottom right square
            if (i + 9 < 64)
                attacks |= (1ULL << (i + 9));   // Top right square
        }
        if (i - 8 >= 0) attacks |= (1ULL << (i - 8));   // Bottom square
        if (i + 8 < 64) attacks |= (1ULL << (i + 8));   // Top square
        if (i % 8 != 0) {
            attacks |= (1ULL << (i - 1));       // Left square
            if (i - 9 >= 0)
                attacks |= (1ULL << (i - 9));   // Bottom left square
            if (i + 7 < 64)
                attacks |= (1ULL << (i + 7));   // Top left square
        }
        kingAttacks[i] = attacks;
    }
}

void initializeAllAttacks() {
    //std::cout << "initializing...\n";
    initializePawnAttacksWhite();
    initializePawnAttacksBlack();
    initializeKnightAttacks();
    initializeBishopAttacks();
    initializeRookAttacks();
    initializeQueenAttacks();
    initializeKingAttacks();
    //std::cout << "initialized...\n";
}

bool isWithinBoard(int index) {
    return index >= 0 && index < 64;
}

void generateWhitePawnMoves(const ChessBoard& cboard, const GameState* state, std::vector<Move>& moves) {
    U64 whitePawns = cboard.getWhitePawns();
    U64 emptySquares = ~(cboard.getAllPieces());

    // Calculate all capturing moves for white pawns
    while (whitePawns) {
        int index = std::countr_zero(whitePawns);        // Get index of the lowest set bit
        // Use the precomputed attack mask
        U64 attackMask = pawnAttacksWhite[index] & cboard.getBlackPieces();

        // Loop through each bit in the attack mask
        while (attackMask) {
            int targetSquare = std::countr_zero(attackMask);     // Get index of the lowest set bit
            // If 7th rank, move to 8th rank and promote
            if (index >= 48 && index < 56) {
                moves.push_back({W_PAWN, index, targetSquare, true, true});
            }
            else {
                moves.push_back({W_PAWN, index, targetSquare, true, false});
            }
            attackMask &= (attackMask - 1);     // Clear the least significant bit
        }
        whitePawns &= (whitePawns - 1);     // Clear the least significant bit
    }
    // Reset whitePawns bitboard
    whitePawns = cboard.getWhitePawns();
    // One square forward
    U64 singlePushes = (whitePawns << 8) & emptySquares;

    // Two squares forward
    // Hex value: each hex digit is 4 bits; 0x0000000000FF corresponds to bits in 3rd rank
    U64 doublePushes = ((singlePushes & 0x0000000000FF0000) << 8) & emptySquares;

    while (singlePushes) {
        int targetSquare = std::countr_zero(singlePushes);
        // If on 7th rank, move to 8th and promote
        if (targetSquare >= 56 && targetSquare < 64) {
            moves.push_back({W_PAWN, targetSquare - 8, targetSquare, false, true});
        } 
        else {
            moves.push_back({W_PAWN, targetSquare - 8, targetSquare, false, false});
        }
        singlePushes &= (singlePushes - 1);
    }

    while (doublePushes) {
        // Can only move two squares if on starting row
        int targetSquare = std::countr_zero(doublePushes);
        moves.push_back({W_PAWN, targetSquare - 16, targetSquare, false, false});
        doublePushes &= (doublePushes - 1);
    }
}

// Generates all psuedo-legal pawn moves
void generatePawnMoves(const ChessBoard& cboard, const GameState* state, std::vector<Move>& moves) {
    U64 emptySquares = ~(cboard.getAllPieces());

    if (state->sideToMove == WHITE) {
        U64 whitePawns = cboard.getWhitePawns();
        // Calculate all capturing moves for white pawns
        while (whitePawns) {
            int index = std::countr_zero(whitePawns);        // Get index of the lowest set bit
            // Use the precomputed attack mask
            U64 attackMask = pawnAttacksWhite[index] & cboard.getBlackPieces();

            // Loop through each bit in the attack mask
            while (attackMask) {
                int targetSquare = std::countr_zero(attackMask);     // Get index of the lowest set bit
                // If 7th rank, move to 8th rank and promote
                if (index >= 48 && index < 56) {
                    Move promoteMove = Move(W_PAWN, index, targetSquare, true, true);
                    //if (moveLeavesKingInCheck())
                    promoteMove.promoteTo = W_KNIGHT;
                    moves.push_back(promoteMove);
                    promoteMove.promoteTo = W_BISHOP;
                    moves.push_back(promoteMove);
                    promoteMove.promoteTo = W_ROOK;
                    moves.push_back(promoteMove);
                    promoteMove.promoteTo = W_QUEEN;
                    moves.push_back(promoteMove);
                }
                else {
                    moves.push_back({W_PAWN, index, targetSquare, true, false});
                }
                attackMask &= (attackMask - 1);     // Clear the least significant bit
            }
            whitePawns &= (whitePawns - 1);     // Clear the least significant bit
        }
        // Reset whitePawns bitboard
        whitePawns = cboard.getWhitePawns();
        // One square forward
        U64 singlePushes = (whitePawns << 8) & emptySquares;

        // Two squares forward
        // Hex value: each hex digit is 4 bits; 0x0000000000FF corresponds to bits in 3rd rank
        U64 doublePushes = ((singlePushes & 0x0000000000FF0000) << 8) & emptySquares;

        while (singlePushes) {
            int targetSquare = std::countr_zero(singlePushes);
            // If on 7th rank, move to 8th and promote
            if (targetSquare >= 56 && targetSquare < 64) {
                Move promoteMove = Move(W_PAWN, targetSquare - 8, targetSquare, false, true);
                promoteMove.promoteTo = W_KNIGHT;
                moves.push_back(promoteMove);
                promoteMove.promoteTo = W_BISHOP;
                moves.push_back(promoteMove);
                promoteMove.promoteTo = W_ROOK;
                moves.push_back(promoteMove);
                promoteMove.promoteTo = W_QUEEN;
                moves.push_back(promoteMove);
            } 
            else {
                moves.push_back({W_PAWN, targetSquare - 8, targetSquare, false, false});
            }
            singlePushes &= (singlePushes - 1);
        }

        while (doublePushes) {
            // Can only move two squares if on starting row
            int targetSquare = std::countr_zero(doublePushes);
            moves.push_back({W_PAWN, targetSquare - 16, targetSquare, false, false});
            doublePushes &= (doublePushes - 1);
        }
    }
    
    else {
        U64 blackPawns = cboard.getBlackPawns();
        // Calculate all capturing moves for black pawns
        while (blackPawns) {
            int index = std::countr_zero(blackPawns);        // Get index of the lowest set bit
            // Use the precomputed attack mask
            U64 attackMask = pawnAttacksBlack[index] & cboard.getWhitePieces();

            // Loop through each bit in the attack mask
            while (attackMask) {
                int targetSquare = std::countr_zero(attackMask);     // Get index of the lowest set bit
                // If 2nd rank, move to 1st rank and promote
                if (index >= 8 && index < 16) {
                    Move promoteMove = Move(B_PAWN, index, targetSquare, true, true);
                    promoteMove.promoteTo = B_KNIGHT;
                    moves.push_back(promoteMove);
                    promoteMove.promoteTo = B_BISHOP;
                    moves.push_back(promoteMove);
                    promoteMove.promoteTo = B_ROOK;
                    moves.push_back(promoteMove);
                    promoteMove.promoteTo = B_QUEEN;
                    moves.push_back(promoteMove);
                }
                else {
                    moves.push_back({B_PAWN, index, targetSquare, true, false});
                }
                attackMask &= (attackMask - 1);     // Clear the least significant bit
            }
            blackPawns &= (blackPawns - 1);     // Clear the least significant bit
        }
        // Reset blackPawns bitboard
        blackPawns = cboard.getBlackPawns();
        // One square forward
        U64 singlePushes = (blackPawns >> 8) & emptySquares;

        // Two squares forward
        // Hex value: each hex digit is 4 bits; 0x0000000000FF corresponds to bits in 6th rank
        U64 doublePushes = ((singlePushes & 0x0000FF0000000000) >> 8) & emptySquares;

        while (singlePushes) {
            int targetSquare = std::countr_zero(singlePushes);
            // If on 2nd rank, move to 1st and promote
            if (targetSquare >= 0 && targetSquare < 8) {
                Move promoteMove = Move(B_PAWN, targetSquare + 8, targetSquare, false, true);
                promoteMove.promoteTo = B_KNIGHT;
                moves.push_back(promoteMove);
                promoteMove.promoteTo = B_BISHOP;
                moves.push_back(promoteMove);
                promoteMove.promoteTo = B_ROOK;
                moves.push_back(promoteMove);
                promoteMove.promoteTo = B_QUEEN;
                moves.push_back(promoteMove);
            } 
            else {
                moves.push_back({B_PAWN, targetSquare + 8, targetSquare, false, false});
            }
            singlePushes &= (singlePushes - 1);
        }

        while (doublePushes) {
            // Can only move two squares if on starting row
            int targetSquare = std::countr_zero(doublePushes);
            moves.push_back({B_PAWN, targetSquare + 16, targetSquare, false, false});
            doublePushes &= (doublePushes - 1);
        }
    }    
}

// Generate all possible en passant captures based on current game state
void generateEnPassantMoves(const ChessBoard &cboard, const GameState* state, std::vector<Move> &moves) {
    // If no en passant square is available, return
    if (state->enPassantSquare == -1) return;

    // Get pawns that could potentially make en passant capture
    if (state->sideToMove == WHITE) {
        // Get white pawns that can capture the black pawn via en passant
        U64 epCaptors = pawnAttacksBlack[state->enPassantSquare] & cboard.getWhitePawns();
        
        // For each white pawn that can make the capture
        while (epCaptors) {
            int from = std::countr_zero(epCaptors);  // Get index of capturing pawn
            Move move{W_PAWN, from, state->enPassantSquare, true, false};
            move.isEnPassant = true;
            moves.push_back(move);
            epCaptors &= (epCaptors - 1);  // Clear least significant bit
        }
    } else {
        // Get black pawns that can capture the white pawn via en passant
        U64 epCaptors = pawnAttacksWhite[state->enPassantSquare] & cboard.getBlackPawns();
        
        // For each black pawn that can make the capture
        while (epCaptors) {
            int from = std::countr_zero(epCaptors);  // Get index of capturing pawn
            Move move{B_PAWN, from, state->enPassantSquare, true, false};
            move.isEnPassant = true;
            moves.push_back(move);
            epCaptors &= (epCaptors - 1);  // Clear least significant bit
        }
    }
}

void generateCastlingMoves(const ChessBoard& cboard, const GameState* state, std::vector<Move>& moves) {
    if (state->sideToMove == WHITE) {
        if (state->canCastleWhiteKingside) {
            Move move(W_KING, 4, 6);
            move.isCastle = true;
            moves.push_back(move);
        }
        if (state->canCastleWhiteQueenside) {
            Move move(W_KING, 4, 2);
            move.isCastle = true;
            moves.push_back(move);
        }
    } else {
        if (state->canCastleBlackKingside) {
            Move move(B_KING, 60, 62);
            move.isCastle = true;
            moves.push_back(move);
        }
        if (state->canCastleBlackQueenside) {
            Move move(B_KING, 60, 58);
            move.isCastle = true;
            moves.push_back(move);
        }
    }
}

// Generate all psuedo-legal moves for one piecetype
void generatePieceMoves(const ChessBoard &cboard, std::vector<Move> &moves, PieceType piece) {
    U64 pieceBB;
    U64 opposition;
    U64 emptySquares = ~(cboard.getAllPieces());

    switch (piece) {
        case W_PAWN:
            // pieceBB = cboard.getWhitePawns();
            // opposition = cboard.getBlackPieces();
            break;
        case W_KNIGHT:
            pieceBB = cboard.getWhiteKnights();
            opposition = cboard.getBlackPieces();
            break;
        case W_BISHOP:
            pieceBB = cboard.getWhiteBishops();
            opposition = cboard.getBlackPieces();
            break;
        case W_ROOK:
            pieceBB = cboard.getWhiteRooks();
            opposition = cboard.getBlackPieces();
            break;
        case W_QUEEN:
            pieceBB = cboard.getWhiteQueens();
            opposition = cboard.getBlackPieces();
            break;
        case W_KING:
            pieceBB = cboard.getWhiteKings();
            opposition = cboard.getBlackPieces();
            break;
        case B_PAWN:
            // pieceBB = cboard.getWhiteKnights();
            // opposition = cboard.getBlackPieces();
            break;
        case B_KNIGHT:
            pieceBB = cboard.getBlackKnights();
            opposition = cboard.getWhitePieces();
            break;
        case B_BISHOP:
            pieceBB = cboard.getBlackBishops();
            opposition = cboard.getWhitePieces();
            break;
        case B_ROOK:
            pieceBB = cboard.getBlackRooks();
            opposition = cboard.getWhitePieces();
            break;
        case B_QUEEN:
            pieceBB = cboard.getBlackQueens();
            opposition = cboard.getWhitePieces();
            break;
        case B_KING:
            pieceBB = cboard.getBlackKings();
            opposition = cboard.getWhitePieces();
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
                attackMask = knightAttacks[index];
                //printBitboard(attackMask);
                break;
            case W_BISHOP:
            case B_BISHOP:
                attackMask = bishopAttacks[index];
                break;
            case W_ROOK:
            case B_ROOK:
                attackMask = rookAttacks[index];
                break;
            case W_QUEEN:
            case B_QUEEN:
                attackMask = queenAttacks[index];
                break;
            case W_KING:
            case B_KING:
                attackMask = kingAttacks[index];
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
            U64 blocked = getRookAttacks(index, ~emptySquares);
            attacks &= blocked;
            freeSpace &= blocked;
        }
        
        if (piece == W_BISHOP || piece == B_BISHOP) {
            U64 blocked = getBishopAttacks(index, ~emptySquares);
            attacks &= blocked;
            freeSpace &= blocked;
        }

        if (piece == W_QUEEN || piece == B_QUEEN){
            U64 blocked = getQueenAttacks(index, ~emptySquares);
            attacks &= blocked;
            freeSpace &= blocked;
            // std::cout << "piece attacks (post-blocking): " << piece << "\n";
            // printBitboard(attacks);
            // std::cout << "piece free spaces (post-blocking): " << piece << "\n";
            // printBitboard(freeSpace);
        }
        

        while (attacks) {
            int targetSquare = std::countr_zero(attacks);       // Get index of the lowest set bit
            moves.push_back({piece, index, targetSquare, true, false});
            attacks &= (attacks - 1);
        }

        while (freeSpace) {
            int targetSquare = std::countr_zero(freeSpace);     // Get index of the lowest set bit
            moves.push_back({piece, index, targetSquare, false, false});
            freeSpace &= (freeSpace - 1);
        }
        pieceBB &= (pieceBB - 1);
    }
}



// Generate the list of moves that can possibly be made by the current side to move,
// not taking into account other legality
std::vector<Move> generatePsuedoMoves(const ChessBoard &cboard, const GameState* state) {
    std::vector<Move> move_list;

    generatePawnMoves(cboard, state, move_list);
    generateEnPassantMoves(cboard, state, move_list);
    generateCastlingMoves(cboard, state, move_list);
    if (state->sideToMove == WHITE) {
        generatePieceMoves(cboard, move_list, W_KNIGHT);
        generatePieceMoves(cboard, move_list, W_BISHOP);
        generatePieceMoves(cboard, move_list, W_ROOK);
        generatePieceMoves(cboard, move_list, W_QUEEN);
        generatePieceMoves(cboard, move_list, W_KING);
    }
    else {
        generatePieceMoves(cboard, move_list, B_KNIGHT);
        generatePieceMoves(cboard, move_list, B_BISHOP);
        generatePieceMoves(cboard, move_list, B_ROOK);
        generatePieceMoves(cboard, move_list, B_QUEEN);
        generatePieceMoves(cboard, move_list, B_KING);
    }
    return move_list;
}

/// @brief Returns a bitboard of possible squares a rook could attack 'square' parameter from
/// @param square 
/// @param blockers 
/// @return 
U64 getRookAttacks(int square, U64 blockers) {
    U64 attacks = rookAttacks[square];

    // std::cout << "initial rook attack bb: \n";
    // printBitboard(attacks);
    // std::cout << "blockers: \n";
    // printBitboard(blockers);

    // Check each direction (N, S, E, W) for blockers
    // South
    for (int sq = square - 8; sq >= 0; sq -= 8) {
        if (blockers & (1ULL << sq)) {
            U64 file = 0ULL;
            //file |= (1ULL * pow(2, sq - 1));
            for (int sSq = sq - 8; sSq >= 0; sSq -= 8) {
                file |= 1ULL << sSq;
            }
            attacks ^= file;
            break;
        }
    }

    // std::cout << "rook attack southed: \n";
    // printBitboard(attacks);

    // West
    for (int sq = square - 1; sq % 8 != 7 && sq >= 0; sq--) {
        if (blockers & (1ULL << sq)) {
            //attacks &= ~(((1ULL << sq) << 1) - 1);
            U64 rank = 0ULL;
            for (int rankSq = sq - 1; rankSq % 8 != 7 && rankSq >= 0; rankSq--) {
                rank |= 1ULL << rankSq;
            }
            attacks ^= rank;
            break;
        }
    }

    // std::cout << "rook attack wested: \n";
    // printBitboard(attacks);

    // East
    for (int sq = square + 1; sq % 8 != 0 && sq < 64; sq++) {
        if (blockers & (1ULL << sq)) {
            //attacks &= ((1ULL << (sq + 1)) - 1);
            U64 rank = 0ULL;
            for (int rankSq = sq + 1; rankSq % 8 != 0 && rankSq < 64; rankSq++) {
                rank |= 1ULL << rankSq;
            }
            attacks ^= rank;
            break;
        }
    }

    // std::cout << "rook attack easted: \n";
    // printBitboard(attacks);

    // North
    for (int sq = square + 8; sq < 64; sq += 8) {
        if (blockers & (1ULL << sq)) {
            U64 file = 0ULL;
            for (int nSq = sq + 8; nSq < 64; nSq += 8){
                file |= 1ULL << nSq;
            }
            attacks ^= file;
            break;
        }
    }

    // std::cout << "rook attack northed: \n";
    // printBitboard(attacks);
    
    return attacks;
}

U64 getBishopAttacks(int square, U64 blockers) {
    U64 attacks = bishopAttacks[square];

    // std::cout << "initial bishop attack bb: \n";
    // printBitboard(attacks);
    // std::cout << "blockers: \n";
    // printBitboard(blockers);

    // Check each diagonal direction (NE, SE, SW, NW) for blockers
    // Northeast
    for (int sq = square + 9; sq % 8 != 0 && sq < 64; sq += 9) {
        if (blockers & (1ULL << sq)) {
            //attacks &= ~((1ULL << sq) - 1);
            U64 diagonal = 0ULL;
            for (int diagSq = sq + 9; diagSq % 8 != 0 && diagSq < 64; diagSq += 9) {
                diagonal |= 1ULL << diagSq;
            }
            attacks ^= diagonal;
            break;
        }
    }

    // std::cout << "bishop attack NEd: \n";
    // printBitboard(attacks);

    // Southeast
    for (int sq = square - 7; sq % 8 != 0 && sq >= 0; sq -= 7) {
        if (blockers & (1ULL << sq)) {
            //attacks &= ~(((1ULL << sq) << 1) - 1);
            U64 diagonal = 0ULL;
            for (int diagSq = sq - 7; diagSq % 8 != 0 && diagSq >= 0; diagSq -= 7) {
                diagonal |= 1ULL << diagSq;
            }
            attacks ^= diagonal;
            break;
        }
    }

    // std::cout << "bishop attack SEd: \n";
    // printBitboard(attacks);

    // Southwest
    for (int sq = square - 9; sq % 8 != 7 && sq >= 0; sq -= 9) {
        if (blockers & (1ULL << sq)) {
            //attacks &= ~(((1ULL << sq) << 1) - 1);
            U64 diagonal = 0ULL;
            for (int diagSq = sq - 9; diagSq % 8 != 7 && diagSq >= 0; diagSq -= 9) {
                diagonal |= 1ULL << diagSq;
            }
            attacks ^= diagonal;
            break;
        }
    }

    // std::cout << "bishop attack SWd: \n";
    // printBitboard(attacks);

    // Northwest
    for (int sq = square + 7; sq % 8 != 7 && sq < 64; sq += 7) {
        if (blockers & (1ULL << sq)) {
            //attacks &= ~((1ULL << sq) - 1);
            U64 diagonal = 0ULL;
            for (int diagSq = sq + 7; diagSq % 8 != 7 && diagSq < 64; diagSq += 7) {
                diagonal |= 1ULL << diagSq;
            }
            attacks ^= diagonal;
            break;
        }
    }

    // std::cout << "bishop attack NWd: \n";
    // printBitboard(attacks);

    return attacks;
}


U64 getQueenAttacks(int square, U64 blockers) {
    U64 attacks = queenAttacks[square];
    // std::cout << "initial queen attack bb: \n";
    // printBitboard(attacks);
    // std::cout << "blockers: \n";
    // printBitboard(blockers);

    // Check each diagonal direction (NE, SE, SW, NW) for blockers
    // Northeast
    for (int sq = square + 9; sq % 8 != 0 && sq < 64; sq += 9) {
        if (blockers & (1ULL << sq)) {
            //attacks &= ~((1ULL << sq) - 1);
            U64 diagonal = 0ULL;
            for (int diagSq = sq + 9; diagSq % 8 != 0 && diagSq < 64; diagSq += 9) {
                diagonal |= 1ULL << diagSq;
            }
            attacks ^= diagonal;
            break;
        }
    }

    // std::cout << "bishop attack NEd: \n";
    // printBitboard(attacks);

    // Southeast
    for (int sq = square - 7; sq % 8 != 0 && sq >= 0; sq -= 7) {
        if (blockers & (1ULL << sq)) {
            //attacks &= ~(((1ULL << sq) << 1) - 1);
            U64 diagonal = 0ULL;
            for (int diagSq = sq - 7; diagSq % 8 != 0 && diagSq >= 0; diagSq -= 7) {
                diagonal |= 1ULL << diagSq;
            }
            attacks ^= diagonal;
            break;
        }
    }

    // std::cout << "bishop attack SEd: \n";
    // printBitboard(attacks);

    // Southwest
    for (int sq = square - 9; sq % 8 != 7 && sq >= 0; sq -= 9) {
        if (blockers & (1ULL << sq)) {
            //attacks &= ~(((1ULL << sq) << 1) - 1);
            U64 diagonal = 0ULL;
            for (int diagSq = sq - 9; diagSq % 8 != 7 && diagSq >= 0; diagSq -= 9) {
                diagonal |= 1ULL << diagSq;
            }
            attacks ^= diagonal;
            break;
        }
    }

    // std::cout << "bishop attack SWd: \n";
    // printBitboard(attacks);

    // Northwest
    for (int sq = square + 7; sq % 8 != 7 && sq < 64; sq += 7) {
        if (blockers & (1ULL << sq)) {
            //attacks &= ~((1ULL << sq) - 1);
            U64 diagonal = 0ULL;
            for (int diagSq = sq + 7; diagSq % 8 != 7 && diagSq < 64; diagSq += 7) {
                diagonal |= 1ULL << diagSq;
            }
            attacks ^= diagonal;
            break;
        }
    }

    // std::cout << "queen diagonal blocked:\n";
    // printBitboard(attacks);

    // Check each direction (N, S, E, W) for blockers
    // South
    for (int sq = square - 8; sq >= 0; sq -= 8) {
        if (blockers & (1ULL << sq)) {
            U64 south = 0ULL;
            for (int sSq = sq - 8; sSq >= 0; sSq -= 8) {
                south |= 1ULL << sSq;
            }
            attacks ^= south;
            break;
        }
    }

    // std::cout << "queen attack southed: \n";
    // printBitboard(attacks);

    // West
    for (int sq = square - 1; sq % 8 != 7 && sq >= 0; sq--) {
        if (blockers & (1ULL << sq)) {
            //attacks &= ~(((1ULL << sq) << 1) - 1);
            U64 rank = 0ULL;
            for (int rankSq = sq - 1; rankSq % 8 != 7 && rankSq >= 0; rankSq--) {
                rank |= 1ULL << rankSq;
            }
            attacks ^= rank;
            break;
        }
    }

    // std::cout << "queen attack wested: \n";
    // printBitboard(attacks);

    // East
    for (int sq = square + 1; sq % 8 != 0 && sq < 64; sq++) {
        if (blockers & (1ULL << sq)) {
            //attacks &= ((1ULL << (sq + 1)) - 1);
            U64 rank = 0ULL;
            for (int rankSq = sq + 1; rankSq % 8 != 0 && rankSq < 64; rankSq++) {
                rank |= 1ULL << rankSq;
            }
            attacks ^= rank;
            break;
        }
    }

    // std::cout << "queen attack easted: \n";
    // printBitboard(attacks);

    // North
    for (int sq = square + 8; sq < 64; sq += 8) {
        if (blockers & (1ULL << sq)) {
            U64 north = 0ULL;
            for (int nSq = sq + 8; nSq < 64; nSq += 8){
                north |= 1ULL << nSq;
            }
            attacks ^= north;
            break;
        }
    }

    // std::cout << "queen attack rooked: \n";
    // printBitboard(attacks);

    return attacks;
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

// Precomputed attack masks for each piece at every square
// on the board
std::array<U64, 64> pawnAttacksWhite = {
    0b0000000000000000000000000000000000000000000000000000001000000000,     // Index 0 (a1)
    0b0000000000000000000000000000000000000000000000000000010100000000,     // Index 1 (b1)
    0b0000000000000000000000000000000000000000000000000000101000000000,     // ...
    0b0000000000000000000000000000000000000000000000000001010000000000,     // All masks go in index order
    0b0000000000000000000000000000000000000000000000000010100000000000,
    0b0000000000000000000000000000000000000000000000000101000000000000,
    0b0000000000000000000000000000000000000000000000001010000000000000,
    0b0000000000000000000000000000000000000000000000000100000000000000,
    0b0000000000000000000000000000000000000000000000100000000000000000,
    0b0000000000000000000000000000000000000000000001010000000000000000,
    0b0000000000000000000000000000000000000000000010100000000000000000,
    0b0000000000000000000000000000000000000000000101000000000000000000,
    0b0000000000000000000000000000000000000000001010000000000000000000,
    0b0000000000000000000000000000000000000000010100000000000000000000,
    0b0000000000000000000000000000000000000000101000000000000000000000,
    0b0000000000000000000000000000000000000000010000000000000000000000,
    0b0000000000000000000000000000000000000010000000000000000000000000,
    0b0000000000000000000000000000000000000101000000000000000000000000,
    0b0000000000000000000000000000000000001010000000000000000000000000,
    0b0000000000000000000000000000000000010100000000000000000000000000,
    0b0000000000000000000000000000000000101000000000000000000000000000,
    0b0000000000000000000000000000000001010000000000000000000000000000,
    0b0000000000000000000000000000000010100000000000000000000000000000,
    0b0000000000000000000000000000000001000000000000000000000000000000,
    0b0000000000000000000000000000001000000000000000000000000000000000,
    0b0000000000000000000000000000010100000000000000000000000000000000,
    0b0000000000000000000000000000101000000000000000000000000000000000,
    0b0000000000000000000000000001010000000000000000000000000000000000,
    0b0000000000000000000000000010100000000000000000000000000000000000,
    0b0000000000000000000000000101000000000000000000000000000000000000,
    0b0000000000000000000000001010000000000000000000000000000000000000,
    0b0000000000000000000000000100000000000000000000000000000000000000,
    0b0000000000000000000000100000000000000000000000000000000000000000,
    0b0000000000000000000001010000000000000000000000000000000000000000,
    0b0000000000000000000010100000000000000000000000000000000000000000,
    0b0000000000000000000101000000000000000000000000000000000000000000,
    0b0000000000000000001010000000000000000000000000000000000000000000,
    0b0000000000000000010100000000000000000000000000000000000000000000,
    0b0000000000000000101000000000000000000000000000000000000000000000,
    0b0000000000000000010000000000000000000000000000000000000000000000,
    0b0000000000000010000000000000000000000000000000000000000000000000,
    0b0000000000000101000000000000000000000000000000000000000000000000,
    0b0000000000001010000000000000000000000000000000000000000000000000,
    0b0000000000010100000000000000000000000000000000000000000000000000,
    0b0000000000101000000000000000000000000000000000000000000000000000,
    0b0000000001010000000000000000000000000000000000000000000000000000,
    0b0000000010100000000000000000000000000000000000000000000000000000,
    0b0000000001000000000000000000000000000000000000000000000000000000,
    0b0000001000000000000000000000000000000000000000000000000000000000,
    0b0000010100000000000000000000000000000000000000000000000000000000,
    0b0000101000000000000000000000000000000000000000000000000000000000,
    0b0001010000000000000000000000000000000000000000000000000000000000,
    0b0010100000000000000000000000000000000000000000000000000000000000,
    0b0101000000000000000000000000000000000000000000000000000000000000,
    0b1010000000000000000000000000000000000000000000000000000000000000,
    0b0100000000000000000000000000000000000000000000000000000000000000,
    0b0000000000000000000000000000000000000000000000000000000000000010,
    0b0000000000000000000000000000000000000000000000000000000000000101,
    0b0000000000000000000000000000000000000000000000000000000000001010,
    0b0000000000000000000000000000000000000000000000000000000001000000,
    0b0000000000000000000000000000000000000000000000000000000000010100,
    0b0000000000000000000000000000000000000000000000000000000000101000,
    0b0000000000000000000000000000000000000000000000000000000001010000,
    0b0000000000000000000000000000000000000000000000000000000010100000,     // Index 63 (h8)

};
std::array<U64, 64> pawnAttacksBlack = {
    0b0000001000000000000000000000000000000000000000000000000000000000,
    0b0000010100000000000000000000000000000000000000000000000000000000,
    0b0000101000000000000000000000000000000000000000000000000000000000,
    0b0001010000000000000000000000000000000000000000000000000000000000,
    0b0010100000000000000000000000000000000000000000000000000000000000,
    0b0101000000000000000000000000000000000000000000000000000000000000,
    0b1010000000000000000000000000000000000000000000000000000000000000,
    0b0100000000000000000000000000000000000000000000000000000000000000,
    0b0000000000000000000000000000000000000000000000000000000000000010,
    0b0000000000000000000000000000000000000000000000000000000000000101,
    0b0000000000000000000000000000000000000000000000000000000000001010,
    0b0000000000000000000000000000000000000000000000000000000000010100,
    0b0000000000000000000000000000000000000000000000000000000000101000,
    0b0000000000000000000000000000000000000000000000000000000001010000,
    0b0000000000000000000000000000000000000000000000000000000010100000,
    0b0000000000000000000000000000000000000000000000000000000001000000,
    0b0000000000000000000000000000000000000000000000000000001000000000,
    0b0000000000000000000000000000000000000000000000000000010100000000,
    0b0000000000000000000000000000000000000000000000000000101000000000,
    0b0000000000000000000000000000000000000000000000000001010000000000,
    0b0000000000000000000000000000000000000000000000000010100000000000,
    0b0000000000000000000000000000000000000000000000000101000000000000,
    0b0000000000000000000000000000000000000000000000001010000000000000,
    0b0000000000000000000000000000000000000000000000000100000000000000,
    0b0000000000000000000000000000000000000000000000100000000000000000,
    0b0000000000000000000000000000000000000000000001010000000000000000,
    0b0000000000000000000000000000000000000000000010100000000000000000,
    0b0000000000000000000000000000000000000000000101000000000000000000,
    0b0000000000000000000000000000000000000000001010000000000000000000,
    0b0000000000000000000000000000000000000000010100000000000000000000,
    0b0000000000000000000000000000000000000000101000000000000000000000,
    0b0000000000000000000000000000000000000000010000000000000000000000,
    0b0000000000000000000000000000000000000010000000000000000000000000,
    0b0000000000000000000000000000000000000101000000000000000000000000,
    0b0000000000000000000000000000000000001010000000000000000000000000,
    0b0000000000000000000000000000000000010100000000000000000000000000,
    0b0000000000000000000000000000000000101000000000000000000000000000,
    0b0000000000000000000000000000000001010000000000000000000000000000,
    0b0000000000000000000000000000000010100000000000000000000000000000,
    0b0000000000000000000000000000000001000000000000000000000000000000,
    0b0000000000000000000000000000001000000000000000000000000000000000,
    0b0000000000000000000000000000010100000000000000000000000000000000,
    0b0000000000000000000000000000101000000000000000000000000000000000,
    0b0000000000000000000000000001010000000000000000000000000000000000,
    0b0000000000000000000000000010100000000000000000000000000000000000,
    0b0000000000000000000000000101000000000000000000000000000000000000,
    0b0000000000000000000000001010000000000000000000000000000000000000,
    0b0000000000000000000000000100000000000000000000000000000000000000,
    0b0000000000000000000000100000000000000000000000000000000000000000,
    0b0000000000000000000001010000000000000000000000000000000000000000,
    0b0000000000000000000010100000000000000000000000000000000000000000,
    0b0000000000000000000101000000000000000000000000000000000000000000,
    0b0000000000000000001010000000000000000000000000000000000000000000,
    0b0000000000000000010100000000000000000000000000000000000000000000,
    0b0000000000000000101000000000000000000000000000000000000000000000,
    0b0000000000000000010000000000000000000000000000000000000000000000,
    0b0000000000000010000000000000000000000000000000000000000000000000,
    0b0000000000000101000000000000000000000000000000000000000000000000,
    0b0000000000001010000000000000000000000000000000000000000000000000,
    0b0000000000010100000000000000000000000000000000000000000000000000,
    0b0000000000101000000000000000000000000000000000000000000000000000,
    0b0000000001010000000000000000000000000000000000000000000000000000,
    0b0000000010100000000000000000000000000000000000000000000000000000,
    0b0000000001000000000000000000000000000000000000000000000000000000
};
std::array<U64, 64> knightAttacks = {
    0b0000000000000000000000000000000000000000000000100000010000000000,
    0b0000000000000000000000000000000000000000000001010000100000000000,
    0b0000000000000000000000000000000000000000000010100001000100000000,
    0b0000000000000000000000000000000000000000000101000010001000000000,
    0b0000000000000000000000000000000000000000001010000100010000000000,
    0b0000000000000000000000000000000000000000010100001000100000000000,
    0b0000000000000000000000000000000000000000101000000001000000000000,
    0b0000000000000000000000000000000000000000010000000010000000000000,
    0b0000000000000000000000000000000000000010000001000000000000000100,
    0b0000000000000000000000000000000000000101000010000000000000001000,
    0b0000000000000000000000000000000000001010000100010000000000010001,
    0b0000000000000000000000000000000000010100001000100000000000100010,
    0b0000000000000000000000000000000000101000010001000000000001000100,
    0b0000000000000000000000000000000001010000100010000000000010001000,
    0b0000000000000000000000000000000010100000000100000000000000010000,
    0b0000000000000000000000000000000001000000001000000000000000100000,
    0b0000000000000000000000000000001000000100000000000000010000000010,
    0b0000000000000000000000000000010100001000000000000000100000000101,
    0b0000000000000000000000000000101000010001000000000001000100001010,
    0b0000000000000000000000000001010000100010000000000010001000010100,
    0b0000000000000000000000000010100001000100000000000100010000101000,
    0b0000000000000000000000000101000010001000000000001000100001010000,
    0b0000000000000000000000001010000000010000000000000001000010100000,
    0b0000000000000000000000000100000000100000000000000010000001000000,
    0b0000000000000000000000100000010000000000000001000000001000000000,
    0b0000000000000000000001010000100000000000000010000000010100000000,
    0b0000000000000000000010100001000100000000000100010000101000000000,
    0b0000000000000000000101000010001000000000001000100001010000000000,
    0b0000000000000000001010000100010000000000010001000010100000000000,
    0b0000000000000000010100001000100000000000100010000101000000000000,
    0b0000000000000000101000000001000000000000000100001010000000000000,
    0b0000000000000000010000000010000000000000001000000100000000000000,
    0b0000000000000010000001000000000000000100000000100000000000000000,
    0b0000000000000101000010000000000000001000000001010000000000000000,
    0b0000000000001010000100010000000000010001000010100000000000000000,
    0b0000000000010100001000100000000000100010000101000000000000000000,
    0b0000000000101000010001000000000001000100001010000000000000000000,
    0b0000000001010000100010000000000010001000010100000000000000000000,
    0b0000000010100000000100000000000000010000101000000000000000000000,
    0b0000000001000000001000000000000000100000010000000000000000000000,
    0b0000001000000100000000000000010000000010000000000000000000000000,
    0b0000010100001000000000000000100000000101000000000000000000000000,
    0b0000101000010001000000000001000100001010000000000000000000000000,
    0b0001010000100010000000000010001000010100000000000000000000000000,
    0b0010100001000100000000000100010000101000000000000000000000000000,
    0b0101000010001000000000001000100001010000000000000000000000000000,
    0b1010000000010000000000000001000010100000000000000000000000000000,
    0b0100000000100000000000000010000001000000000000000000000000000000,
    0b0000010000000000000001000000001000000000000000000000000000000000,
    0b0000100000000000000010000000010100000000000000000000000000000000,
    0b0001000100000000000100010000101000000000000000000000000000000000,
    0b0010001000000000001000100001010000000000000000000000000000000000,
    0b0100010000000000010001000010100000000000000000000000000000000000,
    0b1000100000000000100010000101000000000000000000000000000000000000,
    0b0001000000000000000100001010000000000000000000000000000000000000,
    0b0010000000000000001000000100000000000000000000000000000000000000,
    0b0000000000000100000000100000000000000000000000000000000000000000,
    0b0000000000001000000001010000000000000000000000000000000000000000,
    0b0000000000010001000010100000000000000000000000000000000000000000,
    0b0000000000100010000101000000000000000000000000000000000000000000,
    0b0000000001000100001010000000000000000000000000000000000000000000,
    0b0000000010001000010100000000000000000000000000000000000000000000,
    0b0000000000010000101000000000000000000000000000000000000000000000,
    0b0000000000100000010000000000000000000000000000000000000000000000
};
std::array<U64, 64> bishopAttacks = {
    0b1000000001000000001000000001000000001000000001000000001000000000,
    0b0000000010000000010000000010000000010000000010000000010100000000,
    0b0000000000000000100000000100000000100000000100010000101000000000,
    0b0000000000000000000000001000000001000001001000100001010000000000,
    0b0000000000000000000000000000000110000010010001000010100000000000,
    0b0000000000000000000000010000001000000100100010000101000000000000,
    0b0000000000000001000000100000010000001000000100001010000000000000,
    0b0000000100000010000001000000100000010000001000000100000000000000,
    0b0100000000100000000100000000100000000100000000100000000000000010,
    0b1000000001000000001000000001000000001000000001010000000000000101,
    0b0000000010000000010000000010000000010001000010100000000000001010,
    0b0000000000000000100000000100000100100010000101000000000000010100,
    0b0000000000000000000000011000001001000100001010000000000000101000,
    0b0000000000000001000000100000010010001000010100000000000001010000,
    0b0000000100000010000001000000100000010000101000000000000010100000,
    0b0000001000000100000010000001000000100000010000000000000001000000,
    0b0010000000010000000010000000010000000010000000000000001000000100,
    0b0100000000100000000100000000100000000101000000000000010100001000,
    0b1000000001000000001000000001000100001010000000000000101000010001,
    0b0000000010000000010000010010001000010100000000000001010000100010,
    0b0000000000000001100000100100010000101000000000000010100001000100,
    0b0000000100000010000001001000100001010000000000000101000010001000,
    0b0000001000000100000010000001000010100000000000001010000000010000,
    0b0000010000001000000100000010000001000000000000000100000000100000,
    0b0001000000001000000001000000001000000000000000100000010000001000,
    0b0010000000010000000010000000010100000000000001010000100000010000,
    0b0100000000100000000100010000101000000000000010100001000100100000,
    0b1000000001000001001000100001010000000000000101000010001001000001,
    0b0000000110000010010001000010100000000000001010000100010010000010,
    0b0000001000000100100010000101000000000000010100001000100000000100,
    0b0000010000001000000100001010000000000000101000000001000000001000,
    0b0000100000010000001000000100000000000000010000000010000000010000,
    0b0000100000000100000000100000000000000010000001000000100000010000,
    0b0001000000001000000001010000000000000101000010000001000000100000,
    0b0010000000010001000010100000000000001010000100010010000001000000,
    0b0100000100100010000101000000000000010100001000100100000110000000,
    0b1000001001000100001010000000000000101000010001001000001000000001,
    0b0000010010001000010100000000000001010000100010000000010000000010,
    0b0000100000010000101000000000000010100000000100000000100000000100,
    0b0001000000100000010000000000000001000000001000000001000000001000,
    0b0000010000000010000000000000001000000100000010000001000000100000,
    0b0000100000000101000000000000010100001000000100000010000001000000,
    0b0001000100001010000000000000101000010001001000000100000010000000,
    0b0010001000010100000000000001010000100010010000011000000000000000,
    0b0100010000101000000000000010100001000100100000100000000100000000,
    0b1000100001010000000000000101000010001000000001000000001000000001,
    0b0001000010100000000000001010000000010000000010000000010000000010,
    0b0010000001000000000000000100000000100000000100000000100000000100,
    0b0000001000000000000000100000010000001000000100000010000001000000,
    0b0000010100000000000001010000100000010000001000000100000010000000,
    0b0000101000000000000010100001000100100000010000001000000000000000,
    0b0001010000000000000101000010001001000001100000000000000000000000,
    0b0010100000000000001010000100010010000010000000010000000000000000,
    0b0101000000000000010100001000100000000100000000100000000100000000,
    0b1010000000000000101000000001000000001000000001000000001000000001,
    0b0100000000000000010000000010000000010000000010000000010000000010,
    0b0000000000000010000001000000100000010000001000000100000010000000,
    0b0000000000000101000010000001000000100000010000001000000000000000,
    0b0000000000001010000100010010000001000000100000000000000000000000,
    0b0000000000010100001000100100000110000000000000000000000000000000,
    0b0000000000101000010001001000001000000001000000000000000000000000,
    0b0000000001010000100010000000010000000010000000010000000000000000,
    0b0000000010100000000100000000100000000100000000100000000100000000,
    0b0000000001000000001000000001000000001000000001000000001000000001,
};
std::array<U64, 64> rookAttacks = {
    0b0000000100000001000000010000000100000001000000010000000111111110,
    0b0000001000000010000000100000001000000010000000100000001011111101,
    0b0000010000000100000001000000010000000100000001000000010011111011,
    0b0000100000001000000010000000100000001000000010000000100011110111,
    0b0001000000010000000100000001000000010000000100000001000011101111,
    0b0010000000100000001000000010000000100000001000000010000011011111,
    0b0100000001000000010000000100000001000000010000000100000010111111,
    0b1000000010000000100000001000000010000000100000001000000001111111,
    0b0000000100000001000000010000000100000001000000011111111000000001,
    0b0000001000000010000000100000001000000010000000101111110100000010,
    0b0000010000000100000001000000010000000100000001001111101100000100,
    0b0000100000001000000010000000100000001000000010001111011100001000,
    0b0001000000010000000100000001000000010000000100001110111100010000,
    0b0010000000100000001000000010000000100000001000001101111100100000,
    0b0100000001000000010000000100000001000000010000001011111101000000,
    0b1000000010000000100000001000000010000000100000000111111110000000,
    0b0000000100000001000000010000000100000001111111100000000100000001,
    0b0000001000000010000000100000001000000010111111010000001000000010,
    0b0000010000000100000001000000010000000100111110110000010000000100,
    0b0000100000001000000010000000100000001000111101110000100000001000,
    0b0001000000010000000100000001000000010000111011110001000000010000,
    0b0010000000100000001000000010000000100000110111110010000000100000,
    0b0100000001000000010000000100000001000000101111110100000001000000,
    0b1000000010000000100000001000000010000000011111111000000010000000,
    0b0000000100000001000000010000000111111110000000010000000100000001,
    0b0000001000000010000000100000001011111101000000100000001000000010,
    0b0000010000000100000001000000010011111011000001000000010000000100,
    0b0000100000001000000010000000100011110111000010000000100000001000,
    0b0001000000010000000100000001000011101111000100000001000000010000,
    0b0010000000100000001000000010000011011111001000000010000000100000,
    0b0100000001000000010000000100000010111111010000000100000001000000,
    0b1000000010000000100000001000000001111111100000001000000010000000,
    0b0000000100000001000000011111111000000001000000010000000100000001,
    0b0000001000000010000000101111110100000010000000100000001000000010,
    0b0000010000000100000001001111101100000100000001000000010000000100,
    0b0000100000001000000010001111011100001000000010000000100000001000,
    0b0001000000010000000100001110111100010000000100000001000000010000,
    0b0010000000100000001000001101111100100000001000000010000000100000,
    0b0100000001000000010000001011111101000000010000000100000001000000,
    0b1000000010000000100000000111111110000000100000001000000010000000,
    0b0000000100000001111111100000000100000001000000010000000100000001,
    0b0000001000000010111111010000001000000010000000100000001000000010,
    0b0000010000000100111110110000010000000100000001000000010000000100,
    0b0000100000001000111101110000100000001000000010000000100000001000,
    0b0001000000010000111011110001000000010000000100000001000000010000,
    0b0010000000100000110111110010000000100000001000000010000000100000,
    0b0100000001000000101111110100000001000000010000000100000001000000,
    0b1000000010000000011111111000000010000000100000001000000010000000,
    0b0000000111111110000000010000000100000001000000010000000100000001,
    0b0000001011111101000000100000001000000010000000100000001000000010,
    0b0000010011111011000001000000010000000100000001000000010000000100,
    0b0000100011110111000010000000100000001000000010000000100000001000,
    0b0001000011101111000100000001000000010000000100000001000000010000,
    0b0010000011011111001000000010000000100000001000000010000000100000,
    0b0100000010111111010000000100000001000000010000000100000001000000,
    0b1000000001111111100000001000000010000000100000001000000010000000,
    0b1111111000000001000000010000000100000001000000010000000100000001,
    0b1111110100000010000000100000001000000010000000100000001000000010,
    0b1111101100000100000001000000010000000100000001000000010000000100,
    0b1111011100001000000010000000100000001000000010000000100000001000,
    0b1110111100010000000100000001000000010000000100000001000000010000,
    0b1101111100100000001000000010000000100000001000000010000000100000,
    0b1011111101000000010000000100000001000000010000000100000001000000,
    0b0111111110000000100000001000000010000000100000001000000010000000
};
std::array<U64, 64> queenAttacks = {
    0b1000000101000001001000010001000100001001000001010000001111111110,
    0b0000001010000010010000100010001000010010000010100000011111111101,
    0b0000010000000100100001000100010000100100000101010000111011111011,
    0b0000100000001000000010001000100001001001001010100001110011110111,
    0b0001000000010000000100000001000110010010010101000011100011101111,
    0b0010000000100000001000010010001000100100101010000111000011011111,
    0b0100000001000001010000100100010001001000010100001110000010111111,
    0b1000000110000010100001001000100010010000101000001100000001111111,
    0b0100000100100001000100010000100100000101000000111111111000000011,
    0b1000001001000010001000100001001000001010000001111111110100000111,
    0b0000010010000100010001000010010000010101000011101111101100001110,
    0b0000100000001000100010000100100100101010000111001111011100011100,
    0b0001000000010000000100011001001001010100001110001110111100111000,
    0b0010000000100001001000100010010010101000011100001101111101110000,
    0b0100000101000010010001000100100001010000111000001011111111100000,
    0b1000001010000100100010001001000010100000110000000111111111000000,
    0b0010000100010001000010010000010100000011111111100000001100000101,
    0b0100001000100010000100100000101000000111111111010000011100001010,
    0b1000010001000100001001000001010100001110111110110000111000010101,
    0b0000100010001000010010010010101000011100111101110001110000101010,
    0b0001000000010001100100100101010000111000111011110011100001010100,
    0b0010000100100010001001001010100001110000110111110111000010101000,
    0b0100001001000100010010000101000011100000101111111110000001010000,
    0b1000010010001000100100001010000011000000011111111100000010100000,
    0b0001000100001001000001010000001111111110000000110000010100001001,
    0b0010001000010010000010100000011111111101000001110000101000010010,
    0b0100010000100100000101010000111011111011000011100001010100100100,
    0b1000100001001001001010100001110011110111000111000010101001001001,
    0b0001000110010010010101000011100011101111001110000101010010010010,
    0b0010001000100100101010000111000011011111011100001010100000100100,
    0b0100010001001000010100001110000010111111111000000101000001001000,
    0b1000100010010000101000001100000001111111110000001010000010010000,
    0b0000100100000101000000111111111000000011000001010000100100010001,
    0b0001001000001010000001111111110100000111000010100001001000100010,
    0b0010010000010101000011101111101100001110000101010010010001000100,
    0b0100100100101010000111001111011100011100001010100100100110001000,
    0b1001001001010100001110001110111100111000010101001001001000010001,
    0b0010010010101000011100001101111101110000101010000010010000100010,
    0b0100100001010000111000001011111111100000010100000100100001000100,
    0b1001000010100000110000000111111111000000101000001001000010001000,
    0b0000010100000011111111100000001100000101000010010001000100100001,
    0b0000101000000111111111010000011100001010000100100010001001000010,
    0b0001010100001110111110110000111000010101001001000100010010000100,
    0b0010101000011100111101110001110000101010010010011000100000001000,
    0b0101010000111000111011110011100001010100100100100001000100010000,
    0b1010100001110000110111110111000010101000001001000010001000100001,
    0b0101000011100000101111111110000001010000010010000100010001000010,
    0b1010000011000000011111111100000010100000100100001000100010000100,
    0b0000001111111110000000110000010100001001000100010010000101000001,
    0b0000011111111101000001110000101000010010001000100100001010000010,
    0b0000111011111011000011100001010100100100010001001000010000000100,
    0b0001110011110111000111000010101001001001100010000000100000001000,
    0b0011100011101111001110000101010010010010000100010001000000010000,
    0b0111000011011111011100001010100000100100001000100010000100100000,
    0b1110000010111111111000000101000001001000010001000100001001000001,
    0b1100000001111111110000001010000010010000100010001000010010000010,
    0b1111111000000011000001010000100100010001001000010100000110000001,
    0b1111110100000111000010100001001000100010010000101000001000000010,
    0b1111101100001110000101010010010001000100100001000000010000000100,
    0b1111011100011100001010100100100110001000000010000000100000001000,
    0b1110111100111000010101001001001000010001000100000001000000010000,
    0b1101111101110000101010000010010000100010001000010010000000100000,
    0b1011111111100000010100000100100001000100010000100100000101000000,
    0b0111111111000000101000001001000010001000100001001000001010000001
};
std::array<U64, 64> kingAttacks = {
    0b0000000000000000000000000000000000000000000000000000001100000010,
    0b0000000000000000000000000000000000000000000000000000011100000101,
    0b0000000000000000000000000000000000000000000000000000111000001010,
    0b0000000000000000000000000000000000000000000000000001110000010100,
    0b0000000000000000000000000000000000000000000000000011100000101000,
    0b0000000000000000000000000000000000000000000000000111000001010000,
    0b0000000000000000000000000000000000000000000000001110000010100000,
    0b0000000000000000000000000000000000000000000000001100000001000000,
    0b0000000000000000000000000000000000000000000000110000001000000011,
    0b0000000000000000000000000000000000000000000001110000010100000111,
    0b0000000000000000000000000000000000000000000011100000101000001110,
    0b0000000000000000000000000000000000000000000111000001010000011100,
    0b0000000000000000000000000000000000000000001110000010100000111000,
    0b0000000000000000000000000000000000000000011100000101000001110000,
    0b0000000000000000000000000000000000000000111000001010000011100000,
    0b0000000000000000000000000000000000000000110000000100000011000000,
    0b0000000000000000000000000000000000000011000000100000001100000000,
    0b0000000000000000000000000000000000000111000001010000011100000000,
    0b0000000000000000000000000000000000001110000010100000111000000000,
    0b0000000000000000000000000000000000011100000101000001110000000000,
    0b0000000000000000000000000000000000111000001010000011100000000000,
    0b0000000000000000000000000000000001110000010100000111000000000000,
    0b0000000000000000000000000000000011100000101000001110000000000000,
    0b0000000000000000000000000000000011000000010000001100000000000000,
    0b0000000000000000000000000000001100000010000000110000000000000000,
    0b0000000000000000000000000000011100000101000001110000000000000000,
    0b0000000000000000000000000000111000001010000011100000000000000000,
    0b0000000000000000000000000001110000010100000111000000000000000000,
    0b0000000000000000000000000011100000101000001110000000000000000000,
    0b0000000000000000000000000111000001010000011100000000000000000000,
    0b0000000000000000000000001110000010100000111000000000000000000000,
    0b0000000000000000000000001100000001000000110000000000000000000000,
    0b0000000000000000000000110000001000000011000000000000000000000000,
    0b0000000000000000000001110000010100000111000000000000000000000000,
    0b0000000000000000000011100000101000001110000000000000000000000000,
    0b0000000000000000000111000001010000011100000000000000000000000000,
    0b0000000000000000001110000010100000111000000000000000000000000000,
    0b0000000000000000011100000101000001110000000000000000000000000000,
    0b0000000000000000111000001010000011100000000000000000000000000000,
    0b0000000000000000110000000100000011000000000000000000000000000000,
    0b0000000000000011000000100000001100000000000000000000000000000000,
    0b0000000000000111000001010000011100000000000000000000000000000000,
    0b0000000000001110000010100000111000000000000000000000000000000000,
    0b0000000000011100000101000001110000000000000000000000000000000000,
    0b0000000000111000001010000011100000000000000000000000000000000000,
    0b0000000001110000010100000111000000000000000000000000000000000000,
    0b0000000011100000101000001110000000000000000000000000000000000000,
    0b0000000011000000010000001100000000000000000000000000000000000000,
    0b0000001100000010000000110000000000000000000000000000000000000000,
    0b0000011100000101000001110000000000000000000000000000000000000000,
    0b0000111000001010000011100000000000000000000000000000000000000000,
    0b0001110000010100000111000000000000000000000000000000000000000000,
    0b0011100000101000001110000000000000000000000000000000000000000000,
    0b0111000001010000011100000000000000000000000000000000000000000000,
    0b1110000010100000111000000000000000000000000000000000000000000000,
    0b1100000001000000110000000000000000000000000000000000000000000000,
    0b0000001000000011000000000000000000000000000000000000000000000000,
    0b0000010100000111000000000000000000000000000000000000000000000000,
    0b0000101000001110000000000000000000000000000000000000000000000000,
    0b0001010000011100000000000000000000000000000000000000000000000000,
    0b0010100000111000000000000000000000000000000000000000000000000000,
    0b0101000001110000000000000000000000000000000000000000000000000000,
    0b1010000011100000000000000000000000000000000000000000000000000000,
    0b0100000011000000000000000000000000000000000000000000000000000000,
};