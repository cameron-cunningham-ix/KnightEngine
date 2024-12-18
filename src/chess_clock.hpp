// chess_clock.hpp
#ifndef CHESS_CLOCK_H
#define CHESS_CLOCK_H

#include "types.hpp"
#include <chrono>
#include <mutex>
#include <atomic>


// Time control structure to define various types of time controls
struct TimeControl {
    std::chrono::milliseconds initialTime;      // Initial time allocated
    std::chrono::milliseconds increment;        // Time increment after each move
    std::chrono::milliseconds delay;            // Delay before time starts counting down
    int movesUntilTimeControl;                  // Number of moves until next time control (-1 for no next control)
    bool isInfinite;                           // Flag for infinite time control

    TimeControl(std::chrono::milliseconds initial = std::chrono::hours(1),
               std::chrono::milliseconds inc = std::chrono::seconds(0),
               std::chrono::milliseconds del = std::chrono::seconds(0),
               int moves = -1,
               bool infinite = false)
        : initialTime(initial), increment(inc), delay(del),
          movesUntilTimeControl(moves), isInfinite(infinite) {}
};

class ChessClock {
private:
    // Time remaining for each player
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
    void updateTime();
    bool isInDelay() const;
    void applyIncrement(Color color);

public:
    // Constructor
    explicit ChessClock(const TimeControl& tc);
    
    // Clock control methods
    void start();
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
    
    // Status methods
    bool isClockRunning() const;
    Color getActiveColor() const;
    int getMoveCount() const;
    
    // Time control methods
    void setTimeControl(const TimeControl& tc);
    void addTime(Color color, std::chrono::milliseconds amount);
    void setTime(Color color, std::chrono::milliseconds amount);
};

#endif