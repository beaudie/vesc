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

#include "common/vulkan/vk_headers.h"
#include "libANGLE/renderer/vulkan/PersistentCommandPool.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{
class RendererVk;

namespace vk
{
// CommandProcessorTask is used to queue a task to the worker thread when
//  enableCommandProcessingThread feature is true.
// The "standard" task includes a ContextVk pointer and a CommandBufferHelper (CBH)
//  pointer and the worker thread will process the SecondaryCommandBuffer (SCB) commands
//  in the CBH into its primary CommandBuffer.
// There is a special work block in which all of the pointers are null that will trigger
//  the worker thread to exit, and is sent when the renderer instance shuts down.
// If the work block does not involve processing SCB commands, then a CustomTask
//  will be placed in the 2nd position of the CommandProcessorTask instead.
// Custom tasks are:
//  Flush:End the current command buffer and submit commands to the queue
//  FinishToSerial:Finish queue commands up to given serial value
//  .... More here
enum CustomTask
{
    Invalid = 0,
    Flush,
    FinishToSerial,
    Present
};

struct FlushData
{
    std::vector<VkSemaphore> waitSemaphores;
    std::vector<VkPipelineStageFlags> waitSemaphoreStageMasks;
    const vk::Semaphore *semaphore;
    egl::ContextPriority contextPriority;
    vk::Shared<vk::Fence> submitFence;
    vk::GarbageList currentGarbage;
};

struct FinishToSerialData
{
    Serial serial;
};

struct PresentData
{
    egl::ContextPriority priority;
    VkPresentInfoKHR presentInfo;
};

struct CommandProcessorTask
{
    ContextVk *contextVk;
    // TODO: b/153666475 Removed primaryCB in threading phase2.
    union
    {
        CommandBufferHelper *commandBuffer;
        CustomTask workerCommand;
    };
    void *commandData;
};

struct CommandBatch final : angle::NonCopyable
{
    CommandBatch();
    ~CommandBatch();
    CommandBatch(CommandBatch &&other);
    CommandBatch &operator=(CommandBatch &&other);

    void destroy(VkDevice device);

    vk::PrimaryCommandBuffer primaryCommands;
    // commandPool is for secondary CommandBuffer allocation
    vk::CommandPool commandPool;
    vk::Shared<vk::Fence> fence;
    Serial serial;
};

static const CommandProcessorTask kEndCommandProcessorThread = {nullptr, {nullptr}, nullptr};
}  // namespace vk

class CommandWorkQueue : public vk::Context
{
  public:
    CommandWorkQueue(RendererVk *renderer);
    ~CommandWorkQueue() override;

    angle::Result init();
    void destroy(VkDevice device);
    void handleDeviceLost();
    void handleError(VkResult result,
                     const char *file,
                     const char *function,
                     unsigned int line) override;
    VkDevice getDevice() const;
    bool isRobustResourceInitEnabled() const override;

    bool hasInFlightCommands() const;

    angle::Result allocatePrimaryCommandBuffer(vk::PrimaryCommandBuffer *commandBufferOut);
    angle::Result releasePrimaryCommandBuffer(vk::PrimaryCommandBuffer &&commandBuffer);

    void clearAllGarbage(RendererVk *renderer);

    angle::Result finishToSerial(RendererVk *renderer, Serial serial);

    VkResult present(egl::ContextPriority priority, const VkPresentInfoKHR &presentInfo);

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

    CommandWorkQueue *getPointer() { return this; }

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
    void queueCommands(const vk::CommandProcessorTask &commands);
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
    void handleDeviceLost() { mCommandWorkQueue.handleDeviceLost(); }
    // Stop the command processor loop
    void shutdown(std::thread *commandProcessorThread);

  private:
    std::queue<vk::CommandProcessorTask> mCommandsQueue;
    std::mutex mWorkerMutex;
    // Signal worker thread when work is available
    std::condition_variable mWorkAvailableCondition;
    // Signal main thread when all work completed
    std::condition_variable mWorkerIdleCondition;
    // Track worker thread Idle state for assertion purposes
    bool mWorkerThreadIdle;
    // Command pool to allocate processor thread primary command buffers from
    vk::CommandPool mCommandPool;
    vk::PrimaryCommandBuffer mPrimaryCommandBuffer;
    RendererVk *mRenderer;
    CommandWorkQueue mCommandWorkQueue;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_COMMAND_PROCESSOR_H_
