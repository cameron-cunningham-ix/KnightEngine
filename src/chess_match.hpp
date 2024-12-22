// src/chess_match.hpp
#ifndef CHESS_MATCH_H
#define CHESS_MATCH_H

#include "board_generation.hpp"
#include "moves.hpp"
#include "chess_clock.hpp"
#include "move_history.hpp"
#include "player.hpp"
#include <memory>
#include <string>

// Possible results of a chess match
enum class MatchResult {
    InProgress,
    WhiteWin,
    BlackWin,
    Draw,
    Aborted
};

// Reason for the match ending
enum class TerminationReason {
    None,
    Checkmate,
    Stalemate,
    ThreefoldRepetition,
    FiftyMoveRule,
    InsufficientMaterial,
    MutualAgreement,
    WhiteResigned,
    BlackResigned,
    WhiteTimeout,
    BlackTimeout,
    Aborted
};

class ChessMatch {
private:
    // Core game components
    ChessBoard board;
    GameState state;
    MoveHistory history;
    ChessClock clock;
    
    // Players
    std::unique_ptr<IPlayer> whitePlayer;
    std::unique_ptr<IPlayer> blackPlayer;
    
    // Match status
    MatchResult result;
    TerminationReason terminationReason;
    bool isMatchOver;
    bool isPaused;
    
    // Private helper methods
    bool checkForGameEnd();
    bool isThreefoldRepetition() const;
    bool hasInsufficientMaterial() const;
    void switchPlayers();
    void updateMatchStatus();

public:
    // Constructor takes ownership of both players
    ChessMatch(std::unique_ptr<IPlayer> white, 
               std::unique_ptr<IPlayer> black,
               const TimeControl& tc = TimeControl());
    
    // Match lifecycle methods
    void start();
    void pause();
    void resume();
    void abort();
    
    // Match status methods
    bool isInProgress() const { return !isMatchOver; }
    MatchResult getResult() const { return result; }
    TerminationReason getTerminationReason() const { return terminationReason; }
    
    // Access to game components (const only)
    const ChessBoard& getBoard() const { return board; }
    const GameState& getState() const { return state; }
    const MoveHistory& getHistory() const { return history; }
    const ChessClock& getClock() const { return clock; }
    static std::string getCurrentDate();
    
    // Match configuration
    void setTimeControl(const TimeControl& tc);
    void setInitialPosition(const std::string& fen);
    
    // PGN export
    std::string getPGN() const;
};

#endif