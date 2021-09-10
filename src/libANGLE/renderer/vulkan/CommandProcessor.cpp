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
namespace vk
{
namespace
{
// SVDT: Reduced "rx::vk::kInFlightCommandsLimit" from "100" to "50".
#if SVDT_GLOBAL_CHANGES
constexpr size_t kInFlightCommandsLimit = 50u;
#else
constexpr size_t kInFlightCommandsLimit = 100u;
#endif
#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
constexpr size_t kActiveGCThreshold     = 20u;
#endif
constexpr bool kOutputVmaStatsString    = false;

#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
void InitializeSubmitInfo(VkSubmitInfo *submitInfo,
                          const vk::PrimaryCommandBuffer &commandBuffer,
                          const std::vector<VkSemaphore> &waitSemaphores,
                          const std::vector<VkPipelineStageFlags> &waitSemaphoreStageMasks,
                          const vk::Semaphore *signalSemaphore)
{
    // Verify that the submitInfo has been zero'd out.
    ASSERT(submitInfo->signalSemaphoreCount == 0);
    ASSERT(waitSemaphores.size() == waitSemaphoreStageMasks.size());
    submitInfo->sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo->commandBufferCount = commandBuffer.valid() ? 1 : 0;
    submitInfo->pCommandBuffers    = commandBuffer.ptr();
    submitInfo->waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
    submitInfo->pWaitSemaphores    = waitSemaphores.data();
    submitInfo->pWaitDstStageMask  = waitSemaphoreStageMasks.data();

    if (signalSemaphore)
    {
        submitInfo->signalSemaphoreCount = 1;
        submitInfo->pSignalSemaphores    = signalSemaphore->ptr();
    }
}

bool CommandsHaveValidOrdering(const std::vector<vk::CommandBatch> &commands)
{
    Serial currentSerial;
    for (const vk::CommandBatch &commandBatch : commands)
    {
        if (commandBatch.serial <= currentSerial)
        {
            return false;
        }
        currentSerial = commandBatch.serial;
    }

    return true;
}
#endif
}  // namespace

angle::Result FenceRecycler::newSharedFence(vk::Context *context,
                                            vk::Shared<vk::Fence> *sharedFenceOut)
{
    bool gotRecycledFence = false;
    vk::Fence fence;
    {
        std::lock_guard<std::mutex> lock(mMutex);
        if (!mRecyler.empty())
        {
            mRecyler.fetch(&fence);
            gotRecycledFence = true;
        }
    }

    VkDevice device(context->getDevice());
    if (gotRecycledFence)
    {
        ANGLE_VK_TRY(context, fence.reset(device));
    }
    else
    {
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags             = 0;
        ANGLE_VK_TRY(context, fence.init(device, fenceCreateInfo));
    }
    sharedFenceOut->assign(device, std::move(fence));
    return angle::Result::Continue;
}

void FenceRecycler::destroy(vk::Context *context)
{
    std::lock_guard<std::mutex> lock(mMutex);
    mRecyler.destroy(context->getDevice());
}

// CommandProcessorTask implementation
void CommandProcessorTask::initTask()
{
    mTask                        = CustomTask::Invalid;
// SVDT: Fixed CRASH when "asyncCommandQueue" feature is enabled.
#if !SVDT_GLOBAL_CHANGES
    mRenderPass                  = nullptr;
#endif
    mCommandBuffer               = nullptr;
#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    mSemaphore                   = nullptr;
    mOneOffFence                 = nullptr;
#endif
#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    mSubmitItemIndex             = -1;
#endif
    mPresentInfo                 = {};
    mPresentInfo.pResults        = nullptr;
    mPresentInfo.pSwapchains     = nullptr;
    mPresentInfo.pImageIndices   = nullptr;
    mPresentInfo.pNext           = nullptr;
    mPresentInfo.pWaitSemaphores = nullptr;
#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    mOneOffCommandBufferVk       = VK_NULL_HANDLE;
#endif
    mPriority                    = egl::ContextPriority::Medium;
    mHasProtectedContent         = false;
}

void CommandProcessorTask::initProcessCommands(bool hasProtectedContent,
                                               CommandBufferHelper *commandBuffer,
                                               const RenderPass *renderPass)
{
    mTask                = CustomTask::ProcessCommands;
    mCommandBuffer       = commandBuffer;
// SVDT: Fixed CRASH when "asyncCommandQueue" feature is enabled.
#if !SVDT_GLOBAL_CHANGES
    mRenderPass          = renderPass;
#endif
    mHasProtectedContent = hasProtectedContent;
// SVDT: Fixed CRASH when "asyncCommandQueue" feature is enabled.
#if SVDT_GLOBAL_CHANGES
    // Do not store "renderPass" pointer, because it may point to temp memory inside cache data
    if (renderPass)
    {
        ASSERT(renderPass->valid());
        ASSERT(!mRenderPass.valid());
        mRenderPass.setHandle(renderPass->getHandle());
    }
#endif
}

void CommandProcessorTask::copyPresentInfo(const VkPresentInfoKHR &other)
{
    if (other.sType == 0)
    {
        return;
    }

    mPresentInfo.sType = other.sType;
    mPresentInfo.pNext = other.pNext;

    if (other.swapchainCount > 0)
    {
        ASSERT(other.swapchainCount == 1);
        mPresentInfo.swapchainCount = 1;
        mSwapchain                  = other.pSwapchains[0];
        mPresentInfo.pSwapchains    = &mSwapchain;
        mImageIndex                 = other.pImageIndices[0];
        mPresentInfo.pImageIndices  = &mImageIndex;
    }

    if (other.waitSemaphoreCount > 0)
    {
        ASSERT(other.waitSemaphoreCount == 1);
        mPresentInfo.waitSemaphoreCount = 1;
        mWaitSemaphore                  = other.pWaitSemaphores[0];
        mPresentInfo.pWaitSemaphores    = &mWaitSemaphore;
    }

    mPresentInfo.pResults = other.pResults;

    void *pNext = const_cast<void *>(other.pNext);
    while (pNext != nullptr)
    {
        VkStructureType sType = *reinterpret_cast<VkStructureType *>(pNext);
        switch (sType)
        {
            case VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR:
            {
                const VkPresentRegionsKHR *presentRegions =
                    reinterpret_cast<VkPresentRegionsKHR *>(pNext);
                mPresentRegion = *presentRegions->pRegions;
                mRects.resize(mPresentRegion.rectangleCount);
                for (uint32_t i = 0; i < mPresentRegion.rectangleCount; i++)
                {
                    mRects[i] = presentRegions->pRegions->pRectangles[i];
                }
                mPresentRegion.pRectangles = mRects.data();

                mPresentRegions.sType          = VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR;
                mPresentRegions.pNext          = presentRegions->pNext;
                mPresentRegions.swapchainCount = 1;
                mPresentRegions.pRegions       = &mPresentRegion;
                mPresentInfo.pNext             = &mPresentRegions;
                pNext                          = const_cast<void *>(presentRegions->pNext);
                break;
            }
            default:
                ERR() << "Unknown sType: " << sType << " in VkPresentInfoKHR.pNext chain";
                UNREACHABLE();
                break;
        }
    }
}

void CommandProcessorTask::initPresent(egl::ContextPriority priority,
                                       const VkPresentInfoKHR &presentInfo)
{
    mTask     = CustomTask::Present;
    mPriority = priority;
    copyPresentInfo(presentInfo);
}

#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
void CommandProcessorTask::initFlushAndQueueSubmit(
        GarbageList &&currentGarbage,
        uint32_t submitItemIndex)
{
    mTask            = CustomTask::FlushAndQueueSubmit;
    mGarbage         = std::move(currentGarbage);
    mSubmitItemIndex = submitItemIndex;
}

void CommandProcessorTask::initOneOffQueueSubmit(uint32_t submitItemIndex)
{
    mTask            = CustomTask::OneOffQueueSubmit;
    mSubmitItemIndex = submitItemIndex;
}

#elif !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
void CommandProcessorTask::initFinishToSerial(Serial serial)
{
    // Note: sometimes the serial is not valid and that's okay, the finish will early exit in the
    // TaskProcessor::finishToSerial
    mTask   = CustomTask::FinishToSerial;
    mSerial = serial;
}

void CommandProcessorTask::initFlushAndQueueSubmit(
    const std::vector<VkSemaphore> &waitSemaphores,
    const std::vector<VkPipelineStageFlags> &waitSemaphoreStageMasks,
    const Semaphore *semaphore,
    bool hasProtectedContent,
    egl::ContextPriority priority,
    GarbageList &&currentGarbage,
    Serial submitQueueSerial)
{
    mTask                    = CustomTask::FlushAndQueueSubmit;
    mWaitSemaphores          = waitSemaphores;
    mWaitSemaphoreStageMasks = waitSemaphoreStageMasks;
    mSemaphore               = semaphore;
    mGarbage                 = std::move(currentGarbage);
    mPriority                = priority;
    mHasProtectedContent     = hasProtectedContent;
    mSerial                  = submitQueueSerial;
}

void CommandProcessorTask::initOneOffQueueSubmit(VkCommandBuffer commandBufferHandle,
                                                 bool hasProtectedContent,
                                                 egl::ContextPriority priority,
                                                 const Fence *fence,
                                                 Serial submitQueueSerial)
{
    mTask                  = CustomTask::OneOffQueueSubmit;
    mOneOffCommandBufferVk = commandBufferHandle;
    mOneOffFence           = fence;
    mPriority              = priority;
    mHasProtectedContent   = hasProtectedContent;
    mSerial                = submitQueueSerial;
}
#endif // !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2

CommandProcessorTask &CommandProcessorTask::operator=(CommandProcessorTask &&rhs)
{
    if (this == &rhs)
    {
        return *this;
    }

    std::swap(mRenderPass, rhs.mRenderPass);
    std::swap(mCommandBuffer, rhs.mCommandBuffer);
    std::swap(mTask, rhs.mTask);
#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    std::swap(mWaitSemaphores, rhs.mWaitSemaphores);
    std::swap(mWaitSemaphoreStageMasks, rhs.mWaitSemaphoreStageMasks);
    std::swap(mSemaphore, rhs.mSemaphore);
    std::swap(mOneOffFence, rhs.mOneOffFence);
#endif
    std::swap(mGarbage, rhs.mGarbage);
#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    std::swap(mSubmitItemIndex, rhs.mSubmitItemIndex);
#endif
#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    std::swap(mSerial, rhs.mSerial);
#endif
    std::swap(mPriority, rhs.mPriority);
    std::swap(mHasProtectedContent, rhs.mHasProtectedContent);
#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    std::swap(mOneOffCommandBufferVk, rhs.mOneOffCommandBufferVk);
#endif

    copyPresentInfo(rhs.mPresentInfo);

    // clear rhs now that everything has moved.
    rhs.initTask();

    return *this;
}

#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
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
// SVDT: Removed dead code related to the uninitialized/unused CommandPool.
#if !SVDT_GLOBAL_CHANGES
    std::swap(commandPool, other.commandPool);
#endif
    std::swap(fence, other.fence);
    std::swap(serial, other.serial);
    std::swap(hasProtectedContent, other.hasProtectedContent);
    return *this;
}

void CommandBatch::destroy(VkDevice device)
{
    primaryCommands.destroy(device);
// SVDT: Removed dead code related to the uninitialized/unused CommandPool.
#if !SVDT_GLOBAL_CHANGES
    commandPool.destroy(device);
#endif
    fence.reset(device);
    hasProtectedContent = false;
}
#endif // !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2

// CommandProcessor implementation.
void CommandProcessor::handleError(VkResult errorCode,
                                   const char *file,
                                   const char *function,
                                   unsigned int line)
{
    ASSERT(errorCode != VK_SUCCESS);

// SVDT: Updates and fixes in the "rx::vk::CommandProcessor" class.
#if SVDT_GLOBAL_CHANGES
    WARN() << "Internal Vulkan error (" << errorCode << "): " << VulkanResultString(errorCode)
           << ". (" << function << "(): " << line << ")";
#else
    std::stringstream errorStream;
    errorStream << "Internal Vulkan error (" << errorCode << "): " << VulkanResultString(errorCode)
                << ".";
#endif

    if (errorCode == VK_ERROR_DEVICE_LOST)
    {
// SVDT: Updates and fixes in the "rx::vk::CommandProcessor" class.
#if !SVDT_GLOBAL_CHANGES
        WARN() << errorStream.str();
#endif
        handleDeviceLost(mRenderer);
    }

    std::lock_guard<std::mutex> queueLock(mErrorMutex);
    Error error = {errorCode, file, function, line};
    mErrors.emplace(error);
}

