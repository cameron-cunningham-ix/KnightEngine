// chess_clock.cpp
#include "chess_clock.hpp"

ChessClock::ChessClock(const TimeControl& tc)
    : timeControl(tc),
      whiteTimeRemaining(tc.initialTime),
      blackTimeRemaining(tc.initialTime),
      isRunning(false),
      activeColor(WHITE),
      moveCount(0),
      lastUpdateTime(std::chrono::steady_clock::now()) {}

void ChessClock::start() {
    std::lock_guard<std::mutex> lock(clockMutex);
    if (!isRunning) {
        isRunning = true;
        lastUpdateTime = std::chrono::steady_clock::now();
    }
}

void ChessClock::stop() {
    std::lock_guard<std::mutex> lock(clockMutex);
    if (isRunning) {
        updateTime();
        isRunning = false;
    }
}

void ChessClock::pause() {
    std::lock_guard<std::mutex> lock(clockMutex);
    if (isRunning) {
        updateTime();
        isRunning = false;
    }
}

void ChessClock::resume() {
    std::lock_guard<std::mutex> lock(clockMutex);
    if (!isRunning) {
        isRunning = true;
        lastUpdateTime = std::chrono::steady_clock::now();
    }
}

void ChessClock::switchPlayer() {
    std::lock_guard<std::mutex> lock(clockMutex);
    updateTime();
    
    // Apply increment to the player who just completed their move
    applyIncrement(activeColor);
    
    // Switch active player
    activeColor = (activeColor == WHITE) ? BLACK : WHITE;
    lastUpdateTime = std::chrono::steady_clock::now();
}

void ChessClock::makeMove() {
    std::lock_guard<std::mutex> lock(clockMutex);
    updateTime();
    
    // Increment move count
    moveCount++;
    
    // Check if we've reached time control
    if (timeControl.movesUntilTimeControl > 0 && 
        moveCount % timeControl.movesUntilTimeControl == 0) {
        // Add time for new time control period
        if (activeColor == WHITE) {
            whiteTimeRemaining += timeControl.initialTime;
        } else {
            blackTimeRemaining += timeControl.initialTime;
        }
    }
    
    // Switch players
    switchPlayer();
}

void ChessClock::updateTime() {
    if (!isRunning || timeControl.isInfinite) return;
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - lastUpdateTime);
    
    // Only update time if we're not in delay period
    if (!isInDelay()) {
        if (activeColor == WHITE) {
            whiteTimeRemaining -= elapsed;
            if (whiteTimeRemaining < std::chrono::milliseconds(0)) {
                whiteTimeRemaining = std::chrono::milliseconds(0);
            }
        } else {
            blackTimeRemaining -= elapsed;
            if (blackTimeRemaining < std::chrono::milliseconds(0)) {
                blackTimeRemaining = std::chrono::milliseconds(0);
            }
        }
    }
    
    lastUpdateTime = now;
}

bool ChessClock::isInDelay() const {
    if (timeControl.delay <= std::chrono::milliseconds(0)) return false;
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - lastUpdateTime);
    
    return elapsed < timeControl.delay;
}

void ChessClock::applyIncrement(Color color) {
    if (timeControl.increment <= std::chrono::milliseconds(0)) return;
    
    if (color == WHITE) {
        whiteTimeRemaining += timeControl.increment;
    } else {
        blackTimeRemaining += timeControl.increment;
    }
}

std::chrono::milliseconds ChessClock::getWhiteTime() const {
    std::lock_guard<std::mutex> lock(clockMutex);
    if (isRunning && activeColor == WHITE && !timeControl.isInfinite) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastUpdateTime);
        
        // Only subtract elapsed time if we're not in delay period
        if (!isInDelay()) {
            return std::max(whiteTimeRemaining - elapsed, std::chrono::milliseconds(0));
        }
    }
    return whiteTimeRemaining;
}

std::chrono::milliseconds ChessClock::getBlackTime() const {
    std::lock_guard<std::mutex> lock(clockMutex);
    if (isRunning && activeColor == BLACK && !timeControl.isInfinite) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastUpdateTime);

        // Only subtract elapsed time if we're not in delay period
        if (!isInDelay()) {
            return std::max(blackTimeRemaining - elapsed, std::chrono::milliseconds(0));
        }
    }
    return blackTimeRemaining;
}

bool ChessClock::isTimeUp() const {
    return isWhiteTimeUp() || isBlackTimeUp();
}

bool ChessClock::isWhiteTimeUp() const {
    if (timeControl.isInfinite) return false;
    std::lock_guard<std::mutex> lock(clockMutex);
    return getWhiteTime() <= std::chrono::milliseconds(0);
}

bool ChessClock::isBlackTimeUp() const {
    if (timeControl.isInfinite) return false;
    std::lock_guard<std::mutex> lock(clockMutex);
    return getBlackTime() <= std::chrono::milliseconds(0);
}

bool ChessClock::isClockRunning() const {
    return isRunning;
}

Color ChessClock::getActiveColor() const {
    return activeColor;
}

int ChessClock::getMoveCount() const {
    return moveCount;
}

void ChessClock::setTimeControl(const TimeControl& tc) {
    std::lock_guard<std::mutex> lock(clockMutex);
    timeControl = tc;
    whiteTimeRemaining = tc.initialTime;
    blackTimeRemaining = tc.initialTime;
    moveCount = 0;
}

void ChessClock::addTime(Color color, std::chrono::milliseconds amount) {
    std::lock_guard<std::mutex> lock(clockMutex);
    if (color == WHITE) {
        whiteTimeRemaining += amount;
    } else {
        blackTimeRemaining += amount;
    }
}

void ChessClock::setTime(Color color, std::chrono::milliseconds amount) {
    std::lock_guard<std::mutex> lock(clockMutex);
    if (color == WHITE) {
        whiteTimeRemaining = amount;
    } else {
        blackTimeRemaining = amount;
    }
}
