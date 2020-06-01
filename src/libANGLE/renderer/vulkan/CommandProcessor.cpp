//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CommandProcessor.cpp:
//    Implements the class methods for CommandProcessor.
//

#include "libANGLE/renderer/vulkan/CommandProcessor.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/trace.h"

namespace rx
{
namespace
{
constexpr size_t kInFlightCommandsLimit = 100u;

void InitializeSubmitInfo(VkSubmitInfo *submitInfo,
                          const vk::PrimaryCommandBuffer &commandBuffer,
                          const std::vector<VkSemaphore> &waitSemaphores,
                          std::vector<VkPipelineStageFlags> *waitSemaphoreStageMasks,
                          const vk::Semaphore *signalSemaphore)
{
    // Verify that the submitInfo has been zero'd out.
    ASSERT(submitInfo->signalSemaphoreCount == 0);

    submitInfo->sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo->commandBufferCount = commandBuffer.valid() ? 1 : 0;
    submitInfo->pCommandBuffers    = commandBuffer.ptr();

    if (waitSemaphoreStageMasks->size() < waitSemaphores.size())
    {
        waitSemaphoreStageMasks->resize(waitSemaphores.size(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    }

    submitInfo->waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
    submitInfo->pWaitSemaphores    = waitSemaphores.data();
    submitInfo->pWaitDstStageMask  = waitSemaphoreStageMasks->data();

    if (signalSemaphore)
    {
        submitInfo->signalSemaphoreCount = 1;
        submitInfo->pSignalSemaphores    = signalSemaphore->ptr();
    }
}
}  // namespace

namespace vk
{
// CommandBatch implementation.
CommandBatch::CommandBatch() = default;

CommandBatch::~CommandBatch() = default;

CommandBatch::CommandBatch(CommandBatch &&other)
{
    *this = std::move(other);
}

CommandBatch &CommandBatch::operator=(CommandBatch &&other)
{
    std::swap(primaryCommands, other.primaryCommands);
    std::swap(commandPool, other.commandPool);
    std::swap(fence, other.fence);
    std::swap(serial, other.serial);
    return *this;
}

void CommandBatch::destroy(VkDevice device)
{
    primaryCommands.destroy(device);
    commandPool.destroy(device);
    fence.reset(device);
}
}  // namespace vk

// CommandWorkQueue implementation.
CommandWorkQueue::CommandWorkQueue(RendererVk *renderer) : vk::Context(renderer)
{
    std::lock_guard<std::mutex> queueLock(mErrorMutex);
    resetError();
}

CommandWorkQueue::~CommandWorkQueue() = default;

void CommandWorkQueue::destroy(VkDevice device)
{
    mPrimaryCommandPool.destroy(device);
    ASSERT(mInFlightCommands.empty() && mGarbageQueue.empty());
}

angle::Result CommandWorkQueue::init()
{
    // Initialize the command pool now that we know the queue family index.
    ANGLE_TRY(mPrimaryCommandPool.init(this, mRenderer->getQueueFamilyIndex()));

    return angle::Result::Continue;
}

angle::Result CommandWorkQueue::checkCompletedCommands(RendererVk *renderer)
{
    VkDevice device = renderer->getDevice();

    int finishedCount = 0;

    for (vk::CommandBatch &batch : mInFlightCommands)
    {
        VkResult result = batch.fence.get().getStatus(device);
        if (result == VK_NOT_READY)
        {
            break;
        }
        ANGLE_VK_TRY(this, result);

        renderer->onCompletedSerial(batch.serial);

        renderer->resetSharedFence(&batch.fence);
        ANGLE_TRACE_EVENT0("gpu.angle", "command buffer recycling");
        batch.commandPool.destroy(device);
        ANGLE_TRY(releasePrimaryCommandBuffer(std::move(batch.primaryCommands)));
        ++finishedCount;
    }

    if (finishedCount > 0)
    {
        auto beginIter = mInFlightCommands.begin();
        mInFlightCommands.erase(beginIter, beginIter + finishedCount);
    }

    Serial lastCompleted = renderer->getLastCompletedQueueSerial();

    size_t freeIndex = 0;
    for (; freeIndex < mGarbageQueue.size(); ++freeIndex)
    {
        vk::GarbageAndSerial &garbageList = mGarbageQueue[freeIndex];
        if (garbageList.getSerial() < lastCompleted)
        {
            for (vk::GarbageObject &garbage : garbageList.get())
            {
                garbage.destroy(renderer);
            }
        }
        else
        {
            break;
        }
    }

    // Remove the entries from the garbage list - they should be ready to go.
    if (freeIndex > 0)
    {
        mGarbageQueue.erase(mGarbageQueue.begin(), mGarbageQueue.begin() + freeIndex);
    }

    return angle::Result::Continue;
}

angle::Result CommandWorkQueue::releaseToCommandBatch(RendererVk *renderer,
                                                      vk::PrimaryCommandBuffer &&commandBuffer,
                                                      vk::CommandPool *commandPool,
                                                      vk::CommandBatch *batch)
{
    batch->primaryCommands = std::move(commandBuffer);

    if (commandPool->valid())
    {
        batch->commandPool = std::move(*commandPool);
        // Recreate CommandPool
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags                   = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        poolInfo.queueFamilyIndex        = renderer->getQueueFamilyIndex();

        ANGLE_VK_TRY(this, commandPool->init(renderer->getDevice(), poolInfo));
    }

    return angle::Result::Continue;
}

void CommandWorkQueue::clearAllGarbage(RendererVk *renderer)
{
    for (vk::GarbageAndSerial &garbageList : mGarbageQueue)
    {
        for (vk::GarbageObject &garbage : garbageList.get())
        {
            garbage.destroy(renderer);
        }
    }
    mGarbageQueue.clear();
}

angle::Result CommandWorkQueue::allocatePrimaryCommandBuffer(
    vk::PrimaryCommandBuffer *commandBufferOut)
{
    return mPrimaryCommandPool.allocate(this, commandBufferOut);
}

angle::Result CommandWorkQueue::releasePrimaryCommandBuffer(
    vk::PrimaryCommandBuffer &&commandBuffer)
{
    ASSERT(mPrimaryCommandPool.valid());
    ANGLE_TRY(mPrimaryCommandPool.collect(this, std::move(commandBuffer)));

    return angle::Result::Continue;
}

void CommandWorkQueue::handleDeviceLost()
{
    VkDevice device = mRenderer->getDevice();

    for (vk::CommandBatch &batch : mInFlightCommands)
    {
        // On device loss we need to wait for fence to be signaled before destroying it
        VkResult status = batch.fence.get().wait(device, mRenderer->getMaxFenceWaitTimeNs());
        // If the wait times out, it is probably not possible to recover from lost device
        ASSERT(status == VK_SUCCESS || status == VK_ERROR_DEVICE_LOST);

        // On device lost, here simply destroy the CommandBuffer, it will be fully cleared later by
        // CommandPool::destroy
        batch.primaryCommands.destroy(device);

        batch.commandPool.destroy(device);
        batch.fence.reset(device);
    }
    mInFlightCommands.clear();
}

void CommandWorkQueue::handleError(VkResult errorCode,
                                   const char *file,
                                   const char *function,
                                   unsigned int line)
{
    ASSERT(errorCode != VK_SUCCESS);

    std::stringstream errorStream;
    errorStream << "Internal Vulkan error (" << errorCode << "): " << VulkanResultString(errorCode)
                << ".";

    if (errorCode == VK_ERROR_DEVICE_LOST)
    {
        WARN() << errorStream.str();
        handleDeviceLost();
    }

    std::lock_guard<std::mutex> queueLock(mErrorMutex);
    // TODO: Do we need to support multiple errors? Assert there are no previous errors for now.
    ASSERT(mErrorDetails.errorCode == VK_SUCCESS);
    mErrorDetails.errorCode = errorCode;
    mErrorDetails.file      = file;
    mErrorDetails.function  = function;
    mErrorDetails.line      = line;
}

VkDevice CommandWorkQueue::getDevice() const
{
    return mRenderer->getDevice();
}

bool CommandWorkQueue::isRobustResourceInitEnabled() const
{
    // Unused for worker thread, just return false.
    return false;
}

bool CommandWorkQueue::hasInFlightCommands() const
{
    return !mInFlightCommands.empty();
}

angle::Result CommandWorkQueue::finishToSerial(RendererVk *renderer, Serial serial)
{
    uint64_t timeout = renderer->getMaxFenceWaitTimeNs();
    if (mInFlightCommands.empty())
    {
        return angle::Result::Continue;
    }

    // Find the first batch with serial equal to or bigger than given serial (note that
    // the batch serials are unique, otherwise upper-bound would have been necessary).
    //
    // Note: we don't check for the exact serial, because it may belong to another context.  For
    // example, imagine the following submissions:
    //
    // - Context 1: Serial 1, Serial 3, Serial 5
    // - Context 2: Serial 2, Serial 4, Serial 6
    //
    // And imagine none of the submissions have finished yet.  Now if Context 2 asks for
    // finishToSerial(3), it will have no choice but to finish until Serial 4 instead.
    size_t batchIndex = mInFlightCommands.size() - 1;
    for (size_t i = 0; i < mInFlightCommands.size(); ++i)
    {
        if (mInFlightCommands[i].serial >= serial)
        {
            batchIndex = i;
            break;
        }
    }
    const vk::CommandBatch &batch = mInFlightCommands[batchIndex];

    // Wait for it finish
    VkDevice device = renderer->getDevice();
    VkResult status = batch.fence.get().wait(device, timeout);
    ANGLE_VK_TRY(this, status);

    // Clean up finished batches.
    angle::Result result = checkCompletedCommands(renderer);
    // printf("finishToSerial, mInFlightCommands.size() is %llu\n", mInFlightCommands.size());
    return result;
}

VkResult CommandWorkQueue::present(egl::ContextPriority priority,
                                   const VkPresentInfoKHR &presentInfo)
{
    return mRenderer->commandProcessorThreadQueuePresent(priority, presentInfo);
}

angle::Result CommandWorkQueue::submitFrame(RendererVk *renderer,
                                            egl::ContextPriority priority,
                                            const VkSubmitInfo &submitInfo,
                                            const vk::Shared<vk::Fence> &sharedFence,
                                            vk::GarbageList *currentGarbage,
                                            vk::CommandPool *commandPool,
                                            vk::PrimaryCommandBuffer &&commandBuffer)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandWorkQueue::submitFrame");
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags             = 0;

    VkDevice device = renderer->getDevice();

    vk::DeviceScoped<vk::CommandBatch> scopedBatch(device);
    vk::CommandBatch &batch = scopedBatch.get();
    batch.fence.copy(device, sharedFence);

    ANGLE_TRY(renderer->commandProcessorThreadQueueSubmit(this, priority, submitInfo,
                                                          &batch.fence.get(), &batch.serial));

    if (!currentGarbage->empty())
    {
        mGarbageQueue.emplace_back(std::move(*currentGarbage), batch.serial);
    }

    // Store the primary CommandBuffer and command pool used for secondary CommandBuffers
    // in the in-flight list.
    ANGLE_TRY(releaseToCommandBatch(renderer, std::move(commandBuffer), commandPool, &batch));

    mInFlightCommands.emplace_back(scopedBatch.release());

    ANGLE_TRY(checkCompletedCommands(renderer));

    // CPU should be throttled to avoid mInFlightCommands from growing too fast. Important for
    // off-screen scenarios.
    while (mInFlightCommands.size() > kInFlightCommandsLimit)
    {
        ANGLE_TRY(finishToSerial(renderer, mInFlightCommands[0].serial));
    }

    return angle::Result::Continue;
}

angle::Result CommandWorkQueue::queueWaitIdle(egl::ContextPriority priority)
{
    return mRenderer->queueWaitIdle(this, priority);
}

angle::Result CommandWorkQueue::deviceWaitIdle()
{
    return mRenderer->deviceWaitIdle(this);
}

vk::Shared<vk::Fence> CommandWorkQueue::getLastSubmittedFence(const VkDevice device) const
{
    vk::Shared<vk::Fence> fence;
    if (!mInFlightCommands.empty())
    {
        fence.copy(device, mInFlightCommands.back().fence);
    }

    return fence;
}

void CommandWorkQueue::resetError()
{
    mErrorDetails.errorCode = VK_SUCCESS;
    mErrorDetails.file      = nullptr;
    mErrorDetails.function  = nullptr;
    mErrorDetails.line      = 0;
}

vk::ErrorDetails CommandWorkQueue::getAndClearError()
{
    std::lock_guard<std::mutex> queueLock(mErrorMutex);
    vk::ErrorDetails tmpError = mErrorDetails;
    resetError();
    return tmpError;
}

CommandProcessor::CommandProcessor(RendererVk *renderer)
    : mWorkerThreadIdle(true), mRenderer(renderer), mCommandWorkQueue(renderer)
{}

void CommandProcessor::queueCommand(vk::CommandProcessorTask *command)
{
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "CommandProcessor::queueCommand");
        std::lock_guard<std::mutex> queueLock(mWorkerMutex);
        mCommandsQueue.emplace(std::move(*command));
        mWorkAvailableCondition.notify_one();
    }