CommandProcessor::CommandProcessor(RendererVk *renderer)
    : Context(renderer), mWorkerThreadIdle(false)
{
    std::lock_guard<std::mutex> queueLock(mErrorMutex);
    while (!mErrors.empty())
    {
        mErrors.pop();
    }
}

CommandProcessor::~CommandProcessor() = default;

angle::Result CommandProcessor::checkAndPopPendingError(Context *errorHandlingContext)
{
    std::lock_guard<std::mutex> queueLock(mErrorMutex);
    if (mErrors.empty())
    {
        return angle::Result::Continue;
    }
    else
    {
        Error err = mErrors.front();
        mErrors.pop();
        errorHandlingContext->handleError(err.errorCode, err.file, err.function, err.line);
        return angle::Result::Stop;
    }
}

void CommandProcessor::queueCommand(CommandProcessorTask &&task)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandProcessor::queueCommand");
    // Grab the worker mutex so that we put things on the queue in the same order as we give out
    // serials.
    std::lock_guard<std::mutex> queueLock(mWorkerMutex);

    mTasks.emplace(std::move(task));
    mWorkAvailableCondition.notify_one();
}

void CommandProcessor::processTasks()
{
    while (true)
    {
        bool exitThread      = false;
        angle::Result result = processTasksImpl(&exitThread);
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

angle::Result CommandProcessor::processTasksImpl(bool *exitThread)
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(mWorkerMutex);
        if (mTasks.empty())
        {
            mWorkerThreadIdle = true;
            mWorkerIdleCondition.notify_all();
            // Only wake if notified and command queue is not empty
            mWorkAvailableCondition.wait(lock, [this] { return !mTasks.empty(); });
        }
        mWorkerThreadIdle = false;
        CommandProcessorTask task(std::move(mTasks.front()));
        mTasks.pop();
        lock.unlock();

        ANGLE_TRY(processTask(&task));
        if (task.getTaskCommand() == CustomTask::Exit)
        {

            *exitThread = true;
            lock.lock();
            mWorkerThreadIdle = true;
            mWorkerIdleCondition.notify_one();
            return angle::Result::Continue;
        }
    }

    UNREACHABLE();
    return angle::Result::Stop;
}

angle::Result CommandProcessor::processTask(CommandProcessorTask *task)
{
    switch (task->getTaskCommand())
    {
        case CustomTask::Exit:
        {
            ANGLE_TRY(mCommandQueue.finishToSerial(this, Serial::Infinite(),
                                                   mRenderer->getMaxFenceWaitTimeNs()));
            // Shutting down so cleanup
            mCommandQueue.destroy(this);
// SVDT: Removed dead code related to the uninitialized/unused CommandPool.
#if !SVDT_GLOBAL_CHANGES
            mCommandPool.destroy(mRenderer->getDevice());
#endif
            break;
        }
        case CustomTask::FlushAndQueueSubmit:
        {
            ANGLE_TRACE_EVENT0("gpu.angle", "processTask::FlushAndQueueSubmit");
            // End command buffer

#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
            // Call doSubmitFrame() - see prepareNextSubmit()
            ANGLE_TRY(mCommandQueue.doSubmitFrame(
                this, std::move(task->getGarbage()), task->getSubmitItemIndex()));
#else
            // Call submitFrame()
            ANGLE_TRY(mCommandQueue.submitFrame(
                this, task->hasProtectedContent(), task->getPriority(), task->getWaitSemaphores(),
                task->getWaitSemaphoreStageMasks(), task->getSemaphore(),
// SVDT: Removed dead code related to the uninitialized/unused CommandPool.
#if SVDT_GLOBAL_CHANGES
                std::move(task->getGarbage()), task->getQueueSerial()));
#else
                std::move(task->getGarbage()), &mCommandPool, task->getQueueSerial()));
#endif
#endif  // SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2

            ASSERT(task->getGarbage().empty());
            break;
        }
        case CustomTask::OneOffQueueSubmit:
        {
            ANGLE_TRACE_EVENT0("gpu.angle", "processTask::OneOffQueueSubmit");

#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
            // Call doQueueSubmitOneOff() - see prepareNextSubmit()
            ANGLE_TRY(mCommandQueue.doQueueSubmitOneOff(this, task->getSubmitItemIndex()));
#else
            ANGLE_TRY(mCommandQueue.queueSubmitOneOff(
                this, task->hasProtectedContent(), task->getPriority(),
                task->getOneOffCommandBufferVk(), task->getOneOffFence(),
                SubmitPolicy::EnsureSubmitted, task->getQueueSerial()));
            ANGLE_TRY(mCommandQueue.checkCompletedCommands(this));
#endif
            break;
        }
#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
        case CustomTask::FinishToSerial:
        {
            ANGLE_TRY(mCommandQueue.finishToSerial(this, task->getQueueSerial(),
                                                   mRenderer->getMaxFenceWaitTimeNs()));
            break;
        }
#endif
        case CustomTask::Present:
        {
#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
            ANGLE_TRACE_EVENT0("gpu.angle", "processTask::Present");
            mCommandQueue.doQueuePresent(this, task->getPriority(), task->getPresentInfo(),
                [this](const VkPresentInfoKHR &presentInfo, VkResult presentResult)
                {
                    // Verify that we are presenting one and only one swapchain
                    ASSERT(presentInfo.swapchainCount == 1);
                    ASSERT(presentInfo.pResults == nullptr);
                    {
                        std::lock_guard<std::mutex> lock(mSwapchainStatusMutex);
                        mSwapchainStatus[presentInfo.pSwapchains[0]] = presentResult;
                    }
                    mSwapchainStatusCondition.notify_all();
                });

#else // SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
            VkResult result = present(task->getPriority(), task->getPresentInfo());
            if (ANGLE_UNLIKELY(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR))
            {
                // We get to ignore these as they are not fatal
            }
            else if (ANGLE_UNLIKELY(result != VK_SUCCESS))
            {
                // Save the error so that we can handle it.
                // Don't leave processing loop, don't consider errors from present to be fatal.
                // TODO: https://issuetracker.google.com/issues/170329600 - This needs to improve to
                // properly parallelize present
                handleError(result, __FILE__, __FUNCTION__, __LINE__);
            }
#endif // SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
            break;
        }
        case CustomTask::ProcessCommands:
        {
            ASSERT(!task->getCommandBuffer()->empty());

            CommandBufferHelper *commandBuffer = task->getCommandBuffer();
// SVDT: Fixed CRASH when "asyncCommandQueue" feature is enabled.
#if SVDT_GLOBAL_CHANGES
            if (task->getRenderPass().valid())
#else
            if (task->getRenderPass())
#endif
            {
                ANGLE_TRY(mCommandQueue.flushRenderPassCommands(
// SVDT: Fixed CRASH when "asyncCommandQueue" feature is enabled.
#if SVDT_GLOBAL_CHANGES
                    this, task->hasProtectedContent(), task->getRenderPass(), &commandBuffer));
#else
                    this, task->hasProtectedContent(), *task->getRenderPass(), &commandBuffer));
#endif
            }
            else
            {
                ANGLE_TRY(mCommandQueue.flushOutsideRPCommands(this, task->hasProtectedContent(),
                                                               &commandBuffer));
            }
            ASSERT(task->getCommandBuffer()->empty());
            mRenderer->recycleCommandBufferHelper(task->getCommandBuffer());
            break;
        }
#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
        case CustomTask::CheckCompletedCommands:
        {
            ANGLE_TRY(mCommandQueue.checkCompletedCommands(this));
            break;
        }
#endif
#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
        case CustomTask::CleanupAllGarbage:
        {
            ANGLE_TRY(mCommandQueue.cleanupAllGarbage(this));
            break;
        }
#endif
        default:
            UNREACHABLE();
            break;
    }

    return angle::Result::Continue;
}

angle::Result CommandProcessor::checkCompletedCommands(Context *context)
{
    ANGLE_TRY(checkAndPopPendingError(context));

#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    return mCommandQueue.checkCompletedCommands(context);
#else
    CommandProcessorTask checkCompletedTask;
    checkCompletedTask.initTask(CustomTask::CheckCompletedCommands);
    queueCommand(std::move(checkCompletedTask));

    return angle::Result::Continue;
#endif
}

angle::Result CommandProcessor::waitForWorkComplete(Context *context)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandProcessor::waitForWorkComplete");
    std::unique_lock<std::mutex> lock(mWorkerMutex);
    mWorkerIdleCondition.wait(lock, [this] { return (mTasks.empty() && mWorkerThreadIdle); });
    // Worker thread is idle and command queue is empty so good to continue

#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2 && SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
    mCommandQueue.waitSubmitThreadIdle();
#endif

    // Sync any errors to the context
    bool shouldStop = hasPendingError();
    while (hasPendingError())
    {
        (void)checkAndPopPendingError(context);
    }
    return shouldStop ? angle::Result::Stop : angle::Result::Continue;
}

angle::Result CommandProcessor::init(Context *context, const DeviceQueueMap &queueMap)
{
    ANGLE_TRY(mCommandQueue.init(context, queueMap));

    mTaskThread = std::thread(&CommandProcessor::processTasks, this);

    return angle::Result::Continue;
}

void CommandProcessor::destroy(Context *context)
{
    CommandProcessorTask endTask;
    endTask.initTask(CustomTask::Exit);
    queueCommand(std::move(endTask));
    (void)waitForWorkComplete(context);
    if (mTaskThread.joinable())
    {
        mTaskThread.join();
    }
}

Serial CommandProcessor::getLastCompletedQueueSerial() const
{
#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    std::lock_guard<std::mutex> lock(mQueueSerialMutex);
#endif
    return mCommandQueue.getLastCompletedQueueSerial();
}

Serial CommandProcessor::getLastSubmittedQueueSerial() const
{
#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    std::lock_guard<std::mutex> lock(mQueueSerialMutex);
#endif
    return mCommandQueue.getLastSubmittedQueueSerial();
}

Serial CommandProcessor::getCurrentQueueSerial() const
{
#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    std::lock_guard<std::mutex> lock(mQueueSerialMutex);
#endif
    return mCommandQueue.getCurrentQueueSerial();
}

Serial CommandProcessor::reserveSubmitSerial()
{
#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    std::lock_guard<std::mutex> lock(mQueueSerialMutex);
#endif
    return mCommandQueue.reserveSubmitSerial();
}

// Wait until all commands up to and including serial have been processed
angle::Result CommandProcessor::finishToSerial(Context *context, Serial serial, uint64_t timeout)
{
#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    return mCommandQueue.finishToSerial(context, serial, timeout);
#else
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandProcessor::finishToSerial");

// SVDT: Updates and fixes in the "rx::vk::CommandProcessor" class.
#if !SVDT_GLOBAL_CHANGES
    ANGLE_TRY(checkAndPopPendingError(context));
#endif

    CommandProcessorTask task;
    task.initFinishToSerial(serial);
    queueCommand(std::move(task));

    // Wait until the worker is idle. At that point we know that the finishToSerial command has
    // completed executing, including any associated state cleanup.
    return waitForWorkComplete(context);
#endif
}

void CommandProcessor::handleDeviceLost(RendererVk *renderer)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandProcessor::handleDeviceLost");
    std::unique_lock<std::mutex> lock(mWorkerMutex);
    mWorkerIdleCondition.wait(lock, [this] { return (mTasks.empty() && mWorkerThreadIdle); });

    // Worker thread is idle and command queue is empty so good to continue
    mCommandQueue.handleDeviceLost(renderer);
}

angle::Result CommandProcessor::finishAllWork(Context *context)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandProcessor::finishAllWork");
    // Wait for GPU work to finish
    return finishToSerial(context, Serial::Infinite(), mRenderer->getMaxFenceWaitTimeNs());
}

VkResult CommandProcessor::getLastAndClearPresentResult(VkSwapchainKHR swapchain)
{
    std::unique_lock<std::mutex> lock(mSwapchainStatusMutex);
    if (mSwapchainStatus.find(swapchain) == mSwapchainStatus.end())
    {
        // Wake when required swapchain status becomes available
        mSwapchainStatusCondition.wait(lock, [this, swapchain] {
            return mSwapchainStatus.find(swapchain) != mSwapchainStatus.end();
        });
    }
    VkResult result = mSwapchainStatus[swapchain];
    mSwapchainStatus.erase(swapchain);
    return result;
}

