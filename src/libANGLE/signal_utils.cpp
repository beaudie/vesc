//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// signal_utils:
//   Helper classes for tracking dependent state changes between objects.
//   These changes are signaled to the dependent class via channels.
//   See design document:
//   https://docs.google.com/document/d/15Edfotqg6_l1skTEL8ADQudF_oIdNa7i8Po43k6jMd4/

#include "libANGLE/signal_utils.h"

#include "common/debug.h"

namespace angle
{
// SignalReceiver implementation.
SignalReceiver::~SignalReceiver() = default;

// BroadcastChannel implementation.
BroadcastChannel::BroadcastChannel()
{
}

BroadcastChannel::~BroadcastChannel()
{
    reset();
}

void BroadcastChannel::addReceiver(ChannelBinding *receiver)
{
    ASSERT(std::find(mReceivers.begin(), mReceivers.end(), receiver) == mReceivers.end());
    mReceivers.push_back(receiver);
}

void BroadcastChannel::removeReceiver(ChannelBinding *receiver)
{
    auto iter = std::find(mReceivers.begin(), mReceivers.end(), receiver);
    ASSERT(iter != mReceivers.end());
    mReceivers.erase(iter);
}

void BroadcastChannel::signal(const gl::Context *context, MessageID message) const
{
    if (mReceivers.empty())
        return;

    for (const auto *receiver : mReceivers)
    {
        receiver->signal(context, message);
    }
}

void BroadcastChannel::reset()
{
    for (auto receiver : mReceivers)
    {
        receiver->onChannelClosed();
    }
    mReceivers.clear();
}

bool BroadcastChannel::empty() const
{
    return mReceivers.empty();
}

// ChannelBinding implementation.
ChannelBinding::ChannelBinding(SignalReceiver *receiver, ChannelID channelID)
    : mChannel(nullptr), mReceiver(receiver), mChannelID(channelID)
{
    ASSERT(receiver);
}

ChannelBinding::~ChannelBinding()
{
    reset();
}

ChannelBinding::ChannelBinding(const ChannelBinding &other) = default;

ChannelBinding &ChannelBinding::operator=(const ChannelBinding &other) = default;

void ChannelBinding::bind(BroadcastChannel *channel)
{
    ASSERT(mReceiver);
    if (mChannel)
    {
        mChannel->removeReceiver(this);
    }

    mChannel = channel;

    if (mChannel)
    {
        mChannel->addReceiver(this);
    }
}

void ChannelBinding::reset()
{
    bind(nullptr);
}

void ChannelBinding::signal(const gl::Context *context, MessageID message) const
{
    mReceiver->signal(context, mChannelID, message);
}

void ChannelBinding::onChannelClosed()
{
    mChannel = nullptr;
}
}  // namespace angle
