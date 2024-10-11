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

#if ANGLE_STD_ASYNC_WORKERS

// Note: "Locked" suffix means, that the function must be called under the mutex lock and will end
// while the mutex is still locked.  Mutex may be temporarily unlocked inside the function.

class MultiThreadedTaskQueue final : angle::NonCopyable
{
  public:
    MultiThreadedTaskQueue() = default;
    ~MultiThreadedTaskQueue();

    // Inserts task into the queue.
    // |StartThreadLocked| must be a "bool()" callable, while |StartThreadUnlocked| - "void()".
    // Both functions create new or wake existing thread.  If |startThreadLocked| return true, then
    // operation is complete.  Otherwise, |startThreadUnlocked| is called to complete the job.
    // Before creating a new thread the |mFreeThreadCount| MUST be incremented by calling
    // |incrementFreeThreadCount(1)| method.
    template <class StartThreadLocked, class StartThreadUnlocked>
    void insertTask(std::shared_ptr<AsyncTask> &&task,
                    StartThreadLocked &&startThreadLocked,
                    StartThreadUnlocked &&startThreadUnlocked);

    // Runs tasks from the |mTaskQueue| until |ensureHasNextTaskToRunLocked| returns false.
    // Designed to be called from multiple threads.
    // |EnsureHasNextTaskToRunLocked| must be a "bool(std::unique_lock<std::mutex> *)" callable.
    // It ensures that there is a next task and it can be run, or returns false.  Allowed to wait on
    // the conditional variable using the passed |lock|.  The |lock| MUST be in a locked state after
    // |ensureHasNextTaskToRunLocked| call ends.
    template <class EnsureHasNextTaskToRunLocked>
    void runTasks(EnsureHasNextTaskToRunLocked &&ensureHasNextTaskToRunLocked);

    std::unique_lock<std::mutex> lockMutex() { return std::unique_lock<std::mutex>(mMutex); }
    bool isEmptyLocked() const { return mTaskQueue.empty(); }
    std::shared_ptr<AsyncTask> &getNextTaskLocked();

    void incrementFreeThreadCount(size_t n);
    void decrementFreeThreadCount(size_t n);
    bool isNeedToStartThreadLocked() const;

  private:
    std::mutex mMutex;  // Protects access to the fields in this class
    std::queue<std::shared_ptr<AsyncTask>> mTaskQueue;
    std::atomic<size_t> mFreeThreadCount{0};
};

// MultiThreadedTaskQueue implementation.
MultiThreadedTaskQueue::~MultiThreadedTaskQueue()
{
    ASSERT(mFreeThreadCount == 0);
}

template <class StartThreadLocked, class StartThreadUnlocked>
void MultiThreadedTaskQueue::insertTask(std::shared_ptr<AsyncTask> &&task,
                                        StartThreadLocked &&startThreadLocked,
                                        StartThreadUnlocked &&startThreadUnlocked)
{
    // Thread safety: This function is thread-safe because member access is protected by |mMutex|.
    bool needToStartThreadUnlocked = false;
    {
        std::lock_guard<std::mutex> lock(mMutex);

        mTaskQueue.emplace(std::move(task));

        needToStartThreadUnlocked = isNeedToStartThreadLocked() && !startThreadLocked();
    }
    if (needToStartThreadUnlocked)
    {
        startThreadUnlocked();
    }
}

