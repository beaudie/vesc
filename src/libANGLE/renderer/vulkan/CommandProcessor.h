//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CommandProcessor.h:
//    A class to process and submit Vulkan command buffers that can be
//    used in an asynchronous worker thread.
//

#ifndef LIBANGLE_RENDERER_VULKAN_COMMAND_PROCESSOR_H_
#define LIBANGLE_RENDERER_VULKAN_COMMAND_PROCESSOR_H_

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include "libANGLE/renderer/vulkan/PersistentCommandPool.h"
#include "libANGLE/renderer/vulkan/vk_headers.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{
class RendererVk;

class CommandWorkQueue final : angle::NonCopyable
{
  public:
    CommandWorkQueue();
    ~CommandWorkQueue();

    angle::Result init(RendererVk *context);
    void destroy(VkDevice device);
    void handleDeviceLost(RendererVk *renderer);

    bool hasInFlightCommands() const;

    angle::Result allocatePrimaryCommandBuffer(VkDevice device,
                                               vk::PrimaryCommandBuffer *commandBufferOut);
    angle::Result releasePrimaryCommandBuffer(vk::PrimaryCommandBuffer &&commandBuffer);

    void clearAllGarbage(RendererVk *renderer);

    angle::Result finishToSerial(RendererVk *renderer, Serial serial);

    angle::Result submitFrame(RendererVk *renderer,
                              egl::ContextPriority priority,
                              const VkSubmitInfo &submitInfo,
                              const vk::Shared<vk::Fence> &sharedFence,
                              vk::GarbageList *currentGarbage,
                              vk::CommandPool *commandPool,
                              vk::PrimaryCommandBuffer &&commandBuffer);

    vk::Shared<vk::Fence> getLastSubmittedFence(const VkDevice device) const;

    // Check to see which batches have finished completion (forward progress for
    // mLastCompletedQueueSerial, for example for when the application busy waits on a query
    // result). It would be nice if we didn't have to expose this for QueryVk::getResult.
    angle::Result checkCompletedCommands(RendererVk *renderer);

  private:
    angle::Result releaseToCommandBatch(RendererVk *renderer,
                                        vk::PrimaryCommandBuffer &&commandBuffer,
                                        vk::CommandPool *commandPool,
                                        vk::CommandBatch *batch);

    vk::GarbageQueue mGarbageQueue;
    std::vector<vk::CommandBatch> mInFlightCommands;

    // Keeps a free list of reusable primary command buffers.
    vk::PersistentCommandPool mPrimaryCommandPool;
};

class CommandProcessor : angle::NonCopyable
{
  public:
    CommandProcessor(RendererVk *renderer);
    ~CommandProcessor() = default;

    // Main worker loop that should be launched in its own thread. The
    //  loop waits for work to be submitted from a separate thread.
    angle::Result processCommandProcessorTasks();
    // Called asynchronously from workLoop() thread to queue work that is
    //  then processed by the workLoop() thread
    void queueCommands(vk::CommandProcessorTask commands);
    // Used by separate thread to wait for worker thread to complete all
    //  outstanding work.
    void waitForWorkComplete();
    void clearAllGarbage() { mCommandWorkQueue.clearAllGarbage(mRenderer); }
    angle::Result checkCompletedCommands()
    {
        return mCommandWorkQueue.checkCompletedCommands(mRenderer);
    }
    angle::Result finishToSerial(Serial serial)
    {
        return mCommandWorkQueue.finishToSerial(mRenderer, serial);
    }
    vk::Shared<vk::Fence> getLastSubmittedFence() const;
    bool hasInFlightCommandBuffers() { return mCommandWorkQueue.hasInFlightCommands(); }
    void handleDeviceLost() { mCommandWorkQueue.handleDeviceLost(mRenderer); }

  private:
    std::queue<vk::CommandProcessorTask> mCommandsQueue;
    std::mutex mWorkerMutex;
    // Signal worker thread when work is available
    std::condition_variable mWorkAvailableCondition;
    // Signal main thread when all work completed
    std::condition_variable mWorkerIdleCondition;
    // Track worker thread Idle state for assertion purposes
    bool mWorkerThreadIdle;
    CommandWorkQueue mCommandWorkQueue;
    // Command pool to allocate processor thread primary command buffers from
    vk::CommandPool mCommandPool;
    vk::PrimaryCommandBuffer mPrimaryCommandBuffer;
    RendererVk *mRenderer;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_COMMAND_PROCESSOR_H_
