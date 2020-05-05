//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CommandProcessor.cpp:
//    Implements the class methods for CommandProcessor.
//

#include "libANGLE/renderer/vulkan/CommandProcessor.h"
#include "libANGLE/trace.h"

namespace rx
{

CommandProcessor::CommandProcessor() : mWorkerThreadIdle(true) {}

void CommandProcessor::queueCommands(vk::CommandWorkBlock commandWork)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "RendererVk::queueCommands");
    std::lock_guard<std::mutex> queueLock(mWorkerMutex);
    ASSERT(commandWork.cbh == nullptr || !commandWork.cbh->empty());
    mCommandsQueue.push(commandWork);
    mWorkAvailableCondition.notify_one();
}

angle::Result CommandProcessor::workLoop()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(mWorkerMutex);
        mWorkerIdleCondition.notify_one();
        mWorkerThreadIdle = true;
        // Only wake if notified and command queue is not empty
        mWorkAvailableCondition.wait(lock, [this] { return !mCommandsQueue.empty(); });
        mWorkerThreadIdle        = false;
        vk::CommandWorkBlock cwb = mCommandsQueue.front();
        mCommandsQueue.pop();
        lock.unlock();
        // A work block with null ptrs signals worker thread to exit
        if (cwb.cbh == nullptr && cwb.contextVk == nullptr)
        {
            break;
        }

        ASSERT(!cwb.cbh->empty());
        // TODO: Will need some way to synchronize error reporting between threads
        ANGLE_TRY(cwb.cbh->flushToPrimary(cwb.contextVk, cwb.primaryCB));
        ASSERT(cwb.cbh->empty());
        cwb.cbh->releaseToContextQueue(cwb.contextVk);
    }
    return angle::Result::Continue;
}

void CommandProcessor::waitForWorkComplete()
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandProcessor::waitForWorkerThreadIdle");
    std::unique_lock<std::mutex> lock(mWorkerMutex);
    mWorkerIdleCondition.wait(lock,
                              [this] { return (mCommandsQueue.empty() && mWorkerThreadIdle); });
    // Worker thread is idle and command queue is empty so good to continue
    lock.unlock();
}
}  // namespace rx
