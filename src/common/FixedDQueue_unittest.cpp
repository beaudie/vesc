//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CircularBuffer_unittest:
//   Tests of the CircularBuffer class
//

#include <gtest/gtest.h>

#include "common/FixedDQueue.h"

namespace angle
{
// Make sure the various constructors compile and do basic checks
TEST(FixedDQueue, Constructors)
{
    FixedDQueue<int, 5> defaultContructor;
    EXPECT_EQ(0u, defaultContructor.size());
    EXPECT_EQ(true, defaultContructor.empty());
}

// Make sure the destructor destroys all elements.
TEST(FixedDQueue, Destructor)
{
    struct s
    {
        s() {}
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
        FixedDQueue<s, 11> buf;
        buf.push(s(&destructorCount));
        // Destructor called once for the temporary above.
        EXPECT_EQ(destructorCount, 1);
    }

    // Destructor should be called one more time for the element we pushed.
    EXPECT_EQ(destructorCount, 2);
}

// Test circulating behavior.
TEST(FixedDQueue, WrapAround)
{
    FixedDQueue<int, 7> buf;

    for (int i = 0; i < 7; ++i)
    {
        buf.push(i);
    }

    EXPECT_EQ(buf.front(), 0);
    buf.pop();

    buf.push(8);
    for (int i = 0; i < 7; ++i)
    {
        EXPECT_EQ(buf.front(), i + 1);
        buf.pop();
    }
}
}  // namespace angle
