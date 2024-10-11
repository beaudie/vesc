//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WorkerThread:
//   Task running thread for ANGLE, similar to a TaskRunner in Chromium.
//   Might be implemented differently depending on platform.
//

#include "common/WorkerThread.h"

#include "common/angleutils.h"
#include "common/system_utils.h"

// Controls if our threading code uses std::async or falls back to single-threaded operations.
// Note that we can't easily use std::async in UWPs due to UWP threading restrictions.
#if !defined(ANGLE_STD_ASYNC_WORKERS) && !defined(ANGLE_ENABLE_WINDOWS_UWP)
#    define ANGLE_STD_ASYNC_WORKERS 1
#endif  // !defined(ANGLE_STD_ASYNC_WORKERS) && & !defined(ANGLE_ENABLE_WINDOWS_UWP)

#if ANGLE_DELEGATE_WORKERS || ANGLE_STD_ASYNC_WORKERS
#    include <queue>
#    include <thread>
#endif  // ANGLE_DELEGATE_WORKERS || ANGLE_STD_ASYNC_WORKERS

namespace angle
{
namespace
{
std::shared_ptr<WaitableEvent> RunInline(const std::shared_ptr<WorkerTask> &task)
{
    auto waitable = std::make_shared<AsyncWaitableEvent>();
    (*task)();
    waitable->markAsReady();
    return waitable;
}
}  // anonymous namespace

class AsyncTask final : public AsyncWaitableEvent
{
  public:
    explicit AsyncTask(const std::shared_ptr<WorkerTask> &workerTask);
    ~AsyncTask() override;

    void run();
    void markAsReady();
    void runAndMarkAsReady();

  private:
    std::shared_ptr<WorkerTask> mWorkerTask;
};

class AsyncTaskWorkerPool : public WorkerThreadPool,
                            public std::enable_shared_from_this<AsyncTaskWorkerPool>
{
  public:
    // WorkerThreadPool
    std::shared_ptr<WaitableEvent> postWorkerTask(const std::shared_ptr<WorkerTask> &task) final;

    virtual void postAsyncTask(std::shared_ptr<AsyncTask> &&task) = 0;
};

// WaitableEvent implementation.
WaitableEvent::WaitableEvent()  = default;
WaitableEvent::~WaitableEvent() = default;

// AsyncWaitableEvent implementation.
void AsyncWaitableEvent::markAsReady()
{
    {
        std::lock_guard<std::mutex> lock(mMutex);
        // Using release ordering to provide synchronization to |isReady|'s acquire load, which does
        // not happen under the mutex.
        mIsReady.store(1, std::memory_order_release);
    }
    mCondition.notify_all();
}

void AsyncWaitableEvent::wait()
{
    if (!isReady())
    {
        std::unique_lock<std::mutex> lock(mMutex);
        // Using relaxed ordering because mutex lock provides necessary synchronization.
        mCondition.wait(lock, [this] { return mIsReady.load(std::memory_order_relaxed) != 0; });
    }
}

bool AsyncWaitableEvent::isReady()
{
    // Using acquire ordering to synchronize with |markAsReady|'s release store.
    return mIsReady.load(std::memory_order_acquire) != 0;
}

// AsyncTask implementation.
AsyncTask::AsyncTask(const std::shared_ptr<WorkerTask> &workerTask) : mWorkerTask(workerTask)
{
    ASSERT(mWorkerTask);
}

AsyncTask::~AsyncTask()
{
    ASSERT(!mWorkerTask || !isReady());
}

void AsyncTask::run()
{
    ASSERT(mWorkerTask);

    // Note: always add an ANGLE_TRACE_EVENT* macro in the worker task.  Then the job will show up
    // in traces.
    (*mWorkerTask)();

    // Release shared_ptr<WorkerTask> before notifying the event to allow for destructor based
    // dependencies (example: anglebug.com/42267099)
    mWorkerTask.reset();
}

void AsyncTask::markAsReady()
{
    ASSERT(!mWorkerTask);
    AsyncWaitableEvent::markAsReady();
}

void AsyncTask::runAndMarkAsReady()
{
    run();
    markAsReady();
}

// WorkerThreadPool implementation.
WorkerThreadPool::WorkerThreadPool()  = default;
WorkerThreadPool::~WorkerThreadPool() = default;

// AsyncTaskWorkerPool implementation.
std::shared_ptr<WaitableEvent> AsyncTaskWorkerPool::postWorkerTask(
    const std::shared_ptr<WorkerTask> &task)
{
    // Thread safety: This function is thread-safe because |postAsyncTask| is expected to be
    // thread-safe.
    auto asyncTask = std::make_shared<AsyncTask>(task);

    postAsyncTask(std::shared_ptr<AsyncTask>(asyncTask));

    return asyncTask;
}

class SingleThreadedWorkerPool final : public WorkerThreadPool
{
  public:
    // WorkerThreadPool
    std::shared_ptr<WaitableEvent> postWorkerTask(const std::shared_ptr<WorkerTask> &task) override;
    bool isAsync() override;
};

// SingleThreadedWorkerPool implementation.
std::shared_ptr<WaitableEvent> SingleThreadedWorkerPool::postWorkerTask(
    const std::shared_ptr<WorkerTask> &task)
{
    // Thread safety: This function is thread-safe because the task is run on the calling thread
    // itself.
    return RunInline(task);
}

bool SingleThreadedWorkerPool::isAsync()
{
    return false;
}

#if ANGLE_DELEGATE_WORKERS || ANGLE_STD_ASYNC_WORKERS

// Notes:
//   "Locked" suffix means, that the mutex MUST be locked for the entire duration of the call.
//   "Unlocked" suffix means, that the function MUST not be called under the mutex lock.

class WorkerThreadLoopPolicy
{
  public:
    virtual ~WorkerThreadLoopPolicy() = default;
    // Ensures that there is a next task and it can be run, or returns false.  Allowed to wait on
    // the conditional variable using the passed |lock|.  The |lock| MUST be in a locked state
    // before and after the call.
    virtual bool ensureHasNextTaskToRun(std::unique_lock<std::mutex> *lock) = 0;
};

// Base for the |AsyncWorkerPool| and the |DelegateWorkerPool| classes.
class MultiThreadedWorkerPool : public AsyncTaskWorkerPool
{
  public:
    MultiThreadedWorkerPool() = default;
    ~MultiThreadedWorkerPool() override;