#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
VkResult CommandProcessor::present(egl::ContextPriority priority,
                                   const VkPresentInfoKHR &presentInfo)
{
    std::lock_guard<std::mutex> lock(mSwapchainStatusMutex);
    ANGLE_TRACE_EVENT0("gpu.angle", "vkQueuePresentKHR");
    VkResult result = mCommandQueue.queuePresent(priority, presentInfo);

    // Verify that we are presenting one and only one swapchain
    ASSERT(presentInfo.swapchainCount == 1);
    ASSERT(presentInfo.pResults == nullptr);
    mSwapchainStatus[presentInfo.pSwapchains[0]] = result;

    mSwapchainStatusCondition.notify_all();

    return result;
}
#endif // !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2

angle::Result CommandProcessor::submitFrame(
    Context *context,
    bool hasProtectedContent,
    egl::ContextPriority priority,
    const std::vector<VkSemaphore> &waitSemaphores,
    const std::vector<VkPipelineStageFlags> &waitSemaphoreStageMasks,
    const Semaphore *signalSemaphore,
    GarbageList &&currentGarbage,
// SVDT: Removed dead code related to the uninitialized/unused CommandPool.
#if !SVDT_GLOBAL_CHANGES
    CommandPool *commandPool,
#endif
    Serial submitQueueSerial)
{
    ANGLE_TRY(checkAndPopPendingError(context));

    CommandProcessorTask task;
#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    CommandQueue2::QueueItemIndex itemIndex = -1;
    ANGLE_TRY(mCommandQueue.prepareNextSubmit(context, hasProtectedContent, priority,
                                              waitSemaphores, waitSemaphoreStageMasks.data(),
                                              signalSemaphore, VK_NULL_HANDLE, nullptr,
                                              submitQueueSerial, &itemIndex));
    task.initFlushAndQueueSubmit(std::move(currentGarbage), itemIndex);
#else
    task.initFlushAndQueueSubmit(waitSemaphores, waitSemaphoreStageMasks, signalSemaphore,
                                 hasProtectedContent, priority, std::move(currentGarbage),
                                 submitQueueSerial);
#endif

    queueCommand(std::move(task));

    return angle::Result::Continue;
}

angle::Result CommandProcessor::queueSubmitOneOff(Context *context,
                                                  bool hasProtectedContent,
                                                  egl::ContextPriority contextPriority,
                                                  VkCommandBuffer commandBufferHandle,
                                                  const Fence *fence,
                                                  SubmitPolicy submitPolicy,
                                                  Serial submitQueueSerial)
{
    ANGLE_TRY(checkAndPopPendingError(context));

    CommandProcessorTask task;
#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    CommandQueue2::QueueItemIndex itemIndex = -1;
    ANGLE_TRY(mCommandQueue.prepareNextSubmit(context, hasProtectedContent, contextPriority,
                                              nullptr, nullptr,
                                              nullptr, commandBufferHandle, fence,
                                              submitQueueSerial, &itemIndex));
    task.initOneOffQueueSubmit(itemIndex);
#else
    task.initOneOffQueueSubmit(commandBufferHandle, hasProtectedContent, contextPriority, fence,
                               submitQueueSerial);
#endif
    queueCommand(std::move(task));
    if (submitPolicy == SubmitPolicy::EnsureSubmitted)
    {
        // Caller has synchronization requirement to have work in GPU pipe when returning from this
        // function.
        ANGLE_TRY(waitForWorkComplete(context));
    }

    return angle::Result::Continue;
}

VkResult CommandProcessor::queuePresent(egl::ContextPriority contextPriority,
                                        const VkPresentInfoKHR &presentInfo)
{
    CommandProcessorTask task;
    task.initPresent(contextPriority, presentInfo);

    ANGLE_TRACE_EVENT0("gpu.angle", "CommandProcessor::queuePresent");
    queueCommand(std::move(task));

    // Always return success, when we call acquireNextImage we'll check the return code. This
    // allows the app to continue working until we really need to know the return code from
    // present.
    return VK_SUCCESS;
}

angle::Result CommandProcessor::waitForSerialWithUserTimeout(vk::Context *context,
                                                             Serial serial,
                                                             uint64_t timeout,
                                                             VkResult *result)
{
#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    return mCommandQueue.waitForSerialWithUserTimeout(context, serial, timeout, result);
#else
    // If finishToSerial times out we generate an error. Therefore we a large timeout.
    // TODO: https://issuetracker.google.com/170312581 - Wait with timeout.
    return finishToSerial(context, serial, mRenderer->getMaxFenceWaitTimeNs());
#endif
}

angle::Result CommandProcessor::flushOutsideRPCommands(Context *context,
                                                       bool hasProtectedContent,
                                                       CommandBufferHelper **outsideRPCommands)
{
    ANGLE_TRY(checkAndPopPendingError(context));

    (*outsideRPCommands)->markClosed();
    CommandProcessorTask task;
    task.initProcessCommands(hasProtectedContent, *outsideRPCommands, nullptr);
    queueCommand(std::move(task));
    *outsideRPCommands = mRenderer->getCommandBufferHelper(false);

    return angle::Result::Continue;
}

angle::Result CommandProcessor::flushRenderPassCommands(Context *context,
                                                        bool hasProtectedContent,
                                                        const RenderPass &renderPass,
                                                        CommandBufferHelper **renderPassCommands)
{
    ANGLE_TRY(checkAndPopPendingError(context));

    (*renderPassCommands)->markClosed();
    CommandProcessorTask task;
    task.initProcessCommands(hasProtectedContent, *renderPassCommands, &renderPass);
    queueCommand(std::move(task));
    *renderPassCommands = mRenderer->getCommandBufferHelper(true);

    return angle::Result::Continue;
}

#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
angle::Result CommandProcessor::cleanupAllGarbage(Context *context)
{
    ANGLE_TRY(checkAndPopPendingError(context));

    CommandProcessorTask task;
    task.initTask(CustomTask::CleanupAllGarbage);
    queueCommand(std::move(task));

    return angle::Result::Continue;
}
#endif

#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
// CommandQueue implementation.
CommandQueue::CommandQueue() : mCurrentQueueSerial(mQueueSerialFactory.generate()) {}

CommandQueue::~CommandQueue() = default;

void CommandQueue::destroy(Context *context)
{
    // Force all commands to finish by flushing all queues.
    for (VkQueue queue : mQueueMap)
    {
        if (queue != VK_NULL_HANDLE)
        {
            vkQueueWaitIdle(queue);
        }
    }

    RendererVk *renderer = context->getRenderer();

    mLastCompletedQueueSerial = Serial::Infinite();
    (void)clearAllGarbage(renderer);

    mPrimaryCommands.destroy(renderer->getDevice());
    mPrimaryCommandPool.destroy(renderer->getDevice());

    if (mProtectedCommandPool.valid())
    {
        mProtectedCommands.destroy(renderer->getDevice());
        mProtectedCommandPool.destroy(renderer->getDevice());
    }

    mFenceRecycler.destroy(context);

    ASSERT(mInFlightCommands.empty() && mGarbageQueue.empty());
}

angle::Result CommandQueue::init(Context *context, const vk::DeviceQueueMap &queueMap)
{
    // Initialize the command pool now that we know the queue family index.
    ANGLE_TRY(mPrimaryCommandPool.init(context, false, queueMap.getIndex()));
    mQueueMap = queueMap;

    if (queueMap.isProtected())
    {
        ANGLE_TRY(mProtectedCommandPool.init(context, true, queueMap.getIndex()));
    }

    return angle::Result::Continue;
}

angle::Result CommandQueue::checkCompletedCommands(Context *context)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue::checkCompletedCommandsNoLock");
    RendererVk *renderer = context->getRenderer();
    VkDevice device      = renderer->getDevice();

    int finishedCount = 0;

    for (CommandBatch &batch : mInFlightCommands)
    {
        VkResult result = batch.fence.get().getStatus(device);
        if (result == VK_NOT_READY)
        {
            break;
        }
        ANGLE_VK_TRY(context, result);
        ++finishedCount;
    }

    if (finishedCount == 0)
    {
        return angle::Result::Continue;
    }

    return retireFinishedCommands(context, finishedCount);
}

