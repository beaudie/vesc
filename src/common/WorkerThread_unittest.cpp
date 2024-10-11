//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WorkerThread_unittest:
//   Simple tests for the worker thread class.

#include <gtest/gtest.h>
#include <array>

#include "common/WorkerThread.h"
#include "common/system_utils.h"

using namespace angle;

namespace
{

// Tests simple worker pool application.
TEST(WorkerPoolTest, SimpleTask)
{
    class TestTask : public WorkerTask
    {
      public:
        void operator()(WaitableEvent *event) override { fired = true; }

        bool fired = false;
    };

    std::array<std::shared_ptr<WorkerThreadPool>, 2> pools = {
        {WorkerThreadPool::Create(1, ANGLEPlatformCurrent()),
         WorkerThreadPool::Create(0, ANGLEPlatformCurrent())}};
    for (auto &pool : pools)
    {
        std::array<std::shared_ptr<TestTask>, 4> tasks = {
            {std::make_shared<TestTask>(), std::make_shared<TestTask>(),
             std::make_shared<TestTask>(), std::make_shared<TestTask>()}};
        std::array<std::shared_ptr<WaitableEvent>, 4> waitables = {
            {pool->postWorkerTask(tasks[0]), pool->postWorkerTask(tasks[1]),
             pool->postWorkerTask(tasks[2]), pool->postWorkerTask(tasks[3])}};

        WaitableEvent::WaitMany(&waitables);

        for (const auto &task : tasks)
        {
            EXPECT_TRUE(task->fired);
        }
    }
}

void RunDependencyChainTest(const std::vector<std::shared_ptr<WorkerThreadPool>> &pools)
{
    constexpr int kMaxTaskDelayMicro = 500;
    constexpr size_t kMaxTaskCount   = 100;

    struct TestContext
    {
        const std::vector<std::shared_ptr<WorkerThreadPool>> &pools;

        std::mutex mutex;
        std::condition_variable condVar;
        std::vector<std::shared_ptr<WaitableEvent>> events;
        std::shared_ptr<WaitableEvent> lastTaskEvent;
        size_t taskCounter   = 0;
        size_t nextPoolIndex = 0;

        std::atomic<size_t> readyTaskCount{0};

        explicit TestContext(const std::vector<std::shared_ptr<WorkerThreadPool>> &pools)
            : pools(pools)
        {}

        void tryPostWorkerTask(std::shared_ptr<WorkerTask> &&task,
                               WaitableEvent *dependencyEvent = nullptr)
        {
            constexpr size_t kMaxTaskCountLocal = kMaxTaskCount;

            size_t poolIndex = 0;
            {
                std::lock_guard<std::mutex> lock(mutex);
                ASSERT_LE(taskCounter, kMaxTaskCountLocal);
                if (taskCounter == kMaxTaskCountLocal)
                {
                    return;
                }
                ++taskCounter;
                poolIndex     = nextPoolIndex;
                nextPoolIndex = (nextPoolIndex + 1) % pools.size();
            }

            TaskDependencies dependencies;
            if (dependencyEvent != nullptr)
            {
                dependencies.emplace_back(
                    TaskDependency{dependencyEvent, TaskDependencyHint::Normal});
            }
            auto event = pools[poolIndex]->postWorkerTask(task, dependencies);

            bool isLastTask = false;
            {
                std::lock_guard<std::mutex> lock(mutex);
                events.emplace_back(std::move(event));
                ASSERT_LE(events.size(), kMaxTaskCountLocal);
                if (events.size() == kMaxTaskCountLocal)
                {
                    lastTaskEvent = events.back();
                    isLastTask    = true;
                }
            }
            if (isLastTask)
            {
                condVar.notify_one();
            }
        }

        void waitUntilLastTaskIsPosted()
        {
            std::unique_lock<std::mutex> lock(mutex);
            condVar.wait(lock, [this]() { return lastTaskEvent != nullptr; });
        }
    };

    class TestTask : public WorkerTask
    {
      public:
        TestTask(TestContext *context, size_t expectedReadyNumber)
            : mContext(context), mExpectedReadyNumber(expectedReadyNumber)
        {}

        void operator()(WaitableEvent *event) override
        {
            mContext->tryPostWorkerTask(
                std::make_shared<TestTask>(mContext, mExpectedReadyNumber + 1), event);

            // Add random delay before calculating |readyNumber|.
            const int delayMicro = rand() % kMaxTaskDelayMicro + 1;
            const double endTime = angle::GetCurrentSystemTime() + delayMicro / 1000000.0;
            while (angle::GetCurrentSystemTime() < endTime)
            {
            }

            const size_t readyNumber = ++mContext->readyTaskCount;
            EXPECT_EQ(readyNumber, mExpectedReadyNumber);
        }

      private:
        TestContext *mContext;
        const size_t mExpectedReadyNumber;
    };

    TestContext context(pools);

    // Post the root task.
    context.tryPostWorkerTask(std::make_shared<TestTask>(&context, 1));

    // Each task depends on the previous one, waiting only the last task should also wait for all
    // other tasks.
    context.waitUntilLastTaskIsPosted();
    ASSERT(context.events.size() == kMaxTaskCount);
    ASSERT(context.lastTaskEvent);
    context.lastTaskEvent->wait();
    EXPECT_EQ(context.readyTaskCount, kMaxTaskCount);
    EXPECT_TRUE(WaitableEvent::AllReady(&context.events));

    // Wait all tasks to prevent crash if above check fails.
    WaitableEvent::WaitMany(&context.events);
}

// Tests dependency chain works as expected when using single threaded pool.
TEST(WorkerPoolTest, DependencyChainSTPool)
{
    std::vector<std::shared_ptr<WorkerThreadPool>> pools = {
        WorkerThreadPool::Create(1, ANGLEPlatformCurrent())};
    RunDependencyChainTest(pools);
}

// Tests dependency chain works as expected when using multi threaded pool.
TEST(WorkerPoolTest, DependencyChainMTPool)
{
    std::vector<std::shared_ptr<WorkerThreadPool>> pools = {
        WorkerThreadPool::Create(0, ANGLEPlatformCurrent())};
    RunDependencyChainTest(pools);
}

// Tests dependency chain works as expected when using single and multi threaded pools
// simultaneously.
TEST(WorkerPoolTest, DependencyChainMixedPools)
{
    std::vector<std::shared_ptr<WorkerThreadPool>> pools = {
        WorkerThreadPool::Create(1, ANGLEPlatformCurrent()),
        WorkerThreadPool::Create(0, ANGLEPlatformCurrent())};
    RunDependencyChainTest(pools);
}

void RunRandomDependencyTest(const std::vector<std::shared_ptr<WorkerThreadPool>> &pools)
{
    constexpr int kMaxTaskDelayMicro     = 500;
    constexpr size_t kMaxDependencyCount = 16;
    constexpr size_t kMaxTaskLevel       = 10;
    constexpr size_t kChildTaskCount     = 2;
    constexpr size_t kMaxTaskCount       = ((1 << (kMaxTaskLevel + 1)) - 1);
    static_assert(kChildTaskCount == 2);

    class TestTask;

    using DependencyTasks = std::vector<std::pair<WorkerTask *, WaitableEvent *>>;

    struct TestContext
    {
        const std::vector<std::shared_ptr<WorkerThreadPool>> &pools;

        std::mutex mutex;
        std::condition_variable condVar;
        std::vector<std::shared_ptr<WorkerTask>> tasks;
        std::vector<std::shared_ptr<WaitableEvent>> events;

        std::atomic<size_t> readyTaskCount{0};

        explicit TestContext(const std::vector<std::shared_ptr<WorkerThreadPool>> &pools)
            : pools(pools)
        {}

        void addRandomTasks(size_t requiredCount, DependencyTasks *tasksOut)
        {
            if (tasksOut->size() < requiredCount && !events.empty())
            {
                std::lock_guard<std::mutex> lock(mutex);
                while (tasksOut->size() < requiredCount)
                {
                    const size_t index = rand() % tasks.size();
                    tasksOut->emplace_back(tasks[index].get(), events[index].get());
                }
            }
        }

        void postWorkerTask(size_t poolIndex,
                            std::shared_ptr<WorkerTask> &&task,
                            const TaskDependencies &dependencies)
        {
            constexpr size_t kMaxTaskCountLocal = kMaxTaskCount;

            auto event = pools[poolIndex % pools.size()]->postWorkerTask(task, dependencies);

            bool needNotify = false;
            {
                std::lock_guard<std::mutex> lock(mutex);
                tasks.emplace_back(std::move(task));
                events.emplace_back(std::move(event));
                ASSERT_LE(events.size(), kMaxTaskCountLocal);
                needNotify = (events.size() == kMaxTaskCountLocal);
            }
            if (needNotify)
            {
                condVar.notify_one();
            }
        }

        void waitUntilAllTasksArePosted()
        {
            constexpr size_t kMaxTaskCountLocal = kMaxTaskCount;
            std::unique_lock<std::mutex> lock(mutex);
            condVar.wait(lock, [this]() { return events.size() == kMaxTaskCountLocal; });
        }
    };

    class TestTask : public WorkerTask
    {
      public:
        TestTask(TestContext *context,
                 size_t currentLevel,
                 const DependencyTasks &dependencyTasks = {})
            : mContext(context), mCurrentLevel(currentLevel), mDependencyTasks(dependencyTasks)
        {}

        void operator()(WaitableEvent *event) override
        {
            constexpr size_t kMaxTaskLevelLocal = kMaxTaskLevel;

            const bool isLeafTask = (mCurrentLevel == kMaxTaskLevelLocal);
            ASSERT_LE(mCurrentLevel, kMaxTaskLevelLocal);

            // Spawn child tasks if not yet at max level.
            if (!isLeafTask)
            {
                const size_t childLevel      = mCurrentLevel + 1;
                const size_t dependencyCount = (rand() % kMaxDependencyCount + 1);

                DependencyTasks dependencyTasks;
                // Add this task as dependency with 50% chance.
                if ((dependencyCount & 1) == 1)
                {
                    dependencyTasks.emplace_back(this, event);
                }
                mContext->addRandomTasks(dependencyCount, &dependencyTasks);

                TaskDependencies dependencies;
                for (const auto &item : dependencyTasks)
                {
                    dependencies.emplace_back(
                        TaskDependency{item.second, TaskDependencyHint::Normal});
                }

                for (size_t i = 0; i < kChildTaskCount; ++i)
                {
                    size_t poolIndex = i % mContext->pools.size();
                    mContext->postWorkerTask(
                        poolIndex,
                        std::make_shared<TestTask>(mContext, childLevel, dependencyTasks),
                        dependencies);
                }

                // Add random delay before calculating |mReadyNumber|.
                const int delayMicro = rand() % kMaxTaskDelayMicro + 1;
                const double endTime = angle::GetCurrentSystemTime() + delayMicro / 1000000.0;
                while (angle::GetCurrentSystemTime() < endTime)
                {
                }
            }

            mReadyNumber = ++mContext->readyTaskCount;

            // Check parent tasks finished before the last child (leaf) task.
            if (isLeafTask)
            {
                for (const auto &item : mDependencyTasks)
                {
                    TestTask *task = static_cast<TestTask *>(item.first);
                    EXPECT_TRUE(task->isReady());
                    EXPECT_TRUE(task->isReadyBefore(mReadyNumber));
                    EXPECT_TRUE(item.second->isReady());
                }
            }
        }

        bool isReady() const { return mReadyNumber > 0; }
        bool isReadyBefore(size_t otherReadyNumber) const
        {
            return mReadyNumber < otherReadyNumber;
        }

      private:
        TestContext *mContext;
        const size_t mCurrentLevel;
        DependencyTasks mDependencyTasks;
        size_t mReadyNumber = 0;
    };

    TestContext context(pools);

    // Post the root task.
    context.postWorkerTask(0, std::make_shared<TestTask>(&context, 0), {});

    // Wait until all task are posted.
    context.waitUntilAllTasksArePosted();

    // Wait all tasks. No deadlock should happen.
    WaitableEvent::WaitMany(&context.events);
}

// Tests that random dependencies between tasks work as expected and does not case deadlocks when
// using single threaded pool.
TEST(WorkerPoolTest, RandomDependencySTPool)
{
    std::vector<std::shared_ptr<WorkerThreadPool>> pools = {
        WorkerThreadPool::Create(1, ANGLEPlatformCurrent())};
    RunRandomDependencyTest(pools);
}

// Tests that random dependencies between tasks work as expected and does not case deadlocks when
// using multi threaded pool.
TEST(WorkerPoolTest, RandomDependencyMTPool)
{
    std::vector<std::shared_ptr<WorkerThreadPool>> pools = {
        WorkerThreadPool::Create(0, ANGLEPlatformCurrent())};
    RunRandomDependencyTest(pools);
}

// Tests that random dependencies between tasks work as expected and does not case deadlocks using
// single and multi threaded pools simultaneously.
TEST(WorkerPoolTest, RandomDependencyMixedPools)
{
    std::vector<std::shared_ptr<WorkerThreadPool>> pools = {
        WorkerThreadPool::Create(1, ANGLEPlatformCurrent()),
        WorkerThreadPool::Create(0, ANGLEPlatformCurrent())};
    RunRandomDependencyTest(pools);
}

// Tests that dependency hints work as expected when using |AsyncWorkerPool| with 2 threads.
TEST(WorkerPoolTest, DependencyHint)
{
#if ANGLE_DELEGATE_WORKERS
    // Skip test when using |DelegateWorkerPool| because we can't control the number of threads.
    if (ANGLEPlatformCurrent()->postWorkerTask != nullptr)
    {
        return;
    }
#endif

    struct TestContext
    {
        std::shared_ptr<WorkerThreadPool> pool;
        std::vector<std::shared_ptr<WaitableEvent>> events;
        std::shared_ptr<WaitableEvent> lastDependentTaskEvent;

        std::mutex mutex;
        std::condition_variable condVar;
        bool testIsOver = false;

        std::atomic<size_t> readyTaskCount{0};

        TestContext()
        {
            // Try to create pool with exactly two threads (AsyncWorkerPool).
            pool = WorkerThreadPool::Create(2, ANGLEPlatformCurrent());
        }

        void waitUntilTestIsOver()
        {
            std::unique_lock<std::mutex> lock(mutex);
            condVar.wait(lock, [this]() { return testIsOver; });
        }

        void setTestIsOver()
        {
            {
                std::lock_guard<std::mutex> lock(mutex);
                testIsOver = true;
            }
            condVar.notify_one();
        }
    };

    // This task will block one of two threads, so we can run tasks sequentially in another thread.
    class ThreadBlockingTask : public WorkerTask
    {
      public:
        explicit ThreadBlockingTask(TestContext *context) : mContext(context) {}

        void operator()(WaitableEvent *event) override { mContext->waitUntilTestIsOver(); }

      private:
        TestContext *mContext;
    };

    // This task will be ron on the second thread of the pool.
    class DependentTask : public WorkerTask
    {
      public:
        DependentTask(TestContext *context, size_t expectedReadyNumber)
            : mContext(context), mExpectedReadyNumber(expectedReadyNumber)
        {}

        void operator()(WaitableEvent *event) override
        {
            const size_t readyNumber = ++mContext->readyTaskCount;
            EXPECT_EQ(readyNumber, mExpectedReadyNumber);
        }

      private:
        TestContext *mContext;
        const size_t mExpectedReadyNumber;
    };

    // Must match the number of |DependentTask|s posted by the |MainTask|.
    constexpr size_t kMainTaskDependentTaskCount = 24;

    class MainTask : public WorkerTask
    {
      public:
        explicit MainTask(TestContext *context) : mContext(context) {}

        void operator()(WaitableEvent *event) override
        {
            // All tasks below will be run on a single thread because pool only has two threads,
            // one of which is occupied by the |ThreadBlockingTask| task.  This way we can predict
            // task execution order, while |TaskDependencyHint| affects task start order.
            {
                auto e = postDependentTask(13, event, TaskDependencyHint::Deferred);
                postDependentTask(23, e.get(), TaskDependencyHint::Deferred);
                postDependentTask(14, e.get(), TaskDependencyHint::Normal);
                postDependentTask(15, e.get(), TaskDependencyHint::SubTask);
            }
            {
                auto e = postDependentTask(16, event, TaskDependencyHint::Deferred);
                postDependentTask(24, e.get(), TaskDependencyHint::Deferred);
                postDependentTask(17, e.get(), TaskDependencyHint::Normal);
                postDependentTask(18, e.get(), TaskDependencyHint::SubTask);
            }
            {
                auto e = postDependentTask(5, event, TaskDependencyHint::Normal);
                postDependentTask(21, e.get(), TaskDependencyHint::Deferred);
                postDependentTask(7, e.get(), TaskDependencyHint::Normal);
                postDependentTask(6, e.get(), TaskDependencyHint::SubTask);
            }
            {
                auto e = postDependentTask(8, event, TaskDependencyHint::Normal);
                postDependentTask(22, e.get(), TaskDependencyHint::Deferred);
                postDependentTask(10, e.get(), TaskDependencyHint::Normal);
                postDependentTask(9, e.get(), TaskDependencyHint::SubTask);
            }
            {
                auto e = postDependentTask(1, event, TaskDependencyHint::SubTask);
                postDependentTask(19, e.get(), TaskDependencyHint::Deferred);
                postDependentTask(11, e.get(), TaskDependencyHint::Normal);
                postDependentTask(2, e.get(), TaskDependencyHint::SubTask);
            }
            {
                auto e = postDependentTask(3, event, TaskDependencyHint::SubTask);
                postDependentTask(20, e.get(), TaskDependencyHint::Deferred);
                postDependentTask(12, e.get(), TaskDependencyHint::Normal);
                postDependentTask(4, e.get(), TaskDependencyHint::SubTask);
            }
        }

      private:
        std::shared_ptr<WaitableEvent> postDependentTask(size_t expectedReadyNumber,
                                                         WaitableEvent *dependencyEvent,
                                                         TaskDependencyHint dependencyHint)
        {
            std::shared_ptr<WaitableEvent> event = mContext->pool->postWorkerTask(
                std::make_shared<DependentTask>(mContext, expectedReadyNumber),
                {TaskDependency{dependencyEvent, dependencyHint}});
            {
                std::lock_guard<std::mutex> lock(mContext->mutex);
                mContext->events.emplace_back(event);
                if (expectedReadyNumber == kMainTaskDependentTaskCount)
                {
                    mContext->lastDependentTaskEvent = event;
                }
            }
            return event;
        }

        TestContext *mContext;
    };

    TestContext context;
    // Skip test if pool is not async (SingleThreadedWorkerPool).
    if (!context.pool->isAsync())
    {
        return;
    }

    // Block one of two threads on the pool.
    auto threadBlockingTaskEvent =
        context.pool->postWorkerTask(std::make_shared<ThreadBlockingTask>(&context));

    // Post and wait |MainTask| until it posts all |DependentTask|s.
    auto mainTaskEvent = context.pool->postWorkerTask(std::make_shared<MainTask>(&context));
    mainTaskEvent->wait();

    // Waiting last dependent task should also wait for all tasks.
    EXPECT_EQ(context.events.size(), kMainTaskDependentTaskCount);
    ASSERT(context.lastDependentTaskEvent);
    context.lastDependentTaskEvent->wait();
    EXPECT_EQ(context.readyTaskCount, kMainTaskDependentTaskCount);
    EXPECT_TRUE(WaitableEvent::AllReady(&context.events));

    // Wait all tasks to prevent crash if above check fails.
    WaitableEvent::WaitMany(&context.events);

    // End the test.
    context.setTestIsOver();
    threadBlockingTaskEvent->wait();
}

}  // anonymous namespace
