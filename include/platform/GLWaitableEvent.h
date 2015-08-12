//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GLWaitableEventh:
//   ANGLE platform interface for waitiable events. Implementation provided by the embedder.
//

#ifndef ANGLE_PLATFORM_GLWAITABLEEVENT_H_
#define ANGLE_PLATFORM_GLWAITABLEEVENT_H_

namespace angle
{

// Provides a thread synchronization that can be used to allow one thread to
// wait until another thread to finish some work.
class GLWaitableEvent
{
  public:
    // If ResetPolicy::Manual is specified on creation, to set the event state
    // to non-signaled, a consumer must call reset().  Otherwise, the system
    // automatically resets the event state to non-signaled after a single
    // waiting thread has been released.
    enum class ResetPolicy
    {
        Auto,
        Manual
    };

    // Specify the initial state on creation.
    enum class InitialState
    {
        NonSignaled,
        Signaled
    };

    virtual ~GLWaitableEvent() {}

    // Puts the event in the un-signaled state.
    virtual void reset() {}

    // Waits indefinitely for the event to be signaled.
    virtual void wait() {}

    // Puts the event in the signaled state. Causing any thread blocked on Wait
    // to be woken up. The event state is reset to non-signaled after
    // a waiting thread has been released.
    virtual void signal() {}
};

}  // namespace angle

#endif  // ANGLE_PLATFORM_GLWAITABLEEVENT_H_