angle::Result CommandQueue::retireFinishedCommands(Context *context, size_t finishedCount)
{
    ASSERT(finishedCount > 0);

    RendererVk *renderer = context->getRenderer();
// SVDT: Removed dead code related to the uninitialized/unused CommandPool.
#if !SVDT_GLOBAL_CHANGES
    VkDevice device      = renderer->getDevice();
#endif

    for (size_t commandIndex = 0; commandIndex < finishedCount; ++commandIndex)
    {
        CommandBatch &batch = mInFlightCommands[commandIndex];

        mLastCompletedQueueSerial = batch.serial;
        mFenceRecycler.resetSharedFence(&batch.fence);
        ANGLE_TRACE_EVENT0("gpu.angle", "command buffer recycling");
// SVDT: Removed dead code related to the uninitialized/unused CommandPool.
#if !SVDT_GLOBAL_CHANGES
        batch.commandPool.destroy(device);
#endif
        PersistentCommandPool &commandPool = getCommandPool(batch.hasProtectedContent);
        ANGLE_TRY(commandPool.collect(context, std::move(batch.primaryCommands)));
    }

    if (finishedCount > 0)
    {
        auto beginIter = mInFlightCommands.begin();
        mInFlightCommands.erase(beginIter, beginIter + finishedCount);
    }

    size_t freeIndex = 0;
    for (; freeIndex < mGarbageQueue.size(); ++freeIndex)
    {
        GarbageAndSerial &garbageList = mGarbageQueue[freeIndex];
        if (garbageList.getSerial() < mLastCompletedQueueSerial)
        {
            for (GarbageObject &garbage : garbageList.get())
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

// SVDT: Removed dead code related to the uninitialized/unused CommandPool.
#if !SVDT_GLOBAL_CHANGES
angle::Result CommandQueue::releaseToCommandBatch(Context *context,
                                                  bool hasProtectedContent,
                                                  PrimaryCommandBuffer &&commandBuffer,
                                                  CommandPool *commandPool,
                                                  CommandBatch *batch)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue::releaseToCommandBatch");

    RendererVk *renderer = context->getRenderer();
    VkDevice device      = renderer->getDevice();

    batch->primaryCommands = std::move(commandBuffer);

    if (commandPool->valid())
    {
        batch->commandPool = std::move(*commandPool);
        // Recreate CommandPool
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags                   = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        poolInfo.queueFamilyIndex        = mQueueMap.getIndex();
        if (hasProtectedContent)
        {
            poolInfo.flags |= VK_COMMAND_POOL_CREATE_PROTECTED_BIT;
        }
        batch->hasProtectedContent = hasProtectedContent;
        ANGLE_VK_TRY(context, commandPool->init(device, poolInfo));
    }

    return angle::Result::Continue;
}
#endif

void CommandQueue::clearAllGarbage(RendererVk *renderer)
{
    for (GarbageAndSerial &garbageList : mGarbageQueue)
    {
        for (GarbageObject &garbage : garbageList.get())
        {
            garbage.destroy(renderer);
        }
    }
    mGarbageQueue.clear();
}

void CommandQueue::handleDeviceLost(RendererVk *renderer)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue::handleDeviceLost");

    VkDevice device = renderer->getDevice();

    for (CommandBatch &batch : mInFlightCommands)
    {
        // On device loss we need to wait for fence to be signaled before destroying it
        VkResult status = batch.fence.get().wait(device, renderer->getMaxFenceWaitTimeNs());
        // If the wait times out, it is probably not possible to recover from lost device
        ASSERT(status == VK_SUCCESS || status == VK_ERROR_DEVICE_LOST);

        // On device lost, here simply destroy the CommandBuffer, it will fully cleared later
        // by CommandPool::destroy
        batch.primaryCommands.destroy(device);

// SVDT: Removed dead code related to the uninitialized/unused CommandPool.
#if !SVDT_GLOBAL_CHANGES
        batch.commandPool.destroy(device);
#endif
        batch.fence.reset(device);
    }
    mInFlightCommands.clear();
}

bool CommandQueue::allInFlightCommandsAreAfterSerial(Serial serial)
{
    return mInFlightCommands.empty() || mInFlightCommands[0].serial > serial;
}

angle::Result CommandQueue::finishToSerial(Context *context, Serial finishSerial, uint64_t timeout)
{
    if (mInFlightCommands.empty())
    {
        return angle::Result::Continue;
    }

    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue::finishToSerial");

    // Find the serial in the the list. The serials should be in order.
    ASSERT(CommandsHaveValidOrdering(mInFlightCommands));

    size_t finishedCount = 0;
    while (finishedCount < mInFlightCommands.size() &&
           mInFlightCommands[finishedCount].serial <= finishSerial)
    {
        finishedCount++;
    }

    if (finishedCount == 0)
    {
        return angle::Result::Continue;
    }

    const CommandBatch &batch = mInFlightCommands[finishedCount - 1];

    // Wait for it finish
    VkDevice device = context->getDevice();
    VkResult status = batch.fence.get().wait(device, timeout);

    ANGLE_VK_TRY(context, status);

    // Clean up finished batches.
    ANGLE_TRY(retireFinishedCommands(context, finishedCount));
    ASSERT(allInFlightCommandsAreAfterSerial(finishSerial));

    return angle::Result::Continue;
}

Serial CommandQueue::reserveSubmitSerial()
{
    Serial returnSerial = mCurrentQueueSerial;
    mCurrentQueueSerial = mQueueSerialFactory.generate();
    return returnSerial;
}

angle::Result CommandQueue::submitFrame(
    Context *context,
    bool hasProtectedContent,
    egl::ContextPriority priority,
    const std::vector<VkSemaphore> &waitSemaphores,
    const std::vector<VkPipelineStageFlags> &waitSemaphoreStageMasks,
    const Semaphore *signalSemaphore,
    GarbageList &&currentGarbage,
// SVDT: Removed dead code related to the uninitialized/unused CommandPool.
#if !SVDT_GLOBAL_CHANGES
    CommandPool *commandPool,
#endif
    Serial submitQueueSerial)
{
    // Start an empty primary buffer if we have an empty submit.
    PrimaryCommandBuffer &commandBuffer = getCommandBuffer(hasProtectedContent);
    ANGLE_TRY(ensurePrimaryCommandBufferValid(context, hasProtectedContent));
    ANGLE_VK_TRY(context, commandBuffer.end());

    VkSubmitInfo submitInfo = {};
    InitializeSubmitInfo(&submitInfo, commandBuffer, waitSemaphores, waitSemaphoreStageMasks,
                         signalSemaphore);

    VkProtectedSubmitInfo protectedSubmitInfo = {};
    if (hasProtectedContent)
    {
        protectedSubmitInfo.sType           = VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO;
        protectedSubmitInfo.pNext           = nullptr;
        protectedSubmitInfo.protectedSubmit = true;
        submitInfo.pNext                    = &protectedSubmitInfo;
    }

    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue::submitFrame");

    RendererVk *renderer = context->getRenderer();
    VkDevice device      = renderer->getDevice();

    DeviceScoped<CommandBatch> scopedBatch(device);
    CommandBatch &batch = scopedBatch.get();

    ANGLE_TRY(mFenceRecycler.newSharedFence(context, &batch.fence));
    batch.serial              = submitQueueSerial;
    batch.hasProtectedContent = hasProtectedContent;

    ANGLE_TRY(queueSubmit(context, priority, submitInfo, &batch.fence.get(), batch.serial));

    if (!currentGarbage.empty())
    {
        mGarbageQueue.emplace_back(std::move(currentGarbage), batch.serial);
    }

// SVDT: Removed dead code related to the uninitialized/unused CommandPool.
#if SVDT_GLOBAL_CHANGES
    // Store the primary/protected CommandBuffer in the in-flight list.
#else
    // Store the primary CommandBuffer and command pool used for secondary CommandBuffers
    // in the in-flight list.
#endif
    if (hasProtectedContent)
    {
// SVDT: Removed dead code related to the uninitialized/unused CommandPool.
#if SVDT_GLOBAL_CHANGES
        batch.primaryCommands = std::move(mProtectedCommands);
#else
        ANGLE_TRY(releaseToCommandBatch(context, hasProtectedContent, std::move(mProtectedCommands),
                                        commandPool, &batch));
#endif
    }
    else
    {
// SVDT: Removed dead code related to the uninitialized/unused CommandPool.
#if SVDT_GLOBAL_CHANGES
        batch.primaryCommands = std::move(mPrimaryCommands);
#else
        ANGLE_TRY(releaseToCommandBatch(context, hasProtectedContent, std::move(mPrimaryCommands),
                                        commandPool, &batch));
#endif
    }
    mInFlightCommands.emplace_back(scopedBatch.release());

    ANGLE_TRY(checkCompletedCommands(context));

    // CPU should be throttled to avoid mInFlightCommands from growing too fast. Important for
    // off-screen scenarios.
    if (mInFlightCommands.size() > kInFlightCommandsLimit)
    {
        size_t numCommandsToFinish = mInFlightCommands.size() - kInFlightCommandsLimit;
        Serial finishSerial        = mInFlightCommands[numCommandsToFinish].serial;
        ANGLE_TRY(finishToSerial(context, finishSerial, renderer->getMaxFenceWaitTimeNs()));
    }

    return angle::Result::Continue;
}

angle::Result CommandQueue::waitForSerialWithUserTimeout(vk::Context *context,
                                                         Serial serial,
                                                         uint64_t timeout,
                                                         VkResult *result)
{
    // No in-flight work. This indicates the serial is already complete.
    if (mInFlightCommands.empty())
    {
        *result = VK_SUCCESS;
        return angle::Result::Continue;
    }

    // Serial is already complete.
    if (serial < mInFlightCommands[0].serial)
    {
        *result = VK_SUCCESS;
        return angle::Result::Continue;
    }

    size_t batchIndex = 0;
    while (batchIndex != mInFlightCommands.size() && mInFlightCommands[batchIndex].serial < serial)
    {
        batchIndex++;
    }

    // Serial is not yet submitted. This is undefined behaviour, so we can do anything.
    if (batchIndex >= mInFlightCommands.size())
    {
        WARN() << "Waiting on an unsubmitted serial.";
        *result = VK_TIMEOUT;
        return angle::Result::Continue;
    }

    ASSERT(serial == mInFlightCommands[batchIndex].serial);

    vk::Fence &fence = mInFlightCommands[batchIndex].fence.get();
    ASSERT(fence.valid());
    *result = fence.wait(context->getDevice(), timeout);

    // Don't trigger an error on timeout.
    if (*result != VK_TIMEOUT)
    {
        ANGLE_VK_TRY(context, *result);
    }

    return angle::Result::Continue;
}

angle::Result CommandQueue::ensurePrimaryCommandBufferValid(Context *context,
                                                            bool hasProtectedContent)
{
    PersistentCommandPool &commandPool  = getCommandPool(hasProtectedContent);
    PrimaryCommandBuffer &commandBuffer = getCommandBuffer(hasProtectedContent);

    if (commandBuffer.valid())
    {
        return angle::Result::Continue;
    }

    ANGLE_TRY(commandPool.allocate(context, &commandBuffer));
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo         = nullptr;
    ANGLE_VK_TRY(context, commandBuffer.begin(beginInfo));

    return angle::Result::Continue;
}

angle::Result CommandQueue::flushOutsideRPCommands(Context *context,
                                                   bool hasProtectedContent,
                                                   CommandBufferHelper **outsideRPCommands)
{
    ANGLE_TRY(ensurePrimaryCommandBufferValid(context, hasProtectedContent));
    PrimaryCommandBuffer &commandBuffer = getCommandBuffer(hasProtectedContent);
    return (*outsideRPCommands)
        ->flushToPrimary(context->getRenderer()->getFeatures(), &commandBuffer, nullptr);
}

angle::Result CommandQueue::flushRenderPassCommands(Context *context,
                                                    bool hasProtectedContent,
                                                    const RenderPass &renderPass,
                                                    CommandBufferHelper **renderPassCommands)
{
    ANGLE_TRY(ensurePrimaryCommandBufferValid(context, hasProtectedContent));
    PrimaryCommandBuffer &commandBuffer = getCommandBuffer(hasProtectedContent);
    return (*renderPassCommands)
        ->flushToPrimary(context->getRenderer()->getFeatures(), &commandBuffer, &renderPass);
}

angle::Result CommandQueue::queueSubmitOneOff(Context *context,
                                              bool hasProtectedContent,
                                              egl::ContextPriority contextPriority,
                                              VkCommandBuffer commandBufferHandle,
                                              const Fence *fence,
                                              SubmitPolicy submitPolicy,
                                              Serial submitQueueSerial)
{
    VkSubmitInfo submitInfo = {};
    submitInfo.sType        = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkProtectedSubmitInfo protectedSubmitInfo = {};
    if (hasProtectedContent)
    {
        protectedSubmitInfo.sType           = VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO;
        protectedSubmitInfo.pNext           = nullptr;
        protectedSubmitInfo.protectedSubmit = true;
        submitInfo.pNext                    = &protectedSubmitInfo;
    }

    if (commandBufferHandle != VK_NULL_HANDLE)
    {
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &commandBufferHandle;
    }

    return queueSubmit(context, contextPriority, submitInfo, fence, submitQueueSerial);
}

angle::Result CommandQueue::queueSubmit(Context *context,
                                        egl::ContextPriority contextPriority,
                                        const VkSubmitInfo &submitInfo,
                                        const Fence *fence,
                                        Serial submitQueueSerial)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue::queueSubmit");

    RendererVk *renderer = context->getRenderer();

    if (kOutputVmaStatsString)
    {
        renderer->outputVmaStatString();
    }

    VkFence fenceHandle = fence ? fence->getHandle() : VK_NULL_HANDLE;
    VkQueue queue       = getQueue(contextPriority);
    ANGLE_VK_TRY(context, vkQueueSubmit(queue, 1, &submitInfo, fenceHandle));
    mLastSubmittedQueueSerial = submitQueueSerial;

    // Now that we've submitted work, clean up RendererVk garbage
    return renderer->cleanupGarbage(mLastCompletedQueueSerial);
}

VkResult CommandQueue::queuePresent(egl::ContextPriority contextPriority,
                                    const VkPresentInfoKHR &presentInfo)
{
    VkQueue queue = getQueue(contextPriority);
    return vkQueuePresentKHR(queue, &presentInfo);
}

Serial CommandQueue::getLastSubmittedQueueSerial() const
{
    return mLastSubmittedQueueSerial;
}

Serial CommandQueue::getLastCompletedQueueSerial() const
{
    return mLastCompletedQueueSerial;
}

Serial CommandQueue::getCurrentQueueSerial() const
{
    return mCurrentQueueSerial;
}
#endif // !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2

// QueuePriorities:
constexpr float kVulkanQueuePriorityLow    = 0.0;
constexpr float kVulkanQueuePriorityMedium = 0.4;
constexpr float kVulkanQueuePriorityHigh   = 1.0;

const float QueueFamily::kQueuePriorities[static_cast<uint32_t>(egl::ContextPriority::EnumCount)] =
    {kVulkanQueuePriorityMedium, kVulkanQueuePriorityHigh, kVulkanQueuePriorityLow};

egl::ContextPriority DeviceQueueMap::getDevicePriority(egl::ContextPriority priority) const
{
    return mPriorities[priority];
}

DeviceQueueMap::~DeviceQueueMap() {}

DeviceQueueMap &DeviceQueueMap::operator=(const DeviceQueueMap &other)
{
    ASSERT(this != &other);
    if ((this != &other) && other.valid())
    {
        mIndex                                    = other.mIndex;
        mIsProtected                              = other.mIsProtected;
        mPriorities[egl::ContextPriority::Low]    = other.mPriorities[egl::ContextPriority::Low];
        mPriorities[egl::ContextPriority::Medium] = other.mPriorities[egl::ContextPriority::Medium];
        mPriorities[egl::ContextPriority::High]   = other.mPriorities[egl::ContextPriority::High];
        *static_cast<angle::PackedEnumMap<egl::ContextPriority, VkQueue> *>(this) = other;
    }
    return *this;
}

