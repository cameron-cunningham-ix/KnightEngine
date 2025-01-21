#pragma once

#include "types.hpp"
#include <string>
#include <vector>
#include <map>

// Interface for chess engines implementing the UCI protocol
class IChessEngine {
public:
    virtual ~IChessEngine() = default;
    
    // Core UCI commands
    virtual void uci() = 0;                    // Switch to UCI mode
    virtual void isReady() = 0;                // Check if engine is ready
    virtual void setOption(const std::string& name, 
                           const std::string& value) = 0;  // Set engine option
    virtual void uciNewGame() = 0;             // Start a new game
    virtual void position(const std::string& fen, 
                          const std::string& moves) = 0;  // Set position
    virtual void go(const std::map<std::string, std::string>& searchParams) = 0;    // Start calculating
    virtual void stop() = 0;                   // Stop calculating
    virtual void quit() = 0;                   // Quit the program
    
    // Engine statuses
    virtual bool isInitialized() const = 0;    // Check if engine is initialized
    virtual bool isThinking() const = 0;       // Check if engine is calculating
    
    // Getters for engine information
    virtual std::string getName() const = 0;        // Get engine name
    virtual std::string getAuthor() const = 0;      // Get engine author
    virtual DenseMove getBestMove() const = 0;      // Get best move from last search
    virtual DenseMove getPonderMove() const = 0;    // Get ponder move from last search
};