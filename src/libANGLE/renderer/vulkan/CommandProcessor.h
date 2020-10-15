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
// CommandProcessor is used to dispatch work to the GPU when commandProcessor feature is true.
// If asynchronousCommandProcessing is enabled the work will be queued and handled by a worker
// thread asynchronous to the context. Issuing the CustomTask::Exit command will cause the worker
// thread to clean up it's resources and shut down. This command is sent when the renderer instance
// shuts down. Custom tasks are:

enum CustomTask
{
    Invalid = 0,
    // Process SecondaryCommandBuffer commands into the primary CommandBuffer.
    ProcessCommands,
    // End the current command buffer and submit commands to the queue
    FlushAndQueueSubmit,
    // Submit custom command buffer, excludes some state management
    OneOffQueueSubmit,
    // Finish queue commands up to given serial value, process garbage
    FinishToSerial,
    // Execute QueuePresent
    Present,
    // do cleanup processing on completed commands
    CheckCompletedCommands,
    // Exit the command processor thread
    Exit,
};

class CommandProcessorTask
{
  public:
    CommandProcessorTask() { initTask(); }

    void initTask();

    void initTask(CustomTask command) { mTask = command; }

    void initProcessCommands(ContextVk *contextVk,
                             CommandBufferHelper *commandBuffer,
                             vk::RenderPass *renderPass);

    void initPresent(egl::ContextPriority priority, VkPresentInfoKHR &presentInfo);

    void initFinishToSerial(Serial serial);

    void initFlushAndQueueSubmit(vk::Context *contextVk,
                                 const vk::Shared<vk::Fence> &sharedFence,
                                 std::vector<VkSemaphore> &&waitSemaphores,
                                 std::vector<VkPipelineStageFlags> &&waitSemaphoreStageMasks,
                                 const vk::Semaphore *semaphore,
                                 egl::ContextPriority priority,
                                 vk::GarbageList &&currentGarbage,
                                 vk::ResourceUseList &&currentResources);

    void initOneOffQueueSubmit(VkCommandBuffer oneOffCommandBufferVk,
                               egl::ContextPriority priority,
                               const vk::Fence *fence);

    CommandProcessorTask &operator=(CommandProcessorTask &&rhs);

    CommandProcessorTask(CommandProcessorTask &&other) : CommandProcessorTask()
    {
        *this = std::move(other);
    }

    void setTaskSerial(Serial serial) { mTaskSerial = serial; }
    Serial getTaskSerial() const { return mTaskSerial; }
    void setQueueSerial(Serial serial) { mSerial = serial; }
    Serial getQueueSerial() const { return mSerial; }
    vk::ResourceUseList &getResourceUseList() { return mResourceUseList; }
    vk::CustomTask getTaskCommand() { return mTask; }
    std::vector<VkSemaphore> &getWaitSemaphores() { return mWaitSemaphores; }
    std::vector<VkPipelineStageFlags> &getWaitSemaphoreStageMasks()
    {
        return mWaitSemaphoreStageMasks;
    }
    const vk::Semaphore *getSemaphore() { return mSemaphore; }
    vk::GarbageList &getGarbage() { return mGarbage; }
    egl::ContextPriority getPriority() const { return mPriority; }
    const VkCommandBuffer &getOneOffCommandBufferVk() const { return mOneOffCommandBufferVk; }
    const vk::Fence *getOneOffFence() { return mOneOffFence; }
    const VkPresentInfoKHR &getPresentInfo() const { return mPresentInfo; }
    vk::RenderPass *getRenderPass() const { return mRenderPass; }
    CommandBufferHelper *getCommandBuffer() const { return mCommandBuffer; }
    ContextVk *getContextVk() const { return mContextVk; }
    vk::Shared<vk::Fence> &getSharedFence() { return mSharedFence; }

  private:
    void copyPresentInfo(const VkPresentInfoKHR &other);

    CustomTask mTask;
    Serial mTaskSerial;

    // ProcessCommands
    ContextVk *mContextVk;
    vk::RenderPass *mRenderPass;
    CommandBufferHelper *mCommandBuffer;

    // Flush data
    std::vector<VkSemaphore> mWaitSemaphores;
    std::vector<VkPipelineStageFlags> mWaitSemaphoreStageMasks;
    const vk::Semaphore *mSemaphore;
    vk::GarbageList mGarbage;
    vk::ResourceUseList mResourceUseList;

    // FinishToSerial & Flush command data
    Serial mSerial;

    // Present command data
    VkPresentInfoKHR mPresentInfo;
    VkSwapchainKHR mSwapchain;
    VkSemaphore mWaitSemaphore;
    uint32_t mImageIndex;
    // Used by Present if supportsIncrementalPresent is enabled
    VkPresentRegionKHR mPresentRegion;
    VkPresentRegionsKHR mPresentRegions;
    std::vector<VkRectLayerKHR> mRects;

    // Used by OneOffQueueSubmit
    VkCommandBuffer mOneOffCommandBufferVk;
    const vk::Fence *mOneOffFence;

    // Flush, Present & QueueWaitIdle data
    vk::Shared<vk::Fence> mSharedFence;
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

class TaskProcessor : angle::NonCopyable
{
  public:
    TaskProcessor();
    ~TaskProcessor();

    angle::Result init(vk::Context *context, std::thread::id threadId);
    void destroy(VkDevice device);

