//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CircularBuffer_unittest:
//   Tests of the CircularBuffer class
//

#include <gtest/gtest.h>

#include "common/FixedQueue.h"

#include <chrono>
#include <thread>

namespace angle
{
// Make sure the various constructors compile and do basic checks
TEST(FixedQueue, Constructors)
{
    FixedQueue<int, 5> q;

    EXPECT_EQ(true, q.getConsumer().empty());
    EXPECT_EQ(false, q.getProducer().full());

    for (int i = 0; i < 5; i++)
    {
        q.getProducer().push(i);
    }
    EXPECT_EQ(false, q.getConsumer().empty());
    EXPECT_EQ(true, q.getProducer().full());
}

// Make sure the destructor destroys all elements.
TEST(FixedQueue, Destructor)
{
    struct s
    {
        s() : counter(nullptr) {}
        s(int *c) : counter(c) {}
        ~s()
        {
            if (counter)
            {
                ++*counter;
            }
        }

        s(const s &)            = default;
        s &operator=(const s &) = default;

        int *counter;
    };

    int destructorCount = 0;

    {
        FixedQueue<s, 11> q;
        q.getProducer().push(s(&destructorCount));
        // Destructor called once for the temporary above.
        EXPECT_EQ(1, destructorCount);
    }

    // Destructor should be called one more time for the element we pushed.
    EXPECT_EQ(2, destructorCount);
}

// Make sure the pop destroys the element.
TEST(FixedQueue, Pop)
{
    struct s
    {
        s() : counter(nullptr) {}
        s(int *c) : counter(c) {}
        ~s()
        {
            if (counter)
            {
                ++*counter;
            }
        }

        s(const s &) = default;
        s &operator=(const s &s)
        {
            // increment if we are overwriting the custom initialized object
            if (counter)
            {
                ++*counter;
            }
            counter = s.counter;
            return *this;
        }

        int *counter;
    };

    int destructorCount = 0;

    FixedQueue<s, 11> q;
    q.getProducer().push(s(&destructorCount));
    // Destructor called once for the temporary above.
    EXPECT_EQ(1, destructorCount);
    q.getConsumer().pop();
    // Copy assignment should be called for the element we popped.
    EXPECT_EQ(2, destructorCount);
}

// Test circulating behavior.
TEST(FixedQueue, WrapAround)
{
    FixedQueue<int, 7> q;

    for (int i = 0; i < 7; ++i)
    {
        q.getProducer().push(i);
    }

    EXPECT_EQ(0, q.getConsumer().front());
    q.getConsumer().pop();
    // This should wrap around
    q.getProducer().push(7);
    for (int i = 0; i < 7; ++i)
    {
        EXPECT_EQ(i + 1, q.getConsumer().front());
        q.getConsumer().pop();
    }
}

// Test concurrent push and pop behavior.
TEST(FixedQueue, ConcurrentPushPop)
{
    FixedQueue<uint64_t, 7> q;
    double timeOut            = 1.0;
    uint64_t kMaxLoop         = 10000ull;
    std::thread enqueueThread = std::thread([&]() {
        std::time_t t1 = std::time(nullptr);
        uint64_t value = 0;
        do
        {
            while (q.getProducer().full())
            {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
            q.getProducer().push(value);
            value++;
        } while (difftime(std::time(nullptr), t1) < timeOut && value < kMaxLoop);
    });

    std::thread dequeueThread = std::thread([&]() {
        std::time_t t1         = std::time(nullptr);
        uint64_t expectedValue = 0;
        do
        {
            while (q.getConsumer().empty())
            {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }

            // test iterator
            int i = 0;
            for (uint64_t v : q)
            {
                EXPECT_EQ(expectedValue + i, v);
                i++;
            }

            // test pop
            q.pop();

            // test pop
            q.getConsumer().pop();
            expectedValue++;
        } while (difftime(std::time(nullptr), t1) < timeOut && expectedValue < kMaxLoop);
    });

    enqueueThread.join();
    dequeueThread.join();
}
}  // namespace angle