void QueueFamily::getDeviceQueue(VkDevice device,
                                 bool makeProtected,
                                 uint32_t queueIndex,
                                 VkQueue *queue)
{
    if (makeProtected)
    {
        VkDeviceQueueInfo2 queueInfo2 = {};
        queueInfo2.sType              = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
        queueInfo2.flags              = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
        queueInfo2.queueFamilyIndex   = mIndex;
        queueInfo2.queueIndex         = queueIndex;

        vkGetDeviceQueue2(device, &queueInfo2, queue);
    }
    else
    {
        vkGetDeviceQueue(device, mIndex, queueIndex, queue);
    }
}

DeviceQueueMap QueueFamily::initializeQueueMap(VkDevice device,
                                               bool makeProtected,
                                               uint32_t queueIndex,
                                               uint32_t queueCount)
{
    // QueueIndexing:
    constexpr uint32_t kQueueIndexMedium = 0;
    constexpr uint32_t kQueueIndexHigh   = 1;
    constexpr uint32_t kQueueIndexLow    = 2;

    ASSERT(queueCount);
    ASSERT((queueIndex + queueCount) <= mProperties.queueCount);
    DeviceQueueMap queueMap(mIndex, makeProtected);

    getDeviceQueue(device, makeProtected, queueIndex + kQueueIndexMedium,
                   &queueMap[egl::ContextPriority::Medium]);
    queueMap.mPriorities[egl::ContextPriority::Medium] = egl::ContextPriority::Medium;

    // If at least 2 queues, High has its own queue
    if (queueCount > 1)
    {
        getDeviceQueue(device, makeProtected, queueIndex + kQueueIndexHigh,
                       &queueMap[egl::ContextPriority::High]);
        queueMap.mPriorities[egl::ContextPriority::High] = egl::ContextPriority::High;
    }
    else
    {
        queueMap[egl::ContextPriority::High]             = queueMap[egl::ContextPriority::Medium];
        queueMap.mPriorities[egl::ContextPriority::High] = egl::ContextPriority::Medium;
    }
    // If at least 3 queues, Low has its own queue. Adjust Low priority.
    if (queueCount > 2)
    {
        getDeviceQueue(device, makeProtected, queueIndex + kQueueIndexLow,
                       &queueMap[egl::ContextPriority::Low]);
        queueMap.mPriorities[egl::ContextPriority::Low] = egl::ContextPriority::Low;
    }
    else
    {
        queueMap[egl::ContextPriority::Low]             = queueMap[egl::ContextPriority::Medium];
        queueMap.mPriorities[egl::ContextPriority::Low] = egl::ContextPriority::Medium;
    }
    return queueMap;
}

void QueueFamily::initialize(const VkQueueFamilyProperties &queueFamilyProperties, uint32_t index)
{
    mProperties = queueFamilyProperties;
    mIndex      = index;
}

uint32_t QueueFamily::FindIndex(const std::vector<VkQueueFamilyProperties> &queueFamilyProperties,
                                VkQueueFlags flags,
                                int32_t matchNumber,
                                uint32_t *matchCount)
{
    uint32_t index = QueueFamily::kInvalidIndex;
    uint32_t count = 0;

    for (uint32_t familyIndex = 0; familyIndex < queueFamilyProperties.size(); ++familyIndex)
    {
        const auto &queueInfo = queueFamilyProperties[familyIndex];
        if ((queueInfo.queueFlags & flags) == flags)
        {
            ASSERT(queueInfo.queueCount > 0);
            count++;
            if ((index == QueueFamily::kInvalidIndex) && (matchNumber-- == 0))
            {
                index = familyIndex;
            }
        }
    }
    if (matchCount)
    {
        *matchCount = count;
    }

    return index;
}

#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
// CommandQueue2 implementation.

angle::Result CommandQueue2::init(Context *context, const DeviceQueueMap &queueMap)
{
    mQueueMap = queueMap;

    ANGLE_TRY(mCmdsStateMap[0].pool.init(context, false, queueMap.getIndex()));

    if (queueMap.isProtected())
    {
        ANGLE_TRY(mCmdsStateMap[1].pool.init(context, true, queueMap.getIndex()));
    }

#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
    if (context->getRenderer()->getFeatures().asyncCommandQueue.enabled)
    {
        // CommandProcessor is required for handleError() and Present result handling.
        mSubmitThreadTaskQueue.init();
        mUseSubmitThread = true;
    }
#endif

    return angle::Result::Continue;
}

void CommandQueue2::destroy(Context *context)
{
#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
    mUseSubmitThread = false;
    mSubmitThreadTaskQueue.destroy();
#endif

    for (VkQueue queue : mQueueMap)
    {
        if (queue != VK_NULL_HANDLE)
        {
            (void)vkQueueWaitIdle(queue);
        }
    }

    // Mark everything as completed
    updateLastCompletedQueueSerial({}, Serial::Infinite());

#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
    resetCompletedBuffers(context);
#endif
    clearGarbageQueue(context, Serial::Infinite());

    ASSERT(mPendingStateResetIndex == mNextPrepareIndex);
    ASSERT(mCommandBufferResetIndex == mNextSubmitIndex);
    ASSERT(mSubmittedStateResetIndex == mNextSubmitIndex);
    ASSERT(mNextPrepareIndex == mNextSubmitIndex);
    ASSERT(mGarbageQueue.empty());

    const VkDevice device = context->getDevice();

    for (CmdsState &cmdsState : mCmdsStateMap)
    {
        cmdsState.currentBuffer.destroy(device);
        cmdsState.pool.destroy(device);
    }
    mFenceRecycler.destroy(context);
}

#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
void CommandQueue2::waitSubmitThreadIdle()
{
    if (mUseSubmitThread)
    {
        mSubmitThreadTaskQueue.waitIdle();
    }
}
#endif

void CommandQueue2::handleDeviceLost(RendererVk *renderer)
{
#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
    waitSubmitThreadIdle();
#endif

    VkDevice device = renderer->getDevice();

    for (QueueItemIndex index = 0; index < kQueueCapacity; ++index)
    {
        QueueItem &item = mItemQueue[index];
        item.resetState();

        if (item.fence.isReferenced())
        {
            // On device loss we need to wait for fence to be signaled before destroying it
            VkResult status = item.fence.get().wait(device, renderer->getMaxFenceWaitTimeNs());
            // If the wait times out, it is probably not possible to recover from lost device
            ASSERT(status == VK_SUCCESS || status == VK_ERROR_DEVICE_LOST);
            item.fence.reset(device);
        }

        // On device lost, here simply destroy the CommandBuffer, it will fully cleared later
        // by CommandPool::destroy
        item.commandBuffer.destroy(device);
    }

    mNextPrepareIndex = 0;
    mPendingStateResetIndex = 0;

    mNextSubmitIndex = 0;
    mCommandBufferResetIndex = 0;
    mSubmittedStateResetIndex = 0;
}

CommandQueue2::QueueItemIndex CommandQueue2::incIndex(QueueItemIndex index, QueueSize value)
{
    return (index + value) % kQueueCapacity;
}

CommandQueue2::QueueItemIndex CommandQueue2::decIndex(QueueItemIndex index, QueueSize value)
{
    return (kQueueCapacity + index - value) % kQueueCapacity;
}

template <class GetFenceStatus, class ResetItem>
angle::Result CommandQueue2::checkAndResetCompletedItems(
        Context *context, QueueItemIndex *beginIndex, const QueueItemIndex endIndex,
        GetFenceStatus &&getFenceStatus, ResetItem &&resetItem)
{
    ASSERT(itemsHasValidOrdering(*beginIndex, endIndex));
    const Serial completedSerial = getLastCompletedQueueSerial();

    Serial newCompletedSerial = completedSerial;

    while (*beginIndex != endIndex)
    {
        QueueItem &item = mItemQueue[*beginIndex];
        if (item.serial > completedSerial) // Need check fence status
        {
            const VkResult status = getFenceStatus(item);
            if (status == VK_SUCCESS)
            {
                newCompletedSerial = item.serial;
            }
            else if (status == VK_NOT_READY)
            {
                break;
            }
            else
            {
                ANGLE_VK_TRY(context, status);
                UNREACHABLE();
            }
        }
        resetItem(item);
        *beginIndex = incIndex(*beginIndex);
    }

    if (newCompletedSerial > completedSerial)
    {
        updateLastCompletedQueueSerial(completedSerial, newCompletedSerial);
    }

    return angle::Result::Continue;
}

template <class TryResetItem>
void CommandQueue2::resetCompletedItems(
        QueueItemIndex *beginIndex, const QueueItemIndex endIndex,
        Serial completedSerial, TryResetItem &&tryResetItem)
{
    ASSERT(itemsHasValidOrdering(*beginIndex, endIndex));
    while (*beginIndex != endIndex && mItemQueue[*beginIndex].serial <= completedSerial)
    {
        if (!tryResetItem(mItemQueue[*beginIndex]))
        {
            break;
        }
        *beginIndex = incIndex(*beginIndex);
    }
}

bool CommandQueue2::itemsHasValidOrdering(QueueItemIndex beginIndex, const QueueItemIndex endIndex) const
{
    Serial lastSerial;
    for (QueueItemIndex index = beginIndex; index != endIndex; index = incIndex(index))
    {
        const Serial currentSerial = mItemQueue[index].serial;
        if (currentSerial <= lastSerial)
        {
            return false;
        }
        lastSerial = currentSerial;
    }
    return true;
}

angle::Result CommandQueue2::checkCompletedCommands(Context *context)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::checkCompletedCommands");
    const VkDevice device = context->getDevice();
    return checkAndResetCompletedItems(context, &mPendingStateResetIndex, mNextPrepareIndex,
        [device](QueueItem &item)
        {
            return item.getPendingFenceStatus(device, VK_SUCCESS);
        },
        [device](QueueItem &item)
        {
            item.resetPendingState(device);
        });
}

angle::Result CommandQueue2::finishToSerial(Context *context, Serial finishSerial, uint64_t timeout)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::finishToSerial");
    ANGLE_TRY(waitAndResetPendingItems(context, finishSerial, timeout, true));  // mayUnlock
    return angle::Result::Continue;
}

angle::Result CommandQueue2::waitForSerialWithUserTimeout(
        Context *context, Serial serial, uint64_t timeout, VkResult *result)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::waitForSerialWithUserTimeout");

    const Serial completedSerial = getLastCompletedQueueSerial();
    if (serial <= completedSerial)
    {
        *result = VK_SUCCESS;
        return angle::Result::Continue;
    }

    ASSERT(itemsHasValidOrdering(mPendingStateResetIndex, mNextPrepareIndex));
    QueueItemIndex index = mPendingStateResetIndex;
    while (index != mNextPrepareIndex && mItemQueue[index].serial < serial)
    {
        index = incIndex(index);
    }
    ASSERT(index != mNextPrepareIndex);
    ASSERT(mItemQueue[index].serial == serial);

    {
#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_CONCURRENT_WAIT
        mItemQueue[index].acquireLock();
        MutexUnlock<std::mutex> unlock(&context->getRenderer()->getCommandQueueMutex());
#endif

        DynamicTimeout dynamicTimeout{ timeout };
        *result = mItemQueue[index].waitPendingFence(context->getDevice(), dynamicTimeout, VK_TIMEOUT);

#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_CONCURRENT_WAIT
        mItemQueue[index].releaseLock();
#endif
    }

    if (*result != VK_TIMEOUT) // Timeout is not an error
    {
        ANGLE_VK_TRY(context, *result);
    }

    return angle::Result::Continue;
}

angle::Result CommandQueue2::waitAndResetPendingItems(
        Context *context, Serial waitSerial, uint64_t timeout, bool mayUnlock)
{
    Serial completedSerial = getLastCompletedQueueSerial();

    if (waitSerial > completedSerial)
    {
        ANGLE_TRY(waitPendingItems(context, completedSerial, waitSerial, timeout, mayUnlock));

        updateLastCompletedQueueSerial(completedSerial, waitSerial);
        completedSerial = waitSerial;
    }

    resetPendingItems(context, completedSerial, mayUnlock);

    return angle::Result::Continue;
}

