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

#include "common/angleutils.h"

namespace gl
{
class Context;
}  // namespace gl

namespace angle
{

enum Message
{
    ATTACHMENT_CHANGE,
};

using ChannelID = size_t;

// Interface that the depending class inherits from.
class SignalReceiver
{
  public:
    virtual ~SignalReceiver();
    virtual void signal(const gl::Context *context, ChannelID channelID, Message message) = 0;
};

class ChannelBinding;

// The host class owns the channel. It uses the channel to fire signals to the receiver.
class BroadcastChannel final : NonCopyable
{
  public:
    BroadcastChannel();
    ~BroadcastChannel();

    void signal(const gl::Context *context, Message message) const;

    void reset();

    bool empty() const;

  private:
    // Only the ChannelBinding class should add or remove receivers.
    friend class ChannelBinding;
    void addReceiver(ChannelBinding *receiver);
    void removeReceiver(ChannelBinding *receiver);

    std::vector<ChannelBinding *> mReceivers;
};

// The dependent class keeps bindings to the host's BroadcastChannel.
class ChannelBinding final
{
  public:
    ChannelBinding(SignalReceiver *receiver, ChannelID channelID);
    ~ChannelBinding();
    ChannelBinding(const ChannelBinding &other);
    ChannelBinding &operator=(const ChannelBinding &other);

    void bind(BroadcastChannel *channel);
    void reset();
    void signal(const gl::Context *context, Message message) const;
    void onChannelClosed();

  private:
    BroadcastChannel *mChannel;
    SignalReceiver *mReceiver;
    ChannelID mChannelID;
};

}  // namespace angle

#endif  // LIBANGLE_SIGNAL_UTILS_H_
