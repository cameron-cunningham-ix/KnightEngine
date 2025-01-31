#pragma once

#include "types.hpp"
#include "moves.hpp"
#include "chess_clock.hpp"
#include <string>
#include <map>
#include <atomic>
#include <chrono>

/// @brief Base class for all chess engine implementations
class ChessEngineBase {
protected:
    std::string engineName;         // Name of the engine implementation
    std::string engineVersion;      // Version of the engine
    std::string engineAuthor;       // Author of the engine
    std::atomic<bool> isSearching;  // Whether engine is currently searching
    DenseMove bestMove;             // Best move found in last search
    DenseMove ponderMove;           // Ponder move from last search
    int defaultDepth;               // Default depth for the engine
    int searchDepth;                // Current search depth
    std::chrono::milliseconds minTime;        // Minimum time to spend on a move
    std::chrono::milliseconds maxTime;        // Maximum time to spend on a move

    // Store options in a map for easy lookup
    std::map<std::string, EngineOption> options;
    

public:
    // Constructor takes basic engine information
    ChessEngineBase(const std::string& name, 
                    const std::string& version, 
                    const std::string& author,
                    int defaultDepth = 4,
                    std::chrono::milliseconds minTime = std::chrono::milliseconds(200),
                    std::chrono::milliseconds maxTime = std::chrono::milliseconds(20000))
        : engineName(name)
        , engineVersion(version)
        , engineAuthor(author)
        , isSearching(false)
        , defaultDepth(defaultDepth)
        , searchDepth(defaultDepth)
        , minTime(minTime)
        , maxTime(maxTime) {}
    
    virtual ~ChessEngineBase() = default;

    // Core engine interface that must be implemented by all engines
    virtual DenseMove findBestMove(ChessBoard& board,
                                   ChessClock& clock,
                                   int maxDepth = -1) = 0;

    // Optional evaluation function that engines can override
    virtual int evaluatePosition(const ChessBoard& board) = 0;

    // Common functionality that engines might want to override
    virtual void stopSearch() { isSearching = false; }
    virtual void setSearchDepth(int depth) { searchDepth = depth; }
    virtual void resetSearchDepth() { searchDepth = defaultDepth; }
    // Calculate appropriate search depth based on remaining time
    virtual int calculateSearchDepth(const ChessClock& clock) const {
        return searchDepth;
    };
    // Virtual method for derived classes to handle option changes
    virtual void onOptionChanged(const EngineOption& option) {};

    // Core option handling
    bool setOption(const std::string& name, const std::string& value) {
        auto it = options.find(name);
        if (it == options.end()) return false;
        
        if (it->second.setValue(value)) {
            onOptionChanged(it->second);  // Notify derived class
            return true;
        }
        return false;
    }

    // Getters
    std::string getName() const { return engineName; }
    std::string getVersion() const { return engineVersion; }
    std::string getAuthor() const { return engineAuthor; }
    bool isThinking() const { return isSearching; }
    DenseMove getBestMove() const { return bestMove; }
    DenseMove getPonderMove() const { return ponderMove; }
    int getSearchDepth() const { return searchDepth; }
    // Get all registered options
    const std::map<std::string, EngineOption>& getOptions() const { 
        return options; 
    }


protected:
    // Utility methods for derived classes
    virtual void startSearch() { isSearching = true; }
    virtual void endSearch() { isSearching = false; }
    virtual void setBestMove(const DenseMove& move) { bestMove = move; }
    // Method that allows derived classes to register their options
    void registerOption(const EngineOption& option) {
        // Store the option in our options map, using the option name as the key
        options[option.name] = option;
        
        // Immediately call onOptionChanged to set initial value in the derived class
        onOptionChanged(option);
    }
};