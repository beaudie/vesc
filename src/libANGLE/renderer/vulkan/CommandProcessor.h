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
struct CommandBatch final : angle::NonCopyable
{
    CommandBatch();
    ~CommandBatch();
    CommandBatch(CommandBatch &&other);
    CommandBatch &operator=(CommandBatch &&other);

    void destroy(VkDevice device);

    PrimaryCommandBuffer primaryCommands;
    // commandPool is for secondary CommandBuffer allocation
    CommandPool commandPool;
    Shared<Fence> fence;
    Serial serial;
};

using DeviceQueueMap = angle::PackedEnumMap<egl::ContextPriority, VkQueue>;

class CommandQueueInterface : angle::NonCopyable
{
  public:
    virtual ~CommandQueueInterface() {}

    virtual angle::Result init(Context *context, const DeviceQueueMap &queueMap) = 0;
    virtual void destroy(RendererVk *renderer)                                   = 0;

    virtual void handleDeviceLost(RendererVk *renderer) = 0;

    // Wait until the desired serial has been completed.
    virtual angle::Result finishToSerial(Context *context,
                                         Serial finishSerial,
                                         uint64_t timeout) = 0;
    virtual Serial reserveSubmitSerial()                   = 0;
    virtual angle::Result submitFrame(
        Context *context,
        egl::ContextPriority priority,
        const std::vector<VkSemaphore> &waitSemaphores,
        const std::vector<VkPipelineStageFlags> &waitSemaphoreStageMasks,
        const Semaphore *signalSemaphore,
        Shared<Fence> &&sharedFence,
        GarbageList &&currentGarbage,
        CommandPool *commandPool,
        Serial submitQueueSerial)                                      = 0;
    virtual angle::Result queueSubmitOneOff(Context *context,
                                            egl::ContextPriority contextPriority,
                                            VkCommandBuffer commandBufferHandle,
                                            const Fence *fence,
                                            Serial submitQueueSerial)  = 0;
    virtual VkResult queuePresent(egl::ContextPriority contextPriority,
                                  const VkPresentInfoKHR &presentInfo) = 0;

    virtual angle::Result waitForSerialWithUserTimeout(vk::Context *context,
                                                       Serial serial,
                                                       uint64_t timeout,
                                                       VkResult *result) = 0;

    // Check to see which batches have finished completion (forward progress for
    // the last completed serial, for example for when the application busy waits on a query
    // result). It would be nice if we didn't have to expose this for QueryVk::getResult.
    virtual angle::Result checkCompletedCommands(Context *context) = 0;

    virtual angle::Result flushOutsideRPCommands(Context *context,
                                                 CommandBufferHelper **outsideRPCommands)   = 0;
    virtual angle::Result flushRenderPassCommands(Context *context,
                                                  const RenderPass &renderPass,
                                                  CommandBufferHelper **renderPassCommands) = 0;

    virtual Serial getLastSubmittedQueueSerial() const = 0;
    virtual Serial getLastCompletedQueueSerial() const = 0;
    virtual Serial getCurrentQueueSerial() const       = 0;
};

class CommandQueue final : public CommandQueueInterface
{
  public:
    CommandQueue();
    ~CommandQueue() override;

    angle::Result init(Context *context, const DeviceQueueMap &queueMap) override;
    void destroy(RendererVk *renderer) override;
    void clearAllGarbage(RendererVk *renderer);

    void handleDeviceLost(RendererVk *renderer) override;

    angle::Result finishToSerial(Context *context, Serial finishSerial, uint64_t timeout) override;

    Serial reserveSubmitSerial() override;

    angle::Result submitFrame(Context *context,
                              egl::ContextPriority priority,
                              const std::vector<VkSemaphore> &waitSemaphores,
                              const std::vector<VkPipelineStageFlags> &waitSemaphoreStageMasks,
                              const Semaphore *signalSemaphore,
                              Shared<Fence> &&sharedFence,
                              GarbageList &&currentGarbage,
                              CommandPool *commandPool,
                              Serial submitQueueSerial) override;

    angle::Result queueSubmitOneOff(Context *context,
                                    egl::ContextPriority contextPriority,
                                    VkCommandBuffer commandBufferHandle,
                                    const Fence *fence,
                                    Serial submitQueueSerial) override;

    VkResult queuePresent(egl::ContextPriority contextPriority,
                          const VkPresentInfoKHR &presentInfo) override;

    angle::Result waitForSerialWithUserTimeout(vk::Context *context,
                                               Serial serial,
                                               uint64_t timeout,
                                               VkResult *result) override;

    angle::Result checkCompletedCommands(Context *context) override;

    angle::Result flushOutsideRPCommands(Context *context,
                                         CommandBufferHelper **outsideRPCommands) override;
    angle::Result flushRenderPassCommands(Context *context,
                                          const RenderPass &renderPass,
                                          CommandBufferHelper **renderPassCommands) override;

    Serial getLastSubmittedQueueSerial() const override;
    Serial getLastCompletedQueueSerial() const override;
    Serial getCurrentQueueSerial() const override;

    angle::Result queueSubmit(Context *context,
                              egl::ContextPriority contextPriority,
                              const VkSubmitInfo &submitInfo,
                              const Fence *fence,
                              Serial submitQueueSerial);

  private:
    angle::Result releaseToCommandBatch(Context *context,
                                        PrimaryCommandBuffer &&commandBuffer,
                                        CommandPool *commandPool,
                                        CommandBatch *batch);
    angle::Result retireFinishedCommands(Context *context, size_t finishedCount);
    angle::Result ensurePrimaryCommandBufferValid(Context *context);

