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

#include "common/FixedVector.h"
#include "common/angleutils.h"
#include "common/system_utils.h"

// Controls if our threading code uses std::async or falls back to single-threaded operations.
// Note that we can't easily use std::async in UWPs due to UWP threading restrictions.
#if !defined(ANGLE_STD_ASYNC_WORKERS) && !defined(ANGLE_ENABLE_WINDOWS_UWP)
#    define ANGLE_STD_ASYNC_WORKERS 1
#endif  // !defined(ANGLE_STD_ASYNC_WORKERS) && & !defined(ANGLE_ENABLE_WINDOWS_UWP)

#if ANGLE_DELEGATE_WORKERS || ANGLE_STD_ASYNC_WORKERS
#    include <thread>
#endif  // ANGLE_DELEGATE_WORKERS || ANGLE_STD_ASYNC_WORKERS

namespace angle
{
class AsyncTaskWorkerPool
{
  public:
    virtual ~AsyncTaskWorkerPool()                                = default;
    virtual void postAsyncTask(std::shared_ptr<AsyncTask> &&task) = 0;
};

constexpr size_t kMaxAsyncTaskKeySize = 8;
using AsyncTaskKey                    = FixedVector<uint64_t, kMaxAsyncTaskKeySize>;

class AsyncTask final : public AsyncWaitableEvent, public std::enable_shared_from_this<AsyncTask>
{
  public:
    explicit AsyncTask(const std::shared_ptr<WorkerTask> &workerTask);
    AsyncTask(const std::shared_ptr<WorkerTask> &workerTask,
              std::shared_ptr<AsyncTaskWorkerPool> &&workerPool,
              size_t dependencyCount);
    ~AsyncTask() override;

    void run();
    void markAsReady();
    void runAndMarkAsReady();

    const AsyncTaskKey &getKey() const
    {
        ASSERT(!mKey.empty());
        return mKey;
    }
    bool isSubTask() const { return mKey.size() > 1; }

    void onDependencyEventReady(uint64_t eventSerial, TaskDependencyHint hint);
    void onDependencyTaskReady(const AsyncTaskKey &key, TaskDependencyHint hint);

  protected:
    // AsyncWaitableEvent
    void notifyDependencyReady(AsyncTask *task, TaskDependencyHint hint) override;

  private:
    void handleDependencyReady(const AsyncTaskKey &key, TaskDependencyHint hint);