template <class EnsureHasNextTaskToRunLocked>
void MultiThreadedTaskQueue::runTasks(EnsureHasNextTaskToRunLocked &&ensureHasNextTaskToRunLocked)
{
    while (true)
    {
        std::shared_ptr<AsyncTask> task;
        {
            std::unique_lock<std::mutex> lock(mMutex);
            // Mark this thread as busy running a task, waiting for a task, or terminated.
            decrementFreeThreadCount(1);
            if (!ensureHasNextTaskToRunLocked(&lock))
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

std::shared_ptr<AsyncTask> &MultiThreadedTaskQueue::getNextTaskLocked()
{
    ASSERT(!isEmptyLocked());
    return mTaskQueue.front();
}

void MultiThreadedTaskQueue::incrementFreeThreadCount(size_t n)
{
    mFreeThreadCount.fetch_add(n, std::memory_order_relaxed);
}

void MultiThreadedTaskQueue::decrementFreeThreadCount(size_t n)
{
    const size_t prevCount = mFreeThreadCount.fetch_sub(n, std::memory_order_relaxed);
    ASSERT(prevCount >= n);
}

bool MultiThreadedTaskQueue::isNeedToStartThreadLocked() const
{
    return (mTaskQueue.size() > mFreeThreadCount.load(std::memory_order_relaxed));
}

class AsyncWorkerPool final : public WorkerThreadPool
{
  public:
    explicit AsyncWorkerPool(size_t numThreads);
    ~AsyncWorkerPool() override;

    // WorkerThreadPool
    std::shared_ptr<WaitableEvent> postWorkerTask(const std::shared_ptr<WorkerTask> &task) override;
    bool isAsync() override;

  private:
    void postAsyncTask(std::shared_ptr<AsyncTask> &&task);

    bool startThreadLocked();
    void startThreadUnlocked();

    // Thread's main loop
    void threadLoop();

    bool ensureHasNextTaskToRunLocked(std::unique_lock<std::mutex> *lock);

    void incrementNotifiedThreadCountLocked(size_t n);
    void decrementNotifiedThreadCountLocked(size_t n);

    bool mTerminated = false;
    std::condition_variable mCondVar;  // Signals when work is available in the queue
    MultiThreadedTaskQueue mTaskQueue;
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
        auto lock = mTaskQueue.lockMutex();
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

bool AsyncWorkerPool::startThreadLocked()
{
    // Try to wake a waiting thread.
    if (mWaitingThreadCount > mNotifiedThreadCount)
    {
        incrementNotifiedThreadCountLocked(1);
        // Returning false will call |startThreadUnlocked| below.
        return false;
    }

    // Otherwise, try to create a new thread.
    if (mThreads.size() < mDesiredThreadCount)
    {
        mTaskQueue.incrementFreeThreadCount(1);
        mThreads.emplace_back(&AsyncWorkerPool::threadLoop, this);
        return true;
    }

    // All threads are created and running, so nothing to do.
    return true;
}

void AsyncWorkerPool::startThreadUnlocked()
{
    mCondVar.notify_one();
}

std::shared_ptr<WaitableEvent> AsyncWorkerPool::postWorkerTask(
    const std::shared_ptr<WorkerTask> &task)
{
    // Thread safety: This function is thread-safe because |postAsyncTask| is thread-safe.
    auto asyncTask = std::make_shared<AsyncTask>(task);

    postAsyncTask(std::shared_ptr<AsyncTask>(asyncTask));

    return asyncTask;
}

void AsyncWorkerPool::postAsyncTask(std::shared_ptr<AsyncTask> &&task)
{
    mTaskQueue.insertTask(
        std::move(task), [this]() { return startThreadLocked(); },
        [this]() { startThreadUnlocked(); });
}

void AsyncWorkerPool::threadLoop()
{
    angle::SetCurrentThreadName("ANGLE-Worker");

    mTaskQueue.runTasks([this](auto &&lock) { return ensureHasNextTaskToRunLocked(lock); });
}

bool AsyncWorkerPool::ensureHasNextTaskToRunLocked(std::unique_lock<std::mutex> *lock)
{
    bool isWaiting = false;
    mCondVar.wait(*lock, [this, &isWaiting] {
        const bool isConditionMet = (!mTaskQueue.isEmptyLocked() || mTerminated);
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
    mTaskQueue.incrementFreeThreadCount(n);
    mNotifiedThreadCount += n;
}

void AsyncWorkerPool::decrementNotifiedThreadCountLocked(size_t n)
{
    ASSERT(mNotifiedThreadCount >= n);
    // Thread was marked as free when was notified.
    mTaskQueue.decrementFreeThreadCount(n);
    mNotifiedThreadCount -= n;
}

bool AsyncWorkerPool::isAsync()
{
    return true;
}

#endif  // ANGLE_STD_ASYNC_WORKERS

#if ANGLE_DELEGATE_WORKERS

class DelegateWorkerPool final : public WorkerThreadPool
{
  public:
    explicit DelegateWorkerPool(PlatformMethods *platform) : mPlatform(platform) {}
    ~DelegateWorkerPool() override = default;

    // WorkerThreadPool
    std::shared_ptr<WaitableEvent> postWorkerTask(const std::shared_ptr<WorkerTask> &task) override;
    bool isAsync() override;

  private:
    PlatformMethods *mPlatform;
};

// A function wrapper to execute the worker task and to notify the waitable
// event after the execution.
class DelegateWorkerTask
{
  public:
    DelegateWorkerTask(const std::shared_ptr<WorkerTask> &task,
                       std::shared_ptr<AsyncWaitableEvent> waitable)
        : mTask(task), mWaitable(waitable)
    {}
    DelegateWorkerTask()                     = delete;
    DelegateWorkerTask(DelegateWorkerTask &) = delete;

    static void RunTask(void *userData)
    {
        DelegateWorkerTask *workerTask = static_cast<DelegateWorkerTask *>(userData);
        (*workerTask->mTask)();
        workerTask->mWaitable->markAsReady();

        // Delete the task after its execution.
        delete workerTask;
    }

  private:
    ~DelegateWorkerTask() = default;

    std::shared_ptr<WorkerTask> mTask;
    std::shared_ptr<AsyncWaitableEvent> mWaitable;
};

// DelegateWorkerPool implementation.
ANGLE_NO_SANITIZE_CFI_ICALL
std::shared_ptr<WaitableEvent> DelegateWorkerPool::postWorkerTask(
    const std::shared_ptr<WorkerTask> &task)
{
    if (mPlatform->postWorkerTask == nullptr)
    {
        // In the unexpected case where the platform methods have been changed during execution and
        // postWorkerTask is no longer usable, simply run the task on the calling thread.
        return RunInline(task);
    }

    // Thread safety: This function is thread-safe because the |postWorkerTask| platform method is
    // expected to be thread safe.  For Chromium, that forwards the call to the |TaskTracker| class
    // in base/task/thread_pool/task_tracker.h which is thread-safe.
    auto waitable = std::make_shared<AsyncWaitableEvent>();

    // The task will be deleted by DelegateWorkerTask::RunTask(...) after its execution.
    DelegateWorkerTask *workerTask = new DelegateWorkerTask(task, waitable);
    mPlatform->postWorkerTask(mPlatform, DelegateWorkerTask::RunTask, workerTask);

    return waitable;
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
