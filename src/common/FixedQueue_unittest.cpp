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

namespace angle
{
// Make sure the various constructors compile and do basic checks
TEST(FixedQueue, Constructors)
{
    FixedQueue<int, 5> q;
    EXPECT_EQ(0u, q.size());
    EXPECT_EQ(true, q.empty());
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
        q.push(s(&destructorCount));
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
    q.push(s(&destructorCount));
    // Destructor called once for the temporary above.
    EXPECT_EQ(1, destructorCount);
    q.pop();
    // Copy assignment should be called for the element we popped.
    EXPECT_EQ(2, destructorCount);
}

// Test circulating behavior.
TEST(FixedQueue, WrapAround)
{
    FixedQueue<int, 7> q;

    for (int i = 0; i < 7; ++i)
    {
        q.push(i);
    }

    EXPECT_EQ(0, q.front());
    q.pop();
    // This should wrap around
    q.push(7);
    for (int i = 0; i < 7; ++i)
    {
        EXPECT_EQ(i + 1, q.front());
        q.pop();
    }
}
}  // namespace angle