    if (mRenderer->getFeatures().enableParallelCommandProcessing.enabled)
    {
        return;
    }

    // parallel threads disabled so wait for work to complete before continuing.
    waitForWorkComplete();
}

void CommandProcessor::processCommandProcessorTasks()
{

    while (true)
    {
        bool exitThread      = false;
        angle::Result result = processCommandProcessorTasksImpl(&exitThread);
        if (exitThread)
        {
            // We are doing a controlled exit of the thread, break out of the while loop.
            break;
        }
        if (result != angle::Result::Continue)
        {
            // TODO?: Do something here if we get an error?
            // ContextVk::syncAnyErrorAndQueueCommandToProcessorThread and WindowSurfaceVk::destroy
            // do error processing, is anything required here? Don't think so, mostly need to
            // continue the worker thread until it's been told to exit.
        }
    }
}

angle::Result CommandProcessor::processCommandProcessorTasksImpl(bool *exitThread)
{
    // Initialization prior to work thread loop
    ANGLE_TRY(mCommandWorkQueue.init());
    // Allocate and begin primary command buffer
    ANGLE_TRY(mCommandWorkQueue.allocatePrimaryCommandBuffer(&mPrimaryCommandBuffer));
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo         = nullptr;

    ANGLE_VK_TRY(mCommandWorkQueue.getPointer(), mPrimaryCommandBuffer.begin(beginInfo));

    while (true)
    {
        std::unique_lock<std::mutex> lock(mWorkerMutex);
        mWorkerIdleCondition.notify_one();
        mWorkerThreadIdle = true;
        // Only wake if notified and command queue is not empty
        mWorkAvailableCondition.wait(lock, [this] { return !mCommandsQueue.empty(); });
        mWorkerThreadIdle = false;
        vk::CommandProcessorTask task(std::move(mCommandsQueue.front()));
        mCommandsQueue.pop();
        lock.unlock();
        switch (task.mWorkerCommand)
        {
            case vk::CustomTask::Exit:
            {
                *exitThread = true;
                // Shutting down so cleanup
                mCommandWorkQueue.destroy(mRenderer->getDevice());
                mCommandPool.destroy(mRenderer->getDevice());
                mPrimaryCommandBuffer.destroy(mRenderer->getDevice());
                mWorkerThreadIdle = true;
                mWorkerIdleCondition.notify_one();
                return angle::Result::Continue;
            }
            case vk::CustomTask::Flush:
            {
                // End command buffer
                ANGLE_VK_TRY(mCommandWorkQueue.getPointer(), mPrimaryCommandBuffer.end());
                // 1. Create submitInfo
                VkSubmitInfo submitInfo = {};
                InitializeSubmitInfo(&submitInfo, mPrimaryCommandBuffer, task.mWaitSemaphores,
                                     &task.mWaitSemaphoreStageMasks, task.mSemaphore);
                // 2. Call submitFrame()
                ANGLE_TRY(mRenderer->getNextSubmitFence(&mFence));
                ANGLE_TRY(mCommandWorkQueue.submitFrame(
                    mRenderer, task.mPriority, submitInfo, mFence, &task.mCurrentGarbage,
                    &mCommandPool, std::move(mPrimaryCommandBuffer)));
                // 3. Allocate & begin new primary command buffer
                ANGLE_TRY(mCommandWorkQueue.allocatePrimaryCommandBuffer(&mPrimaryCommandBuffer));
                ANGLE_VK_TRY(mCommandWorkQueue.getPointer(),
                             mPrimaryCommandBuffer.begin(beginInfo));
                // TODO: This is hacky to prevent double fence use.
                //  Also not sure I'm getting fence reuse doing this.
                mRenderer->resetSharedFence(&mFence);
                mRenderer->resetNextSharedFence();
                ASSERT(task.mCurrentGarbage.empty());
                break;
            }
            case vk::CustomTask::FinishToSerial:
            {
                ANGLE_TRY(mCommandWorkQueue.finishToSerial(mRenderer, task.mSerial));
                break;
            }
            case vk::CustomTask::Present:
            {
                VkResult result = mCommandWorkQueue.present(task.mPriority, task.mPresentInfo);
                if (ANGLE_UNLIKELY(result != VK_SUCCESS))
                {
                    // Save the error so that we can handle it (e.g. VK_OUT_OF_DATE)
                    // Don't leave processing loop, don't consider errors from present to be fatal.
                    // TODO: This needs to improve to properly parallelize present
                    mCommandWorkQueue.getPointer()->handleError(result, __FILE__, __FUNCTION__,
                                                                __LINE__);
                }
                break;
            }
            case vk::CustomTask::DeviceWaitIdle:
            {
                ANGLE_TRY(mCommandWorkQueue.deviceWaitIdle());
                break;
            }
            case vk::CustomTask::QueueWaitIdle:
            {
                ANGLE_TRY(mCommandWorkQueue.queueWaitIdle(task.mPriority));
                break;
            }
            case vk::CustomTask::FlushToPrimary:
            {
                ASSERT(!task.mCommandBuffer->empty());
                ANGLE_TRY(
                    task.mCommandBuffer->flushToPrimary(task.mContextVk, &mPrimaryCommandBuffer));
                ASSERT(task.mCommandBuffer->empty());
                task.mCommandBuffer->releaseToContextQueue(task.mContextVk);
                break;
            }
            default:
                UNREACHABLE();
                break;
        }
    }

    UNREACHABLE();
    return angle::Result::Stop;
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

void CommandProcessor::shutdown(std::thread *commandProcessorThread)
{
    waitForWorkComplete();
    vk::CommandProcessorTask endTask(vk::CustomTask::Exit);
    queueCommand(&endTask);
    if (commandProcessorThread->joinable())
    {
        commandProcessorThread->join();
    }
}

vk::Shared<vk::Fence> CommandProcessor::getLastSubmittedFence() const
{
    return mCommandWorkQueue.getLastSubmittedFence(mRenderer->getDevice());
}

}  // namespace rx
