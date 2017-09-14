//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// signal_utils_unittest:
//   Unit tests for signals and related utils.

#include <gtest/gtest.h>

#include "libANGLE/signal_utils.h"

using namespace angle;
using namespace testing;

namespace
{

using TestReceiver = SignalReceiver<gl::Error, int>;
using TestChannel  = BroadcastChannel<gl::Error, int>;
using TestBinding  = ChannelBinding<gl::Error, int>;

struct SignalThing : public TestReceiver
{
    gl::Error signal(int context, uint32_t channelID) override
    {
        wasSignaled = true;

        if (triggerDelete)
        {
            currentBinding->bind(nullptr);
        }

        return gl::NoError();
    }
    bool wasSignaled = false;
    bool triggerDelete          = false;
    TestBinding *currentBinding = nullptr;
};

// Test that broadcast signals work.
TEST(SignalTest, BroadcastSignals)
{
    TestChannel channel;
    SignalThing thing;
    TestBinding binding(&thing, 0u);

    binding.bind(&channel);
    ASSERT_FALSE(thing.wasSignaled);
    ASSERT_EQ(gl::NoError(), channel.signal(0));
    ASSERT_TRUE(thing.wasSignaled);
}

// Test that we can delete from the channel listeners while we're in the signal method.
TEST(SignalTest, DeleteWhileInSignal)
{
    TestChannel channel;
    SignalThing thing;
    SignalThing thing2;
    TestBinding binding(&thing, 0u);
    TestBinding binding2(&thing2, 1u);

    thing.triggerDelete  = true;
    thing.currentBinding = &binding;

    thing2.triggerDelete = false;
    thing2.currentBinding = &binding2;

    binding.bind(&channel);
    binding2.bind(&channel);

    ASSERT_TRUE(channel.hasReceivers());
    ASSERT_EQ(gl::NoError(), channel.signal(0));
    ASSERT_TRUE(thing.wasSignaled);
    ASSERT_TRUE(thing2.wasSignaled);

    binding2.bind(nullptr);

    ASSERT_FALSE(channel.hasReceivers());
}

}  // anonymous namespace
