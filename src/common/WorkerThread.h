//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WorkerThread:
//   Asynchronous tasks/threads for ANGLE, similar to a TaskRunner in Chromium.
//   Can be implemented as different targets, depending on platform.
//

#ifndef COMMON_WORKER_THREAD_H_
#define COMMON_WORKER_THREAD_H_

#include <array>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

#include "common/FastVector.h"
#include "common/debug.h"
#include "platform/PlatformMethods.h"

namespace angle
{

class WaitableEvent;
class AsyncTask;

// A callback function representing a |WorkerThreadPool| task.
class WorkerTask
{
  public:
    virtual ~WorkerTask() = default;
    // |event| may be used to post a sub-task with the |SubTask| hint dependency from within the
    // parent task, which will cause execution of the sub-task right away, possibly even from the
    // same thread.  Using dependency may also avoid waking new worker thread unnecessarily.
    virtual void operator()(WaitableEvent *event) = 0;
};

// Affects how dependent task will be scheduled when all dependency events are ready.
enum class TaskDependencyHint
{
    // Dependent task is a subtask of the dependency.  Execution will be scheduled right after the
    // dependency task (regardless when the task was posted into the worker pool).  If there are
    // multiple dependencies with this hint, the one with earlier scheduling time takes precedence.
    SubTask,
    // Task will be scheduled according to the posting order into the worker pool.  Ignored if
    // there is at least one |SubTask| dependency.
    Normal,
    // Task will be scheduled after all other existing tasks in the worker pool (as if it is posted
    // right after all dependency events are ready).  Ignored if there is at least one |SubTask| or
    // |Normal| dependency.
    Deferred,
};

struct TaskDependency final
{
    WaitableEvent *event;
    TaskDependencyHint hint;
};

constexpr size_t kFastTaskDependencyCount = 4;
using TaskDependencies                    = FastVector<TaskDependency, kFastTaskDependencyCount>;

// An event that we can wait on, useful for joining worker threads.
class WaitableEvent : angle::NonCopyable
{
  public:
    WaitableEvent();
    virtual ~WaitableEvent();

    // Adds dependent task, that will be notified when this event becomes ready (or is ready).
    virtual void addDependentTask(const std::shared_ptr<AsyncTask> &task,
                                  TaskDependencyHint hint) = 0;

    // Waits indefinitely for the event to be signaled.
    virtual void wait() = 0;

    // Peeks whether the event is ready. If ready, wait() will not block.
    virtual bool isReady() = 0;

    template <class T>
    // Waits on multiple events. T should be some container of std::shared_ptr<WaitableEvent>.
    static void WaitMany(T *waitables)
    {
        for (auto &waitable : *waitables)
        {
            waitable->wait();
        }
    }

    template <class T>
    // Checks if all events are ready. T should be some container of std::shared_ptr<WaitableEvent>.
    static bool AllReady(T *waitables)
    {
        for (auto &waitable : *waitables)
        {
            if (!waitable->isReady())
            {
                return false;
            }
        }
        return true;
    }
};

// A waitable event that can be completed asynchronously and is a base class for the |AsyncTask|.
class AsyncWaitableEvent : public WaitableEvent
{
  public:
    AsyncWaitableEvent();
    ~AsyncWaitableEvent() override = default;

    void addDependentTask(const std::shared_ptr<AsyncTask> &task, TaskDependencyHint hint) override;

    void wait() override;
    bool isReady() override;

    void markAsReady();

  protected:
    virtual void notifyDependencyReady(AsyncTask *task, TaskDependencyHint hint);

    const uint64_t mSerial;
    // To protect the concurrent accesses from both main thread and background
    // threads to the member fields.
    std::mutex mMutex;

  private:
    struct DependentTask
    {
        std::shared_ptr<AsyncTask> task;
        TaskDependencyHint hint;
    };

    std::atomic_int mIsReady{0};
    std::condition_variable mCondition;
    std::vector<DependentTask> mDependentTasks;
};

// Request WorkerThreads from the WorkerThreadPool. Each pool can keep worker threads around so
// we avoid the costly spin up and spin down time.
class WorkerThreadPool : angle::NonCopyable
{
  public:
    WorkerThreadPool();
    virtual ~WorkerThreadPool();

    // Creates a new thread pool.
    // If numThreads is 0, the pool will choose the best number of threads to run.
    // If numThreads is 1, the pool will be single-threaded. Tasks will run on the calling thread.
    // Other numbers indicate how many threads the pool should spawn.
    // Note that based on build options, this class may not actually run tasks in threads, or it may
    // hook into the provided PlatformMethods::postWorkerTask, in which case numThreads is ignored.
    static std::shared_ptr<WorkerThreadPool> Create(size_t numThreads, PlatformMethods *platform);

    // Returns an event to wait on for the task to finish.  If the pool fails to create the task,
    // returns null.  This function is thread-safe.
    virtual std::shared_ptr<WaitableEvent> postWorkerTask(
        const std::shared_ptr<WorkerTask> &task) = 0;

    // Similar to the regular function plus supports task dependencies.
    // Note: in case of a single-threaded pool, execution of the task will be deferred until all
    // dependency events are ready, and performed on the thread where the last event was made ready.
    virtual std::shared_ptr<WaitableEvent> postWorkerTask(const std::shared_ptr<WorkerTask> &task,
                                                          const TaskDependencies &dependencies) = 0;

    virtual bool isAsync() = 0;

  private:
};

}  // namespace angle

#endif  // COMMON_WORKER_THREAD_H_
