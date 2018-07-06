//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WorkerThread:
//   Task running thread for ANGLE, similar to a TaskRunner in Chromium.
//   Might be implemented differently depending on platform.
//

#include "libANGLE/WorkerThread.h"

namespace angle
{

namespace priv
{
// SingleThreadedWorkerPool implementation.
SingleThreadedWorkerPool::SingleThreadedWorkerPool(size_t maxThreads) : WorkerThreadPoolBase(0)
{
}

SingleThreadedWorkerPool::~SingleThreadedWorkerPool()
{
}

SingleThreadedWaitableEvent *SingleThreadedWorkerPool::postWorkerTaskImpl(Closure *task)
{
    (*task)();
    return new SingleThreadedWaitableEvent(EventResetPolicy::Automatic,
                                           EventInitialState::Signaled);
}

// SingleThreadedWaitableEvent implementation.
SingleThreadedWaitableEvent::SingleThreadedWaitableEvent()
    : SingleThreadedWaitableEvent(EventResetPolicy::Automatic, EventInitialState::NonSignaled)
{
}

SingleThreadedWaitableEvent::SingleThreadedWaitableEvent(EventResetPolicy resetPolicy,
                                                         EventInitialState initialState)
    : WaitableEventBase(resetPolicy, initialState)
{
}

SingleThreadedWaitableEvent::~SingleThreadedWaitableEvent()
{
}

void SingleThreadedWaitableEvent::resetImpl()
{
    mSignaled = false;
}

void SingleThreadedWaitableEvent::waitImpl()
{
}

void SingleThreadedWaitableEvent::signalImpl()
{
    mSignaled = true;
}

bool SingleThreadedWaitableEvent::isReadyImpl()
{
    return true;
}

#if (ANGLE_STD_ASYNC_WORKERS == ANGLE_ENABLED)
// AsyncWorkerPool implementation.
AsyncWorkerPool::AsyncWorkerPool(size_t maxThreads) : WorkerThreadPoolBase(maxThreads)
{
}

AsyncWorkerPool::~AsyncWorkerPool()
{
    ASSERT(mEvents.empty());
}

AsyncWaitableEvent *AsyncWorkerPool::postWorkerTaskImpl(Closure *task)
{
    size_t runningTasksCount = 0;
    for (auto *event : mEvents)
    {
        ASSERT(event);
        if (!event->isReady())
        {
            ++runningTasksCount;
        }
    }
    AsyncWaitableEvent *waitable = nullptr;
    if (runningTasksCount < getMaxThreads())
    {
        auto future = std::async(std::launch::async, [task] { (*task)(); });

        waitable = new AsyncWaitableEvent(this, EventResetPolicy::Automatic,
                                          EventInitialState::NonSignaled);

        waitable->setFuture(std::move(future));
    }
    else
    {
        (*task)();
        waitable =
            new AsyncWaitableEvent(this, EventResetPolicy::Automatic, EventInitialState::Signaled);
    }
    mEvents.push_back(waitable);
    return waitable;
}

void AsyncWorkerPool::cleanUpEvent(AsyncWaitableEvent *event)
{
    mEvents.remove(event);
}

// AsyncWaitableEvent implementation.

AsyncWaitableEvent::AsyncWaitableEvent(AsyncWorkerPool *pool,
                                       EventResetPolicy resetPolicy,
                                       EventInitialState initialState)
    : WaitableEventBase(resetPolicy, initialState), mPool(pool)
{
}

AsyncWaitableEvent::~AsyncWaitableEvent()
{
    ASSERT(mPool);
    mPool->cleanUpEvent(this);
}

void AsyncWaitableEvent::setFuture(std::future<void> &&future)
{
    mFuture = std::move(future);
}

void AsyncWaitableEvent::resetImpl()
{
    mSignaled = false;
    mFuture   = std::future<void>();
}

void AsyncWaitableEvent::waitImpl()
{
    if (mSignaled || !mFuture.valid())
    {
        return;
    }

    mFuture.wait();
    signal();
}

void AsyncWaitableEvent::signalImpl()
{
    mSignaled = true;

    if (mResetPolicy == EventResetPolicy::Automatic)
    {
        reset();
    }
}

bool AsyncWaitableEvent::isReadyImpl()
{
    if (mSignaled || !mFuture.valid())
    {
        return true;
    }
    return mFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

#endif  // (ANGLE_STD_ASYNC_WORKERS == ANGLE_ENABLED)

}  // namespace priv

}  // namespace angle
