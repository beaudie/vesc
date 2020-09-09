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
class CommandProcessor;

namespace vk
{
// CommandProcessorTask is used to queue a task to the worker thread when
//  enableCommandProcessingThread feature is true.
// Issuing the CustomTask::Exit command will cause the worker thread to clean up it's resources and
// shut down. This command is sent when the renderer instance shuts down. Custom tasks are:
//  Flush:End the current command buffer and submit commands to the queue
//  FinishToSerial:Finish queue commands up to given serial value
//  FlushToPrimary: Process SecondaryCommandBuffer commands into the primary CommandBuffer.
//  Present:Execute QueuePresent
//  QueueWaitIdle:Execute QueueWaitIdle
//  DeviceWaitIdle:Execute DeviceWaitIdle
//  Exit:Exit the command processor thread
enum CustomTask
{
    Invalid = 0,
    FlushToPrimary,
    Flush,
    FinishToSerial,
    Present,
    QueueWaitIdle,
    DeviceWaitIdle,
    Exit,
};

// Save error details in worker thread to sync to main thread
struct ErrorDetails
{
    VkResult errorCode;
    const char *file;
    const char *function;
    unsigned int line;
};

class CommandProcessorTask
{
    friend class rx::CommandProcessor;

  public:
    CommandProcessorTask(CustomTask command)
        : mContextVk(nullptr), mCommandBuffer(nullptr), mWorkerCommand(command), mSemaphore(nullptr)
    {}
    CommandProcessorTask(ContextVk *contextVk, CommandBufferHelper *commandBuffer)
        : mContextVk(contextVk),
          mCommandBuffer(commandBuffer),
          mWorkerCommand(vk::CustomTask::FlushToPrimary),
          mSemaphore(nullptr)
    {}
    CommandProcessorTask(egl::ContextPriority priority, VkPresentInfoKHR presentInfo)
        : mContextVk(nullptr),
          mCommandBuffer(nullptr),
          mWorkerCommand(vk::CustomTask::Present),
          mSemaphore(nullptr),
          mPresentInfo(presentInfo),
          mPriority(priority)
    {}
    CommandProcessorTask(egl::ContextPriority priority)
        : mContextVk(nullptr),
          mCommandBuffer(nullptr),
          mWorkerCommand(vk::CustomTask::QueueWaitIdle),
          mSemaphore(nullptr),
          mPriority(priority)
    {}
    CommandProcessorTask(Serial serial)
        : mContextVk(nullptr),
          mCommandBuffer(nullptr),
          mWorkerCommand(vk::CustomTask::FinishToSerial),
          mSerial(serial)
    {}
    CommandProcessorTask(std::vector<VkSemaphore> waitSemaphores,
                         std::vector<VkPipelineStageFlags> waitSemaphoreStageMasks,
                         const vk::Semaphore *semaphore,
                         egl::ContextPriority priority,
                         vk::GarbageList &currentGarbage)
        : mContextVk(nullptr),
          mCommandBuffer(nullptr),
          mWorkerCommand(vk::CustomTask::Flush),
          mWaitSemaphores(waitSemaphores),
          mWaitSemaphoreStageMasks(waitSemaphoreStageMasks),
          mSemaphore(semaphore),
          mCurrentGarbage(std::move(currentGarbage)),
          mPriority(priority)
    {}

    CommandProcessorTask &operator=(CommandProcessorTask &&rhs)
    {
        if (this != &rhs)
        {
            mContextVk     = rhs.mContextVk;
            mCommandBuffer = rhs.mCommandBuffer;
            std::swap(mWorkerCommand, rhs.mWorkerCommand);
            std::swap(mWaitSemaphores, rhs.mWaitSemaphores);
            std::swap(mWaitSemaphoreStageMasks, mWaitSemaphoreStageMasks);
            mSemaphore = rhs.mSemaphore;
            std::swap(mCurrentGarbage, rhs.mCurrentGarbage);
            std::swap(mSerial, rhs.mSerial);
            std::swap(mPresentInfo, rhs.mPresentInfo);
            std::swap(mPriority, rhs.mPriority);

            // clear rhs now that everything has moved.
            rhs.mCommandBuffer = nullptr;
            rhs.mSemaphore     = nullptr;
        }
        return *this;
    }

    CommandProcessorTask(CommandProcessorTask &&other) { *this = std::move(other); }

  private:
    ContextVk *mContextVk;
    CommandBufferHelper *mCommandBuffer;
    CustomTask mWorkerCommand;
    uint64_t mCommandId;

    // Flush data
    std::vector<VkSemaphore> mWaitSemaphores;
    std::vector<VkPipelineStageFlags> mWaitSemaphoreStageMasks;
    const vk::Semaphore *mSemaphore;
    vk::GarbageList mCurrentGarbage;

    // FinishToSerial command data
    Serial mSerial;

    // Present command data
    VkPresentInfoKHR mPresentInfo;

    // Flush, Present & QueueWaitIdle data
    egl::ContextPriority mPriority;
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
    angle::Result queueWaitIdle(egl::ContextPriority priority);
    angle::Result deviceWaitIdle();

    // Check to see which batches have finished completion (forward progress for
    // mLastCompletedQueueSerial, for example for when the application busy waits on a query
    // result). It would be nice if we didn't have to expose this for QueryVk::getResult.
    angle::Result checkCompletedCommands(RendererVk *renderer);

    CommandWorkQueue *getPointer() { return this; }
    bool hasError() const { return mErrorDetails.errorCode != VK_SUCCESS; }
    vk::ErrorDetails getAndClearError();

  private:
    void resetError();
    angle::Result releaseToCommandBatch(RendererVk *renderer,
                                        vk::PrimaryCommandBuffer &&commandBuffer,
                                        vk::CommandPool *commandPool,
                                        vk::CommandBatch *batch);

    vk::GarbageQueue mGarbageQueue;
    std::vector<vk::CommandBatch> mInFlightCommands;

    // Keeps a free list of reusable primary command buffers.
    vk::PersistentCommandPool mPrimaryCommandPool;
    std::mutex mErrorMutex;
    vk::ErrorDetails mErrorDetails;
};

class CommandProcessor : angle::NonCopyable
{
  public:
    CommandProcessor(RendererVk *renderer);
    ~CommandProcessor() = default;

    // Entry point for command processor thread, calls processCommandProcessorTasksImpl to do the
    // work.
    void processCommandProcessorTasks();

    // Main worker loop called by processCommandProcessorTasks. The
    //  loop waits for work to be submitted from a separate thread.
    angle::Result processCommandProcessorTasksImpl(bool *exitThread);

    // Called asynchronously from main thread to queue work that is
    //  then processed by the worker thread
    void queueCommand(vk::CommandProcessorTask *command);
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
    bool hasPendingError() const { return mCommandWorkQueue.hasError(); }
    vk::ErrorDetails getAndClearPendingError() { return mCommandWorkQueue.getAndClearError(); }
    // Stop the command processor loop
    void shutdown(std::thread *commandProcessorThread);
    vk::Context *getContextPointer() { return &mCommandWorkQueue; }

  private:
    std::queue<vk::CommandProcessorTask> mCommandsQueue;
    uint64_t mCommandId;
    uint64_t mOutputCommandId;
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
    // Shared fence used for Queue submission
    vk::Shared<vk::Fence> mFence;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_COMMAND_PROCESSOR_H_