angle::Result CommandQueue2::waitPendingItems(
        Context *context, Serial completedSerial, Serial waitSerial,
        uint64_t timeout, bool mayUnlock)
{
    ASSERT(waitSerial > completedSerial);

    QueueItemIndex index = mPendingStateResetIndex;
    while (index != mNextPrepareIndex && mItemQueue[index].serial <= waitSerial)
    {
        index = incIndex(index);
    }
    if (index == mPendingStateResetIndex)
    {
        return angle::Result::Continue;
    }

    // Serial:      1 2 3 4 5 6 7 8 9
    // waitSerial:              ^
    // VkQueue:     1 1 2 2 1 2 2 1 2
    // wait fences:         ^   ^     - Need to wait latest fence from each queue up to waitSerial

    VkResult waitResult = VK_SUCCESS;

    std::array<VkQueue, kVkQueueCount> waitedQueues{};
    size_t waitedQueueCount = 0;
    VkQueue curQueue = mItemQueue[decIndex(index)].queue;
    bool isCurQueueWaited = false;
    bool isLastSubmitWaited = false;

#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_CONCURRENT_WAIT
    std::mutex *commandQueueMutex = &context->getRenderer()->getCommandQueueMutex();
#endif
    const VkDevice device = context->getDevice();
    DynamicTimeout dynamicTimeout{ timeout };

    do
    {
        index = decIndex(index);
        QueueItem &item = mItemQueue[index];
        const Serial itemSerial = item.serial;  // Need local copy for later use
        if (itemSerial <= completedSerial)
        {
            break;
        }
        if (ANGLE_UNLIKELY(item.queue != curQueue))
        {
            curQueue = item.queue;
            const auto last = waitedQueues.begin() + waitedQueueCount;
            isCurQueueWaited = (std::find(waitedQueues.begin(), last, curQueue) != last);
        }
        if (ANGLE_UNLIKELY(!isCurQueueWaited))
        {
#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_CONCURRENT_WAIT
            bool wasUnlock = false;
#endif
            waitResult = VK_NOT_READY;
            if (isLastSubmitWaited)
            {
                const QueueItem::State state = item.getState();
                if (state == QueueItem::State::Submitted)
                {
                    waitResult = item.getFenceStatus(device);
                }
                else
                {
                    ASSERT(state == QueueItem::State::Error);
                    waitResult = VK_INCOMPLETE;
                }
            }
            if (waitResult == VK_NOT_READY)
            {
#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_CONCURRENT_WAIT
                MutexUnlock<std::mutex> unlock;
                if (mayUnlock)
                {
                    item.acquireLock();
                    unlock.unlock(commandQueueMutex);
                    wasUnlock = true;
                }
#endif
                if (isLastSubmitWaited)
                {
                    waitResult = item.waitFence(device, dynamicTimeout);
                }
                else
                {
                    waitResult = item.waitPendingFence(device, dynamicTimeout, VK_INCOMPLETE);
                    isLastSubmitWaited = true;
                }
#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_CONCURRENT_WAIT
                if (mayUnlock)
                {
                    item.releaseLock(); // Do not access "item" after this line
                }
#endif
            }
            if (waitResult == VK_SUCCESS)
            {
                waitedQueues[waitedQueueCount] = curQueue;
                ++waitedQueueCount;
                isCurQueueWaited = true;
            }
            else if (waitResult != VK_INCOMPLETE) // Submit OK, but wait timeout/failed
            {
                break;
            }
            else // Submit failed - skip fence
            {
                waitResult = VK_SUCCESS;
            }
#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_CONCURRENT_WAIT
            // Items were reset while this thread was waiting inside the "unlock".
            if (wasUnlock &&
                ((mPendingStateResetIndex == mNextPrepareIndex) ||
                (mItemQueue[mPendingStateResetIndex].serial > itemSerial)))
            {
                break;
            }
#endif
        }
    }
    while (index != mPendingStateResetIndex);

    ANGLE_VK_TRY(context, waitResult);

    return angle::Result::Continue;
}

void CommandQueue2::resetPendingItems(Context *context, Serial completedSerial, bool maySkip)
{
    const VkDevice device = context->getDevice();
    resetCompletedItems(&mPendingStateResetIndex, mNextPrepareIndex, completedSerial,
        [device, maySkip](QueueItem &item)
        {
#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_CONCURRENT_WAIT
            if (item.isLocked())
            {
                if (maySkip)
                {
                    return false;
                }
                WARN() << "item.isLocked() - spin-wait until unlocked...";
                do
                {
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                }
                while (item.isLocked());
            }
#else
            (void)maySkip;
#endif
            item.resetPendingState(device);
            return true;
        });
}

angle::Result CommandQueue2::flushOutsideRPCommands(
        Context *context, bool hasProtectedContent,
        CommandBufferHelper **outsideRPCommands)
{
    PrimaryCommandBuffer *buffer = nullptr;
    ANGLE_TRY(getValidPrimaryCommandBuffer(context, hasProtectedContent, &buffer));
    return (*outsideRPCommands)->flushToPrimary(context->getRenderer()->getFeatures(), buffer, nullptr);
}

angle::Result CommandQueue2::flushRenderPassCommands(
        Context *context, bool hasProtectedContent,
        const RenderPass &renderPass, CommandBufferHelper **renderPassCommands)
{
    PrimaryCommandBuffer *buffer = nullptr;
    ANGLE_TRY(getValidPrimaryCommandBuffer(context, hasProtectedContent, &buffer));
    return (*renderPassCommands)->flushToPrimary(context->getRenderer()->getFeatures(), buffer, &renderPass);
}

angle::Result CommandQueue2::submitFrame(
        Context *context,
        bool hasProtectedContent,
        QueuePriority priority,
        const std::vector<VkSemaphore> &waitSemaphores,
        const std::vector<VkPipelineStageFlags> &waitSemaphoreStageMasks,
        const Semaphore *signalSemaphore,
        GarbageList &&currentGarbage,
        Serial submitQueueSerial)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::submitFrame");
    ASSERT(!context->getRenderer()->getFeatures().asyncCommandQueue.enabled);

    QueueItemIndex itemIndex = -1;
    ASSERT(waitSemaphores.size() == waitSemaphoreStageMasks.size());
    ANGLE_TRY(prepareNextSubmit(context, hasProtectedContent, priority,
                                waitSemaphores, waitSemaphoreStageMasks.data(), signalSemaphore,
                                VK_NULL_HANDLE, nullptr, submitQueueSerial, &itemIndex));

    ANGLE_TRY(doSubmitFrame(context, std::move(currentGarbage), itemIndex));

    return angle::Result::Continue;
}

angle::Result CommandQueue2::queueSubmitOneOff(
        Context *context,
        bool hasProtectedContent,
        QueuePriority priority,
        VkCommandBuffer commandBufferHandle,
        const Fence *fence,
        SubmitPolicy submitPolicy,
        Serial submitQueueSerial)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::queueSubmitOneOff");
    ASSERT(!context->getRenderer()->getFeatures().asyncCommandQueue.enabled);

    QueueItemIndex itemIndex = -1;
    ANGLE_TRY(prepareNextSubmit(context, hasProtectedContent, priority,
                                nullptr, nullptr, nullptr,
                                commandBufferHandle, fence, submitQueueSerial, &itemIndex));

    ANGLE_TRY(doQueueSubmitOneOff(context, itemIndex));

    return angle::Result::Continue;
}

angle::Result CommandQueue2::prepareNextSubmit(
        Context *context,
        bool hasProtectedContent,
        QueuePriority priority,
        VkSemaphores waitSemaphores,
        const VkPipelineStageFlags *waitSemaphoreStageMasks,
        const Semaphore *signalSemaphore,
        VkCommandBuffer oneOffCommandBuffer,
        const Fence *oneOffFence,
        Serial submitQueueSerial,
        QueueItemIndex *itemIndexOut)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::prepareNextSubmit");

    ANGLE_TRY(throttlePendingItemQueue(context));

    QueueItem &item = mItemQueue[mNextPrepareIndex];
    ANGLE_TRY(item.waitIdle(context->getRenderer()->getMaxFenceWaitTimeNs()));
    ASSERT(!item.fence.isReferenced());
    ASSERT(!item.commandBuffer.valid());
    item.hasProtectedContent = hasProtectedContent;
    item.serial = submitQueueSerial;
    item.queue = mQueueMap[priority];

    item.submitInfo = {};
    item.submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    if (waitSemaphores.count > 0)
    {
        item.waitSemaphores.assign(waitSemaphores.items,
                                   waitSemaphores.items + waitSemaphores.count);
        item.waitSemaphoreStageMasks.assign(waitSemaphoreStageMasks,
                                            waitSemaphoreStageMasks + waitSemaphores.count);
        item.submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.count);
        item.submitInfo.pWaitSemaphores = item.waitSemaphores.data();
        item.submitInfo.pWaitDstStageMask = item.waitSemaphoreStageMasks.data();
    }
    else
    {
        item.waitSemaphores.clear();
        item.waitSemaphoreStageMasks.clear();
    }
    if (signalSemaphore)
    {
        item.signalSemaphore = signalSemaphore->getHandle();
        item.submitInfo.signalSemaphoreCount = 1;
        item.submitInfo.pSignalSemaphores = &item.signalSemaphore;
    }
    else
    {
        item.signalSemaphore = VK_NULL_HANDLE;
    }

    if (oneOffCommandBuffer != VK_NULL_HANDLE)
    {
        item.oneOffCommandBuffer = oneOffCommandBuffer;
        item.submitInfo.commandBufferCount = 1;
        item.submitInfo.pCommandBuffers = &item.oneOffCommandBuffer;
    }
    else
    {
        item.oneOffCommandBuffer = VK_NULL_HANDLE;
    }
    item.oneOffFence = oneOffFence ? oneOffFence->getHandle() : VK_NULL_HANDLE;

    *itemIndexOut = mNextPrepareIndex;
    mNextPrepareIndex = incIndex(mNextPrepareIndex);
    ASSERT(getNumPendingItems() <= kInFlightCommandsLimit);

    // It is not actually submitted, but already can be used for waiting
    updateLastSubmittedQueueSerial(submitQueueSerial);

    return angle::Result::Continue;
}

angle::Result CommandQueue2::throttlePendingItemQueue(Context *context)
{
    static_assert(kActiveGCThreshold <= kInFlightCommandsLimit, "BAD kActiveGCThreshold");
    static_assert(CommandQueue2::kMaxQueueSize >= kInFlightCommandsLimit, "BAD CommandQueue2::kMaxQueueSize");

    const QueueSize numPendingItems = getNumPendingItems();
    if (ANGLE_UNLIKELY(numPendingItems >= kActiveGCThreshold))
    {
        ASSERT(numPendingItems <= kInFlightCommandsLimit);
        if (ANGLE_UNLIKELY(numPendingItems >= kInFlightCommandsLimit))
        {
            // CPU should be throttled to avoid mPendingItemQueue from growing too fast.
            // Important for off-screen scenarios.
            const QueueItemIndex finishIndex = decIndex(mNextPrepareIndex, kInFlightCommandsLimit);

            ANGLE_TRY(waitAndResetPendingItems(context, mItemQueue[finishIndex].serial,
                    context->getRenderer()->getMaxFenceWaitTimeNs()));
        }
        else
        {
            // Reset any items that was already completed
            resetPendingItems(context, getLastCompletedQueueSerial());
        }
    }
    return angle::Result::Continue;
}

CommandQueue2::QueueSize CommandQueue2::getNumPendingItems() const
{
    return (kQueueCapacity + mNextPrepareIndex - mPendingStateResetIndex) % kQueueCapacity;
}

angle::Result CommandQueue2::doSubmitFrame(
        Context *context, GarbageList &&currentGarbage, QueueItemIndex itemIndex)
{
    QueueItem::SubmitScope itemSubmitScope(context, this, itemIndex);
    QueueItem &item = itemSubmitScope.getItem();

    // Execute at the start, before any ANGLE_TRY()
    collectGarbage(std::move(currentGarbage), item.serial);

    PrimaryCommandBuffer *buffer = nullptr;
    ANGLE_TRY(getValidPrimaryCommandBuffer(context, item.hasProtectedContent, &buffer));
    ANGLE_VK_TRY(context, buffer->end());

    ASSERT(!item.commandBuffer.valid());
    item.commandBuffer = std::move(*buffer);

    ASSERT(item.submitInfo.commandBufferCount == 0);
    ASSERT(item.submitInfo.pCommandBuffers == nullptr);
    item.submitInfo.commandBufferCount = 1;
    item.submitInfo.pCommandBuffers = item.commandBuffer.ptr();

#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
    if (mUseSubmitThread)
    {
        mSubmitThreadTaskQueue.enqueue([this, context,
                scope = std::move(itemSubmitScope)]() mutable
            {
                ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::doSubmitFrameJob");
                QueueItem::SubmitScope itemSubmitScope(std::move(scope));
                itemSubmitScope.setInSubmitThread();
                return doSubmitFrameJob(context, std::move(itemSubmitScope));
            });
        resetCompletedBuffers(context);
    }
    else
#endif
    {
        ANGLE_TRY(doSubmitFrameJob(context, std::move(itemSubmitScope)));
    }

    return angle::Result::Continue;
}