    // AsyncTaskWorkerPool
    void postAsyncTask(std::shared_ptr<AsyncTask> &&task) final;

  protected:
    // Runs tasks from the |mTaskQueue| until |policy->ensureHasNextTaskToRun| returns false.
    // Designed to be called from multiple threads.
    void runThreadLoop(WorkerThreadLoopPolicy *policy);

    bool isTaskQueueEmptyLocked() const { return mTaskQueue.empty(); }
    std::shared_ptr<AsyncTask> &getNextTaskLocked();

    void incrementFreeThreadCount(size_t n);
    void decrementFreeThreadCount(size_t n);
    bool needToStartThreadLocked() const;

    // Begins to create new or wake existing thread.
    // |*needToEndStartThreadOut| is true upon entry, assigning false will skip the end call.
    // Before creating a new thread the |mFreeThreadCount| MUST be incremented by calling
    // |incrementFreeThreadCount(1)| method.
    virtual void beginStartThreadLocked(bool *needToEndStartThreadOut) = 0;
    // Ends to create new or wake existing thread.
    // Called if |*needToEndStartThreadOut| remains true when the correspondng begin call ends.
    virtual void endStartThreadUnlocked() = 0;

    std::mutex mMutex;  // Protects access to the fields in this class

  private:
    std::queue<std::shared_ptr<AsyncTask>> mTaskQueue;
    std::atomic<size_t> mFreeThreadCount{0};
};

// MultiThreadedWorkerPool implementation.
MultiThreadedWorkerPool::~MultiThreadedWorkerPool()
{
    ASSERT(mFreeThreadCount == 0);
}

void MultiThreadedWorkerPool::postAsyncTask(std::shared_ptr<AsyncTask> &&task)
{
    // Thread safety: This function is thread-safe because member access is protected by |mMutex|.
    bool needToEndStartThread = false;
    {
        std::lock_guard<std::mutex> lock(mMutex);

        mTaskQueue.emplace(std::move(task));

        if (needToStartThreadLocked())
        {
            needToEndStartThread = true;
            beginStartThreadLocked(&needToEndStartThread);
        }
    }
    if (needToEndStartThread)
    {
        endStartThreadUnlocked();
    }
}

void MultiThreadedWorkerPool::runThreadLoop(WorkerThreadLoopPolicy *policy)
{
    while (true)
    {
        std::shared_ptr<AsyncTask> task;
        {
            std::unique_lock<std::mutex> lock(mMutex);
            // Mark this thread as busy running a task, waiting for a task, or terminated.
            decrementFreeThreadCount(1);
            if (!policy->ensureHasNextTaskToRun(&lock))
            {
                return;
            }
            task = std::move(getNextTaskLocked());
            mTaskQueue.pop();
        }

        task->run();

        // Mark this thread as free before marking task as ready.
        incrementFreeThreadCount(1);

        task->markAsReady();
    }
}

std::shared_ptr<AsyncTask> &MultiThreadedWorkerPool::getNextTaskLocked()
{
    ASSERT(!isTaskQueueEmptyLocked());
    return mTaskQueue.front();
}

void MultiThreadedWorkerPool::incrementFreeThreadCount(size_t n)
{
    mFreeThreadCount.fetch_add(n, std::memory_order_relaxed);
}

void MultiThreadedWorkerPool::decrementFreeThreadCount(size_t n)
{
    const size_t prevCount = mFreeThreadCount.fetch_sub(n, std::memory_order_relaxed);
    ASSERT(prevCount >= n);
}

bool MultiThreadedWorkerPool::needToStartThreadLocked() const
{
    return (mTaskQueue.size() > mFreeThreadCount.load(std::memory_order_relaxed));
}

#endif  // ANGLE_DELEGATE_WORKERS || ANGLE_STD_ASYNC_WORKERS

#if ANGLE_STD_ASYNC_WORKERS

class AsyncWorkerPool final : public MultiThreadedWorkerPool, WorkerThreadLoopPolicy
{
  public:
    explicit AsyncWorkerPool(size_t numThreads);
    ~AsyncWorkerPool() override;

