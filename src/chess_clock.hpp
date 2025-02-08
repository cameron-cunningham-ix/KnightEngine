#pragma once

#include "types.hpp"
#include <chrono>
#include <mutex>
#include <atomic>

/// @brief TimeControl struct represents a particular time control that
/// a chess game can be played under. 
struct TimeControl {
    std::chrono::milliseconds initialTime;      // Initial time allocated
    std::chrono::milliseconds increment;        // Time increment after each move
    std::chrono::milliseconds delay;            // Delay before time starts counting down
    int movesUntilTimeControl;                  // Number of moves until next time control (-1 for no next control)
    bool isInfinite;                            // Flag for infinite time control

    /// @brief Default constructor
    /// @param initial 
    /// @param inc 
    /// @param del 
    /// @param moves 
    /// @param infinite 
    TimeControl(std::chrono::milliseconds initial = std::chrono::hours(1),
               std::chrono::milliseconds inc = std::chrono::seconds(0),
               std::chrono::milliseconds del = std::chrono::seconds(0),
               int moves = -1,
               bool infinite = false)
        : initialTime(initial), increment(inc), delay(del),
          movesUntilTimeControl(moves), isInfinite(infinite) {}
    std::string toString() const;
};

/// @brief 
class ChessClock {
private:
    std::chrono::milliseconds whiteTimeRemaining;
    std::chrono::milliseconds blackTimeRemaining;
    
    // Time control parameters
    TimeControl timeControl;
    
    // Current state variables
    std::atomic<bool> isRunning;
    std::atomic<Color> activeColor;
    std::atomic<int> moveCount;
    
    // Last update timestamp
    std::chrono::steady_clock::time_point lastUpdateTime;
    
    // Mutex for thread safety
    mutable std::mutex clockMutex;
    
    // Private helper methods
    bool isInDelay() const;
    void applyIncrement(Color color);

    // Private unlocked versions for internal use
    std::chrono::milliseconds getWhiteTimeNoLock() const {
        if (!isRunning || timeControl.isInfinite || activeColor != WHITE) {
            return whiteTimeRemaining;
        }

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastUpdateTime);
        
        // Check delay period
        if (timeControl.delay > std::chrono::milliseconds(0) && 
            elapsed < timeControl.delay) {
            return whiteTimeRemaining;
        }

        return std::max(whiteTimeRemaining - elapsed, std::chrono::milliseconds(0));
    }

    std::chrono::milliseconds getBlackTimeNoLock() const {
        if (!isRunning || timeControl.isInfinite || activeColor != BLACK) {
            return blackTimeRemaining;
        }

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastUpdateTime);
        
        // Check delay period
        if (timeControl.delay > std::chrono::milliseconds(0) && 
            elapsed < timeControl.delay) {
            return blackTimeRemaining;
        }

        return std::max(blackTimeRemaining - elapsed, std::chrono::milliseconds(0));
    }

public:
    // Default constructor
    explicit ChessClock();
    // Constructor
    explicit ChessClock(const TimeControl& tc);

    ChessClock& operator=(const ChessClock& other) {
        if (this != &other) {
            timeControl = other.timeControl;
            whiteTimeRemaining = other.whiteTimeRemaining;
            blackTimeRemaining = other.blackTimeRemaining;
            isRunning.store(other.isRunning.load());
            activeColor.store(other.activeColor.load());
            moveCount.store(other.moveCount.load());
            lastUpdateTime = other.lastUpdateTime;
        }
        return *this;
    }
    
    // Clock control methods
    void start();
    void updateTime();
    void stop();
    void pause();
    void resume();
    
    // Player action methods
    void switchPlayer();
    void makeMove();
    
    // Time query methods
    std::chrono::milliseconds getWhiteTime() const;
    std::chrono::milliseconds getBlackTime() const;
    bool isTimeUp() const;
    bool isWhiteTimeUp() const;
    bool isBlackTimeUp() const;
    bool isInfinite() const;
    
    // Status methods
    bool isClockRunning() const;
    Color getActiveColor() const;
    int getMoveCount() const;
    TimeControl getTimeControl() const { return timeControl; }
    
    // Time control methods
    void setTimeControl(const TimeControl& tc);
    void addTime(Color color, std::chrono::milliseconds amount);
    void setTime(Color color, std::chrono::milliseconds amount);
    void setInfinite(bool isInf);
};