#pragma once

#include "player.hpp"
#include <iostream>

class HumanPlayer : public IPlayer {
private:
    std::string name;
    bool isResigned;
    bool acceptedDraw;

public:
    // Constructor takes the player's name
    explicit HumanPlayer(const std::string& playerName) 
        : name(playerName), isResigned(false), acceptedDraw(false) {}
    
    // Implementation of IPlayer interface
    Move getMove(const ChessBoard& board,
                 const GameState& state, 
                 const ChessClock& clock) override;
    
    void notifyOpponentMove(const Move& move) override;
    std::string getName() const override { return name; }
    PlayerType getType() const override { return PlayerType::Human; }
    bool getIsResigned() const { return isResigned; }
    bool acceptsDraw() const override { return acceptedDraw; }
    void onGameEnd() override;
    
    // Human-specific methods
    void offerDraw();
    void resign();
};