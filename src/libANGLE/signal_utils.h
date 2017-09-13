//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// signal_utils:
//   Helper classes for tracking dependent state changes between objects.
//   These changes are signaled to the dependent class via channels.
//   See design document:
//   https://docs.google.com/document/d/15Edfotqg6_l1skTEL8ADQudF_oIdNa7i8Po43k6jMd4/

#ifndef LIBANGLE_SIGNAL_UTILS_H_
#define LIBANGLE_SIGNAL_UTILS_H_

#include <set>

#include "common/angleutils.h"
#include "common/debug.h"
#include "libANGLE/Error.h"

namespace angle
{

// Interface that the depending class inherits from.
template <typename RetT, typename ContextT, typename ChannelID = uint32_t, typename... MessageT>
class SignalReceiver
{
  public:
    virtual ~SignalReceiver() = default;
    virtual RetT signal(ContextT context, ChannelID channelID, MessageT... message) = 0;
};

template <typename RetT, typename ContextT, typename ChannelID, typename... MessageT>
class ChannelBinding;

// The host class owns the channel. It uses the channel to fire signals to the receiver.
template <typename RetT, typename ContextT, typename ChannelID = uint32_t, typename... MessageT>
class BroadcastChannel final : NonCopyable
{
  public:
    BroadcastChannel();
    ~BroadcastChannel();

    // Signal can trigger remove (but not add).
    RetT signal(ContextT context, MessageT... message);

    void reset();

    bool hasReceivers() const { return !mReceivers.empty(); }

  private:
    // Only the ChannelBinding class should add or remove receivers.
    friend class ChannelBinding<RetT, ContextT, ChannelID, MessageT...>;
    void addReceiver(ChannelBinding<RetT, ContextT, ChannelID, MessageT...> *receiver);
    void removeReceiver(ChannelBinding<RetT, ContextT, ChannelID, MessageT...> *receiver);

    std::vector<ChannelBinding<RetT, ContextT, ChannelID, MessageT...> *> mReceivers;

    // Use a bit of a workaround to handle erasing receivers while in signal.
    ChannelBinding<RetT, ContextT, ChannelID, MessageT...> *mToRemove;
    bool mInSignal;
};

template <typename RetT, typename ContextT, typename ChannelID, typename... MessageT>
BroadcastChannel<RetT, ContextT, ChannelID, MessageT...>::BroadcastChannel()
    : mToRemove(nullptr), mInSignal(false)
{
}

template <typename RetT, typename ContextT, typename ChannelID, typename... MessageT>
BroadcastChannel<RetT, ContextT, ChannelID, MessageT...>::~BroadcastChannel()
{
    reset();
}

template <typename RetT, typename ContextT, typename ChannelID, typename... MessageT>
void BroadcastChannel<RetT, ContextT, ChannelID, MessageT...>::addReceiver(
    ChannelBinding<RetT, ContextT, ChannelID, MessageT...> *receiver)
{
    ASSERT(std::find(mReceivers.begin(), mReceivers.end(), receiver) == mReceivers.end());
    mReceivers.push_back(receiver);
}

template <typename RetT, typename ContextT, typename ChannelID, typename... MessageT>
void BroadcastChannel<RetT, ContextT, ChannelID, MessageT...>::removeReceiver(
    ChannelBinding<RetT, ContextT, ChannelID, MessageT...> *receiver)
{
    if (mInSignal)
    {
        mToRemove = receiver;
        return;
    }

    auto iter = std::find(mReceivers.begin(), mReceivers.end(), receiver);
    ASSERT(iter != mReceivers.end());
    mReceivers.erase(iter);
}

template <typename RetT, typename ContextT, typename ChannelID, typename... MessageT>
RetT BroadcastChannel<RetT, ContextT, ChannelID, MessageT...>::signal(ContextT context,
                                                                      MessageT... message)
{
    if (mReceivers.empty())
        return RetT(0);

    mInSignal = true;
    for (auto iter = mReceivers.begin(); iter != mReceivers.end();)
    {
        ANGLE_TRY((*iter)->signal(context, message...));
        if (mToRemove != nullptr)
        {
            ASSERT(mToRemove == *iter);
            iter = mReceivers.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
    mInSignal = false;

    return RetT(0);
}

template <typename RetT, typename ContextT, typename ChannelID, typename... MessageT>
void BroadcastChannel<RetT, ContextT, ChannelID, MessageT...>::reset()
{
    for (auto receiver : mReceivers)
    {
        receiver->onChannelClosed();
    }
    mReceivers.clear();
}

// The dependent class keeps bindings to the host's BroadcastChannel.
template <typename RetT, typename ContextT, typename ChannelID = uint32_t, typename... MessageT>
class ChannelBinding final
{
  public:
    ChannelBinding(SignalReceiver<RetT, ContextT, ChannelID, MessageT...> *receiver,
                   ChannelID channelID);
    ~ChannelBinding();
    ChannelBinding(const ChannelBinding &other) = default;
    ChannelBinding &operator=(const ChannelBinding &other) = default;

    void bind(BroadcastChannel<RetT, ContextT, ChannelID, MessageT...> *channel);
    void reset();
    RetT signal(ContextT context, MessageT... message) const;
    void onChannelClosed();

  private:
    BroadcastChannel<RetT, ContextT, ChannelID, MessageT...> *mChannel;
    SignalReceiver<RetT, ContextT, ChannelID, MessageT...> *mReceiver;
    ChannelID mChannelID;
};

template <typename RetT, typename ContextT, typename ChannelID, typename... MessageT>
ChannelBinding<RetT, ContextT, ChannelID, MessageT...>::ChannelBinding(
    SignalReceiver<RetT, ContextT, ChannelID, MessageT...> *receiver,
    ChannelID channelID)
    : mChannel(nullptr), mReceiver(receiver), mChannelID(channelID)
{
    ASSERT(receiver);
}

template <typename RetT, typename ContextT, typename ChannelID, typename... MessageT>
ChannelBinding<RetT, ContextT, ChannelID, MessageT...>::~ChannelBinding()
{
    reset();
}

template <typename RetT, typename ContextT, typename ChannelID, typename... MessageT>
void ChannelBinding<RetT, ContextT, ChannelID, MessageT...>::bind(
    BroadcastChannel<RetT, ContextT, ChannelID, MessageT...> *channel)
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

template <typename RetT, typename ContextT, typename ChannelID, typename... MessageT>
void ChannelBinding<RetT, ContextT, ChannelID, MessageT...>::reset()
{
    bind(nullptr);
}

template <typename RetT, typename ContextT, typename ChannelID, typename... MessageT>
RetT ChannelBinding<RetT, ContextT, ChannelID, MessageT...>::signal(ContextT context,
                                                                    MessageT... message) const
{
    return mReceiver->signal(context, mChannelID, message...);
}

template <typename RetT, typename ContextT, typename ChannelID, typename... MessageT>
void ChannelBinding<RetT, ContextT, ChannelID, MessageT...>::onChannelClosed()
{
    mChannel = nullptr;
}

}  // namespace angle

#endif  // LIBANGLE_SIGNAL_UTILS_H_
