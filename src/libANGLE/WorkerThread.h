//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WorkerThread:
//   Asychronous tasks/threads for ANGLE, similar to a TaskRunner in Chromium.
//   Can be implemented as different targets, depending on platform.
//

#ifndef LIBANGLE_WORKER_THREAD_H_
#define LIBANGLE_WORKER_THREAD_H_

#include <array>
#include <list>

#include "common/debug.h"
#include "libANGLE/RefCountObject.h"
#include "libANGLE/features.h"

#if (ANGLE_STD_ASYNC_WORKERS == ANGLE_ENABLED)
#include <future>
#endif  // (ANGLE_STD_ASYNC_WORKERS == ANGLE_ENABLED)

namespace angle
{
// Indicates whether a WaitableEvent should automatically reset the event state after a single
// waiting thread has been released or remain signaled until reset() is manually invoked.
enum class EventResetPolicy
{
    Manual,
    Automatic
};

// Specify the initial state on creation.
enum class EventInitialState
{
    NonSignaled,
    Signaled
};

// A callback function with no return value and no arguments.
class Closure
{
  public:
    virtual ~Closure()        = default;
    virtual void operator()() = 0;
};

namespace priv
{
// An event that we can wait on, useful for joining worker threads.
template <typename Impl>
class WaitableEventBase : angle::NonCopyable
{
  public:
    WaitableEventBase(EventResetPolicy resetPolicy, EventInitialState initialState);

    // Puts the event in the un-signaled state.
    void reset();

    // Waits indefinitely for the event to be signaled.
    void wait();

    // Puts the event in the signaled state, causing any thread blocked on Wait to be woken up.
    // The event state is reset to non-signaled after a waiting thread has been released.
    void signal();

    // Peeks whether the event is ready. If ready, wait() will not block.
    bool isReady();

  protected:
    template <size_t Count>
    static size_t WaitManyBase(std::array<std::unique_ptr<Impl>, Count> *waitables);

    EventResetPolicy mResetPolicy;
    bool mSignaled;
};

template <typename Impl>
WaitableEventBase<Impl>::WaitableEventBase(EventResetPolicy resetPolicy,
                                           EventInitialState initialState)
    : mResetPolicy(resetPolicy), mSignaled(initialState == EventInitialState::Signaled)
{
}

template <typename Impl>
void WaitableEventBase<Impl>::reset()
{
    static_cast<Impl *>(this)->resetImpl();
}

template <typename Impl>
void WaitableEventBase<Impl>::wait()
{
    static_cast<Impl *>(this)->waitImpl();
}

template <typename Impl>
void WaitableEventBase<Impl>::signal()
{
    static_cast<Impl *>(this)->signalImpl();
}

template <typename Impl>
bool WaitableEventBase<Impl>::isReady()
{
    return static_cast<Impl *>(this)->isReadyImpl();
}

template <typename Impl>
template <size_t Count>
// static
size_t WaitableEventBase<Impl>::WaitManyBase(std::array<std::unique_ptr<Impl>, Count> *waitables)
{
    ASSERT(Count > 0);

    for (size_t index = 0; index < Count; ++index)
    {
        (*waitables)[index]->wait();
    }

    return 0;
}

class SingleThreadedWaitableEvent : public WaitableEventBase<SingleThreadedWaitableEvent>
{
  public:
    SingleThreadedWaitableEvent();
    SingleThreadedWaitableEvent(EventResetPolicy resetPolicy, EventInitialState initialState);
    ~SingleThreadedWaitableEvent();

    SingleThreadedWaitableEvent(SingleThreadedWaitableEvent &&other);
    SingleThreadedWaitableEvent &operator=(SingleThreadedWaitableEvent &&other);

    void resetImpl();
    void waitImpl();
    void signalImpl();
    bool isReadyImpl();

    // Wait, synchronously, on multiple events.
    // returns the index of a WaitableEvent which has been signaled.
    template <size_t Count>
    static size_t WaitMany(
        std::array<std::unique_ptr<SingleThreadedWaitableEvent>, Count> *waitables);
};

template <size_t Count>
// static
size_t SingleThreadedWaitableEvent::WaitMany(
    std::array<std::unique_ptr<SingleThreadedWaitableEvent>, Count> *waitables)
{
    return WaitableEventBase<SingleThreadedWaitableEvent>::WaitManyBase(waitables);
}

#if (ANGLE_STD_ASYNC_WORKERS == ANGLE_ENABLED)
class AsyncWorkerPool;
class AsyncWaitableEvent : public WaitableEventBase<AsyncWaitableEvent>
{
  public:
    AsyncWaitableEvent() = delete;
    AsyncWaitableEvent(AsyncWorkerPool *pool,
                       EventResetPolicy resetPolicy,
                       EventInitialState initialState);
    ~AsyncWaitableEvent();