    AsyncTaskKey mKey;
    std::shared_ptr<WorkerTask> mWorkerTask;
    std::shared_ptr<AsyncTaskWorkerPool> mWorkerPool;
    size_t mDependencyCount;
};

namespace
{
std::shared_ptr<WaitableEvent> RunInline(const std::shared_ptr<WorkerTask> &task)
{
    auto waitable = std::make_shared<AsyncWaitableEvent>();
    (*task)(waitable.get());
    waitable->markAsReady();
    return waitable;
}

uint64_t GetNextEventSerial()
{
    static std::atomic<uint64_t> sSerialCounter{0};
    return (sSerialCounter.fetch_add(1, std::memory_order_relaxed) + 1);
}

std::shared_ptr<WaitableEvent> PostWorkerTask(AsyncTaskWorkerPool *workerPool,
                                              const std::shared_ptr<WorkerTask> &task)
{
    // Thread safety: This function is thread-safe because |postAsyncTask| is expected to be
    // thread-safe.
    auto asyncTask = std::make_shared<AsyncTask>(task);

    workerPool->postAsyncTask(std::shared_ptr<AsyncTask>(asyncTask));

    return asyncTask;
}

std::shared_ptr<WaitableEvent> PostWorkerTask(std::shared_ptr<AsyncTaskWorkerPool> &&workerPool,
                                              const std::shared_ptr<WorkerTask> &task,
                                              const TaskDependencies &dependencies)
{
    if (dependencies.empty())
    {
        return PostWorkerTask(workerPool.get(), task);
    }

    // Thread safety: This function is thread-safe because |addDependentTask| is expected to be
    // thread-safe.
    auto asyncTask = std::make_shared<AsyncTask>(task, std::move(workerPool), dependencies.size());

    // Task will call |postAsyncTask| once all dependency events are ready.
    for (const TaskDependency &dependency : dependencies)
    {
        ASSERT(dependency.event != nullptr);
        dependency.event->addDependentTask(asyncTask, dependency.hint);
    }

    return asyncTask;
}
}  // anonymous namespace

// WaitableEvent implementation.
WaitableEvent::WaitableEvent()  = default;
WaitableEvent::~WaitableEvent() = default;

// AsyncWaitableEvent implementation.
AsyncWaitableEvent::AsyncWaitableEvent() : mSerial(GetNextEventSerial()) {}

void AsyncWaitableEvent::addDependentTask(const std::shared_ptr<AsyncTask> &task,
                                          TaskDependencyHint hint)
{
    ASSERT(task);
    {
        std::lock_guard<std::mutex> lock(mMutex);
        if (!mIsReady)
        {
            // Add task inside the lock to prevent marking the event ready and therefore iterating
            // over the vector from other thread.
            mDependentTasks.emplace_back(DependentTask{task, hint});
            return;
        }
    }
    // If event is ready notify the task right away while not holding the lock.
    notifyDependencyReady(task.get(), hint);
}

void AsyncWaitableEvent::notifyDependencyReady(AsyncTask *task, TaskDependencyHint hint)
{
    task->onDependencyEventReady(mSerial, hint);
}

void AsyncWaitableEvent::markAsReady()
{
    ASSERT(!isReady());
    {
        std::lock_guard<std::mutex> lock(mMutex);
        // Using release ordering to provide synchronization to |isReady|'s acquire load, which does
        // not happen under the mutex.
        mIsReady.store(1, std::memory_order_release);
    }
    mCondition.notify_all();

    // Notify tasks without mutex lock, since no new tasks will be added when event is ready.
    // Also calling callbacks while holding the lock may potentially cause a deadlock.
    for (DependentTask &dependentTask : mDependentTasks)
    {
        notifyDependencyReady(dependentTask.task.get(), dependentTask.hint);
    }
    mDependentTasks.clear();
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
AsyncTask::AsyncTask(const std::shared_ptr<WorkerTask> &workerTask)
    : mWorkerTask(workerTask), mDependencyCount(0)
{
    ASSERT(mWorkerTask);
    // Initialize key now, since it will not change.
    mKey.emplace_back(mSerial);
}

AsyncTask::AsyncTask(const std::shared_ptr<WorkerTask> &workerTask,
                     std::shared_ptr<AsyncTaskWorkerPool> &&workerPool,
                     size_t dependencyCount)
    : mWorkerTask(workerTask), mWorkerPool(std::move(workerPool)), mDependencyCount(dependencyCount)
{
    ASSERT(mWorkerTask);
    ASSERT(mWorkerPool);
    ASSERT(mDependencyCount > 0);
    // Key will be calculated later based on dependency hints.
}

AsyncTask::~AsyncTask()
{
    ASSERT(!mWorkerTask || !isReady());
}

void AsyncTask::notifyDependencyReady(AsyncTask *task, TaskDependencyHint hint)
{
    task->onDependencyTaskReady(mKey, hint);
}

void AsyncTask::onDependencyEventReady(uint64_t eventSerial, TaskDependencyHint hint)
{
    // Convert event serial into the task key.
    AsyncTaskKey key = {eventSerial};
    handleDependencyReady(key, hint);
}

void AsyncTask::onDependencyTaskReady(const AsyncTaskKey &key, TaskDependencyHint hint)
{
    handleDependencyReady(key, hint);
}

void AsyncTask::handleDependencyReady(const AsyncTaskKey &key, TaskDependencyHint hint)
{
    ASSERT(!key.empty());
    bool isAllDependenciesReady = false;
    {
        std::lock_guard<std::mutex> lock(mMutex);

        ASSERT(mDependencyCount > 0);
        --mDependencyCount;
        isAllDependenciesReady = (mDependencyCount == 0);

        // Treat SubTask dependency as Normal if key is already at maximum size.
        if (hint == TaskDependencyHint::SubTask && key.size() < kMaxAsyncTaskKeySize)
        {
            if (mKey.empty())
            {
                mKey = key;
                mKey.emplace_back(mSerial);
            }
            else
            {
                AsyncTaskKey newKey = key;
                newKey.emplace_back(mSerial);
                if (newKey < mKey)
                {
                    mKey = newKey;
                }
            }
        }
        else if (mKey.empty())
        {
            if (hint != TaskDependencyHint::Deferred)
            {
                mKey.emplace_back(mSerial);
            }
            else if (isAllDependenciesReady)
            {
                mKey.emplace_back(GetNextEventSerial());
            }
        }
    }
    if (isAllDependenciesReady)
    {
        // Only single thread may be there.  Do not hold the lock to avoid potential problems when
        // calling |postAsyncTask| below and to reduce the lock scope.
        ASSERT(mWorkerPool);
        // |mWorkerPool| must be rest prior to posting a task, since it may be inlined.
        auto workerPool = std::move(mWorkerPool);
        workerPool->postAsyncTask(shared_from_this());
    }
}

void AsyncTask::run()
{
    ASSERT(mDependencyCount == 0);
    ASSERT(!mWorkerPool);
    ASSERT(mWorkerTask);

    // Note: always add an ANGLE_TRACE_EVENT* macro in the worker task.  Then the job will show up
    // in traces.
    (*mWorkerTask)(this);

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

class SingleThreadedWorkerPool final : public WorkerThreadPool,
                                       public AsyncTaskWorkerPool,
                                       public std::enable_shared_from_this<AsyncTaskWorkerPool>
{
  public:
    // WorkerThreadPool
    std::shared_ptr<WaitableEvent> postWorkerTask(const std::shared_ptr<WorkerTask> &task) override;
    std::shared_ptr<WaitableEvent> postWorkerTask(const std::shared_ptr<WorkerTask> &task,
                                                  const TaskDependencies &dependencies) override;
    bool isAsync() override;

    // AsyncTaskWorkerPool
    void postAsyncTask(std::shared_ptr<AsyncTask> &&task) override;
};

// SingleThreadedWorkerPool implementation.
std::shared_ptr<WaitableEvent> SingleThreadedWorkerPool::postWorkerTask(
    const std::shared_ptr<WorkerTask> &task)
{
    // Thread safety: This function is thread-safe because the task is run on the calling thread
    // itself.
    return RunInline(task);
}

std::shared_ptr<WaitableEvent> SingleThreadedWorkerPool::postWorkerTask(
    const std::shared_ptr<WorkerTask> &task,
    const TaskDependencies &dependencies)
{
    return PostWorkerTask(shared_from_this(), task, dependencies);
}

bool SingleThreadedWorkerPool::isAsync()
{
    return false;
}

void SingleThreadedWorkerPool::postAsyncTask(std::shared_ptr<AsyncTask> &&task)
{
    // Thread safety: This function is thread-safe because the task is run on the calling thread
    // itself.
    task->runAndMarkAsReady();
}

#if ANGLE_DELEGATE_WORKERS || ANGLE_STD_ASYNC_WORKERS

// Note: "Locked" suffix means, that the function must be called under the mutex lock.

class MultiThreadedTaskQueue final : angle::NonCopyable
{
  public:
    MultiThreadedTaskQueue() = default;
    ~MultiThreadedTaskQueue();

    // Inserts task into the queue.
    // |StartThreadLocked| must be a callable compatible with the following signature:
    //     void(std::unique_lock<std::mutex> lock)
    // It creates new or wakes existing thread.  Before creating a new thread the |mFreeThreadCount|
    // MUST be incremented by calling |incrementFreeThreadCount(1)|.
    template <class StartThreadLocked>
    void insertTask(std::shared_ptr<AsyncTask> &&task, StartThreadLocked &&startThreadLocked);

    // Runs tasks from the |mTaskQueue| until |ensureHasNextTaskToRunLocked| returns false.
    // Designed to be called from multiple threads.
    // |EnsureHasNextTaskToRunLocked| must be a callable compatible with the following signature:
    //     std::pair<bool, std::unique_lock<std::mutex>>(std::unique_lock<std::mutex> lock)
    // It ensures that there is a next task and it can be run, in which case |result.first| will be
    // true and |result.second| will be a locked |lock| argument.  Otherwise, |result.first| will be
    // false and |result.second| may be anything.  Allowed to wait on the conditional variable using
    // the passed lock.
    template <class EnsureHasNextTaskToRunLocked>
    void runTasks(EnsureHasNextTaskToRunLocked &&ensureHasNextTaskToRunLocked);

    std::unique_lock<std::mutex> getMutexLock() { return std::unique_lock<std::mutex>(mMutex); }
    bool isTaskQueueEmptyLocked() const { return mTaskQueue.empty(); }
    std::shared_ptr<AsyncTask> &getNextTaskLocked();

    void incrementFreeThreadCount(size_t n);
    void decrementFreeThreadCount(size_t n);
    template <class StartThreadLocked>
    void startThreadIfNeededLocked(std::unique_lock<std::mutex> lock,
                                   StartThreadLocked &&startThreadLocked);

  private:
    struct SharedAsyncTaskGreater
    {
        bool operator()(const std::shared_ptr<AsyncTask> &a,
                        const std::shared_ptr<AsyncTask> &b) const
        {
            return b->getKey() < a->getKey();  // Using operator less in reverse.
        }
    };

    std::mutex mMutex;  // Protects access to the fields in this class
    std::vector<std::shared_ptr<AsyncTask>> mTaskQueue;
    std::atomic<size_t> mFreeThreadCount{0};
};

// MultiThreadedTaskQueue implementation.
MultiThreadedTaskQueue::~MultiThreadedTaskQueue()
{
    ASSERT(mFreeThreadCount == 0);
}

template <class StartThreadLocked>
void MultiThreadedTaskQueue::insertTask(std::shared_ptr<AsyncTask> &&task,
                                        StartThreadLocked &&startThreadLocked)
{
    // Thread safety: This function is thread-safe because member access is protected by |mMutex|.
    std::unique_lock<std::mutex> lock(mMutex);

    mTaskQueue.emplace_back(std::move(task));
    std::push_heap(mTaskQueue.begin(), mTaskQueue.end(), SharedAsyncTaskGreater{});

    startThreadIfNeededLocked(std::move(lock), std::forward<StartThreadLocked>(startThreadLocked));
}

template <class EnsureHasNextTaskToRunLocked>
void MultiThreadedTaskQueue::runTasks(EnsureHasNextTaskToRunLocked &&ensureHasNextTaskToRunLocked)
{
    while (true)
    {
        std::shared_ptr<AsyncTask> task;
        {
            std::unique_lock<std::mutex> tempLock(mMutex);
            // Mark this thread as busy running a task, waiting for a task, or terminated.
            decrementFreeThreadCount(1);
            const auto [hasNextTaskToRun, lock] = ensureHasNextTaskToRunLocked(std::move(tempLock));
            if (!hasNextTaskToRun)
            {
                return;
            }
            // |pop_heap| doesn't access first element's value, so we can move it to the |task|.
            task = std::move(getNextTaskLocked());
            std::pop_heap(mTaskQueue.begin(), mTaskQueue.end(), SharedAsyncTaskGreater{});
            mTaskQueue.pop_back();
        }

        task->run();

        // Mark this thread as free before marking task as ready.
        incrementFreeThreadCount(1);

        // This may trigger |postAsyncTask| call in case of a dependency.  Because of the added
        // free thread above, we can avoid starting a thread unnecessarily.
        task->markAsReady();
    }
}

std::shared_ptr<AsyncTask> &MultiThreadedTaskQueue::getNextTaskLocked()
{
    ASSERT(!isTaskQueueEmptyLocked());
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

template <class StartThreadLocked>
void MultiThreadedTaskQueue::startThreadIfNeededLocked(std::unique_lock<std::mutex> lock,
                                                       StartThreadLocked &&startThreadLocked)
{
    if (mTaskQueue.size() > mFreeThreadCount.load(std::memory_order_relaxed))
    {
        startThreadLocked(std::move(lock));
    }
}

#endif  // ANGLE_DELEGATE_WORKERS || ANGLE_STD_ASYNC_WORKERS

#if ANGLE_STD_ASYNC_WORKERS

class AsyncWorkerPool final : public WorkerThreadPool,
                              public AsyncTaskWorkerPool,
                              public std::enable_shared_from_this<AsyncTaskWorkerPool>
{
  public:
    explicit AsyncWorkerPool(size_t numThreads);
    ~AsyncWorkerPool() override;

    // WorkerThreadPool
    std::shared_ptr<WaitableEvent> postWorkerTask(const std::shared_ptr<WorkerTask> &task) override;
    std::shared_ptr<WaitableEvent> postWorkerTask(const std::shared_ptr<WorkerTask> &task,
                                                  const TaskDependencies &dependencies) override;
    bool isAsync() override;

    // AsyncTaskWorkerPool
    void postAsyncTask(std::shared_ptr<AsyncTask> &&task) override;

  private:
    // Thread's main loop
    void threadLoop();

    void incrementNotifiedThreadCountLocked(size_t n);
    void decrementNotifiedThreadCountLocked(size_t n);

    MultiThreadedTaskQueue mTaskQueue;
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
        std::unique_lock<std::mutex> lock = mTaskQueue.getMutexLock();
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

std::shared_ptr<WaitableEvent> AsyncWorkerPool::postWorkerTask(
    const std::shared_ptr<WorkerTask> &task)
{
    return PostWorkerTask(this, task);
}

std::shared_ptr<WaitableEvent> AsyncWorkerPool::postWorkerTask(
    const std::shared_ptr<WorkerTask> &task,
    const TaskDependencies &dependencies)
{
    return PostWorkerTask(shared_from_this(), task, dependencies);
}

void AsyncWorkerPool::postAsyncTask(std::shared_ptr<AsyncTask> &&task)
{
    mTaskQueue.insertTask(std::move(task), [this](std::unique_lock<std::mutex> &&lock) {
        // Try to wake a waiting thread.
        if (mWaitingThreadCount > mNotifiedThreadCount)
        {
            incrementNotifiedThreadCountLocked(1);
            lock.unlock();
            mCondVar.notify_one();
            return;
        }

        // Otherwise, try to create a new thread.
        if (mThreads.size() < mDesiredThreadCount)
        {
            mTaskQueue.incrementFreeThreadCount(1);
            mThreads.emplace_back(&AsyncWorkerPool::threadLoop, this);
            return;
        }

        // All threads are created and running, so nothing to do.
    });
}

void AsyncWorkerPool::threadLoop()
{
    angle::SetCurrentThreadName("ANGLE-Worker");

    mTaskQueue.runTasks([this](std::unique_lock<std::mutex> &&lock) {
        bool isWaiting = false;
        mCondVar.wait(lock, [this, &isWaiting] {
            const bool isConditionMet = (!mTaskQueue.isTaskQueueEmptyLocked() || mTerminated);
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
        return std::make_pair(!mTerminated, std::move(lock));
    });
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

class DelegateWorkerPool final : public WorkerThreadPool,
                                 public AsyncTaskWorkerPool,
                                 public std::enable_shared_from_this<AsyncTaskWorkerPool>
{
  public:
    explicit DelegateWorkerPool(PlatformMethods *platform) : mPlatform(platform) {}
    ~DelegateWorkerPool() override;

    // WorkerThreadPool
    std::shared_ptr<WaitableEvent> postWorkerTask(const std::shared_ptr<WorkerTask> &task) override;
    std::shared_ptr<WaitableEvent> postWorkerTask(const std::shared_ptr<WorkerTask> &task,
                                                  const TaskDependencies &dependencies) override;
    bool isAsync() override;

    // AsyncTaskWorkerPool
    void postAsyncTask(std::shared_ptr<AsyncTask> &&task) override;

  private:
    void startThreadLocked(std::unique_lock<std::mutex> &&lock);

    PlatformMethods *mPlatform;
    MultiThreadedTaskQueue mTaskQueue;

    friend class DelegateWorkerThread;
};

// Represents a delegate worker thread that will call the |runTasks| method.
class DelegateWorkerThread final : angle::NonCopyable
{
  public:
    DelegateWorkerThread(std::shared_ptr<DelegateWorkerPool> &&workerPool)
        : mWorkerPool(std::move(workerPool))
    {}

    static void Run(void *userData)
    {
        auto workerThread = static_cast<DelegateWorkerThread *>(userData);
        workerThread->mWorkerPool->mTaskQueue.runTasks(
            [workerThread](std::unique_lock<std::mutex> &&lock) {
                return workerThread->ensureHasNextTaskToRunLocked(std::move(lock));
            });

        // Delete the worker after its execution.
        delete workerThread;
    }

  private:
    std::pair<bool, std::unique_lock<std::mutex>> ensureHasNextTaskToRunLocked(
        std::unique_lock<std::mutex> &&lock)
    {
        auto taskQueue = &mWorkerPool->mTaskQueue;
        // Always run a first task or if it is a sub-task.
        if (!taskQueue->isTaskQueueEmptyLocked() &&
            (mIsFirstRun || taskQueue->getNextTaskLocked()->isSubTask()))
        {
            mIsFirstRun = false;
            return {true, std::move(lock)};
        }
        // |MultiThreadedWorkerPool::postAsyncTask| may skip starting a thread in hope that exiting
        // free thread will run the task.  However in reality, free thread may stop running tasks
        // because of the above condition, casing lack of free threads.  To account for this
        // problem, call |startThreadIfNeededLocked| below.
        taskQueue->startThreadIfNeededLocked(std::move(lock),
                                             [this](std::unique_lock<std::mutex> &&lock) {
                                                 mWorkerPool->startThreadLocked(std::move(lock));
                                             });
        return {false, std::unique_lock<std::mutex>()};
    }

    std::shared_ptr<DelegateWorkerPool> mWorkerPool;
    bool mIsFirstRun = true;
};

// DelegateWorkerPool implementation.
DelegateWorkerPool::~DelegateWorkerPool()
{
    // No lock because all threads must not be running at this point.
    ASSERT(mTaskQueue.isTaskQueueEmptyLocked());
}

std::shared_ptr<WaitableEvent> DelegateWorkerPool::postWorkerTask(
    const std::shared_ptr<WorkerTask> &task)
{
    return PostWorkerTask(this, task);
}

std::shared_ptr<WaitableEvent> DelegateWorkerPool::postWorkerTask(
    const std::shared_ptr<WorkerTask> &task,
    const TaskDependencies &dependencies)
{
    return PostWorkerTask(shared_from_this(), task, dependencies);
}

void DelegateWorkerPool::postAsyncTask(std::shared_ptr<AsyncTask> &&task)
{
    mTaskQueue.insertTask(std::move(task), [this](std::unique_lock<std::mutex> &&lock) {
        startThreadLocked(std::move(lock));
    });
}

ANGLE_NO_SANITIZE_CFI_ICALL
void DelegateWorkerPool::startThreadLocked(std::unique_lock<std::mutex> &&lock)
{
    mTaskQueue.incrementFreeThreadCount(1);
    lock.unlock();

    if (mPlatform->postWorkerTask == nullptr)
    {
        // In the unexpected case where the platform methods have been changed during execution and
        // postWorkerTask is no longer usable, simply run all tasks on the calling thread.
        return mTaskQueue.runTasks([this](std::unique_lock<std::mutex> &&lock) {
            return std::make_pair(!mTaskQueue.isTaskQueueEmptyLocked(), std::move(lock));
        });
    }

    // Thread safety: This function is thread-safe because the |postWorkerTask| platform method is
    // expected to be thread safe.  For Chromium, that forwards the call to the |TaskTracker| class
    // in base/task/thread_pool/task_tracker.h which is thread-safe.

    // The worker will be deleted by DelegateWorkerThread::Run(...) after its execution.
    auto workerThread =
        new DelegateWorkerThread(static_pointer_cast<DelegateWorkerPool>(shared_from_this()));
    mPlatform->postWorkerTask(mPlatform, DelegateWorkerThread::Run, workerThread);
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
