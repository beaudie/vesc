//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Mutex_unittest:
//   Tests of the Mutex class
//

#include <gtest/gtest.h>

#include "common/Mutex.h"

namespace angle
{
namespace
{
struct NoOpMutex
{
    void lock() {}
    void unlock() {}
};

template <typename TestMutex>
void runBasicMutexTest()
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
            // Wait for all threads to start, so the following loop is as simultaneously executed as
            // possible.
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

    if (std::is_same<TestMutex, NoOpMutex>::value)
    {
        // Technically not _guaranteed_ to calculate the wrong value, but highly likely to do so.
        EXPECT_NE(testVar.load(), kThreadCount * kIterationCount);
    }
    else
    {
        EXPECT_EQ(testVar.load(), kThreadCount * kIterationCount);
    }
}
}  // anonymous namespace

// Tests basic usage of std::mutex.
TEST(MutexTest, BasicStdMutex)
{
    runBasicMutexTest<std::mutex>();
}

// Tests basic usage of angle::Mutex.
TEST(MutexTest, BasicMutex)
{
    runBasicMutexTest<Mutex>();
}

// Tests failure with NoOpMutex.
TEST(MutexTest, BasicNoOpMutex)
{
    runBasicMutexTest<NoOpMutex>();
}
}  // namespace angle
