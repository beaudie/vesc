//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WorkerThread_unittest:
//   Simple tests for the worker thread class.

#include <array>
#include <gtest/gtest.h>

#include "libANGLE/WorkerThread.h"

using namespace angle;

namespace
{

// Tests simple worker pool application.
TEST(WorkerPoolTest, SimpleTask)
{
    class TestTask : public Closure
    {
      public:
        void operator()() override { fired = true; }

        bool fired = false;
    };

    std::array<std::shared_ptr<WorkerThreadPool>, 2> pools = {
        {WorkerThreadPool::Create(false), WorkerThreadPool::Create(true)}};
    for (auto &pool : pools)
    {
        std::array<TestTask, 4> tasks;
        std::array<std::shared_ptr<WaitableEvent>, 4> waitables = {
            {pool->postWorkerTask(std::shared_ptr<Closure>(&tasks[0])),
             pool->postWorkerTask(std::shared_ptr<Closure>(&tasks[1])),
             pool->postWorkerTask(std::shared_ptr<Closure>(&tasks[2])),
             pool->postWorkerTask(std::shared_ptr<Closure>(&tasks[3]))}};

        WaitableEvent::WaitMany(&waitables);

        for (const auto &task : tasks)
        {
            EXPECT_TRUE(task.fired);
        }
    }
}

}  // anonymous namespace
