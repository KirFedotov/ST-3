// Copyright 2024 Fedotov Kirill

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <chrono> // NOLINT [build/c++11]
#include <thread> // NOLINT [build/c++11]

#include "TimedDoor.h"

class MockTimerClient : public TimerClient {
 public:
    MOCK_METHOD(void, Timeout, (), (override));
};

class TimedDoorTest : public ::testing::Test {
 protected:
    TimedDoor door;
    MockTimerClient mockClient;
    Timer timer;

    TimedDoorTest() : door(1), timer() {}

    void SetUp() override {
        timer.tregister(door.getTimeOut(), &mockClient);
    }

    void TearDown() override {
        testing::Mock::VerifyAndClearExpectations(&mockClient);
    }
};


TEST_F(TimedDoorTest, lock_door_then_unlocked_test) {
    std::this_thread::sleep_for(std::chrono::seconds(door.getTimeOut() + 1));
    EXPECT_NO_THROW(door.throwState());
}

TEST_F(TimedDoorTest, unlock_door_then_timeout_test) {
    door.unlock();
    std::this_thread::sleep_for(std::chrono::seconds(door.getTimeOut()));
    EXPECT_THROW(door.throwState(), std::runtime_error);
}

TEST_F(TimedDoorTest, lock_before_timeout_then_open_test) {
    std::this_thread::sleep_for(std::chrono::seconds(door.getTimeOut() + 1));
    door.unlock();
    EXPECT_THROW(door.throwState(), std::runtime_error);
}

TEST_F(TimedDoorTest, first_door_closed_test) {
    EXPECT_FALSE(door.isDoorOpened());
}

TEST_F(TimedDoorTest, unlock_method_test) {
    door.unlock();
    EXPECT_TRUE(door.isDoorOpened());
}

TEST_F(TimedDoorTest, lock_method_test) {
    door.unlock();
    door.lock();
    EXPECT_FALSE(door.isDoorOpened());
}

TEST_F(TimedDoorTest, close_before_test) {
    EXPECT_NO_THROW(door.throwState());
}

TEST_F(TimedDoorTest, unlock_door_twice_test) {
    door.unlock();
    EXPECT_THROW(door.unlock(), std::logic_error);
}

TEST_F(TimedDoorTest, opened_exeption_test) {
    door.unlock();
    EXPECT_THROW(door.throwState(), std::runtime_error);
}

TEST_F(TimedDoorTest, lock_door_twice_test) {
    EXPECT_THROW(door.lock(), std::logic_error);
}
