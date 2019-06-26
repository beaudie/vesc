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

#if (ANGLE_STD_ASYNC_WORKERS == ANGLE_ENABLED)
#    include <condition_variable>
#    include <future>
#    include <mutex>
#    include <queue>
#    include <thread>
#endif  // (ANGLE_STD_ASYNC_WORKERS == ANGLE_ENABLED)

#if defined(ANGLE_PLATFORM_WINDOWS)
#    include <processthreadsapi.h>
#endif  // ANGLE_PLATFORM_WINDOWS

namespace angle
{

WaitableEvent::WaitableEvent()  = default;
WaitableEvent::~WaitableEvent() = default;

void WaitableEventDone::wait() {}

bool WaitableEventDone::isReady()
{
    return true;
}

WorkerThreadPool::WorkerThreadPool()  = default;
WorkerThreadPool::~WorkerThreadPool() = default;

class SingleThreadedWaitableEvent final : public WaitableEvent
{
  public:
    SingleThreadedWaitableEvent()           = default;
    ~SingleThreadedWaitableEvent() override = default;

    void wait() override;
    bool isReady() override;
};

void SingleThreadedWaitableEvent::wait() {}

bool SingleThreadedWaitableEvent::isReady()
{
    return true;
}

class SingleThreadedWorkerPool final : public WorkerThreadPool
{
  public:
    std::shared_ptr<WaitableEvent> postWorkerTask(std::shared_ptr<Closure> task) override;
    void setMaxThreads(size_t maxThreads) override;
    bool isAsync() override;
};

// SingleThreadedWorkerPool implementation.
std::shared_ptr<WaitableEvent> SingleThreadedWorkerPool::postWorkerTask(
    std::shared_ptr<Closure> task)
{
    (*task)();
    return std::make_shared<SingleThreadedWaitableEvent>();
}

void SingleThreadedWorkerPool::setMaxThreads(size_t maxThreads) {}

bool SingleThreadedWorkerPool::isAsync()
{
    return false;
}

#if (ANGLE_STD_ASYNC_WORKERS == ANGLE_ENABLED)
class AsyncWaitableEvent final : public WaitableEvent
{
  public:
    AsyncWaitableEvent() : mFuture(mPromise.get_future()) {}
    ~AsyncWaitableEvent() override = default;

    void wait() override;
    bool isReady() override;

  private:
    friend class AsyncWorkerPool;
    std::promise<void> mPromise;
    std::future<void> mFuture;
};

void AsyncWaitableEvent::wait()
{
    ASSERT(mFuture.valid());
    mFuture.wait();
}

bool AsyncWaitableEvent::isReady()
{
    ASSERT(mFuture.valid());
    return mFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

class AsyncWorkerPool final : public WorkerThreadPool
{
  public:
    AsyncWorkerPool(size_t maxTheads);
    ~AsyncWorkerPool() override;

    std::shared_ptr<WaitableEvent> postWorkerTask(std::shared_ptr<Closure> task) override;
    void setMaxThreads(size_t maxThreads) override;
    bool isAsync() override;

  private:
    void checkToRunPendingTasks(size_t threadIndex);

    // To protect the concurrent accesses from both main thread and background
    // threads to the member fields.
    std::mutex mMutex;

    size_t mMaxThreads;
    std::vector<std::thread> mThreads;
    std::condition_variable mWakeUpThreads;

    std::queue<std::pair<std::shared_ptr<AsyncWaitableEvent>, std::shared_ptr<Closure>>> mTaskQueue;
};

// AsyncWorkerPool implementation.
AsyncWorkerPool::AsyncWorkerPool(size_t maxThreads)
{
    AsyncWorkerPool::setMaxThreads(maxThreads);
}

AsyncWorkerPool::~AsyncWorkerPool()
{
    AsyncWorkerPool::setMaxThreads(0);
}

std::shared_ptr<WaitableEvent> AsyncWorkerPool::postWorkerTask(std::shared_ptr<Closure> task)
{
    ASSERT(mMaxThreads > 0);

    auto waitable = std::make_shared<AsyncWaitableEvent>();
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mTaskQueue.push(std::make_pair(waitable, task));
    }
    mWakeUpThreads.notify_one();
    return waitable;
}

void AsyncWorkerPool::setMaxThreads(size_t maxThreads)
{
    std::vector<std::thread> shutdownThreads;
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mMaxThreads = (maxThreads == 0xFFFFFFFF ? std::thread::hardware_concurrency() : maxThreads);

        for (size_t i = mThreads.size(); i < mMaxThreads; ++i)
            mThreads.emplace_back(&AsyncWorkerPool::checkToRunPendingTasks, this, i);

        for (size_t i = mMaxThreads; i < mThreads.size(); ++i)
            shutdownThreads.emplace_back(std::move(mThreads[i]));

        mThreads.erase(mThreads.begin() + mMaxThreads, mThreads.end());
    }

    mWakeUpThreads.notify_all();

    for (auto &thread : shutdownThreads)
        thread.join();
}

bool AsyncWorkerPool::isAsync()
{
    return true;
}

void AsyncWorkerPool::checkToRunPendingTasks(size_t threadIndex)
{
#    if defined(ANGLE_PLATFORM_WINDOWS)
    ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#    endif  // ANGLE_PLATFORM_WINDOWS

    std::unique_lock<std::mutex> lock(mMutex);
    while (threadIndex < mMaxThreads)
    {
        mWakeUpThreads.wait(lock);

        if (mTaskQueue.empty())
            continue;

        auto task = mTaskQueue.front();
        mTaskQueue.pop();

        auto waitable = task.first;
        auto closure  = task.second;
        (*closure)();
        waitable->mPromise.set_value();
    }
}
#endif  // (ANGLE_STD_ASYNC_WORKERS == ANGLE_ENABLED)

// static
std::shared_ptr<WorkerThreadPool> WorkerThreadPool::Create(bool multithreaded)
{
    std::shared_ptr<WorkerThreadPool> pool(nullptr);
#if (ANGLE_STD_ASYNC_WORKERS == ANGLE_ENABLED)
    if (multithreaded)
    {
        pool = std::shared_ptr<WorkerThreadPool>(static_cast<WorkerThreadPool *>(
            new AsyncWorkerPool(std::thread::hardware_concurrency())));
    }
#endif
    if (!pool)
    {
        return std::shared_ptr<WorkerThreadPool>(
            static_cast<WorkerThreadPool *>(new SingleThreadedWorkerPool()));
    }
    return pool;
}

// static
std::shared_ptr<WaitableEvent> WorkerThreadPool::PostWorkerTask(
    std::shared_ptr<WorkerThreadPool> pool,
    std::shared_ptr<Closure> task)
{
    std::shared_ptr<WaitableEvent> event = pool->postWorkerTask(task);
    if (event.get())
    {
        event->setWorkerThreadPool(pool);
    }
    return event;
}

}  // namespace angle
