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
    ASSERT(commandWork.commandBuffer == nullptr || !commandWork.commandBuffer->empty());
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
        mWorkerThreadIdle              = false;
        vk::CommandWorkBlock workBlock = mCommandsQueue.front();
        mCommandsQueue.pop();
        lock.unlock();
        // Either both ptrs should be null or non-null
        ASSERT((workBlock.commandBuffer != nullptr && workBlock.contextVk != nullptr) ||
               (workBlock.commandBuffer == nullptr && workBlock.contextVk == nullptr));
        // A work block with null ptrs signals worker thread to exit
        if (workBlock.commandBuffer == nullptr && workBlock.contextVk == nullptr)
        {
            break;
        }

        ASSERT(!workBlock.commandBuffer->empty());
        // TODO: Will need some way to synchronize error reporting between threads
        ANGLE_TRY(
            workBlock.commandBuffer->flushToPrimary(workBlock.contextVk, workBlock.primaryCB));
        ASSERT(workBlock.commandBuffer->empty());
        workBlock.commandBuffer->releaseToContextQueue(workBlock.contextVk);
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
