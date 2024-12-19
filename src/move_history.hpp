// src/move_history.hpp
#ifndef MOVE_HISTORY_H
#define MOVE_HISTORY_H

#include "types.hpp"
#include "board_generation.hpp"
#include <vector>
#include <string>
#include <chrono>

// Represents a single move in the game history with associated metadata
struct HistoryEntry {
    Move move;                      // The move that was played
    std::string san;                // Standard Algebraic Notation representation
    std::string fen;                // FEN position after move
    std::chrono::milliseconds time; // Time when move was made
    std::string comment;            // Optional comment for the move
    std::vector<std::string> nags;  // Numeric Annotation Glyphs (e.g., "!", "?", etc.)
    
    HistoryEntry(const Move& m, const std::string& s, const std::string& f,
                 std::chrono::milliseconds t = std::chrono::milliseconds(0),
                 const std::string& c = "", const std::vector<std::string>& n = {})
        : move(m), san(s), fen(f), time(t), comment(c), nags(n) {}
};

// Manages the history of moves in a chess game
class MoveHistory {
private:
    std::vector<HistoryEntry> moves;        // Sequential list of moves
    std::vector<std::pair<std::string, std::string>> tags;  // PGN tags (e.g., [Event "..."])
    std::string startingFen;                // Starting position FEN if not standard
    bool isStandardStart;                   // Whether game started from standard position

    // Helper methods
    
    std::string moveToPGN(const HistoryEntry& entry, int moveNumber, bool isWhite) const;
    Move sanToMove(const std::string &san, const ChessBoard &board, const GameState &state);
    void parseNAG(const std::string &nag, HistoryEntry &entry);

public:
    // Constructor
    MoveHistory();
    explicit MoveHistory(const std::string& initialFEN);

    // Core functionality
    void addMove(const Move& move, const ChessBoard& board, const GameState& state,
                std::chrono::milliseconds timeStamp = std::chrono::milliseconds(0));
    void addComment(const std::string& comment, size_t moveIndex);
    void addNAG(const std::string& nag, size_t moveIndex);
    
    std::string generateSAN(const Move& move, const ChessBoard& board, const GameState& state) const;

    // Tag handling
    void setTag(const std::string& name, const std::string& value);
    std::string getTag(const std::string& name) const;
    bool hasTag(const std::string& name) const;
    
    // Export/Import
    std::string toPGN() const;
    bool fromPGN(const std::string& pgn);

    void processMoveToken(const std::string &token, ChessBoard &board, GameState &state);

    std::string getFEN(ChessBoard board, GameState state);

    // Accessors
    const std::vector<HistoryEntry>& getMoves() const { return moves; }
    const std::string& getStartingFEN() const { return startingFen; }
    bool isStartStandard() const { return isStandardStart; }
    size_t length() const { return moves.size(); }
    
    // Navigation
    const HistoryEntry& getMove(size_t index) const;
    const HistoryEntry& getLastMove() const;
    bool empty() const { return moves.empty(); }
    void clear();

    // Iterator support
    auto begin() const { return moves.begin(); }
    auto end() const { return moves.end(); }
};

#endif // MOVE_HISTORY_H