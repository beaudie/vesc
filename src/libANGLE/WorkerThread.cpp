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

WaitableEvent::WaitableEvent(EventResetPolicy resetPolicy, EventInitialState initialState)
    : mResetPolicy(resetPolicy), mSignaled(initialState == EventInitialState::Signaled)
{
}

WorkerThreadPool::WorkerThreadPool(size_t maxThreads) : mMaxThreads(maxThreads)
{
}

class SingleThreadedWaitableEvent final : public WaitableEvent
{
  public:
    SingleThreadedWaitableEvent(EventResetPolicy resetPolicy, EventInitialState initialState)
        : WaitableEvent(resetPolicy, initialState)
    {
    }
    ~SingleThreadedWaitableEvent() = default;

    void reset() override;
    void wait() override;
    void signal() override;
    bool isReady() override;
};

void SingleThreadedWaitableEvent::reset()
{
    mSignaled = false;
}

void SingleThreadedWaitableEvent::wait()
{
}

void SingleThreadedWaitableEvent::signal()
{
    mSignaled = true;
}

bool SingleThreadedWaitableEvent::isReady()
{
    return true;
}

// SingleThreadedWorkerPool implementation.
std::unique_ptr<WaitableEvent> SingleThreadedWorkerPool::postWorkerTask(Closure *task)
{
    (*task)();
    return std::make_unique<SingleThreadedWaitableEvent>(EventResetPolicy::Automatic,
                                                         EventInitialState::Signaled);
}

#if (ANGLE_STD_ASYNC_WORKERS == ANGLE_ENABLED)
class AsyncWaitableEvent final : public WaitableEvent
{
  public:
    AsyncWaitableEvent(EventResetPolicy resetPolicy, EventInitialState initialState)
        : WaitableEvent(resetPolicy, initialState)
    {
    }
    ~AsyncWaitableEvent() = default;

    void reset() override;
    void wait() override;
    void signal() override;
    bool isReady() override;

  private:
    friend class AsyncWorkerPool;
    void setFuture(std::future<void> &&future);

    std::future<void> mFuture;
};

void AsyncWaitableEvent::setFuture(std::future<void> &&future)
{
    mFuture = std::move(future);
}

void AsyncWaitableEvent::reset()
{
    mSignaled = false;
    mFuture   = std::future<void>();
}

void AsyncWaitableEvent::wait()
{
    if (mSignaled || !mFuture.valid())
    {
        return;
    }

    mFuture.wait();
    signal();
}

void AsyncWaitableEvent::signal()
{
    mSignaled = true;

    if (mResetPolicy == EventResetPolicy::Automatic)
    {
        reset();
    }
}

bool AsyncWaitableEvent::isReady()
{
    if (mSignaled || !mFuture.valid())
    {
        return true;
    }
    return mFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

// AsyncWorkerPool implementation.
std::unique_ptr<WaitableEvent> AsyncWorkerPool::postWorkerTask(Closure *task)
{
    auto future = std::async(std::launch::async, [task] { (*task)(); });

    auto waitable = std::make_unique<AsyncWaitableEvent>(EventResetPolicy::Automatic,
                                                         EventInitialState::NonSignaled);

    waitable->setFuture(std::move(future));

    return waitable;
}

// static
std::shared_ptr<WorkerThreadPool> WorkerThreadPool::Create()
{
#if (ANGLE_STD_ASYNC_WORKERS == ANGLE_ENABLED)
    return std::shared_ptr<WorkerThreadPool>(
        static_cast<WorkerThreadPool *>(new AsyncWorkerPool(0)));
#endif
    return std::shared_ptr<WorkerThreadPool>(
        static_cast<WorkerThreadPool *>(new SingleThreadedWorkerPool(0)));
}

#endif  // (ANGLE_STD_ASYNC_WORKERS == ANGLE_ENABLED)

}  // namespace angle
