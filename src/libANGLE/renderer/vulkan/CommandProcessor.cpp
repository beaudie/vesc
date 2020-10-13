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
    else
    {
        submitInfo->signalSemaphoreCount = 0;
        submitInfo->pSignalSemaphores    = nullptr;
    }
}
}  // namespace

namespace vk
{
// CommandProcessorTask implementation
void CommandProcessorTask::initProcessCommands(CommandBufferHelper *commandBuffer)
{
    mTask          = vk::CustomTask::ProcessCommands;
    mCommandBuffer = commandBuffer;
}

void CommandProcessorTask::initPresent(egl::ContextPriority priority, VkPresentInfoKHR presentInfo)
{
    mTask        = vk::CustomTask::Present;
    mPresentInfo = presentInfo;
    mPriority    = priority;
}

void CommandProcessorTask::initFinishToSerial(Serial serial)
{
    ASSERT(serial.valid());
    mTask   = vk::CustomTask::FinishToSerial;
    mSerial = serial;
}

void CommandProcessorTask::initFlushAndQueueSubmit(
    const std::vector<VkSemaphore> waitSemaphores,
    const std::vector<VkPipelineStageFlags> waitSemaphoreStageMasks,
    const vk::Semaphore *semaphore,
    egl::ContextPriority priority,
    vk::GarbageList &&currentGarbage,
    vk::ResourceUseList &&currentResources)
{
    mTask                    = vk::CustomTask::FlushAndQueueSubmit;
    mWaitSemaphores          = waitSemaphores;
    mWaitSemaphoreStageMasks = waitSemaphoreStageMasks;
    mSemaphore               = semaphore;
    mGarbage                 = std::move(currentGarbage);
    mResourceUseList         = std::move(currentResources);
    mPriority                = priority;
}

void CommandProcessorTask::initOneOffQueueSubmit(VkCommandBuffer oneOffCommandBufferVk,
                                                 egl::ContextPriority priority,
                                                 const vk::Fence *fence)
{
    mTask                  = vk::CustomTask::OneOffQueueSubmit;
    mOneOffCommandBufferVk = oneOffCommandBufferVk;
    mOneOffFence           = fence;
    mPriority              = priority;
}

CommandProcessorTask &CommandProcessorTask::operator=(CommandProcessorTask &&rhs)
{
    if (this == &rhs)
    {
        return *this;
    }

    mCommandBuffer = rhs.mCommandBuffer;
    std::swap(mTask, rhs.mTask);
    std::swap(mWaitSemaphores, rhs.mWaitSemaphores);
    std::swap(mWaitSemaphoreStageMasks, rhs.mWaitSemaphoreStageMasks);
    mSemaphore   = rhs.mSemaphore;
    mOneOffFence = rhs.mOneOffFence;
    std::swap(mGarbage, rhs.mGarbage);
    std::swap(mSerial, rhs.mSerial);
    std::swap(mPresentInfo, rhs.mPresentInfo);
    std::swap(mPriority, rhs.mPriority);
    std::swap(mResourceUseList, rhs.mResourceUseList);
    mOneOffCommandBufferVk = rhs.mOneOffCommandBufferVk;

    // clear rhs now that everything has moved.
    rhs.mCommandBuffer         = nullptr;
    rhs.mOneOffCommandBufferVk = VK_NULL_HANDLE;
    rhs.mSemaphore             = nullptr;
    rhs.mOneOffFence           = nullptr;

    return *this;
}

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

// CommandTaskProcessor implementation.
CommandTaskProcessor::CommandTaskProcessor(RendererVk *renderer) : vk::Context(renderer)
{
    std::lock_guard<std::mutex> queueLock(mErrorMutex);
    resetError();
}

CommandTaskProcessor::~CommandTaskProcessor() = default;

void CommandTaskProcessor::destroy(VkDevice device)
{
    mPrimaryCommandPool.destroy(device);
    ASSERT(mInFlightCommands.empty() && mGarbageQueue.empty());
}

angle::Result CommandTaskProcessor::init(std::thread::id threadId)
{
    mThreadId = threadId;

    // Initialize the command pool now that we know the queue family index.
    ANGLE_TRY(mPrimaryCommandPool.init(this, mRenderer->getQueueFamilyIndex()));

    return angle::Result::Continue;
}

angle::Result CommandTaskProcessor::checkCompletedCommands(RendererVk *renderer)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandTaskProcessor::checkCompletedCommands");
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