angle::Result CommandQueue2::getValidPrimaryCommandBuffer(
            Context *context, bool hasProtectedContent, PrimaryCommandBuffer **primaryCommandBufferOut)
{
    CmdsState &cmdsState = mCmdsStateMap[hasProtectedContent];

    if (cmdsState.currentBuffer.valid())
    {
        *primaryCommandBufferOut = &cmdsState.currentBuffer;
        return angle::Result::Continue;
    }

    ANGLE_TRY(cmdsState.pool.allocate(context, &cmdsState.currentBuffer));

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    ANGLE_VK_TRY(context, cmdsState.currentBuffer.begin(beginInfo));

    *primaryCommandBufferOut = &cmdsState.currentBuffer;

    return angle::Result::Continue;
}

#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
void CommandQueue2::resetCompletedBuffers(Context *context)
{
    {
        std::lock_guard<std::mutex> lock(mCompletedBuffersMutex);

        for (CmdsState &cmdsState : mCmdsStateMap)
        {
            ASSERT(cmdsState.completedBuffersPop.empty());
            if (!cmdsState.completedBuffersPush.empty())
            {
                swap(cmdsState.completedBuffersPush, cmdsState.completedBuffersPop);
            }
        }
    }

    for (CmdsState &cmdsState : mCmdsStateMap)
    {
        if (!cmdsState.completedBuffersPop.empty())
        {
            for (vk::PrimaryCommandBuffer &buffer : cmdsState.completedBuffersPop)
            {
                const angle::Result result = cmdsState.pool.collect(context, std::move(buffer));
                ASSERT(result == angle::Result::Continue);
            }
            cmdsState.completedBuffersPop.clear();
        }
    }
}
#endif

angle::Result CommandQueue2::doSubmitFrameJob(
        Context *context, QueueItem::SubmitScope &&itemSubmitScope)
{
    QueueItem &item = itemSubmitScope.getItem();
    const Serial submitSerial = item.serial;

    ANGLE_TRY(queueSubmit(context, std::move(itemSubmitScope)));

    ASSERT(submitSerial >= mLastClearGarbageSerial);
    if (ANGLE_UNLIKELY(submitSerial.getValue() > mLastClearGarbageSerial.getValue() + kActiveGCThreshold))
    {
        ANGLE_TRY(clearGarbage(context, getLastCompletedQueueSerial()));
    }

    return angle::Result::Continue;
}

angle::Result CommandQueue2::doQueueSubmitOneOff(Context *context, QueueItemIndex itemIndex)
{
    QueueItem::SubmitScope itemSubmitScope(context, this, itemIndex);

#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
    if (mUseSubmitThread)
    {
        mSubmitThreadTaskQueue.enqueue([this, context,
                scope = std::move(itemSubmitScope)]() mutable
            {
                ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::doQueueSubmitOneOffJob");
                QueueItem::SubmitScope itemSubmitScope(std::move(scope));
                itemSubmitScope.setInSubmitThread();
                return queueSubmit(context, std::move(itemSubmitScope));
            });
        resetCompletedBuffers(context);
    }
    else
#endif
    {
        ANGLE_TRY(queueSubmit(context, std::move(itemSubmitScope)));
    }

    return angle::Result::Continue;
}

angle::Result CommandQueue2::queueSubmit(Context *context, QueueItem::SubmitScope &&itemSubmitScope)
{
    QueueItem &item = itemSubmitScope.getItem();

    if (kOutputVmaStatsString)
    {
        context->getRenderer()->outputVmaStatString();
    }

    VkProtectedSubmitInfo protectedSubmitInfo = {};
    if (item.hasProtectedContent)
    {
        protectedSubmitInfo.sType           = VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO;
        protectedSubmitInfo.pNext           = nullptr;
        protectedSubmitInfo.protectedSubmit = VK_TRUE;
        item.submitInfo.pNext               = &protectedSubmitInfo;
    }

    ASSERT(!item.fence.isReferenced());
    ANGLE_TRY(mFenceRecycler.newSharedFence(context, &item.fence));

    if (item.oneOffFence == VK_NULL_HANDLE)
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "vkQueueSubmit");
        ANGLE_VK_TRY(context, vkQueueSubmit(item.queue, 1, &item.submitInfo, item.fence.get().getHandle()));
    }
    else
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "vkQueueSubmit");
        VkSubmitInfo fenceSubmitInfo = {};
        fenceSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        if (item.hasProtectedContent)
        {
            fenceSubmitInfo.pNext = &protectedSubmitInfo;
        }
        ANGLE_VK_TRY(context, vkQueueSubmit(item.queue, 1, &item.submitInfo, item.oneOffFence));
        ANGLE_VK_TRY(context, vkQueueSubmit(item.queue, 1, &fenceSubmitInfo, item.fence.get().getHandle()));
    }

    itemSubmitScope.setSubmitOK();

    return angle::Result::Continue;
}

void CommandQueue2::advanceSubmitIndex(QueueItemIndex submitItemIndex)
{
    ASSERT(submitItemIndex == mNextSubmitIndex);
    mNextSubmitIndex = incIndex(mNextSubmitIndex);
}

void CommandQueue2::checkAndCollectCommandBuffers(Context *context)
{
    const VkDevice device = context->getDevice();

    (void)checkAndResetCompletedItems(context, &mCommandBufferResetIndex, mNextSubmitIndex,
        [device](QueueItem &item)
        {
            return item.fence.isReferenced() ? item.fence.get().getStatus(device) : VK_SUCCESS;
        },
        [context, this](QueueItem &item)
        {
            item.collectCommandBuffer(context, this);
        });

    resetSubmittedItems();
}

void CommandQueue2::collectCommandBuffers(Context *context, Serial completedSerial)
{
    resetCompletedItems(&mCommandBufferResetIndex, mNextSubmitIndex, completedSerial,
        [context, this](QueueItem &item)
        {
            item.collectCommandBuffer(context, this);
            return true;
        });
}

void CommandQueue2::resetSubmittedItems()
{
    while (mSubmittedStateResetIndex != mCommandBufferResetIndex &&
            mItemQueue[mSubmittedStateResetIndex].acquireState() == QueueItem::State::Finished)
    {
        mItemQueue[mSubmittedStateResetIndex].resetSubmittedState(this);
        mSubmittedStateResetIndex = incIndex(mSubmittedStateResetIndex);
    }
}

VkResult CommandQueue2::queuePresent(
        Context *context, QueuePriority priority, const VkPresentInfoKHR &presentInfo)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::queuePresent");
    ASSERT(!context->getRenderer()->getFeatures().asyncCommandQueue.enabled);

    VkResult result = VK_ERROR_DEVICE_LOST;

    doQueuePresent(context, priority, presentInfo,
        [&result](const VkPresentInfoKHR &presentInfo, VkResult presentResult)
        {
            result = presentResult;
        });

    return result;
}

template <class ON_PRESENT_RESULT>
void CommandQueue2::doQueuePresent(
        Context *context, QueuePriority priority, const VkPresentInfoKHR &presentInfo,
        ON_PRESENT_RESULT &&onPresentResult)
{
#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
    if (mUseSubmitThread)
    {
        ASSERT(presentInfo.swapchainCount == 1);
        ASSERT(presentInfo.waitSemaphoreCount == 1);
        ASSERT(presentInfo.pResults == nullptr);

        std::vector<VkRectLayerKHR> vkRects;
        if (presentInfo.pNext)
        {
            const VkStructureType sType = *static_cast<const VkStructureType *>(presentInfo.pNext);
            if (sType == VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR)
            {
                const auto &regions = *static_cast<const VkPresentRegionsKHR *>(presentInfo.pNext);
                ASSERT(regions.pNext == nullptr);
                ASSERT(regions.swapchainCount == 1);
                const VkPresentRegionKHR &region = regions.pRegions[0];
                vkRects.resize(region.rectangleCount);
                std::copy(region.pRectangles, region.pRectangles + region.rectangleCount, vkRects.begin());
            }
            else
            {
                ERR() << "Unknown sType: " << sType << " in VkPresentInfoKHR.pNext chain";
                UNREACHABLE();
            }
        }

        mSubmitThreadTaskQueue.enqueue([this, context, priority,
                swapchain = presentInfo.pSwapchains[0],
                imageIndex = presentInfo.pImageIndices[0],
                waitSemaphore = presentInfo.pWaitSemaphores[0],
                vkRects = std::move(vkRects),
                onPresentResult = std::forward<ON_PRESENT_RESULT>(onPresentResult)
            ]() mutable
            {
                ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::doQueuePresentJob");
                VkPresentInfoKHR presentInfo = {};
                presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
                presentInfo.swapchainCount     = 1;
                presentInfo.pSwapchains        = &swapchain;
                presentInfo.pImageIndices      = &imageIndex;
                presentInfo.waitSemaphoreCount = 1;
                presentInfo.pWaitSemaphores    = &waitSemaphore;
                VkPresentRegionsKHR presentRegions = {};
                VkPresentRegionKHR presentRegion = {};
                if (!vkRects.empty())
                {
                    presentInfo.pNext = &presentRegions;
                    presentRegions.sType          = VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR;
                    presentRegions.swapchainCount = 1;
                    presentRegions.pRegions       = &presentRegion;
                    presentRegion.pRectangles    = vkRects.data();
                    presentRegion.rectangleCount = static_cast<uint32_t>(vkRects.size());
                }
                doQueuePresentJob(context, priority, presentInfo, std::move(onPresentResult));
                return angle::Result::Continue;
            });
    }
    else
#endif
    {
        doQueuePresentJob(context, priority, presentInfo, std::forward<ON_PRESENT_RESULT>(onPresentResult));
    }
}

template <class ON_PRESENT_RESULT>
void CommandQueue2::doQueuePresentJob(
        Context *context, QueuePriority priority, const VkPresentInfoKHR &presentInfo,
        ON_PRESENT_RESULT &&onPresentResult)
{
    const VkResult result = vkQueuePresentKHR(mQueueMap[priority], &presentInfo);
    onPresentResult(presentInfo, result);

    // vkQueuePresentKHR() may wait for GPU previous frame.
    // There is high chance for successful garbage cleanup.
    checkAndCollectCommandBuffers(context);
    (void)clearGarbage(context, getLastCompletedQueueSerial());
}

angle::Result CommandQueue2::cleanupAllGarbage(Context *context)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::cleanupAllGarbage");

#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
    // Wait here, to be able to safely call below methods
    waitSubmitThreadIdle();
#endif

    const Serial completedSerial = getLastCompletedQueueSerial();

    collectCommandBuffers(context, completedSerial);
#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
    if (mUseSubmitThread)
    {
        resetCompletedBuffers(context);
    }
#endif
    resetSubmittedItems();

    ANGLE_TRY(clearGarbage(context, completedSerial));

    return angle::Result::Continue;
}

angle::Result CommandQueue2::clearGarbage(Context *context, Serial completedSerial)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::clearGarbage");

    clearGarbageQueue(context, completedSerial);

    ANGLE_TRY(context->getRenderer()->cleanupGarbage(completedSerial));

    mLastClearGarbageSerial = completedSerial;

    return angle::Result::Continue;
}

void CommandQueue2::collectGarbage(GarbageList &&garbage, Serial serial)
{
    if (!garbage.empty())
    {
        std::lock_guard<std::mutex> queueLock(mGarbageQueueMutex);
        mGarbageQueue.emplace_back(std::move(garbage), serial);
    }
}