    angle::Result allocatePrimaryCommandBuffer(vk::Context *context,
                                               vk::PrimaryCommandBuffer *commandBufferOut);
    angle::Result releasePrimaryCommandBuffer(vk::Context *context,
                                              vk::PrimaryCommandBuffer &&commandBuffer);

    angle::Result finishToSerial(vk::Context *context, Serial serial);

    VkResult present(VkQueue queue, const VkPresentInfoKHR &presentInfo);

    angle::Result submitFrame(vk::Context *context,
                              VkQueue queue,
                              const VkSubmitInfo &submitInfo,
                              const vk::Shared<vk::Fence> &sharedFence,
                              vk::GarbageList *currentGarbage,
                              vk::CommandPool *commandPool,
                              vk::PrimaryCommandBuffer &&commandBuffer,
                              const Serial &queueSerial);
    angle::Result queueSubmit(vk::Context *context,
                              VkQueue queue,
                              const VkSubmitInfo &submitInfo,
                              const vk::Fence *fence);

    void handleDeviceLost(vk::Context *context);

    // Called by CommandProcessor to process any completed work
    angle::Result checkCompletedCommands(vk::Context *context);

    VkResult getLastPresentResult(VkSwapchainKHR swapchain);

  private:
    bool isValidWorkerThread(vk::Context *context) const;

    angle::Result releaseToCommandBatch(vk::Context *context,
                                        vk::PrimaryCommandBuffer &&commandBuffer,
                                        vk::CommandPool *commandPool,
                                        vk::CommandBatch *batch);

    // Check to see which batches have finished completion (forward progress for
    // mLastCompletedQueueSerial, for example for when the application busy waits on a query
    // result). It would be nice if we didn't have to expose this for QueryVk::getResult.
    angle::Result checkCompletedCommandsWithLock(vk::Context *context);

    vk::GarbageQueue mGarbageQueue;

    mutable std::mutex mInFlightCommandsMutex;
    std::vector<vk::CommandBatch> mInFlightCommands;

    // Keeps a free list of reusable primary command buffers.
    vk::PersistentCommandPool mPrimaryCommandPool;
    std::thread::id mThreadId;

    // Track present info
    std::mutex mSwapchainStatusMutex;
    std::condition_variable mSwapchainStatusCondition;
    std::map<VkSwapchainKHR, VkResult> mSwapchainStatus;
};

class CommandProcessor : public vk::Context
{
  public:
    CommandProcessor(RendererVk *renderer);
    ~CommandProcessor() override;

    angle::Result initTaskProcessor(vk::Context *context);

    void handleError(VkResult result,
                     const char *file,
                     const char *function,
                     unsigned int line) override;

    bool isRobustResourceInitEnabled() const override;

    // Entry point for command processor thread, calls processTasksImpl to do the
    // work. called by RendererVk::initialization on main thread
    void processTasks();

    // Called asynchronously from main thread to queue work that is then processed by the worker
    // thread
    void queueCommand(vk::Context *context, vk::CommandProcessorTask *task);

    void checkCompletedCommands(vk::Context *context);

    // Used by main thread to wait for worker thread to complete all outstanding work.
    void waitForWorkComplete(vk::Context *context);
    Serial getCurrentQueueSerial();
    Serial getLastSubmittedSerial();

    // Wait until desired serial has been processed.
    void finishToSerial(vk::Context *context, Serial serial);

    void handleDeviceLost();

    bool hasPendingError() const
    {
        std::lock_guard<std::mutex> queueLock(mErrorMutex);
        return !mErrors.empty();
    }
    vk::Error getAndClearPendingError();

    // Stop the command processor thread
    void shutdown(std::thread *commandProcessorThread);

    void finishAllWork(vk::Context *context);

    VkResult getLastPresentResult(VkSwapchainKHR swapchain)
    {
        return mTaskProcessor.getLastPresentResult(swapchain);
    }

  private:
    // Command processor thread, called by processTasks. The loop waits for work to
    // be submitted from a separate thread.
    angle::Result processTasksImpl(bool *exitThread);

    // Command processor thread, process a task
    angle::Result processTask(vk::Context *context, vk::CommandProcessorTask *task);

    std::queue<vk::CommandProcessorTask> mTasks;
    mutable std::mutex mWorkerMutex;
    // Signal worker thread when work is available
    std::condition_variable mWorkAvailableCondition;
    // Signal main thread when all work completed
    mutable std::condition_variable mWorkerIdleCondition;
    // Track worker thread Idle state for assertion purposes
    bool mWorkerThreadIdle;
    // Command pool to allocate processor thread primary command buffers from
    vk::CommandPool mCommandPool;
    vk::PrimaryCommandBuffer mPrimaryCommandBuffer;
    TaskProcessor mTaskProcessor;

    AtomicSerialFactory mQueueSerialFactory;
    AtomicSerialFactory mTaskSerialFactory;
    std::mutex mCommandProcessorQueueSerialMutex;
    Serial mCommandProcessorLastSubmittedSerial;
    Serial mCommandProcessorCurrentQueueSerial;
    Serial mTaskSerial;

    mutable std::mutex mErrorMutex;
    std::queue<vk::Error> mErrors;
};

}  // namespace vk

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_COMMAND_PROCESSOR_H_
