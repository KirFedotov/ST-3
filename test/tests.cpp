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


TEST_F(TimedDoorTest, DoorInitialState) {
  TimedDoor door(5);
  EXPECT_FALSE(door.isDoorOpened());
  EXPECT_EQ(door.getTimeOut(), 5);
}

TEST_F(TimedDoorTest, UnlockDoor) {
  TimedDoor door(5);
  door.unlock();
  EXPECT_TRUE(door.isDoorOpened());
}

TEST_F(TimedDoorTest, LockDoor) {
  TimedDoor door(5);
  door.unlock();
  door.lock();
  EXPECT_FALSE(door.isDoorOpened());
}

TEST_F(TimedDoorTest, UnlockDoorAlreadyOpened) {
  TimedDoor door(5);
  door.unlock();
  EXPECT_THROW(door.unlock(), std::logic_error);
}

TEST_F(TimedDoorTest, LockDoorAlreadyClosed) {
  TimedDoor door(5);
  EXPECT_THROW(door.lock(), std::logic_error);
}

TEST_F(TimedDoorTest, DoorTimerAdapterTimeout) {
  TimedDoor door(5);
  DoorTimerAdapter adapter(door);
  EXPECT_NO_THROW(adapter.Timeout());
}

TEST_F(TimedDoorTest, DoorTimerAdapterTimeoutDoorOpened) {
  TimedDoor door(5);
  DoorTimerAdapter adapter(door);
  door.unlock();
  EXPECT_THROW(adapter.Timeout(), std::runtime_error);
}

TEST_F(TimedDoorTest, TimerRegister) {
  TimedDoor door(5);
  timer.tregister(5, new DoorTimerAdapter(door));
  EXPECT_THROW(timer.tregister(5, new DoorTimerAdapter(door)),
   std::runtime_error);
}

TEST_F(TimedDoorTest, TimerRegisterNoTimeout) {
  TimedDoor door(5);
  timer.tregister(5, new DoorTimerAdapter(door));
  EXPECT_NO_THROW(timer.tregister(5, new DoorTimerAdapter(door)));
}

TEST_F(TimedDoorTest, TimedDoorThrowState) {
  TimedDoor door(5);
  EXPECT_NO_THROW(door.throwState());
}

TEST_F(TimedDoorTest, TimedDoorThrowStateOpened) {
  TimedDoor door(5);
  door.unlock();
  EXPECT_THROW(door.throwState(), std::runtime_error);
}

TEST_F(TimedDoorTest, MultipleUnlockLockCycles) {
  TimedDoor door(5);
  for (int i = 0; i < 10; ++i) {
    door.unlock();
    EXPECT_TRUE(door.isDoorOpened());
    door.lock();
    EXPECT_FALSE(door.isDoorOpened());
  }
}

TEST_F(TimedDoorTest, LockUnlockAfterTimeout) {
  TimedDoor door(5);
  door.unlock();
  timer.tregister(5, new DoorTimerAdapter(door));
  door.lock();
  EXPECT_FALSE(door.isDoorOpened());
}

TEST_F(TimedDoorTest, TimerClientPolymorphism) {
  TimerClient *client = new DoorTimerAdapter(TimedDoor(5));
  EXPECT_NO_THROW(client->Timeout());
  delete client;
}

TEST_F(TimedDoorTest, TimerClientPolymorphismWithException) {
  TimedDoor door(5);
  door.unlock();
  TimerClient *client = new DoorTimerAdapter(door);
  EXPECT_THROW(client->Timeout(), std::runtime_error);
  delete client;
}
