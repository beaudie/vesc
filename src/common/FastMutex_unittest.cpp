//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FastMutex_unittest:
//   Tests of the FastMutex classes
//

#include <gtest/gtest.h>

#include "common/FastMutex.h"

namespace angle
{
namespace
{
struct NoOpMutex
{
    void lock() {}
    void unlock() {}
};

template <class TestMutex>
void runBasicMutexTest(bool expectToPass = true)
{
    constexpr size_t kThreadCount    = 16;
    constexpr size_t kIterationCount = 50'000;

    std::array<std::thread, kThreadCount> threads;

    std::mutex mutex;
    std::condition_variable condVar;
    size_t readyCount = 0;

    TestMutex testMutex;
    std::atomic<size_t> testVar;

    for (size_t i = 0; i < kThreadCount; ++i)
    {
        threads[i] = std::thread([&]() {
            {
                std::unique_lock<std::mutex> lock(mutex);
                ++readyCount;
                if (readyCount < kThreadCount)
                {
                    condVar.wait(lock, [&]() { return readyCount == kThreadCount; });
                }
                else
                {
                    condVar.notify_all();
                }
            }
            for (size_t j = 0; j < kIterationCount; ++j)
            {
                std::lock_guard<TestMutex> lock(testMutex);
                const int local    = testVar.load(std::memory_order_relaxed);
                const int newValue = local + 1;
                testVar.store(newValue, std::memory_order_relaxed);
            }
        });
    }

    for (size_t i = 0; i < kThreadCount; ++i)
    {
        threads[i].join();
    }

    if (expectToPass)
    {
        EXPECT_EQ(testVar.load(), kThreadCount * kIterationCount);
    }
    else
    {
        EXPECT_NE(testVar.load(), kThreadCount * kIterationCount);
    }
}

template <class TestMutex>
void runMutexTryLockTest()
{
    constexpr size_t kIterationCount = 10;

    TestMutex mutex;

    auto tryLockInThread = [&]() {
        bool lockOK = false;
        std::thread([&]() {
            if (mutex.try_lock())
            {
                mutex.unlock();
                lockOK = true;
            }
        }).join();
        return lockOK;
    };

    for (size_t i = 0; i < kIterationCount; ++i)
    {
        EXPECT_TRUE(mutex.try_lock());
        EXPECT_FALSE(mutex.try_lock());
        mutex.unlock();

        {
            bool lockOK = tryLockInThread();
            EXPECT_TRUE(lockOK);
        }

        {
            std::lock_guard<TestMutex> lock(mutex);
            bool lockOK = tryLockInThread();
            EXPECT_FALSE(lockOK);
        }
    }
}
}  // anonymous namespace

// Tests basic usage of std::mutex.
TEST(FastMutexTest, BasicStdMutex)
{
    runBasicMutexTest<std::mutex>();
}

// Tests basic usage of FastMutex1.
TEST(FastMutexTest, BasicFastMutex1)
{
    runBasicMutexTest<FastMutex1>();
}

// Tests basic usage of FastMutex2.
TEST(FastMutexTest, BasicFastMutex2)
{
    runBasicMutexTest<FastMutex2>();
}

// Tests basic usage of FastMutex3.
TEST(FastMutexTest, BasicFastMutex3)
{
    runBasicMutexTest<FastMutex3>();
}

// Tests failure with NoOpMutex.
TEST(FastMutexTest, BasicNoOpMutex)
{
    runBasicMutexTest<NoOpMutex>(false);
}

// Tests try_lock of std::mutex.
TEST(FastMutexTest, TryLockStdMutex)
{
    runMutexTryLockTest<std::mutex>();
}

// Tests try_lock of FastMutex1.
TEST(FastMutexTest, TryLockFastMutex1)
{
    runMutexTryLockTest<FastMutex1>();
}

// Tests try_lock of FastMutex2.
TEST(FastMutexTest, TryLockFastMutex2)
{
    runMutexTryLockTest<FastMutex2>();
}

// Tests try_lock of FastMutex3.
TEST(FastMutexTest, TryLockFastMutex3)
{
    runMutexTryLockTest<FastMutex3>();
}

}  // namespace angle