    // WorkerThreadPool
    bool isAsync() override;

  protected:
    // MultiThreadedWorkerPool
    void beginStartThreadLocked(bool *needToEndStartThreadOut) override;
    void endStartThreadUnlocked() override;

  private:
    // Thread's main loop
    void threadLoop();

    void incrementNotifiedThreadCountLocked(size_t n);
    void decrementNotifiedThreadCountLocked(size_t n);

    // WorkerThreadLoopPolicy
    bool ensureHasNextTaskToRun(std::unique_lock<std::mutex> *lock) override;

    bool mTerminated = false;
    std::condition_variable mCondVar;  // Signals when work is available in the queue
    std::vector<std::thread> mThreads;
    size_t mDesiredThreadCount;
    size_t mWaitingThreadCount  = 0;
    size_t mNotifiedThreadCount = 0;
};

// AsyncWorkerPool implementation.
AsyncWorkerPool::AsyncWorkerPool(size_t numThreads) : mDesiredThreadCount(numThreads)
{
    ASSERT(numThreads != 0);
}

AsyncWorkerPool::~AsyncWorkerPool()
{
    {
        std::lock_guard<std::mutex> lock(mMutex);
        ASSERT(mWaitingThreadCount >= mNotifiedThreadCount);
        incrementNotifiedThreadCountLocked(mWaitingThreadCount - mNotifiedThreadCount);
        mTerminated = true;
    }
    mCondVar.notify_all();
    for (auto &thread : mThreads)
    {
        ASSERT(thread.get_id() != std::this_thread::get_id());
        thread.join();
    }
    ASSERT(mWaitingThreadCount == 0);
    ASSERT(mNotifiedThreadCount == 0);
}

void AsyncWorkerPool::beginStartThreadLocked(bool *needToEndStartThreadOut)
{
    // Try to wake a waiting thread to avoid creating new thread unnecessarily.
    if (mWaitingThreadCount > mNotifiedThreadCount)
    {
        incrementNotifiedThreadCountLocked(1);
        ASSERT(*needToEndStartThreadOut);
        return;
    }

    // Otherwise, try to create a new thread (does not require the end call).
    *needToEndStartThreadOut = false;
    if (mThreads.size() < mDesiredThreadCount)
    {
        incrementFreeThreadCount(1);
        mThreads.emplace_back(&AsyncWorkerPool::threadLoop, this);
        return;
    }

    // All threads are created and running, so nothing to do.
}

void AsyncWorkerPool::endStartThreadUnlocked()
{
    mCondVar.notify_one();
}

void AsyncWorkerPool::threadLoop()
{
    angle::SetCurrentThreadName("ANGLE-Worker");

    runThreadLoop(this);
}

bool AsyncWorkerPool::ensureHasNextTaskToRun(std::unique_lock<std::mutex> *lock)
{
    bool isWaiting = false;
    mCondVar.wait(*lock, [this, &isWaiting] {
        const bool isConditionMet = (!isTaskQueueEmptyLocked() || mTerminated);
        // Start or skip waiting.
        if (!isWaiting)
        {
            if (!isConditionMet)
            {
                ++mWaitingThreadCount;
                isWaiting = true;
            }
        }
        // Filter spurious wakeups.
        else if (mNotifiedThreadCount == 0)
        {
            // When terminated, all threads MUST be notified.
            ASSERT(!mTerminated);
            return false;
        }
        // Continue or stop waiting.
        else
        {
            decrementNotifiedThreadCountLocked(1);
            if (isConditionMet)
            {
                ASSERT(mWaitingThreadCount > 0);
                --mWaitingThreadCount;
            }
        }
        return isConditionMet;
    });
    return !mTerminated;
}

void AsyncWorkerPool::incrementNotifiedThreadCountLocked(size_t n)
{
    ASSERT(mNotifiedThreadCount + n <= mWaitingThreadCount);
    // Mark the notified thread as free to prevent unecessary notifies in the future.
    incrementFreeThreadCount(n);
    mNotifiedThreadCount += n;
}

void AsyncWorkerPool::decrementNotifiedThreadCountLocked(size_t n)
{
    ASSERT(mNotifiedThreadCount >= n);
    // Thread was marked as free when was notified.
    decrementFreeThreadCount(n);
    mNotifiedThreadCount -= n;
}

bool AsyncWorkerPool::isAsync()
{
    return true;
}

#endif  // ANGLE_STD_ASYNC_WORKERS

#if ANGLE_DELEGATE_WORKERS

class DelegateWorkerPool final : public MultiThreadedWorkerPool, WorkerThreadLoopPolicy
{
  public:
    explicit DelegateWorkerPool(PlatformMethods *platform) : mPlatform(platform) {}
    ~DelegateWorkerPool() override;

