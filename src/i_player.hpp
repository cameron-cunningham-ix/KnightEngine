#pragma once

#include "types.hpp"
#include "board_generation.hpp"
#include "chess_clock.hpp"
#include <string>
#include <memory>

// Enum to identify the type of player
enum class PlayerType {
    Human,
    Engine,
    Network
};

// Interface that all player types must implement
class IPlayer {
public:
    virtual ~IPlayer() = default;
    
    // Get a move from this player given the current game state
    // Returns the chosen move, or throws if no valid move can be made
    virtual DenseMove getMove(ChessBoard& board, 
                              const ChessClock& clock) = 0;
    
    // Notify the player of the opponent's move
    // Useful for AI players to update their game tree
    virtual void notifyOpponentMove(const DenseMove& move) = 0;
    
    // Get the player's displayed name
    virtual std::string getName() const = 0;
    
    // Get the type of player (human, engine, network)
    virtual PlayerType getType() const = 0;
    
    // Check if the player accepts draw offers
    virtual bool acceptsDraw() const = 0;
    
    // Called when the game ends to cleanup resources
    virtual void onGameEnd() = 0;

    // Optional UCI support
    virtual bool supportsUCI() const { return false; }
};