// test/chess_clock_tests.cpp
#include <gtest/gtest.h>
#include "../src/chess_clock.hpp"
#include <thread>

class ChessClockTest : public ::testing::Test {
protected:
    // Helper function to create a standard time control
    TimeControl createStandardTimeControl() {
        return TimeControl(
            std::chrono::minutes(5),    // 5 minutes initial time
            std::chrono::seconds(3),    // 3 second increment
            std::chrono::seconds(0)     // No delay
        );
    }

    // Helper function to simulate passage of time
    void waitFor(std::chrono::milliseconds duration) {
        std::this_thread::sleep_for(duration);
    }
};

// Test initialization
TEST_F(ChessClockTest, Initialization) {
    TimeControl tc = createStandardTimeControl();
    ChessClock clock(tc);

    EXPECT_EQ(clock.getWhiteTime(), std::chrono::minutes(5));
    EXPECT_EQ(clock.getBlackTime(), std::chrono::minutes(5));
    EXPECT_EQ(clock.getActiveColor(), WHITE);
    EXPECT_FALSE(clock.isClockRunning());
    EXPECT_EQ(clock.getMoveCount(), 0);
}

// Test basic clock functionality
TEST_F(ChessClockTest, BasicClockOperation) {
    TimeControl tc = createStandardTimeControl();
    ChessClock clock(tc);

    clock.start();
    EXPECT_TRUE(clock.isClockRunning());

    waitFor(std::chrono::seconds(1));
    EXPECT_LT(clock.getWhiteTime(), std::chrono::minutes(5));

    clock.stop();
    EXPECT_FALSE(clock.isClockRunning());
}

// Test player switching
TEST_F(ChessClockTest, PlayerSwitching) {
    TimeControl tc = createStandardTimeControl();
    ChessClock clock(tc);

    clock.start();
    EXPECT_EQ(clock.getActiveColor(), WHITE);

    auto initialWhiteTime = clock.getWhiteTime();
    waitFor(std::chrono::seconds(1));
    
    // Calculate expected time: initial time - elapsed time + increment
    auto expectedWhiteTime = initialWhiteTime - std::chrono::seconds(1) + std::chrono::seconds(3);
    
    clock.makeMove();

    EXPECT_EQ(clock.getActiveColor(), BLACK);
    EXPECT_NEAR(clock.getWhiteTime().count(), expectedWhiteTime.count(), 100); // Allow 100ms tolerance
    EXPECT_NEAR(clock.getBlackTime().count(), std::chrono::milliseconds(300000).count(), 10);
}

// Test time increment
TEST_F(ChessClockTest, TimeIncrement) {
    TimeControl tc = createStandardTimeControl();
    ChessClock clock(tc);

    clock.start();
    waitFor(std::chrono::seconds(1));
    auto whiteTimeBeforeMove = clock.getWhiteTime();
    clock.makeMove();

    // White should get 3 second increment after move
    EXPECT_EQ(clock.getWhiteTime(), whiteTimeBeforeMove + std::chrono::seconds(3));
}

// Test delay
TEST_F(ChessClockTest, MoveDelay) {
    TimeControl tc(
        std::chrono::minutes(5),    // 5 minutes initial time
        std::chrono::seconds(0),    // No increment
        std::chrono::seconds(2)     // 2 second delay
    );
    ChessClock clock(tc);

    clock.start();
    auto initialTime = clock.getWhiteTime();
    waitFor(std::chrono::milliseconds(1500));  // Wait less than delay
    
    // Time shouldn't decrease during delay period
    EXPECT_EQ(clock.getWhiteTime(), initialTime);

    waitFor(std::chrono::milliseconds(1000));  // Wait past delay
    EXPECT_LT(clock.getWhiteTime(), initialTime);
}

// Test infinite time control
TEST_F(ChessClockTest, InfiniteTimeControl) {
    TimeControl tc(
        std::chrono::minutes(5),    // Initial time doesn't matter
        std::chrono::seconds(0),
        std::chrono::seconds(0),
        -1,                         // No moves until time control
        true                        // Infinite time
    );
    ChessClock clock(tc);

    clock.start();
    waitFor(std::chrono::seconds(2));
    
    // Time shouldn't decrease in infinite mode
    EXPECT_EQ(clock.getWhiteTime(), std::chrono::minutes(5));
    EXPECT_FALSE(clock.isTimeUp());
}