        WARN() << "completed serial: " << batch.serial.getValue();
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

angle::Result CommandTaskProcessor::releaseToCommandBatch(RendererVk *renderer,
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

void CommandTaskProcessor::clearAllGarbage(RendererVk *renderer)
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

angle::Result CommandTaskProcessor::allocatePrimaryCommandBuffer(
    vk::PrimaryCommandBuffer *commandBufferOut)
{
    return mPrimaryCommandPool.allocate(this, commandBufferOut);
}

angle::Result CommandTaskProcessor::releasePrimaryCommandBuffer(
    vk::PrimaryCommandBuffer &&commandBuffer)
{
    ASSERT(mPrimaryCommandPool.valid());
    return mPrimaryCommandPool.collect(this, std::move(commandBuffer));
}

void CommandTaskProcessor::handleDeviceLost()
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

void CommandTaskProcessor::handleError(VkResult errorCode,
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
    // TODO: https://issuetracker.google.com/issues/170311829 - Do we need to support multiple
    // errors? Assert there are no previous errors for now.
    ASSERT(mErrorDetails.mErrorCode == VK_SUCCESS);
    mErrorDetails.mErrorCode = errorCode;
    mErrorDetails.mFile      = file;
    mErrorDetails.mFunction  = function;
    mErrorDetails.mLine      = line;
}

bool CommandTaskProcessor::isRobustResourceInitEnabled() const
{
    // Unused for worker thread, just return false.
    return false;
}

// If there are any inflight commands worker will look for fence that corresponds to the request
// serial or the last available fence and wait on that fence. Will then do necessary cleanup work.
// This can cause the worker thread to block.
// TODO: https://issuetracker.google.com/issues/170312581 - A more optimal solution might be to do
// the wait in CommandProcessor rather than the worker thread. That would require protecting access
// to mInFlightCommands
angle::Result CommandTaskProcessor::finishToSerial(RendererVk *renderer, Serial serial)
{
    uint64_t timeout = renderer->getMaxFenceWaitTimeNs();

    if (mInFlightCommands.empty())
    {
        // No outstanding work, nothing to wait for.
        return angle::Result::Continue;
    }

    // Find the first batch with serial equal to or bigger than given serial (note that
    // the batch serials are unique, otherwise upper-bound would have been necessary).
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
    VkDevice device = getDevice();
    ANGLE_VK_TRY(this, batch.fence.get().wait(device, timeout));

    // Clean up finished batches.
    return checkCompletedCommands(renderer);
}

VkResult CommandTaskProcessor::present(VkQueue queue, const VkPresentInfoKHR &presentInfo)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "vkQueuePresentKHR");
    return vkQueuePresentKHR(queue, &presentInfo);
}