    void resetImpl();
    void waitImpl();
    void signalImpl();
    bool isReadyImpl();

    // Wait, synchronously, on multiple events.
    // returns the index of a WaitableEvent which has been signaled.
    template <size_t Count>
    static size_t WaitMany(std::array<std::unique_ptr<AsyncWaitableEvent>, Count> *waitables);

  private:
    friend class AsyncWorkerPool;

    void setFuture(std::future<void> &&future);

    std::future<void> mFuture;
    AsyncWorkerPool *mPool;
};

template <size_t Count>
// static
size_t AsyncWaitableEvent::WaitMany(
    std::array<std::unique_ptr<AsyncWaitableEvent>, Count> *waitables)
{
    return WaitableEventBase<AsyncWaitableEvent>::WaitManyBase(waitables);
}
#endif  // (ANGLE_STD_ASYNC_WORKERS == ANGLE_ENABLED)

// The traits class allows the the thread pool to return the "Typed" waitable event from postTask.
// Otherwise postTask would always think it returns the current active type, so the unit tests
// could not run on multiple worker types in the same compilation.
template <typename Impl>
struct WorkerThreadPoolTraits;

class SingleThreadedWorkerPool;
template <>
struct WorkerThreadPoolTraits<SingleThreadedWorkerPool>
{
    using WaitableEventType = SingleThreadedWaitableEvent;
};

#if (ANGLE_STD_ASYNC_WORKERS == ANGLE_ENABLED)
class AsyncWorkerPool;
template <>
struct WorkerThreadPoolTraits<AsyncWorkerPool>
{
    using WaitableEventType = AsyncWaitableEvent;
};
#endif  // (ANGLE_STD_ASYNC_WORKERS == ANGLE_ENABLED)

// Request WorkerThreads from the WorkerThreadPool. Each pool can keep worker threads around so
// we avoid the costly spin up and spin down time.
template <typename Impl>
class WorkerThreadPoolBase : public gl::RefCountObjectNoID
{
  public:
    WorkerThreadPoolBase(size_t maxThreads);
    ~WorkerThreadPoolBase();

    using WaitableEventType = typename WorkerThreadPoolTraits<Impl>::WaitableEventType;

    // Returns an event to wait on for the task to finish.
    // If the pool fails to create the task, returns null.
    WaitableEventType *postWorkerTask(Closure *task);

    void setMaxThreads(size_t maxThreads);
    size_t getMaxThreads();

  protected:
    size_t mMaxThreads;
};

template <typename Impl>
WorkerThreadPoolBase<Impl>::WorkerThreadPoolBase(size_t maxThreads)
{
}

template <typename Impl>
WorkerThreadPoolBase<Impl>::~WorkerThreadPoolBase()
{
}

template <typename Impl>
typename WorkerThreadPoolBase<Impl>::WaitableEventType *WorkerThreadPoolBase<Impl>::postWorkerTask(
    Closure *task)
{
    return static_cast<Impl *>(this)->postWorkerTaskImpl(task);
}

class SingleThreadedWorkerPool : public WorkerThreadPoolBase<SingleThreadedWorkerPool>
{
  public:
    SingleThreadedWorkerPool(size_t maxThreads);
    ~SingleThreadedWorkerPool();

    SingleThreadedWaitableEvent *postWorkerTaskImpl(Closure *task);
};

#if (ANGLE_STD_ASYNC_WORKERS == ANGLE_ENABLED)
class AsyncWorkerPool : public WorkerThreadPoolBase<AsyncWorkerPool>
{
  public:
    AsyncWorkerPool(size_t maxThreads);
    ~AsyncWorkerPool();

    AsyncWaitableEvent *postWorkerTaskImpl(Closure *task);

  private:
    friend class AsyncWaitableEvent;

    void cleanUpEvent(AsyncWaitableEvent *event);

    std::list<AsyncWaitableEvent *> mEvents;
};
#endif  // (ANGLE_STD_ASYNC_WORKERS == ANGLE_ENABLED)

}  // namespace priv

#if (ANGLE_STD_ASYNC_WORKERS == ANGLE_ENABLED)
using WaitableEvent    = priv::AsyncWaitableEvent;
using WorkerThreadPool = priv::AsyncWorkerPool;
#else
using WaitableEvent    = priv::SingleThreadedWaitableEvent;
using WorkerThreadPool = priv::SingleThreadedWorkerPool;
#endif  // (ANGLE_STD_ASYNC_WORKERS == ANGLE_ENABLED)

}  // namespace angle

#endif  // LIBANGLE_WORKER_THREAD_H_