// Test time forfeiture
TEST_F(ChessClockTest, TimeForfeit) {
    TimeControl tc(
        std::chrono::seconds(1),    // Only 1 second
        std::chrono::seconds(0),
        std::chrono::seconds(0)
    );
    ChessClock clock(tc);

    clock.start();
    waitFor(std::chrono::milliseconds(1100));  // Wait longer than time limit
    
    EXPECT_TRUE(clock.isWhiteTimeUp());
    EXPECT_TRUE(clock.isTimeUp());
    EXPECT_FALSE(clock.isBlackTimeUp());
}

// Test multiple time periods
TEST_F(ChessClockTest, MultipleTimePeriods) {
    TimeControl tc(
        std::chrono::minutes(5),    // 5 minutes initial time
        std::chrono::seconds(0),    // No increment
        std::chrono::seconds(0),    // No delay
        2                           // New time control every 2 moves
    );
    ChessClock clock(tc);

    clock.start();
    waitFor(std::chrono::seconds(1));
    clock.makeMove();  // Move 1
    clock.makeMove();  // Move 2 - should trigger new time control

    // After 2 moves, white should get another 5 minutes
    EXPECT_GT(clock.getWhiteTime(), std::chrono::minutes(4));
}

// Test pause and resume
TEST_F(ChessClockTest, PauseResume) {
    TimeControl tc = createStandardTimeControl();
    ChessClock clock(tc);

    clock.start();
    waitFor(std::chrono::seconds(1));
    auto timeBeforePause = clock.getWhiteTime();
    
    clock.pause();
    waitFor(std::chrono::seconds(1));
    EXPECT_EQ(clock.getWhiteTime(), timeBeforePause);
    
    clock.resume();
    waitFor(std::chrono::seconds(1));
    EXPECT_LT(clock.getWhiteTime(), timeBeforePause);
}

// Test time adjustment
TEST_F(ChessClockTest, TimeAdjustment) {
    TimeControl tc = createStandardTimeControl();
    ChessClock clock(tc);

    // Test adding time
    clock.addTime(WHITE, std::chrono::minutes(1));
    EXPECT_EQ(clock.getWhiteTime(), std::chrono::minutes(6));

    // Test setting specific time
    clock.setTime(BLACK, std::chrono::minutes(3));
    EXPECT_EQ(clock.getBlackTime(), std::chrono::minutes(3));
}

// Test move counter
TEST_F(ChessClockTest, MoveCounter) {
    TimeControl tc = createStandardTimeControl();
    ChessClock clock(tc);

    EXPECT_EQ(clock.getMoveCount(), 0);
    
    clock.start();
    clock.makeMove();  // White moves
    EXPECT_EQ(clock.getMoveCount(), 1);
    
    clock.makeMove();  // Black moves
    EXPECT_EQ(clock.getMoveCount(), 2);
}

// Test thread safety
TEST_F(ChessClockTest, ThreadSafety) {
    TimeControl tc = createStandardTimeControl();
    ChessClock clock(tc);
    clock.start();

    // Create multiple threads that query and update the clock
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&clock]() {
            for (int j = 0; j < 100; ++j) {
                clock.getWhiteTime();
                clock.getBlackTime();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // If we got here without crashes or data races, the test passes
    SUCCEED();
}

// Test edge cases
TEST_F(ChessClockTest, EdgeCases) {
    TimeControl tc = createStandardTimeControl();
    ChessClock clock(tc);

    // Test stopping when already stopped
    clock.stop();
    EXPECT_FALSE(clock.isClockRunning());
    clock.stop();
    EXPECT_FALSE(clock.isClockRunning());

    // Test starting when already started
    clock.start();
    EXPECT_TRUE(clock.isClockRunning());
    clock.start();
    EXPECT_TRUE(clock.isClockRunning());

    // Test switching players when clock is stopped
    clock.stop();
    auto whiteTime = clock.getWhiteTime();
    clock.makeMove();
    EXPECT_EQ(clock.getWhiteTime(), whiteTime + std::chrono::seconds(3));  // Should still get increment
}