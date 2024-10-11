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
    return std::static_pointer_cast<WaitableEvent>(std::move(waitable));
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
    std::shared_ptr<WaitableEvent> postWorkerTask(const std::shared_ptr<WorkerTask> &task) override;

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

    std::shared_ptr<WaitableEvent> waitable = asyncTask;
    postAsyncTask(std::move(asyncTask));

    return waitable;
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

class WorkerThreadLoopPolicy
{
  public:
    virtual ~WorkerThreadLoopPolicy() = default;
    // Ensures that there is a next task and it can be run, or returns false.  Allowed to wait on
    // the conditional variable using the passed lock.  When returned false, the lock may be
    // unlocked after the call.
    virtual bool ensureHasNextTaskToRunLocked(std::unique_lock<std::mutex> *lock) = 0;
};

// Base for the |AsyncWorkerPool| and the |DelegateWorkerPool| classes.
class MultiThreadedWorkerPool : public AsyncTaskWorkerPool
{
  public:
    MultiThreadedWorkerPool() = default;
    ~MultiThreadedWorkerPool() override;

    // AsyncTaskWorkerPool
    void postAsyncTask(std::shared_ptr<AsyncTask> &&task) override;

  protected:
    // Runs tasks from the |mTaskQueue| until |policy->ensureHasNextTaskToRunLocked| returns false.
    // Designed to be called from multiple threads.
    void runThreadLoop(WorkerThreadLoopPolicy *policy);

    bool isTaskQueueEmptyLocked() const { return mTaskQueue.empty(); }
    std::shared_ptr<AsyncTask> &getNextTaskLocked();

    void incrementFreeThreadCount(size_t n);
    void decrementFreeThreadCount(size_t n);
    void startThreadIfNeededLocked(std::unique_lock<std::mutex> *lock);

    // Creates new or wakes existing thread.  The lock may be unlocked after the call.
    // When creating a new thread the |mFreeThreadCount| MUST be incremented.
    virtual void startThreadLocked(std::unique_lock<std::mutex> *lock) = 0;

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
    std::unique_lock<std::mutex> lock(mMutex);

    mTaskQueue.emplace(std::move(task));

    startThreadIfNeededLocked(&lock);
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
            if (!policy->ensureHasNextTaskToRunLocked(&lock))
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

void MultiThreadedWorkerPool::startThreadIfNeededLocked(std::unique_lock<std::mutex> *lock)
{
    if (mTaskQueue.size() > mFreeThreadCount.load(std::memory_order_relaxed))
    {
        startThreadLocked(lock);
    }
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
    void startThreadLocked(std::unique_lock<std::mutex> *lock) override;

  private:
    // Thread's main loop
    void threadLoop();

    void incrementNotifiedThreadCountLocked(size_t n);
    void decrementNotifiedThreadCountLocked(size_t n);

    // WorkerThreadLoopPolicy
    bool ensureHasNextTaskToRunLocked(std::unique_lock<std::mutex> *lock) override;

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
        std::unique_lock<std::mutex> lock(mMutex);
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

void AsyncWorkerPool::startThreadLocked(std::unique_lock<std::mutex> *lock)
{
    // Try to wake a waiting thread.
    if (mWaitingThreadCount > mNotifiedThreadCount)
    {
        incrementNotifiedThreadCountLocked(1);
        lock->unlock();
        mCondVar.notify_one();
        return;
    }

    // Otherwise, try to create a new thread.
    if (mThreads.size() < mDesiredThreadCount)
    {
        incrementFreeThreadCount(1);
        mThreads.emplace_back(&AsyncWorkerPool::threadLoop, this);
        return;
    }

    // All threads are created and running, so nothing to do.
}

void AsyncWorkerPool::threadLoop()
{
    angle::SetCurrentThreadName("ANGLE-Worker");

    runThreadLoop(this);
}

bool AsyncWorkerPool::ensureHasNextTaskToRunLocked(std::unique_lock<std::mutex> *lock)
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
            // When terminated, all threads must be notified.
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
    void startThreadLocked(std::unique_lock<std::mutex> *lock) override;

  private:
    // WorkerThreadLoopPolicy
    bool ensureHasNextTaskToRunLocked(std::unique_lock<std::mutex> *lock) override;

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
        auto workerThread = static_cast<DelegateWorkerThread *>(userData);
        workerThread->mWorkerPool->runThreadLoop(workerThread);

        // Delete the worker after its execution.
        delete workerThread;
    }

  private:
    ~DelegateWorkerThread() override = default;

    // WorkerThreadLoopPolicy
    bool ensureHasNextTaskToRunLocked(std::unique_lock<std::mutex> *lock) override
    {
        // Only run a first task.
        if (!mWorkerPool->isTaskQueueEmptyLocked() && mIsFirstRun)
        {
            mIsFirstRun = false;
            return true;
        }
        // |MultiThreadedWorkerPool::postAsyncTask| may skip starting a thread in hope that exiting
        // free thread will run the task.  However in reality, free thread may stop running tasks
        // because of the above condition, casing lack of free threads.  To account for this
        // problem, call |startThreadIfNeededLocked| below.
        mWorkerPool->startThreadIfNeededLocked(lock);
        return false;
    }

    std::shared_ptr<DelegateWorkerPool> mWorkerPool;
    bool mIsFirstRun = true;
};

// DelegateWorkerPool implementation.
DelegateWorkerPool::~DelegateWorkerPool()
{
    // No lock because all threads must not be running at this point.
    ASSERT(isTaskQueueEmptyLocked());
}

ANGLE_NO_SANITIZE_CFI_ICALL
void DelegateWorkerPool::startThreadLocked(std::unique_lock<std::mutex> *lock)
{
    incrementFreeThreadCount(1);
    lock->unlock();

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
    auto workerThread =
        new DelegateWorkerThread(std::static_pointer_cast<DelegateWorkerPool>(shared_from_this()));
    mPlatform->postWorkerTask(mPlatform, DelegateWorkerThread::Run, workerThread);
}

// This implementation is used in a case if |mPlatform->postWorkerTask| is nullptr.
bool DelegateWorkerPool::ensureHasNextTaskToRunLocked(std::unique_lock<std::mutex> *lock)
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
