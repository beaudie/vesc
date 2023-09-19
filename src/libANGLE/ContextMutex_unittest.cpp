//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Unit tests for ContextMutex.
//

#include "gtest/gtest.h"

#include "libANGLE/ContextMutex.h"

namespace
{

template <class GetContextMutex>
void runBasicContextMutexTest(bool expectToPass, GetContextMutex &&getContextMutex)
{
    constexpr size_t kThreadCount    = 16;
    constexpr size_t kIterationCount = 50'000;

    std::array<std::thread, kThreadCount> threads;
    std::array<egl::ContextMutex *, kThreadCount> contextMutexes = {};

    std::mutex mutex;
    std::condition_variable condVar;
    size_t readyCount = 0;

    std::atomic<size_t> testVar;

    for (size_t i = 0; i < kThreadCount; ++i)
    {
        threads[i] = std::thread([&, i]() {
            {
                std::unique_lock<std::mutex> lock(mutex);
                contextMutexes[i] = getContextMutex();
                contextMutexes[i]->addRef();
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
                egl::ScopedContextMutexLock lock(contextMutexes[i]);
                const int local    = testVar.load(std::memory_order_relaxed);
                const int newValue = local + 1;
                testVar.store(newValue, std::memory_order_relaxed);
            }
        });
    }

    for (size_t i = 0; i < kThreadCount; ++i)
    {
        threads[i].join();
        contextMutexes[i]->release();
    }

    if (expectToPass)
    {
        EXPECT_EQ(testVar.load(), kThreadCount * kIterationCount);
    }
    else
    {
        EXPECT_LE(testVar.load(), kThreadCount * kIterationCount);
    }
}

// Tests locking of single ContextMutex mutex.
TEST(ContextMutexTest, SingleMutexLock)
{
    egl::ContextMutex *contextMutex = new egl::ContextMutex();
    contextMutex->addRef();
    runBasicContextMutexTest(true, [&]() { return contextMutex; });
    contextMutex->release();
}

// Tests locking of multiple merged ContextMutex mutexes.
TEST(ContextMutexTest, MultipleMergedMutexLock)
{
    egl::ContextMutex *contextMutex = new egl::ContextMutex();
    contextMutex->addRef();
    runBasicContextMutexTest(true, [&]() {
        egl::ScopedContextMutexLock lock(contextMutex);
        egl::ContextMutex *threadMutex = new egl::ContextMutex();
        egl::ContextMutex::Merge(contextMutex, threadMutex);
        return threadMutex;
    });
    contextMutex->release();
}

// Tests locking of multiple unmerged ContextMutex mutexes.
TEST(ContextMutexTest, MultipleUnmergedMutexLock)
{
    runBasicContextMutexTest(false, [&]() { return new egl::ContextMutex(); });
}

// Creates 2N mutexes and 2 threads, then merges N mutex paris in each thread. Merging order of
// the first thread is reversed in the second thread.
TEST(ContextMutexTest, TwoThreadsCrossMerge)
{
    constexpr size_t kThreadCount    = 2;
    constexpr size_t kIterationCount = 100;
    static_assert(kThreadCount % 2 == 0);

    std::array<std::thread, kThreadCount> threads;
    std::array<std::array<egl::ContextMutex *, kThreadCount>, kIterationCount> mutexParis = {};

    // Create mutexes.
    for (uint32_t i = 0; i < kIterationCount; ++i)
    {
        for (uint32_t j = 0; j < kThreadCount; ++j)
        {
            mutexParis[i][j] = new egl::ContextMutex();
            // Call without a lock because no concurrent acces is possible.
            mutexParis[i][j]->addRef();
        }
    }

    std::mutex mutex;
    std::condition_variable mergeCondVar;
    std::condition_variable readyCondVar;
    std::array<bool, kThreadCount> canMerge = {};
    size_t numReadyToMerge                  = 0;

    auto threadJob = [&](size_t lockMutexIndex) {
        for (size_t i = 0; i < kIterationCount; ++i)
        {
            // Lock the first mutex.
            egl::ScopedContextMutexLock contextMutexLock(mutexParis[i][lockMutexIndex]);
            // Wait until allowed to merge...
            {
                std::unique_lock<std::mutex> lock(mutex);
                ++numReadyToMerge;
                if (numReadyToMerge == kThreadCount)
                {
                    readyCondVar.notify_one();
                }
                mergeCondVar.wait(lock, [&]() { return canMerge[lockMutexIndex]; });
                canMerge[lockMutexIndex] = false;
            }
            // Merge mutexes.
            egl::ContextMutex::Merge(mutexParis[i][lockMutexIndex],
                                     mutexParis[i][kThreadCount - lockMutexIndex - 1]);
        }
    };

    // Start threads...
    for (size_t i = 0; i < kThreadCount; ++i)
    {
        threads[i] = std::thread([&, i]() { threadJob(i); });
    }

    // Control threads execution...
    {
        std::unique_lock<std::mutex> lock(mutex);
        for (size_t i = 0; i < kIterationCount; ++i)
        {
            // Wait until all thread a re ready to merge...
            readyCondVar.wait(lock, [&]() { return numReadyToMerge == kThreadCount; });
            numReadyToMerge = 0;
            // Allow all thread to merge...
            for (bool &f : canMerge)
            {
                ASSERT_FALSE(f);
                f = true;
            }
            mergeCondVar.notify_all();
        }
    }

    // Join with threads...
    for (std::thread &thread : threads)
    {
        thread.join();
    }

    // Destroy mutexes.
    for (size_t i = 0; i < kIterationCount; ++i)
    {
        for (size_t j = 0; j < kThreadCount; ++j)
        {
            // Call without a lock because no concurrent acces is possible.
            mutexParis[i][j]->release();
        }
    }
}

}  // anonymous namespace