angle::Result CommandTaskProcessor::submitFrame(RendererVk *renderer,
                                                VkQueue queue,
                                                const VkSubmitInfo &submitInfo,
                                                const vk::Shared<vk::Fence> &sharedFence,
                                                vk::GarbageList *currentGarbage,
                                                vk::CommandPool *commandPool,
                                                vk::PrimaryCommandBuffer &&commandBuffer,
                                                const Serial &queueSerial)
{
    ASSERT(std::this_thread::get_id() == mThreadId);
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandTaskProcessor::submitFrame");

    VkDevice device = getDevice();

    vk::DeviceScoped<vk::CommandBatch> scopedBatch(device);
    vk::CommandBatch &batch = scopedBatch.get();
    batch.fence.copy(device, sharedFence);
    batch.serial = queueSerial;

    ANGLE_TRY(queueSubmit(this, queue, submitInfo, &batch.fence.get()));

    if (!currentGarbage->empty())
    {
        mGarbageQueue.emplace_back(std::move(*currentGarbage), queueSerial);
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

vk::Shared<vk::Fence> CommandTaskProcessor::getLastSubmittedFenceWithLock(VkDevice device) const
{
    vk::Shared<vk::Fence> fence;
    // Note: this must be called when the work queue is empty and while holding mWorkerMutex to
    // ensure that worker isn't touching mInFlightCommands
    if (!mInFlightCommands.empty())
    {
        fence.copy(device, mInFlightCommands.back().fence);
    }

    return fence;
}

angle::Result CommandTaskProcessor::queueSubmit(vk::Context *context,
                                                VkQueue queue,
                                                const VkSubmitInfo &submitInfo,
                                                const vk::Fence *fence)
{
    ASSERT(std::this_thread::get_id() == mThreadId);

    // Don't need a QueueMutex since all queue accesses are serialized through the worker.
    VkFence handle = fence ? fence->getHandle() : VK_NULL_HANDLE;
    ANGLE_VK_TRY(context, vkQueueSubmit(queue, 1, &submitInfo, handle));

    return angle::Result::Continue;
}

void CommandTaskProcessor::resetError()
{
    mErrorDetails.mErrorCode = VK_SUCCESS;
    mErrorDetails.mFile      = nullptr;
    mErrorDetails.mFunction  = nullptr;
    mErrorDetails.mLine      = 0;
}

vk::Error CommandTaskProcessor::getAndClearError()
{
    std::lock_guard<std::mutex> queueLock(mErrorMutex);
    vk::Error tmpError = mErrorDetails;
    resetError();
    return tmpError;
}

CommandProcessor::CommandProcessor(RendererVk *renderer)
    : mWorkerThreadIdle(false),
      mRenderer(renderer),
      mCommandWorkQueue(renderer),
      mCommandProcessorLastSubmittedSerial(mQueueSerialFactory.generate()),
      mCommandProcessorCurrentQueueSerial(mQueueSerialFactory.generate())
{}

void CommandProcessor::queueCommand(vk::CommandProcessorTask *command)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandProcessor::queueCommand");
    {
        // Grab the worker mutex so that we put things on the queue in the same order as we give out
        // serials.
        std::lock_guard<std::mutex> queueLock(mWorkerMutex);

        if (command->mTask == vk::CustomTask::FlushAndQueueSubmit ||
            command->mTask == vk::CustomTask::OneOffQueueSubmit)
        {
            std::lock_guard<std::mutex> lock(mCommandProcessorQueueSerialMutex);
            // Flush submits work, so give it the current serial and generate a new one.
            command->mSerial                     = mCommandProcessorCurrentQueueSerial;
            mCommandProcessorLastSubmittedSerial = mCommandProcessorCurrentQueueSerial;
            mCommandProcessorCurrentQueueSerial  = mQueueSerialFactory.generate();

            command->mResourceUseList.releaseResourceUsesAndUpdateSerials(command->mSerial);
        }

        WARN() << "Queue task: " << command->mTask << ", serial: " << command->mSerial.getValue();
        mCommandsQueue.emplace(std::move(*command));
        mWorkAvailableCondition.notify_one();
    }

    if (mRenderer->getFeatures().enableParallelCommandProcessing.enabled)
    {
        return;
    }

    // parallel command processing disabled so wait for work to complete.
    waitForWorkComplete();
}

void CommandProcessor::processTasks()
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
            // TODO: https://issuetracker.google.com/issues/170311829 - follow-up on error handling
            // ContextVk::commandProcessorSyncErrorsAndQueueCommand and WindowSurfaceVk::destroy
            // do error processing, is anything required here? Don't think so, mostly need to
            // continue the worker thread until it's been told to exit.
            UNREACHABLE();
        }
    }
}