    bool allInFlightCommandsAreAfterSerial(Serial serial) const;

    GarbageQueue mGarbageQueue;
    std::vector<CommandBatch> mInFlightCommands;

    // Keeps a free list of reusable primary command buffers.
    PrimaryCommandBuffer mPrimaryCommands;
    PersistentCommandPool mPrimaryCommandPool;

    // Queue serial management.
    AtomicSerialFactory mQueueSerialFactory;
    Serial mLastCompletedQueueSerial;
    Serial mLastSubmittedQueueSerial;
    Serial mCurrentQueueSerial;

    // Devices queues.
    DeviceQueueMap mQueues;
};

// CommandProcessor is used to dispatch work to the GPU when the asyncCommandQueue feature is
// enabled. Issuing the |destroy| command will cause the worker thread to clean up it's resources
// and shut down. This command is sent when the renderer instance shuts down. Tasks are defined by
// the CommandQueue interface.

class CommandProcessor : public Context, public CommandQueueInterface
{
  public:
    CommandProcessor(RendererVk *renderer);
    ~CommandProcessor() override;

    // Used by main thread to wait for worker thread to complete all outstanding work.
    // TODO(jmadill): Make private. b/172704839
    angle::Result waitForWorkComplete(Context *context);
    angle::Result finishAllWork(Context *context);

    VkResult getLastPresentResult(VkSwapchainKHR swapchain)
    {
        return getLastAndClearPresentResult(swapchain);
    }

    // vk::Context
    void handleError(VkResult result,
                     const char *file,
                     const char *function,
                     unsigned int line) override;

    // CommandQueueInterface
    angle::Result init(Context *context, const DeviceQueueMap &queueMap) override;

    void destroy(RendererVk *renderer) override;

    void handleDeviceLost(RendererVk *renderer) override;

    angle::Result finishToSerial(Context *context, Serial finishSerial, uint64_t timeout) override;

    Serial reserveSubmitSerial() override;

    angle::Result submitFrame(Context *context,
                              egl::ContextPriority priority,
                              const std::vector<VkSemaphore> &waitSemaphores,
                              const std::vector<VkPipelineStageFlags> &waitSemaphoreStageMasks,
                              const Semaphore *signalSemaphore,
                              Shared<Fence> &&sharedFence,
                              GarbageList &&currentGarbage,
                              CommandPool *commandPool,
                              Serial submitQueueSerial) override;

    angle::Result queueSubmitOneOff(Context *context,
                                    egl::ContextPriority contextPriority,
                                    VkCommandBuffer commandBufferHandle,
                                    const Fence *fence,
                                    Serial submitQueueSerial) override;
    VkResult queuePresent(egl::ContextPriority contextPriority,
                          const VkPresentInfoKHR &presentInfo) override;

    angle::Result waitForSerialWithUserTimeout(vk::Context *context,
                                               Serial serial,
                                               uint64_t timeout,
                                               VkResult *result) override;

    angle::Result checkCompletedCommands(Context *context) override;

    angle::Result flushOutsideRPCommands(Context *context,
                                         CommandBufferHelper **outsideRPCommands) override;
    angle::Result flushRenderPassCommands(Context *context,
                                          const RenderPass &renderPass,
                                          CommandBufferHelper **renderPassCommands) override;

    Serial getLastSubmittedQueueSerial() const override;
    Serial getLastCompletedQueueSerial() const override;
    Serial getCurrentQueueSerial() const override;

  private:
    class Task;

    bool hasPendingError() const
    {
        std::lock_guard<std::mutex> queueLock(mErrorMutex);
        return !mErrors.empty();
    }
    angle::Result checkAndPopPendingError(Context *errorHandlingContext);

    // Entry point for command processor thread, calls processTasksImpl to do the
    // work. called by Rendererinitialization on main thread
    void processTasks(const DeviceQueueMap &queueMap);

    // Called asynchronously from main thread to queue work that is then processed by the worker
    // thread
    void queueCommand(Task &&task);

    // Command processor thread, called by processTasks. The loop waits for work to
    // be submitted from a separate thread.
    angle::Result processTasksImpl(bool *exitThread);

    // Command processor thread, process a task
    angle::Result processTask(Task *task);

    VkResult getLastAndClearPresentResult(VkSwapchainKHR swapchain);
    VkResult present(egl::ContextPriority priority, const VkPresentInfoKHR &presentInfo);

    std::queue<Task> mTasks;
    mutable std::mutex mWorkerMutex;
    // Signal worker thread when work is available
    std::condition_variable mWorkAvailableCondition;
    // Signal main thread when all work completed
    mutable std::condition_variable mWorkerIdleCondition;
    // Track worker thread Idle state for assertion purposes
    bool mWorkerThreadIdle;
    // Command pool to allocate processor thread primary command buffers from
    CommandPool mCommandPool;
    CommandQueue mCommandQueue;

    mutable std::mutex mQueueSerialMutex;

    mutable std::mutex mErrorMutex;
    std::queue<Error> mErrors;

    // Track present info
    std::mutex mSwapchainStatusMutex;
    std::condition_variable mSwapchainStatusCondition;
    std::map<VkSwapchainKHR, VkResult> mSwapchainStatus;

    // Command queue worker thread.
    std::thread mTaskThread;
};

}  // namespace vk

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_COMMAND_PROCESSOR_H_