void CommandQueue2::clearGarbageQueue(Context *context, Serial completedSerial)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::clearGarbageQueue");

    RendererVk *renderer = context->getRenderer();

    size_t clearCount = 0;

    for (;;)
    {
        GarbageAndSerial garbageList;
        {
            std::lock_guard<std::mutex> queueLock(mGarbageQueueMutex);
            if (clearCount >= mGarbageQueue.size() || mGarbageQueue[clearCount].getSerial() > completedSerial)
            {
                break;
            }
            garbageList = std::move(mGarbageQueue[clearCount]);
        }
        for (GarbageObject &garbage : garbageList.get())
        {
            garbage.destroy(renderer);
        }
        ++clearCount;
    }

    if (clearCount > 0)
    {
        std::lock_guard<std::mutex> queueLock(mGarbageQueueMutex);
        mGarbageQueue.erase(mGarbageQueue.begin(), mGarbageQueue.begin() + clearCount);
    }
}

Serial CommandQueue2::reserveSubmitSerial()
{
    return mCurrentQueueSerial.exchange(mQueueSerialFactory.generate(),
            std::memory_order::memory_order_relaxed);
}

Serial CommandQueue2::getLastSubmittedQueueSerial() const
{
    return mLastSubmittedQueueSerial.load(std::memory_order::memory_order_relaxed);
}

Serial CommandQueue2::getLastCompletedQueueSerial() const
{
    return mLastCompletedQueueSerial.load(std::memory_order::memory_order_relaxed);
}

Serial CommandQueue2::getCurrentQueueSerial() const
{
    return mCurrentQueueSerial.load(std::memory_order::memory_order_relaxed);
}

void CommandQueue2::updateLastSubmittedQueueSerial(Serial newSerial)
{
    ASSERT(newSerial > getLastSubmittedQueueSerial());
    mLastSubmittedQueueSerial.store(newSerial, std::memory_order_relaxed);
}

void CommandQueue2::updateLastCompletedQueueSerial(Serial oldSerial, Serial newSerialCandidate)
{
    ASSERT(newSerialCandidate > oldSerial);
    while (ANGLE_UNLIKELY(!mLastCompletedQueueSerial.compare_exchange_weak(
            oldSerial, newSerialCandidate, std::memory_order::memory_order_relaxed)))
    {
        if (newSerialCandidate <= oldSerial)
        {
            return;
        }
    }
}

CommandQueue2::DynamicTimeout::DynamicTimeout(uint64_t original)
    : mOriginal(original)
    , mStartTime(std::numeric_limits<double>::max())
{
}

uint64_t CommandQueue2::DynamicTimeout::getRemaining()
{
    if (mStartTime == std::numeric_limits<double>::max())
    {
        mStartTime = angle::GetCurrentTime();
        return mOriginal;
    }
    return mOriginal - std::min(mOriginal, uint64_t((angle::GetCurrentTime() - mStartTime) * 1e+9));
}

CommandQueue2::QueueItem::SubmitScope::SubmitScope(
        Context *context, CommandQueue2 *owner, QueueItemIndex itemIndex)
    : mContext(context)
    , mOwner(owner)
    , mItemIndex(itemIndex)
    , mItem(owner->mItemQueue + itemIndex)
{
}

CommandQueue2::QueueItem::SubmitScope::SubmitScope(SubmitScope &&other)
    : mContext(other.mContext)
    , mOwner(other.mOwner)
    , mItemIndex(other.mItemIndex)
    , mItem(other.mItem)
{
    other.mOwner = nullptr;
    other.mItem = nullptr;
}

CommandQueue2::QueueItem::SubmitScope::~SubmitScope()
{
    if (!mOwner)
    {
        return;
    }

    if (ANGLE_UNLIKELY(mItem)) // Submit failed
    {
#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
        if (!mIsInSubmitThread)
        {
            // If failed not in the Submit thread, need to wait before cleanup
            mOwner->waitSubmitThreadIdle();
        }
#endif

        mOwner->advanceSubmitIndex(mItemIndex);

        ASSERT(mItem->getState() == State::Idle);
        mItem->collectCommandBuffer(mContext, mOwner);
        mItem->resetFence(mOwner);
        mItem->setStateAndNotify(State::Error);
    }
#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
    else
    {
        ASSERT(!mOwner->mUseSubmitThread || mIsInSubmitThread);
    }
#endif

    mOwner->checkAndCollectCommandBuffers(mContext);
}

#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
void CommandQueue2::QueueItem::SubmitScope::setInSubmitThread()
{
    ASSERT(mOwner->mUseSubmitThread && !mIsInSubmitThread);
    mIsInSubmitThread = true;
}
#endif

void CommandQueue2::QueueItem::SubmitScope::setSubmitOK()
{
#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
    ASSERT(!mOwner->mUseSubmitThread || mIsInSubmitThread);
#endif

    mOwner->advanceSubmitIndex(mItemIndex);

    ASSERT(mItem != nullptr);
    ASSERT(mItem->fence.isReferenced());
    ASSERT(mItem->getState() == State::Idle);
    mItem->setStateAndNotify(State::Submitted);
    mItem = nullptr;
}

angle::Result CommandQueue2::QueueItem::waitIdle(uint64_t timeout) const
{
    DynamicTimeout dynamicTimeout{ timeout };
    const State state = waitState(dynamicTimeout, [](State state) { return state == State::Idle; });
    if (ANGLE_UNLIKELY(state != State::Idle))
    {
        ASSERT(state == State::Finished);
        return angle::Result::Stop;
    }
    return angle::Result::Continue;
}

void CommandQueue2::QueueItem::resetPendingState(VkDevice device)
{
    ASSERT(getPendingFenceStatus(device, VK_SUCCESS) == VK_SUCCESS);

    setState(State::Finished);
}

#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_CONCURRENT_WAIT
void CommandQueue2::QueueItem::acquireLock()
{
    // Relaxed, because need to be called from inside "mCommandQueueMutex"
    const int oldValue = mLockCounter.fetch_add(1, std::memory_order_relaxed);
    ASSERT(oldValue >= 0);
}

void CommandQueue2::QueueItem::releaseLock()
{
    const int oldValue = mLockCounter.fetch_sub(1, std::memory_order_release);
    ASSERT(oldValue > 0);
}

bool CommandQueue2::QueueItem::isLocked() const
{
    const int value = mLockCounter.load(std::memory_order_acquire);
    ASSERT(value >= 0);
    return (value > 0);
}
#endif

VkResult CommandQueue2::QueueItem::getPendingFenceStatus(VkDevice device, VkResult errorStateResult) const
{
    const State state = acquireState();
    if (ANGLE_UNLIKELY(state != State::Submitted))
    {
        ASSERT(state == State::Idle || state == State::Error);
        return (state == State::Idle) ? VK_NOT_READY : errorStateResult;
    }
    return getFenceStatus(device);
}

VkResult CommandQueue2::QueueItem::getFenceStatus(VkDevice device) const
{
    ASSERT(getState() == State::Submitted && fence.isReferenced());
    return fence.get().getStatus(device);
}

VkResult CommandQueue2::QueueItem::waitPendingFence(
        VkDevice device, DynamicTimeout &timeout, VkResult errorStateResult) const
{
    const State state = waitSubmitted(timeout);
    if (ANGLE_UNLIKELY(state != State::Submitted))
    {
        ASSERT(state == State::Idle || state == State::Error);
        return (state == State::Idle) ? VK_TIMEOUT : errorStateResult;
    }
    return waitFence(device, timeout);
}

CommandQueue2::QueueItem::State CommandQueue2::QueueItem::waitSubmitted(DynamicTimeout &timeout) const
{
    const State state = waitState(timeout, [](State state)
        {
            return (state == State::Submitted || state == State::Error);
        });
    ASSERT(state == State::Submitted || state == State::Idle || state == State::Error);
    return state;
}

VkResult CommandQueue2::QueueItem::waitFence(VkDevice device, DynamicTimeout &timeout) const
{
    ASSERT(getState() == State::Submitted && fence.isReferenced());
    return fence.get().wait(device, timeout.getRemaining());
}

void CommandQueue2::QueueItem::collectCommandBuffer(Context *context, CommandQueue2 *owner)
{
    ASSERT(getState() != State::Submitted || (fence.isReferenced() &&
            fence.get().getStatus(context->getDevice()) == VK_SUCCESS));

    if (commandBuffer.valid())
    {
        CmdsState &cmdsState = owner->mCmdsStateMap[hasProtectedContent];
#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
        if (owner->mUseSubmitThread)
        {
            std::lock_guard<std::mutex> lock(owner->mCompletedBuffersMutex);
            cmdsState.completedBuffersPush.emplace_back(std::move(commandBuffer));
        }
        else
#endif
        {
            const angle::Result result = cmdsState.pool.collect(context, std::move(commandBuffer));
            ASSERT(result == angle::Result::Continue);
        }
    }
}

void CommandQueue2::QueueItem::resetSubmittedState(CommandQueue2 *owner)
{
    ASSERT(getState() == State::Finished);

    resetFence(owner);

    setStateAndNotify(State::Idle);
}

void CommandQueue2::QueueItem::resetFence(CommandQueue2 *owner)
{
    owner->mFenceRecycler.resetSharedFence(&fence);
}

void CommandQueue2::QueueItem::setState(State newState)
{
    mState.store(newState, std::memory_order_release);
}

void CommandQueue2::QueueItem::setStateAndNotify(State newState)
{
    std::unique_lock<std::mutex> lock(mMutex);
    setState(newState);
    mCondVar.unlockAndNotifyAll(lock);
}

template <class PRED>
CommandQueue2::QueueItem::State CommandQueue2::QueueItem::waitState(
        DynamicTimeout &timeout, PRED &&pred) const
{
    QueueItem::State state = acquireState();
    if (ANGLE_UNLIKELY(!pred(state)))
    {
        std::unique_lock<std::mutex> lock(mMutex);

        for (;;)
        {
            state = getState();
            if (pred(state))
            {
                break;
            }

            const std::chrono::duration<uint64_t, std::nano> duration(timeout.getRemaining());
            const std::cv_status status = mCondVar.waitFor(lock, duration);
            if (status == std::cv_status::timeout)
            {
                break;
            }
        }
    }
    return state;
}

#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
CommandQueue2::SubmitThreadTaskQueue::~SubmitThreadTaskQueue()
{
    ASSERT(!mSubmitThread.joinable());
    ASSERT(mTaskQueue.empty());
}

void CommandQueue2::SubmitThreadTaskQueue::init()
{
    ASSERT(!mSubmitThread.joinable());
    mSubmitThread = std::thread([this]() { processTaskQueue(); });
}

void CommandQueue2::SubmitThreadTaskQueue::destroy()
{
    if (mSubmitThread.joinable())
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mNeedExit = true;
        mCondVar.unlockAndNotifyAll(lock);
        mSubmitThread.join();
    }
}

template <class F>
void CommandQueue2::SubmitThreadTaskQueue::enqueue(F &&functor)
{
    ASSERT(mSubmitThread.joinable());
    std::unique_lock<std::mutex> lock(mMutex);
    mTaskQueue.emplace_back(std::forward<F>(functor));
    mIsBusy = true;
    mCondVar.unlockAndNotifyAll(lock);
}

void CommandQueue2::SubmitThreadTaskQueue::waitIdle()
{
    ASSERT(mSubmitThread.joinable());
    ANGLE_TRACE_EVENT0("gpu.angle", "SubmitThreadTaskQueue::waitIdle");
    std::unique_lock<std::mutex> lock(mMutex);

    while (mIsBusy)
    {
        ASSERT(!mNeedExit);
        mCondVar.wait(lock);
    }
}

void CommandQueue2::SubmitThreadTaskQueue::processTaskQueue()
{
    for (;;)
    {
        QueueTask task;
        if (!tryPopNextTask(&task))
        {
            break;
        }
        const angle::Result result = task.execute();
        if (result != angle::Result::Continue)
        {
            UNREACHABLE(); // TODO: Investigate this case
        }
    }
}

bool CommandQueue2::SubmitThreadTaskQueue::tryPopNextTask(QueueTask *taskOut)
{
    std::unique_lock<std::mutex> lock(mMutex);

    while (mTaskQueue.empty() && !mNeedExit)
    {
        mIsBusy = false;
        mCondVar.notifyAll(lock);
        mCondVar.wait(lock);
    }

    if (mTaskQueue.empty())
    {
        ASSERT(mNeedExit);
        return false;
    }

    *taskOut = std::move(mTaskQueue.front());
    mTaskQueue.pop_front();

    return true;
}
#endif // SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
#endif // SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2

}  // namespace vk
}  // namespace rx