angle::Result CommandProcessor::processCommandProcessorTasksImpl(bool *exitThread)
{
    // Initialization prior to work thread loop
    ANGLE_TRY(mCommandWorkQueue.init(std::this_thread::get_id()));
    // Allocate and begin primary command buffer
    ANGLE_TRY(mCommandWorkQueue.allocatePrimaryCommandBuffer(&mPrimaryCommandBuffer));
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo         = nullptr;

    ANGLE_VK_TRY(&mCommandWorkQueue, mPrimaryCommandBuffer.begin(beginInfo));

    while (true)
    {
        std::unique_lock<std::mutex> lock(mWorkerMutex);
        if (mCommandsQueue.empty())
        {
            mWorkerThreadIdle = true;
            mWorkerIdleCondition.notify_all();
            // Only wake if notified and command queue is not empty
            mWorkAvailableCondition.wait(lock, [this] { return !mCommandsQueue.empty(); });
        }
        mWorkerThreadIdle = false;
        vk::CommandProcessorTask task(std::move(mCommandsQueue.front()));
        mCommandsQueue.pop();
        lock.unlock();
        WARN() << "process task: " << task.mTask << ", serial: " << task.mSerial.getValue();
        switch (task.mTask)
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
            default:
                ANGLE_TRY(processTask(task));
                break;
        }
    }

    UNREACHABLE();
    return angle::Result::Stop;
}

angle::Result CommandProcessor::processTask(vk::CommandProcessorTask &task)
{
    switch (task.mTask)
    {
        case vk::CustomTask::FlushAndQueueSubmit:
        {
            // End command buffer
            ANGLE_VK_TRY(&mCommandWorkQueue, mPrimaryCommandBuffer.end());
            // 1. Create submitInfo
            VkSubmitInfo submitInfo = {};
            InitializeSubmitInfo(&submitInfo, mPrimaryCommandBuffer, task.mWaitSemaphores,
                                 &task.mWaitSemaphoreStageMasks, task.mSemaphore);

            // 2. Get shared submit fence. It's possible there are other users of this fence that
            // must wait for the work to be submitted before waiting on the fence. Reset the fence
            // immediately so we are sure to get a fresh one next time.
            vk::Shared<vk::Fence> fence;
            ANGLE_TRY(mRenderer->getNextSubmitFence(&fence, true));

            // 3. Call submitFrame()
            ANGLE_TRY(mCommandWorkQueue.submitFrame(
                mRenderer, mRenderer->getVkQueue(task.mPriority), submitInfo, fence, &task.mGarbage,
                &mCommandPool, std::move(mPrimaryCommandBuffer), task.mSerial));
            // 4. Allocate & begin new primary command buffer
            ANGLE_TRY(mCommandWorkQueue.allocatePrimaryCommandBuffer(&mPrimaryCommandBuffer));

            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            beginInfo.pInheritanceInfo         = nullptr;
            ANGLE_VK_TRY(&mCommandWorkQueue, mPrimaryCommandBuffer.begin(beginInfo));

            // Free this local reference
            mRenderer->resetSharedFence(&fence);

            ASSERT(task.mGarbage.empty());
            break;
        }
        case vk::CustomTask::OneOffQueueSubmit:
        {
            VkSubmitInfo submitInfo = {};
            submitInfo.sType        = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            if (task.mOneOffCommandBufferVk != VK_NULL_HANDLE)
            {
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers    = &task.mOneOffCommandBufferVk;
            }

            // TODO: https://issuetracker.google.com/issues/170328907 - vkQueueSubmit should be
            // owned by CommandTaskProcessor to ensure proper synchronization
            ANGLE_TRY(mCommandWorkQueue.queueSubmit(&mCommandWorkQueue,
                                                    mRenderer->getVkQueue(task.mPriority),
                                                    submitInfo, task.mOneOffFence));
            ANGLE_TRY(mCommandWorkQueue.checkCompletedCommands(mRenderer));
            break;
        }
        case vk::CustomTask::FinishToSerial:
        {
            ANGLE_TRY(mCommandWorkQueue.finishToSerial(mRenderer, task.mSerial));
            break;
        }
        case vk::CustomTask::Present:
        {
            VkResult result =
                mCommandWorkQueue.present(mRenderer->getVkQueue(task.mPriority), task.mPresentInfo);
            if (ANGLE_UNLIKELY(result != VK_SUCCESS))
            {
                // Save the error so that we can handle it (e.g. VK_OUT_OF_DATE)
                // Don't leave processing loop, don't consider errors from present to be fatal.
                // TODO: https://issuetracker.google.com/issues/170329600 - This needs to improve to
                // properly parallelize present
                mCommandWorkQueue.handleError(result, __FILE__, __FUNCTION__, __LINE__);
            }
            break;
        }
        case vk::CustomTask::ProcessCommands:
        {
            ASSERT(!task.mCommandBuffer->empty());
            // TODO: Add renderPass to task info
            ANGLE_TRY(task.mCommandBuffer->flushToPrimary(mRenderer->getFeatures(),
                                                          &mPrimaryCommandBuffer, nullptr));
            ASSERT(task.mCommandBuffer->empty());
            // TODO: recycle the command buffer, do we need the caller's ContextVk?
            // task.mCommandBuffer->releaseToContextQueue(mCommandWorkQueue);
            break;
        }
        case vk::CustomTask::ClearAllGarbage:
        {
            mCommandWorkQueue.clearAllGarbage(mRenderer);
            break;
        }
        default:
            UNREACHABLE();
            break;
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
}

void CommandProcessor::shutdown(std::thread *commandProcessorThread)
{
    finishToSerial(Serial::Infinite());
    vk::CommandProcessorTask clearAllGarbage;
    clearAllGarbage.initTask(vk::CustomTask::ClearAllGarbage);
    queueCommand(&clearAllGarbage);
    vk::CommandProcessorTask endTask;
    endTask.initTask(vk::CustomTask::Exit);
    queueCommand(&endTask);
    waitForWorkComplete();
    if (commandProcessorThread->joinable())
    {
        commandProcessorThread->join();
    }
}

// Return the fence for the last submit. This may mean waiting on the worker to process tasks to
// actually get to the last submit
vk::Shared<vk::Fence> CommandProcessor::getLastSubmittedFence() const
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandProcessor::getLastSubmittedFence");
    std::unique_lock<std::mutex> lock(mWorkerMutex);
    mWorkerIdleCondition.wait(lock,
                              [this] { return (mCommandsQueue.empty() && mWorkerThreadIdle); });

    // Worker thread is idle and command queue is empty so good to continue

    return mCommandWorkQueue.getLastSubmittedFenceWithLock(mRenderer->getDevice());
}

