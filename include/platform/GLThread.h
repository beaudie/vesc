//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GLThread:
//   Platform API for a hardware thread, where the embedder provides the implementation.
//

#ifndef ANGLE_PLATFORM_GLTHREAD_H_
#define ANGLE_PLATFORM_GLTHREAD_H_

#include <stdint.h>

#include "../export.h"

namespace angle
{

class GLTraceLocation;

// Always an integer value.
typedef uintptr_t PlatformThreadId;

// Provides an interface to an embedder-defined thread implementation.
//
// Deleting the thread blocks until all pending, non-delayed tasks have been
// run.
class ANGLE_EXPORT GLThread
{
  public:
    // An IdleTask is passed a deadline in CLOCK_MONOTONIC seconds and is
    // expected to complete before this deadline.
    class IdleTask
    {
      public:
        virtual ~IdleTask() {}
        virtual void run(double deadlineSeconds) = 0;
    };

    class ANGLE_EXPORT Task
    {
      public:
        virtual ~Task() {}
        virtual void run() = 0;
    };

    // postTask() and postDelayedTask() take ownership of the passed Task
    // object. It is safe to invoke postTask() and postDelayedTask() from any
    // thread.
    virtual void postTask(const GLTraceLocation &, Task *) = 0;
    virtual void postDelayedTask(const GLTraceLocation &, Task *, long long delayMs) = 0;

    virtual bool isCurrentThread() const = 0;
    virtual PlatformThreadId threadId() const { return 0; }

    virtual ~GLThread() {}
};

#ifdef LIBANGLE_IMPLEMENTATION
// Helper function for when the platform implementation doesn't support threading.
inline void PostTaskOrRun(GLThread *maybeNullThread,
                          const GLTraceLocation &location,
                          GLThread::Task *task)
{
    if (maybeNullThread != nullptr)
    {
        maybeNullThread->postTask(location, task);
    }
    else
    {
        task->run();
        delete task;
    }
}
#endif

}  // namespace angle

#endif  // ANGLE_PLATFORM_GLTHREAD_H_