    // WorkerThreadPool
    bool isAsync() override;

  protected:
    // MultiThreadedWorkerPool
    void beginStartThreadLocked(bool *needToEndStartThreadOut) override;
    void endStartThreadUnlocked() override;

  private:
    // WorkerThreadLoopPolicy
    bool ensureHasNextTaskToRun(std::unique_lock<std::mutex> *lock) override;

    PlatformMethods *mPlatform;

    friend class DelegateWorkerThread;
};

// Represents a delegate worker thread that will call the |runThreadLoop| method.
class DelegateWorkerThread final : WorkerThreadLoopPolicy, angle::NonCopyable
{
  public:
    DelegateWorkerThread(std::shared_ptr<DelegateWorkerPool> &&workerPool)
        : mWorkerPool(std::move(workerPool))
    {}

    static void Run(void *userData)
    {
        auto worker = static_cast<DelegateWorkerThread *>(userData);
        worker->run();
        // Delete the worker after its execution.
        delete worker;
    }

  private:
    ~DelegateWorkerThread() override = default;

    void run()
    {
        mWorkerPool->runThreadLoop(this);
        if (mNeedToEndStartThread)
        {
            mWorkerPool->endStartThreadUnlocked();
        }
    }

    // WorkerThreadLoopPolicy
    bool ensureHasNextTaskToRun(std::unique_lock<std::mutex> *lock) override
    {
        // Only run a first task.
        if (!mWorkerPool->isTaskQueueEmptyLocked() && mIsFirstRun)
        {
            mIsFirstRun = false;
            return true;
        }
        // |MultiThreadedWorkerPool::postAsyncTask| may skip starting a thread in hope that exiting
        // free thread will run the task.  However in reality, free thread may stop running tasks
        // because of the above condition, causing lack of free threads.  To account for this
        // problem, call |needToStartThreadLocked| below and start a thread if needed.
        if (mWorkerPool->needToStartThreadLocked())
        {
            mNeedToEndStartThread = true;
            mWorkerPool->beginStartThreadLocked(&mNeedToEndStartThread);
        }
        return false;
    }

