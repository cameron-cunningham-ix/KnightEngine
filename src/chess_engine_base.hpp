#pragma once

#include "types.hpp"
#include "moves.hpp"
#include <string>
#include <map>
#include <atomic>

/// @brief Base class for all chess engine implementations
class ChessEngineBase {
protected:
    std::string engineName;         // Name of the engine implementation
    std::string engineVersion;      // Version of the engine
    std::string engineAuthor;       // Author of the engine
    std::atomic<bool> isSearching;  // Whether engine is currently searching
    DenseMove bestMove;             // Best move found in last search
    int defaultDepth;               // Default depth for the engine
    int searchDepth;                // Current search depth
    
    // Engine options with their current values
    std::map<std::string, std::string> options;

public:
    // Constructor takes basic engine information
    ChessEngineBase(const std::string& name, 
                    const std::string& version, 
                    const std::string& author,
                    int defaultDepth = 4)
        : engineName(name)
        , engineVersion(version)
        , engineAuthor(author)
        , isSearching(false)
        , defaultDepth(defaultDepth)
        , searchDepth(defaultDepth) {}
    
    virtual ~ChessEngineBase() = default;

    // Core engine interface that must be implemented by all engines
    virtual DenseMove findBestMove(ChessBoard& board, 
                                   int maxDepth = -1) = 0;

    // Optional evaluation function that engines can override
    virtual int evaluatePosition(const ChessBoard& board) = 0;

    // Common functionality that engines might want to override
    virtual void stopSearch() { isSearching = false; }
    virtual void setSearchDepth(int depth) { searchDepth = depth; }
    virtual void resetSearchDepth() { searchDepth = defaultDepth; }
    virtual bool setOption(const std::string& name, const std::string& value);

    // Getters
    std::string getName() const { return engineName; }
    std::string getVersion() const { return engineVersion; }
    std::string getAuthor() const { return engineAuthor; }
    bool isThinking() const { return isSearching; }
    DenseMove getBestMove() const { return bestMove; }
    int getSearchDepth() const { return searchDepth; }

protected:
    // Utility methods for derived classes
    virtual void startSearch() { isSearching = true; }
    virtual void endSearch() { isSearching = false; }
    virtual void setBestMove(const DenseMove& move) { bestMove = move; }
};