Serial CommandProcessor::getLastSubmittedSerial()
{
    std::lock_guard<std::mutex> lock(mCommandProcessorQueueSerialMutex);
    return mCommandProcessorLastSubmittedSerial;
}

Serial CommandProcessor::getCurrentQueueSerial()
{
    std::lock_guard<std::mutex> lock(mCommandProcessorQueueSerialMutex);
    return mCommandProcessorCurrentQueueSerial;
}

// Wait until all commands up to and including serial have been processed
void CommandProcessor::finishToSerial(Serial serial)
{
    vk::CommandProcessorTask finishToSerial;
    finishToSerial.initFinishToSerial(serial);
    queueCommand(&finishToSerial);

    // Wait until the worker is idle. At that point we know that the finishToSerial command has
    // completed executing, including any associated state cleanup.
    waitForWorkComplete();
}

void CommandProcessor::handleDeviceLost()
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandProcessor::handleDeviceLost");
    std::unique_lock<std::mutex> lock(mWorkerMutex);
    mWorkerIdleCondition.wait(lock,
                              [this] { return (mCommandsQueue.empty() && mWorkerThreadIdle); });

    // Worker thread is idle and command queue is empty so good to continue
    mCommandWorkQueue.handleDeviceLost();
}

void CommandProcessor::clearAllGarbage()
{
    // Issue command to CommandProcessor to clear garbage.
    vk::CommandProcessorTask clearAllGarbage;
    clearAllGarbage.initTask(vk::CustomTask::ClearAllGarbage);
    queueCommand(&clearAllGarbage);

    waitForWorkComplete();
}

}  // namespace rx
