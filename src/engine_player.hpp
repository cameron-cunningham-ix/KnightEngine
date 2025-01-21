#pragma once

#include "i_player.hpp"
#include "i_chess_engine.hpp"
#include "chess_engine_base.hpp"
#include "pext_bitboard.hpp"
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>

class EnginePlayer : public IPlayer, public IChessEngine {
private:
    std::unique_ptr<ChessEngineBase> engine;  // The actual engine implementation
    bool acceptDraws;                         // Whether engine accepts draw offers
    std::chrono::milliseconds minTime;        // Minimum time to spend on a move
    std::chrono::milliseconds maxTime;        // Maximum time to spend on a move
    
    // UCI-specific members
    bool initialized;                         // Whether UCI mode is initialized
    bool thinking;                            // Whether engine is calculating
    DenseMove bestMove;                       // Best move from last search
    DenseMove ponderMove;                     // Ponder move from last search
    
    // Thread management for UCI
    std::thread uciThread;                   // Thread for UCI communication
    mutable std::mutex mutex;                // Mutex for thread synchronization
    std::condition_variable cv;              // Condition variable for synchronization
    std::queue<std::string> commandQueue;    // Queue of UCI commands to process
    bool shouldQuit;                         // Whether engine should quit

    ChessBoard currentBoard;                 // Track current position
    ChessClock currentClock;
    std::unique_ptr<std::thread> searchThread;
    std::mutex boardMutex;

    // Calculate appropriate search depth based on remaining time
    int calculateSearchDepth(const ChessClock& clock) const;
    
    // UCI helper methods
    void uciLoop();
    void processCommand(const std::string& cmd);
    void sendResponse(const std::string& response);
    std::string moveToUCI(const DenseMove& move) const;
    DenseMove uciToMove(const std::string& uciMove, const ChessBoard& board) const;

public:
    // Constructor takes ownership of an engine implementation
    explicit EnginePlayer(std::unique_ptr<ChessEngineBase> engineImpl, 
                         bool acceptDrawOffers = false,
                         std::chrono::milliseconds minTimePerMove = std::chrono::milliseconds(100),
                         std::chrono::milliseconds maxTimePerMove = std::chrono::milliseconds(30000));
    
    ~EnginePlayer();

    // IPlayer interface implementation
    DenseMove getMove(ChessBoard& board, 
                      const ChessClock& clock) override;
    void notifyOpponentMove(const DenseMove& move) override;
    std::string getName() const override { return engine->getName(); }
    PlayerType getType() const override { return PlayerType::Engine; }
    bool acceptsDraw() const override { return acceptDraws; }
    void onGameEnd() override;
    
    // IChessEngine interface implementation
    void uci() override;
    void isReady() override;
    void setOption(const std::string& name, const std::string& value) override;
    void uciNewGame() override;
    void position(const std::string& fen, const std::string& moves) override;
    void go(const std::map<std::string, std::string>& searchParams) override;
    void stop() override;
    void quit() override;
    
    bool isInitialized() const override;
    bool isThinking() const override { return thinking; }
    bool waitForInitialization(std::chrono::milliseconds timeout = std::chrono::seconds(5)) {
        auto start = std::chrono::steady_clock::now();
        while (!isInitialized()) {
            if (std::chrono::steady_clock::now() - start > timeout) {
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return true;
    }
    
    std::string getAuthor() const override { return engine->getAuthor(); }
    DenseMove getBestMove() const override { return bestMove; }
    DenseMove getPonderMove() const override { return ponderMove; }
    ChessEngineBase* getEngineForTesting() { return engine.get(); }
    
    // Engine-specific methods
    void setSearchDepth(int depth) { engine->setSearchDepth(depth); }
    void setTimeControls(std::chrono::milliseconds min, std::chrono::milliseconds max) {
        minTime = min;
        maxTime = max;
    }
};