    std::shared_ptr<DelegateWorkerPool> mWorkerPool;
    bool mIsFirstRun           = true;
    bool mNeedToEndStartThread = false;
};

// DelegateWorkerPool implementation.
DelegateWorkerPool::~DelegateWorkerPool()
{
    // No lock because all threads MUST not be running at this point.
    ASSERT(isTaskQueueEmptyLocked());
}

void DelegateWorkerPool::beginStartThreadLocked(bool *needToEndStartThreadOut)
{
    incrementFreeThreadCount(1);
    ASSERT(*needToEndStartThreadOut);
}

ANGLE_NO_SANITIZE_CFI_ICALL
void DelegateWorkerPool::endStartThreadUnlocked()
{
    if (mPlatform->postWorkerTask == nullptr)
    {
        // In the unexpected case where the platform methods have been changed during execution and
        // postWorkerTask is no longer usable, simply run all tasks on the calling thread.
        return runThreadLoop(this);
    }

    // Thread safety: This function is thread-safe because the |postWorkerTask| platform method is
    // expected to be thread safe.  For Chromium, that forwards the call to the |TaskTracker| class
    // in base/task/thread_pool/task_tracker.h which is thread-safe.

    // The worker will be deleted by DelegateWorkerThread::Run(...) after its execution.
    auto worker =
        new DelegateWorkerThread(static_pointer_cast<DelegateWorkerPool>(shared_from_this()));
    mPlatform->postWorkerTask(mPlatform, DelegateWorkerThread::Run, worker);
}

// This implementation is used in a case if |mPlatform->postWorkerTask| is nullptr.
bool DelegateWorkerPool::ensureHasNextTaskToRun(std::unique_lock<std::mutex> *lock)
{
    return !isTaskQueueEmptyLocked();
}

bool DelegateWorkerPool::isAsync()
{
    return mPlatform->postWorkerTask != nullptr;
}

#endif  // ANGLE_DELEGATE_WORKERS

// static
std::shared_ptr<WorkerThreadPool> WorkerThreadPool::Create(size_t numThreads,
                                                           PlatformMethods *platform)
{
    const bool multithreaded = numThreads != 1;
    std::shared_ptr<WorkerThreadPool> pool(nullptr);

#if ANGLE_DELEGATE_WORKERS
    const bool hasPostWorkerTaskImpl = platform->postWorkerTask != nullptr;
    if (hasPostWorkerTaskImpl && multithreaded)
    {
        pool = std::shared_ptr<WorkerThreadPool>(new DelegateWorkerPool(platform));
    }
#endif
#if ANGLE_STD_ASYNC_WORKERS
    if (!pool && multithreaded)
    {
        pool = std::shared_ptr<WorkerThreadPool>(new AsyncWorkerPool(
            numThreads == 0 ? std::thread::hardware_concurrency() : numThreads));
    }
#endif
    if (!pool)
    {
        return std::shared_ptr<WorkerThreadPool>(new SingleThreadedWorkerPool());
    }
    return pool;
}
}  // namespace angle
