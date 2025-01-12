#include "chess_clock.hpp"
#include <iostream>

ChessClock::ChessClock()
    : timeControl(TimeControl(std::chrono::milliseconds(300000))),  // 5 minutes
      whiteTimeRemaining(timeControl.initialTime),
      blackTimeRemaining(timeControl.initialTime),
      isRunning(false),
      activeColor(WHITE),
      moveCount(0),
      lastUpdateTime(std::chrono::steady_clock::now()) {}


ChessClock::ChessClock(const TimeControl &tc)
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
    if (timeControl.movesUntilTimeControl > 0 ) {
        // Add time for new time control period
        if (moveCount % timeControl.movesUntilTimeControl == timeControl.movesUntilTimeControl - 1 && activeColor == WHITE) {
            whiteTimeRemaining += timeControl.initialTime;
        } else if (moveCount % timeControl.movesUntilTimeControl == 0 && activeColor == BLACK) {
            blackTimeRemaining += timeControl.initialTime;
        }
    }
    
    // Apply increment to the current player
    applyIncrement(activeColor);
    
    // Switch active player
    activeColor = (activeColor == WHITE) ? BLACK : WHITE;
    lastUpdateTime = std::chrono::steady_clock::now();
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
    if (!isRunning || timeControl.isInfinite || activeColor != WHITE) {
        return whiteTimeRemaining;
    }

    auto now = std::chrono::steady_clock::now();
    auto elapsedDuration = now - lastUpdateTime;
    
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedDuration);

    // Check delay period
    if (timeControl.delay > std::chrono::milliseconds(0)) {
        if (elapsed < timeControl.delay) {
            return whiteTimeRemaining;
        }
    }

    auto finalTime = std::max(whiteTimeRemaining - elapsed, std::chrono::milliseconds(0));
    return finalTime;
}

std::chrono::milliseconds ChessClock::getBlackTime() const {
    std::lock_guard<std::mutex> lock(clockMutex);
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

bool ChessClock::isTimeUp() const {
    if (timeControl.isInfinite) return false;
    std::lock_guard<std::mutex> lock(clockMutex);
    return getWhiteTimeNoLock() <= std::chrono::milliseconds(0) ||
           getBlackTimeNoLock() <= std::chrono::milliseconds(0);
}

bool ChessClock::isWhiteTimeUp() const {
    if (timeControl.isInfinite) return false;
    return getWhiteTimeNoLock() <= std::chrono::milliseconds(0);
}

bool ChessClock::isBlackTimeUp() const {
    if (timeControl.isInfinite) return false;
    return getBlackTimeNoLock() <= std::chrono::milliseconds(0);
}

bool ChessClock::isInfinite() const {
    return timeControl.isInfinite;
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

void ChessClock::setInfinite(bool isInf) {
    timeControl.isInfinite = isInf;
}

std::string TimeControl::toString() const {
    if (isInfinite) return "-";

    std::stringstream ss;
    // Format: initial/increment
    // Example: "300+2" for 5 minutes + 2 seconds increment
    ss << std::chrono::duration_cast<std::chrono::seconds>(initialTime).count();
    
    if (increment > std::chrono::milliseconds(0)) {
        ss << "+" << std::chrono::duration_cast<std::chrono::seconds>(increment).count();
    }
    
    if (delay > std::chrono::milliseconds(0)) {
        ss << "d" << std::chrono::duration_cast<std::chrono::seconds>(delay).count();
    }
    
    if (movesUntilTimeControl > 0) {
        ss << "/" << movesUntilTimeControl;
    }
    
    return ss.str();
}