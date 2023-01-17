//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CommandProcessor.cpp:
//    Implements the class methods for CommandProcessor.
//

#include "common/system_utils.h"

#include "libANGLE/renderer/vulkan/CommandProcessor.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"

namespace rx
{
namespace vk
{
namespace
{
constexpr size_t kInFlightCommandsLimit = 50u;
constexpr size_t kActiveGCThreshold     = 20u;
constexpr bool kOutputVmaStatsString    = false;
// When suballocation garbages is more than this, we may wait for GPU to finish and free up some
// memory for allocation.
constexpr VkDeviceSize kMaxBufferSuballocationGarbageSize = 64 * 1024 * 1024;

template <typename SecondaryCommandBufferListT>
void ResetSecondaryCommandBuffers(VkDevice device,
                                  CommandPool *commandPool,
                                  SecondaryCommandBufferListT *commandBuffers)
{
    // Nothing to do when using ANGLE secondary command buffers.
}

template <>
[[maybe_unused]] void ResetSecondaryCommandBuffers<std::vector<VulkanSecondaryCommandBuffer>>(
    VkDevice device,
    CommandPool *commandPool,
    std::vector<VulkanSecondaryCommandBuffer> *commandBuffers)
{
    // Note: we currently free the command buffers individually, but we could potentially reset the
    // entire command pool.  https://issuetracker.google.com/issues/166793850
    for (VulkanSecondaryCommandBuffer &secondary : *commandBuffers)
    {
        commandPool->freeCommandBuffers(device, 1, secondary.ptr());
        secondary.releaseHandle();
    }
    commandBuffers->clear();
}
}  // namespace

// SharedFence implementation
SharedFence::SharedFence() : mRefCountedFence(nullptr), mRecycler(nullptr) {}
SharedFence::SharedFence(const SharedFence &other)
    : mRefCountedFence(other.mRefCountedFence), mRecycler(other.mRecycler)
{
    if (mRefCountedFence != nullptr)
    {
        mRefCountedFence->addRef();
    }
}
SharedFence::SharedFence(SharedFence &&other)
    : mRefCountedFence(other.mRefCountedFence), mRecycler(other.mRecycler)
{
    other.mRecycler        = nullptr;
    other.mRefCountedFence = nullptr;
}

SharedFence::~SharedFence()
{
    release();
}

VkResult SharedFence::init(VkDevice device, FenceRecycler *recycler)
{
    ASSERT(mRecycler == nullptr && mRefCountedFence == nullptr);
    Fence fence;

    // First try to fetch from recycler. If that failed, try to create a new VkFence
    recycler->fetch(device, &fence);
    if (!fence.valid())
    {
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags             = 0;
        VkResult result                   = fence.init(device, fenceCreateInfo);
        if (result != VK_SUCCESS)
        {
            return result;
        }
    }

    // Create a new refcounted object to hold onto VkFence
    mRefCountedFence = new RefCounted<Fence>(std::move(fence));
    mRefCountedFence->addRef();
    mRecycler = recycler;

    return VK_SUCCESS;
}

SharedFence &SharedFence::operator=(const SharedFence &other)
{
    release();

    mRecycler = other.mRecycler;
    if (other.mRefCountedFence != nullptr)
    {
        mRefCountedFence = other.mRefCountedFence;
        mRefCountedFence->addRef();
    }
    return *this;
}

SharedFence &SharedFence::operator=(SharedFence &&other)
{
    release();
    mRecycler              = other.mRecycler;
    mRefCountedFence       = other.mRefCountedFence;
    other.mRecycler        = nullptr;
    other.mRefCountedFence = nullptr;
    return *this;
}

void SharedFence::destroy(VkDevice device)
{
    if (mRefCountedFence != nullptr)
    {
        mRefCountedFence->releaseRef();
        if (!mRefCountedFence->isReferenced())
        {
            mRefCountedFence->get().destroy(device);
            SafeDelete(mRefCountedFence);
        }
        else
        {
            mRefCountedFence = nullptr;
        }
        mRecycler = nullptr;
    }
}

void SharedFence::release()
{
    if (mRefCountedFence != nullptr)
    {
        mRefCountedFence->releaseRef();
        if (!mRefCountedFence->isReferenced())
        {
            mRecycler->recycle(std::move(mRefCountedFence->get()));
            ASSERT(!mRefCountedFence->get().valid());
            SafeDelete(mRefCountedFence);
        }
        else
        {
            mRefCountedFence = nullptr;
        }
        mRecycler = nullptr;
    }
}

SharedFence::operator bool() const
{
    ASSERT(mRefCountedFence == nullptr || mRefCountedFence->isReferenced());
    return mRefCountedFence != nullptr;
}

VkResult SharedFence::getStatus(VkDevice device) const
{
    if (mRefCountedFence != nullptr)
    {
        return mRefCountedFence->get().getStatus(device);
    }
    return VK_SUCCESS;
}

VkResult SharedFence::wait(VkDevice device, uint64_t timeout) const
{
    if (mRefCountedFence != nullptr)
    {
        return mRefCountedFence->get().wait(device, timeout);
    }
    return VK_SUCCESS;
}

// FenceRecycler implementation
void FenceRecycler::destroy(Context *context)
{
    std::lock_guard<std::mutex> lock(mMutex);
    mRecyler.destroy(context->getDevice());
}

void FenceRecycler::fetch(VkDevice device, Fence *fenceOut)
{
    ASSERT(fenceOut != nullptr && !fenceOut->valid());
    std::lock_guard<std::mutex> lock(mMutex);
    if (!mRecyler.empty())
    {
        mRecyler.fetch(fenceOut);
        fenceOut->reset(device);
    }
}

void FenceRecycler::recycle(Fence &&fence)
{
    std::lock_guard<std::mutex> lock(mMutex);
    mRecyler.recycle(std::move(fence));
}

// CommandProcessorTask implementation
void CommandProcessorTask::initTask()
{
    mTask                           = CustomTask::Invalid;
    mOutsideRenderPassCommandBuffer = nullptr;
    mRenderPassCommandBuffer        = nullptr;
    mRenderPass                     = nullptr;
    mCommandPools                   = nullptr;
    mSubmitItemIndex                = -1;
    mPresentInfo                    = {};
    mPresentInfo.pResults           = nullptr;
    mPresentInfo.pSwapchains        = nullptr;
    mPresentInfo.pImageIndices      = nullptr;
    mPresentInfo.pNext              = nullptr;
    mPresentInfo.pWaitSemaphores    = nullptr;
    mPresentFence                   = VK_NULL_HANDLE;
    mSwapchainStatus                = nullptr;
    mPriority                       = egl::ContextPriority::Medium;
    mHasProtectedContent            = false;
}

void CommandProcessorTask::initOutsideRenderPassProcessCommands(
    bool hasProtectedContent,
    egl::ContextPriority priority,
    OutsideRenderPassCommandBufferHelper *commandBuffer)
{
    mTask                           = CustomTask::ProcessOutsideRenderPassCommands;
    mOutsideRenderPassCommandBuffer = commandBuffer;
    mHasProtectedContent            = hasProtectedContent;
    mPriority                       = priority;
}

void CommandProcessorTask::initRenderPassProcessCommands(
    bool hasProtectedContent,
    egl::ContextPriority priority,
    RenderPassCommandBufferHelper *commandBuffer,
    const RenderPass *renderPass)
{
    mTask                    = CustomTask::ProcessRenderPassCommands;
    mRenderPassCommandBuffer = commandBuffer;
    mRenderPass              = renderPass;
    mHasProtectedContent     = hasProtectedContent;
    mPriority                = priority;
}

void CommandProcessorTask::copyPresentInfo(const VkPresentInfoKHR &other)
{
    if (other.sType == 0)
    {
        return;
    }

    mPresentInfo.sType = other.sType;
    mPresentInfo.pNext = nullptr;

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
                AddToPNextChain(&mPresentInfo, &mPresentRegions);
                pNext = const_cast<void *>(presentRegions->pNext);
                break;
            }
            case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_EXT:
            {
                const VkSwapchainPresentFenceInfoEXT *presentFenceInfo =
                    reinterpret_cast<VkSwapchainPresentFenceInfoEXT *>(pNext);
                ASSERT(presentFenceInfo->swapchainCount == 1);
                mPresentFence = presentFenceInfo->pFences[0];

                mPresentFenceInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_EXT;
                mPresentFenceInfo.pNext = nullptr;
                mPresentFenceInfo.swapchainCount = 1;
                mPresentFenceInfo.pFences        = &mPresentFence;
                AddToPNextChain(&mPresentInfo, &mPresentFenceInfo);
                pNext = const_cast<void *>(presentFenceInfo->pNext);
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
                                       const VkPresentInfoKHR &presentInfo,
                                       SwapchainStatus *swapchainStatus)
{
    mTask            = CustomTask::Present;
    mPriority        = priority;
    mSwapchainStatus = swapchainStatus;
    copyPresentInfo(presentInfo);
}

void CommandProcessorTask::initFlushAndQueueSubmit(
    SecondaryCommandPools *commandPools,
    SecondaryCommandBufferList &&commandBuffersToReset,
    uint32_t submitItemIndex)
{
    mTask                  = CustomTask::FlushAndQueueSubmit;
    mCommandPools          = commandPools;
    mCommandBuffersToReset = std::move(commandBuffersToReset);
    mSubmitItemIndex       = submitItemIndex;
}

void CommandProcessorTask::initOneOffQueueSubmit(uint32_t submitItemIndex)
{
    mTask            = CustomTask::OneOffQueueSubmit;
    mSubmitItemIndex = submitItemIndex;
}

CommandProcessorTask &CommandProcessorTask::operator=(CommandProcessorTask &&rhs)
{
    if (this == &rhs)
    {
        return *this;
    }

    std::swap(mRenderPass, rhs.mRenderPass);
    std::swap(mOutsideRenderPassCommandBuffer, rhs.mOutsideRenderPassCommandBuffer);
    std::swap(mRenderPassCommandBuffer, rhs.mRenderPassCommandBuffer);
    std::swap(mTask, rhs.mTask);
    std::swap(mCommandPools, rhs.mCommandPools);
    std::swap(mCommandBuffersToReset, rhs.mCommandBuffersToReset);
    std::swap(mSubmitItemIndex, rhs.mSubmitItemIndex);
    std::swap(mPriority, rhs.mPriority);
    std::swap(mHasProtectedContent, rhs.mHasProtectedContent);

    copyPresentInfo(rhs.mPresentInfo);
    std::swap(mSwapchainStatus, rhs.mSwapchainStatus);

    // clear rhs now that everything has moved.
    rhs.initTask();

    return *this;
}

// CommandProcessor implementation.
void CommandProcessor::handleError(VkResult errorCode,
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
        handleDeviceLost(mRenderer);
    }

    std::lock_guard<std::mutex> queueLock(mErrorMutex);
    Error error = {errorCode, file, function, line};
    mErrors.emplace(error);
}

CommandProcessor::CommandProcessor(RendererVk *renderer, std::mutex &mutex)
    : Context(renderer), mWorkerThreadIdle(false), mCommandQueue(mutex)
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
            ANGLE_TRY(mCommandQueue.waitIdle(this, mRenderer->getMaxFenceWaitTimeNs()));
            // Shutting down so cleanup
            mCommandQueue.destroy(this);
            break;
        }
        case CustomTask::FlushAndQueueSubmit:
        {
            ANGLE_TRACE_EVENT0("gpu.angle", "processTask::FlushAndQueueSubmit");
            // End command buffer

            // Call doSubmitCommands() - see prepareNextSubmit()
            ANGLE_TRY(mCommandQueue.doSubmitCommands(
                this, std::move(task->getCommandBuffersToReset()), task->getCommandPools(),
                task->getSubmitItemIndex()));
            break;
        }
        case CustomTask::OneOffQueueSubmit:
        {
            ANGLE_TRACE_EVENT0("gpu.angle", "processTask::OneOffQueueSubmit");

            // Call doQueueSubmitOneOff() - see prepareNextSubmit()
            ANGLE_TRY(mCommandQueue.doQueueSubmitOneOff(this, task->getSubmitItemIndex()));
            break;
        }
        case CustomTask::Present:
        {
            ANGLE_TRACE_EVENT0("gpu.angle", "processTask::Present");
            mCommandQueue.doQueuePresent(
                this, task->getPriority(), task->getPresentInfo(),
                [this, swapchainStatus = task->getSwapchainStatus()](
                    const VkPresentInfoKHR &presentInfo, VkResult presentResult) {
                    // Verify that we are presenting one and only one swapchain
                    ASSERT(presentInfo.swapchainCount == 1);
                    ASSERT(presentInfo.pResults == nullptr);
                    updateSwapchainStatus(swapchainStatus, presentResult);
                });
            break;
        }
        case CustomTask::ProcessOutsideRenderPassCommands:
        {
            OutsideRenderPassCommandBufferHelper *commandBuffer =
                task->getOutsideRenderPassCommandBuffer();
            ANGLE_TRY(mCommandQueue.doFlushOutsideRPCommands(this, task->hasProtectedContent(),
                                                             task->getPriority(), &commandBuffer));

            OutsideRenderPassCommandBufferHelper *originalCommandBuffer =
                task->getOutsideRenderPassCommandBuffer();
            mRenderer->recycleOutsideRenderPassCommandBufferHelper(mRenderer->getDevice(),
                                                                   &originalCommandBuffer);
            break;
        }
        case CustomTask::ProcessRenderPassCommands:
        {
            RenderPassCommandBufferHelper *commandBuffer = task->getRenderPassCommandBuffer();
            ANGLE_TRY(mCommandQueue.doFlushRenderPassCommands(
                this, task->hasProtectedContent(), task->getPriority(), *task->getRenderPass(),
                &commandBuffer));

            RenderPassCommandBufferHelper *originalCommandBuffer =
                task->getRenderPassCommandBuffer();
            mRenderer->recycleRenderPassCommandBufferHelper(mRenderer->getDevice(),
                                                            &originalCommandBuffer);
            break;
        }
        default:
            UNREACHABLE();
            break;
    }

    return angle::Result::Continue;
}

angle::Result CommandProcessor::checkCompletedCommands(Context *context)
{
    return mCommandQueue.checkCompletedCommands(context);
}

angle::Result CommandProcessor::waitForWorkComplete(Context *context)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandProcessor::waitForWorkComplete");
    std::unique_lock<std::mutex> lock(mWorkerMutex);
    mWorkerIdleCondition.wait(lock, [this] { return (mTasks.empty() && mWorkerThreadIdle); });
    // Worker thread is idle and command queue is empty so good to continue

    mCommandQueue.waitSubmitThreadIdle();

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

void CommandProcessor::cleanupAllCompletedGarbage(Context *context)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandProcessor::cleanupAllCompletedGarbage");
    std::unique_lock<std::mutex> lock(mWorkerMutex);
    mWorkerIdleCondition.wait(lock, [this] { return (mTasks.empty() && mWorkerThreadIdle); });

    // Worker thread is idle and command queue is empty so good to continue
    mCommandQueue.cleanupAllCompletedGarbage(context);
}

void CommandProcessor::handleDeviceLost(RendererVk *renderer)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandProcessor::handleDeviceLost");
    std::unique_lock<std::mutex> lock(mWorkerMutex);
    mWorkerIdleCondition.wait(lock, [this] { return (mTasks.empty() && mWorkerThreadIdle); });

    // Worker thread is idle and command queue is empty so good to continue
    mCommandQueue.handleDeviceLost(renderer);
}

void CommandProcessor::updateSwapchainStatus(SwapchainStatus *swapchainStatus,
                                             VkResult presentResult)
{
    ASSERT(swapchainStatus);

    swapchainStatus->lastPresentResult = presentResult;

    {
        std::unique_lock<std::mutex> lock(swapchainStatus->mutex);
        ASSERT(swapchainStatus->isPending);
        swapchainStatus->isPending = false;
        swapchainStatus->condVar.unlockAndNotifyAll(lock);
    }
}

angle::Result CommandProcessor::waitForQueueSerialActuallySubmitted(vk::Context *context,
                                                                    const QueueSerial &queueSerial)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandProcessor::waitForQueueSerialActuallySubmitted");
    angle::Result result = checkAndPopPendingError(context);
    ANGLE_TRY(
        mCommandQueue.waitForQueueSerialActuallySubmitted(context->getRenderer(), queueSerial));
    return result;
}

// Wait until all commands up to and including serial have been processed
angle::Result CommandProcessor::finishResourceUse(Context *context,
                                                  const ResourceUse &use,
                                                  uint64_t timeout)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandProcessor::finishResourceUse");
    angle::Result result = checkAndPopPendingError(context);
    ANGLE_TRY(mCommandQueue.finishResourceUse(context, use, timeout));
    return result;
}

angle::Result CommandProcessor::finishQueueSerial(Context *context,
                                                  const QueueSerial &queueSerial,
                                                  uint64_t timeout)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandProcessor::finishQueueSerial");
    angle::Result result = checkAndPopPendingError(context);
    ANGLE_TRY(mCommandQueue.finishQueueSerial(context, queueSerial, timeout));
    return result;
}

angle::Result CommandProcessor::waitIdle(Context *context, uint64_t timeout)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandProcessor::waitIdle");
    // Using waitForWorkComplete() just to proccess all errors.
    angle::Result result = waitForWorkComplete(context);
    ANGLE_TRY(mCommandQueue.waitIdle(context, timeout));
    return result;
}

angle::Result CommandProcessor::waitForResourceUseToFinishWithUserTimeout(Context *context,
                                                                          const ResourceUse &use,
                                                                          uint64_t timeout,
                                                                          VkResult *result)
{
    return mCommandQueue.waitForResourceUseToFinishWithUserTimeout(context, use, timeout, result);
}

bool CommandProcessor::isBusy() const
{
    std::lock_guard<std::mutex> workerLock(mWorkerMutex);
    return !mTasks.empty() || !mWorkerThreadIdle || mCommandQueue.isBusy();
}

angle::Result CommandProcessor::submitCommands(Context *context,
                                               bool hasProtectedContent,
                                               egl::ContextPriority priority,
                                               const VkSemaphore signalSemaphore,
                                               SecondaryCommandBufferList &&commandBuffersToReset,
                                               SecondaryCommandPools *commandPools,
                                               const QueueSerial &submitQueueSerial)
{
    ANGLE_TRY(checkAndPopPendingError(context));

    CommandQueue2::QueueItemIndex itemIndex = -1;
    ANGLE_TRY(mCommandQueue.prepareNextSubmit(context, hasProtectedContent, priority,
                                              signalSemaphore, submitQueueSerial, &itemIndex));

    CommandProcessorTask task;
    task.initFlushAndQueueSubmit(commandPools, std::move(commandBuffersToReset), itemIndex);

    queueCommand(std::move(task));

    return angle::Result::Continue;
}

angle::Result CommandProcessor::queueSubmitOneOff(Context *context,
                                                  bool hasProtectedContent,
                                                  egl::ContextPriority contextPriority,
                                                  VkCommandBuffer commandBufferHandle,
                                                  const Semaphore *waitSemaphore,
                                                  VkPipelineStageFlags waitSemaphoreStageMask,
                                                  const Fence *fence,
                                                  SubmitPolicy submitPolicy,
                                                  const QueueSerial &submitQueueSerial)
{
    ANGLE_TRY(checkAndPopPendingError(context));

    CommandQueue2::QueueItemIndex itemIndex = -1;
    ANGLE_TRY(mCommandQueue.prepareNextSubmitOneOff(
        context, hasProtectedContent, contextPriority, commandBufferHandle, waitSemaphore,
        waitSemaphoreStageMask, fence, submitQueueSerial, &itemIndex));

    CommandProcessorTask task;
    task.initOneOffQueueSubmit(itemIndex);
    queueCommand(std::move(task));

    if (submitPolicy == SubmitPolicy::EnsureSubmitted)
    {
        // Caller has synchronization requirement to have work in GPU pipe when returning from this
        // function.
        ANGLE_TRY(mCommandQueue.waitForQueueSerialActuallySubmitted(context->getRenderer(),
                                                                    submitQueueSerial));
    }

    return angle::Result::Continue;
}

VkResult CommandProcessor::queuePresent(egl::ContextPriority contextPriority,
                                        const VkPresentInfoKHR &presentInfo,
                                        SwapchainStatus *swapchainStatus)
{
    {
        std::lock_guard<std::mutex> lock(swapchainStatus->mutex);
        ASSERT(!swapchainStatus->isPending);
        swapchainStatus->isPending = true;
    }

    CommandProcessorTask task;
    task.initPresent(contextPriority, presentInfo, swapchainStatus);

    ANGLE_TRACE_EVENT0("gpu.angle", "CommandProcessor::queuePresent");
    queueCommand(std::move(task));

    // Always return success, when we call acquireNextImage we'll check the return code. This
    // allows the app to continue working until we really need to know the return code from
    // present.
    return VK_SUCCESS;
}

angle::Result CommandProcessor::flushOutsideRPCommands(
    Context *context,
    bool hasProtectedContent,
    egl::ContextPriority priority,
    OutsideRenderPassCommandBufferHelper **outsideRPCommands)
{
    ANGLE_TRY(checkAndPopPendingError(context));

    mCommandQueue.onCommandsFlush(hasProtectedContent, priority);

    (*outsideRPCommands)->markClosed();

    // Detach functions are only used for ring buffer allocators.
    SecondaryCommandMemoryAllocator *allocator = (*outsideRPCommands)->detachAllocator();

    CommandProcessorTask task;
    task.initOutsideRenderPassProcessCommands(hasProtectedContent, priority, *outsideRPCommands);
    queueCommand(std::move(task));

    ANGLE_TRY(mRenderer->getOutsideRenderPassCommandBufferHelper(
        context, (*outsideRPCommands)->getCommandPool(), allocator, outsideRPCommands));

    return angle::Result::Continue;
}

angle::Result CommandProcessor::flushRenderPassCommands(
    Context *context,
    bool hasProtectedContent,
    egl::ContextPriority priority,
    const RenderPass &renderPass,
    RenderPassCommandBufferHelper **renderPassCommands)
{
    ANGLE_TRY(checkAndPopPendingError(context));

    mCommandQueue.onCommandsFlush(hasProtectedContent, priority);

    (*renderPassCommands)->markClosed();

    // Detach functions are only used for ring buffer allocators.
    SecondaryCommandMemoryAllocator *allocator = (*renderPassCommands)->detachAllocator();

    CommandProcessorTask task;
    task.initRenderPassProcessCommands(hasProtectedContent, priority, *renderPassCommands,
                                       &renderPass);
    queueCommand(std::move(task));

    ANGLE_TRY(mRenderer->getRenderPassCommandBufferHelper(
        context, (*renderPassCommands)->getCommandPool(), allocator, renderPassCommands));

    return angle::Result::Continue;
}

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

// CommandQueue2 implementation.

CommandQueue2::CommandQueue2(std::mutex &mutex) : mMutex(mutex) {}

angle::Result CommandQueue2::init(Context *context, const DeviceQueueMap &queueMap)
{
    // In case of RendererVk gets re-initialized, we can't rely on constructor to do initialization
    // for us.
    mLastSubmittedSerials.fill(kZeroSerial);
    mLastCompletedSerials.fill(kZeroSerial);
    mLastCompletedSerial = kZeroSerial;

    mQueueMap = queueMap;

    ANGLE_TRY(mCmdsStateMap[0].pool.init(context, false, queueMap.getIndex()));

    if (queueMap.isProtected())
    {
        ANGLE_TRY(mCmdsStateMap[1].pool.init(context, true, queueMap.getIndex()));
    }

    if (context->getRenderer()->isAsyncCommandQueueEnabled() &&
        context->getRenderer()->getFeatures().useSubmitThread.enabled)
    {
        // CommandProcessor is required for handleError() and Present result handling.
        mSubmitThreadTaskQueue.init();
        mUseSubmitThread = true;
    }

    return angle::Result::Continue;
}

void CommandQueue2::destroy(Context *context)
{
    mUseSubmitThread = false;
    mSubmitThreadTaskQueue.destroy();

    for (VkQueue queue : mQueueMap)
    {
        if (queue != VK_NULL_HANDLE)
        {
            (void)vkQueueWaitIdle(queue);
        }
    }

    // Mark everything as completed
    mLastCompletedSerials.fill(Serial::Infinite());
    mLastCompletedSerial = Serial::Infinite();

    collectCommandBuffers(context, Serial::Infinite());
    resetSubmittedItems();
    resetCompletedBuffers(context);

    ASSERT(mPendingStateResetIndex == mNextPrepareIndex);
    ASSERT(mCommandBufferResetIndex == mNextSubmitIndex);
    ASSERT(mSubmittedStateResetIndex == mNextSubmitIndex);
    ASSERT(mNextPrepareIndex == mNextSubmitIndex);

    const VkDevice device = context->getDevice();

    for (CmdsState &state : mCmdsStateMap)
    {
        for (CmdsState::QueueState &queueState : state.queueStates)
        {
            queueState.currentBuffer.destroy(device);
            queueState.waitSemaphores.clear();
            queueState.waitSemaphoreStageMasks.clear();
        }
        state.pool.destroy(device);
    }

    mFenceRecycler.destroy(context);
}

void CommandQueue2::waitSubmitThreadIdle()
{
    if (mUseSubmitThread)
    {
        mSubmitThreadTaskQueue.waitIdle();
    }
}

void CommandQueue2::cleanupAllCompletedGarbage(Context *context)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::cleanupAllCompletedGarbage");

    // Wait here, to be able to safely call below methods
    waitSubmitThreadIdle();

    // On some Vulkan drivers, vkResetCommandBuffer() may cause undefined behaviour, when resetting
    // primary command buffer that has recorded secondary command buffers with it's pool destroyed.
    // In order to circumvents this issue, such primary command buffers must be reset before
    // destroying secondary command pools (before ContextVk destruction).
    // In normal cases, reseting command buffers is not necessary, but performed for consistency.

    const Serial completedSerial = mLastCompletedSerial.getSerial();

    collectCommandBuffers(context, completedSerial);
    resetSubmittedItems();

    if (mUseSubmitThread)
    {
        resetCompletedBuffers(context);
    }

    cleanupGarbage(context->getRenderer(), completedSerial);
}

void CommandQueue2::handleDeviceLost(RendererVk *renderer)
{
    waitSubmitThreadIdle();

    VkDevice device = renderer->getDevice();

    for (QueueItemIndex index = 0; index < kQueueCapacity; ++index)
    {
        QueueItem &item = mItemQueue[index];
        item.resetState();

        if (item.fence)
        {
            // On device loss we need to wait for fence to be signaled before destroying it
            VkResult status = item.fence.get().wait(device, renderer->getMaxFenceWaitTimeNs());
            // If the wait times out, it is probably not possible to recover from lost device
            ASSERT(status == VK_SUCCESS || status == VK_ERROR_DEVICE_LOST);
            item.fence.destroy(device);
        }

        // On device lost, here simply destroy the CommandBuffer, it will fully cleared later
        // by CommandPool::destroy
        item.commandBuffer.destroy(device);
        item.resetSecondaryCommands(device);

        mLastCompletedSerials.trySetQueueSerial(item.queueSerial);
        mLastCompletedSerial.updateIfIncrease(item.serial);
    }

    mNextPrepareIndex       = 0;
    mPendingStateResetIndex = 0;

    mNextSubmitIndex          = 0;
    mCommandBufferResetIndex  = 0;
    mSubmittedStateResetIndex = 0;
}

void CommandQueue2::resetPerFramePerfCounters()
{
    mPerfCounters.commandQueueSubmitCallsPerFrame = 0;
    mPerfCounters.vkQueueSubmitCallsPerFrame      = 0;
}

CommandQueue2::QueueItemIndex CommandQueue2::IncIndex(QueueItemIndex index, QueueSize value)
{
    return (index + value) % kQueueCapacity;
}

CommandQueue2::QueueItemIndex CommandQueue2::DecIndex(QueueItemIndex index, QueueSize value)
{
    return (kQueueCapacity + index - value) % kQueueCapacity;
}

template <class GetFenceStatus>
angle::Result CommandQueue2::checkCompletedItems(Context *context,
                                                 QueueItemIndex beginIndex,
                                                 QueueItemIndex endIndex,
                                                 GetFenceStatus &&getFenceStatus,
                                                 Serial *completedSerialOut)
{
    const Serial completedSerial = mLastCompletedSerial.acquireSerial();

    angle::FastMap<Serial, kMaxFastQueueSerials> completedSerials;
    *completedSerialOut = completedSerial;

    for (QueueItemIndex index = beginIndex; index != endIndex; index = IncIndex(index))
    {
        QueueItem &item = mItemQueue[index];
        if (item.serial > completedSerial)  // Need check fence status
        {
            const VkResult status = getFenceStatus(item);
            if (status == VK_SUCCESS)
            {
                completedSerials[item.queueSerial.getIndex()] = item.queueSerial.getSerial();
                *completedSerialOut                           = item.serial;
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
    }

    if (*completedSerialOut > completedSerial)
    {
        updateLastCompletedSerials(completedSerials, *completedSerialOut);
    }

    return angle::Result::Continue;
}

template <class TryResetItem>
void CommandQueue2::resetCompletedItems(QueueItemIndex *beginIndex,
                                        QueueItemIndex endIndex,
                                        Serial completedSerial,
                                        TryResetItem &&tryResetItem)
{
    while (*beginIndex != endIndex && mItemQueue[*beginIndex].serial <= completedSerial)
    {
        if (!tryResetItem(mItemQueue[*beginIndex]))
        {
            break;
        }
        *beginIndex = IncIndex(*beginIndex);
    }
}

angle::Result CommandQueue2::checkCompletedCommands(Context *context)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::checkCompletedCommands");
    const VkDevice device = context->getDevice();

    Serial completedSerial;
    ANGLE_TRY(checkCompletedItems(
        context, mPendingStateResetIndex, mNextPrepareIndex,
        [device](QueueItem &item) {
            return item.hasSubmit ? item.getPendingFenceStatus(device, VK_SUCCESS) : VK_SUCCESS;
        },
        &completedSerial));

    resetPendingItems(device, completedSerial, true);  // maySkip

    return angle::Result::Continue;
}

angle::Result CommandQueue2::finishResourceUse(Context *context,
                                               const ResourceUse &use,
                                               uint64_t timeout)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::finishResourceUse");
    ANGLE_TRY(waitAndResetPendingItems(context, use, kZeroSerial, timeout, true));  // mayUnlock
    return angle::Result::Continue;
}

angle::Result CommandQueue2::finishQueueSerial(Context *context,
                                               const QueueSerial &queueSerial,
                                               uint64_t timeout)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::finishQueueSerial");
    vk::ResourceUse use(queueSerial);
    ANGLE_TRY(waitAndResetPendingItems(context, use, kZeroSerial, timeout, true));  // mayUnlock
    return angle::Result::Continue;
}

angle::Result CommandQueue2::waitIdle(Context *context, uint64_t timeout)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::waitIdle");
    if (mNextPrepareIndex != mPendingStateResetIndex)
    {
        CommandQueue2::QueueItem &item = mItemQueue[DecIndex(mNextPrepareIndex)];
        ANGLE_TRY(waitAndResetPendingItems(context, item, timeout, true));  // mayUnlock
    }
    return angle::Result::Continue;
}

bool CommandQueue2::isBusy() const
{
    return mNextPrepareIndex != mPendingStateResetIndex &&
           mItemQueue[DecIndex(mNextPrepareIndex)].queueSerial > mLastCompletedSerials;
}

angle::Result CommandQueue2::waitForResourceUseToFinishWithUserTimeout(Context *context,
                                                                       const ResourceUse &use,
                                                                       uint64_t timeout,
                                                                       VkResult *result)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::waitForResourceUseToFinishWithUserTimeout");

    const Serial completedSerial = mLastCompletedSerial.acquireSerial();
    QueueItemIndex index         = kInvalidQueueItemIndex;
    if (!forEachUncompletedItem(
            completedSerial, use.getSerials(), [](const QueueItem &item) {}, &index) ||
        index == kInvalidQueueItemIndex)
    {
        *result = VK_SUCCESS;
        return angle::Result::Continue;
    }

    {
        mItemQueue[index].acquireLock();
        MutexUnlock<std::mutex> unlock(&mMutex);

        DynamicTimeout dynamicTimeout{timeout};
        *result =
            mItemQueue[index].waitPendingFence(context->getDevice(), dynamicTimeout, VK_TIMEOUT);

        mItemQueue[index].releaseLock();
    }

    if (*result != VK_TIMEOUT)  // Timeout is not an error
    {
        ANGLE_VK_TRY(context, *result);
    }

    return angle::Result::Continue;
}

angle::Result CommandQueue2::waitForQueueSerialActuallySubmitted(RendererVk *renderer,
                                                                 const QueueSerial &queueSerial)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::waitForQueueSerialActuallySubmitted");
    ASSERT(renderer->isAsyncCommandQueueEnabled());
    ASSERT(queueSerial.getSerial() <= mLastSubmittedSerials[queueSerial.getIndex()]);

    QueueItemIndex index = mNextPrepareIndex;

    // Search in backwards order, because it is more likely that we wait for the recent submission.
    do
    {
        if (index == mPendingStateResetIndex)
        {
            // If not found must be already completed. Otherwise - invalid queueSerial passed.
            ASSERT(queueSerial.getSerial() <= mLastCompletedSerials[queueSerial.getIndex()]);
            return angle::Result::Continue;
        }
        index = DecIndex(index);
    } while (mItemQueue[index].queueSerial != queueSerial);

    QueueItem::State state = mItemQueue[index].getState();

    if (state == QueueItem::State::Idle)
    {
        mItemQueue[index].acquireLock();
        MutexUnlock<std::mutex> unlock(&mMutex);

        DynamicTimeout dynamicTimeout{renderer->getMaxFenceWaitTimeNs()};
        state = mItemQueue[index].waitSubmitted(dynamicTimeout);

        mItemQueue[index].releaseLock();
    }

    if (state != QueueItem::State::Submitted && state != QueueItem::State::Finished)
    {
        return angle::Result::Stop;
    }

    return angle::Result::Continue;
}

template <class OnEachItem>
bool CommandQueue2::forEachUncompletedItem(Serial completedSerial,
                                           const Serials &serials,
                                           OnEachItem &&onEachItem,
                                           QueueItemIndex *waitIndexOut)
{
    angle::FastVector<bool, kMaxFastQueueSerials> indexesNeedWait(serials.size(), false);
    uint32_t numIndexesNeedWait = 0;

    // Find what QueueSerialIndexes need waiting.
    // This information guaranteed to be not older than "completedSerial" acquired before.
    for (SerialIndex index = 0; index < serials.size(); ++index)
    {
        ASSERT(serials[index] <= mLastSubmittedSerials[index]);
        if (serials[index] > mLastCompletedSerials[index])
        {
            indexesNeedWait[index] = true;
            ++numIndexesNeedWait;
        }
    }

    // No QueueSerialIndexes to wait on...
    if (numIndexesNeedWait == 0)
    {
        return false;
    }

    // Find latest "QueueItem" with submit that should be waited (may have none).
    // Call "onEachItem()" callback for additional processing.
    for (QueueItemIndex index                                        = mPendingStateResetIndex;
         numIndexesNeedWait > 0 && index != mNextPrepareIndex; index = IncIndex(index))
    {
        const QueueItem &queueItem = mItemQueue[index];
        if (queueItem.serial <= completedSerial)
        {
            continue;
        }
        if (queueItem.hasSubmit)
        {
            *waitIndexOut = index;
        }
        const QueueSerial &queueSerial = queueItem.queueSerial;
        if (queueSerial.getIndex() < serials.size() && indexesNeedWait[queueSerial.getIndex()] &&
            queueSerial.getSerial() >= serials[queueSerial.getIndex()])
        {
            indexesNeedWait[queueSerial.getIndex()] = false;
            --numIndexesNeedWait;
        }
        onEachItem(queueItem);
    }

    return true;
}

angle::Result CommandQueue2::waitAndResetPendingItems(Context *context,
                                                      const QueueItem &waitItem,
                                                      uint64_t timeout,
                                                      bool mayUnlock)
{
    vk::ResourceUse use(waitItem.queueSerial);
    return waitAndResetPendingItems(context, use, waitItem.serial, timeout, mayUnlock);
}

angle::Result CommandQueue2::waitAndResetPendingItems(Context *context,
                                                      const ResourceUse &use,
                                                      Serial waitSerial,
                                                      uint64_t timeout,
                                                      bool mayUnlock)
{
    // Acquire "mLastCompletedSerial" before "mLastCompletedSerials" for proper memory ordering.
    // This Serial is used to remove internal logic dependency on "mLastCompletedSerials".
    Serial completedSerial = mLastCompletedSerial.acquireSerial();

    angle::FastMap<Serial, kMaxFastQueueSerials> completedSerialsAfterWait;
    Serial completedSerialAfterWait;
    QueueItemIndex waitIndex = kInvalidQueueItemIndex;

    if (forEachUncompletedItem(
            completedSerial, use.getSerials(),
            [&completedSerialsAfterWait, &completedSerialAfterWait](const QueueItem &item) {
                const QueueSerial &queueSerial                    = item.queueSerial;
                completedSerialsAfterWait[queueSerial.getIndex()] = queueSerial.getSerial();
                completedSerialAfterWait                          = item.serial;
            },
            &waitIndex))
    {
        // Wait up to the "mItemQueue[waitIndex]" if there is such item.
        if (waitIndex != kInvalidQueueItemIndex)
        {
            ANGLE_TRY(waitPendingItems(context, completedSerial, waitIndex, timeout, mayUnlock));
        }

        ASSERT(completedSerialAfterWait > completedSerial);
        ASSERT(waitSerial == kZeroSerial || waitSerial == completedSerialAfterWait);
        updateLastCompletedSerials(completedSerialsAfterWait, completedSerialAfterWait);
        completedSerial = completedSerialAfterWait;
    }
    // No QueueSerialIndexes to wait on
    else if (waitSerial > completedSerial)
    {
        // "completedSerial" may be slightly outdated, because other thread updated
        // "mLastCompletedSerial" after "mLastCompletedSerials". In most cases this is ok.
        // But there are cases when it is critical to "resetPendingState()" for all completed items.
        // Luckily, in all such cases we may pass "waitSerial" directly from a "QueueItem",
        // so we do not need to search for the "completedSerial".
        mLastCompletedSerial.updateIfIncrease(waitSerial);
        completedSerial = waitSerial;
    }

    ASSERT(completedSerial >= waitSerial);
    resetPendingItems(context->getDevice(), completedSerial, mayUnlock);

    return angle::Result::Continue;
}

angle::Result CommandQueue2::waitPendingItems(Context *context,
                                              Serial completedSerial,
                                              QueueItemIndex waitIndex,
                                              uint64_t timeout,
                                              bool mayUnlock)
{
    ASSERT(waitIndex != kInvalidQueueItemIndex);
    ASSERT(mItemQueue[waitIndex].serial > completedSerial);

    const VkDevice device = context->getDevice();

    // Serial:      1 2 3 4 5 6 7 8 9
    // waitIndex:               ^
    // VkQueue:     1 1 2 2 1 2 2 1 2
    // wait fences:         ^   ^     - Need to wait latest fence from each queue up to waitIndex

    VkResult waitResult = VK_SUCCESS;

    DynamicTimeout dynamicTimeout{timeout};
    std::array<VkQueue, kVkQueueCount> waitedQueues{};
    size_t waitedQueueCount = 0;
    QueueItemIndex index    = waitIndex;
    VkQueue curQueue        = mItemQueue[index].queue;
    bool isCurQueueWaited   = false;
    bool isLastSubmitWaited = false;

    while (true)
    {
        ASSERT(waitResult == VK_SUCCESS);
        QueueItem &item         = mItemQueue[index];
        const Serial itemSerial = item.serial;  // Need local copy for later use
        if (itemSerial <= completedSerial)
        {
            break;
        }
        if (ANGLE_UNLIKELY(item.queue != curQueue))
        {
            curQueue         = item.queue;
            const auto last  = waitedQueues.begin() + waitedQueueCount;
            isCurQueueWaited = (std::find(waitedQueues.begin(), last, curQueue) != last);
        }
        if (ANGLE_UNLIKELY(!isCurQueueWaited && item.hasSubmit))
        {
            bool wasUnlock = false;
            if (isLastSubmitWaited)
            {
                waitResult = item.getFenceStatus(device, VK_INCOMPLETE);
            }
            else
            {
                waitResult = VK_NOT_READY;
            }
            if (waitResult == VK_NOT_READY)
            {
                MutexUnlock<std::mutex> unlock;
                if (mayUnlock)
                {
                    item.acquireLock();
                    unlock.unlock(&mMutex);
                    wasUnlock = true;
                }
                if (isLastSubmitWaited)
                {
                    waitResult = item.waitFence(device, dynamicTimeout);
                }
                else
                {
                    waitResult = item.waitPendingFence(device, dynamicTimeout, VK_INCOMPLETE);
                    isLastSubmitWaited = true;
                }
                if (mayUnlock)
                {
                    item.releaseLock();  // Do not access "item" after this line
                }
            }
            if (waitResult == VK_SUCCESS)
            {
                waitedQueues[waitedQueueCount] = curQueue;
                ++waitedQueueCount;
                isCurQueueWaited = true;
            }
            else if (waitResult != VK_INCOMPLETE)  // Submit OK, but wait timeout/failed
            {
                break;
            }
            else  // Submit failed - skip fence
            {
                waitResult = VK_SUCCESS;
            }
            // Items were reset while this thread was waiting inside the "unlock".
            if (wasUnlock && ((mPendingStateResetIndex == mNextPrepareIndex) ||
                              (mItemQueue[mPendingStateResetIndex].serial > itemSerial)))
            {
                break;
            }
        }
        if (index == mPendingStateResetIndex)
        {
            break;
        }
        index = DecIndex(index);
    }

    ANGLE_VK_TRY(context, waitResult);

    return angle::Result::Continue;
}

void CommandQueue2::resetPendingItems(VkDevice device, Serial completedSerial, bool maySkip)
{
    resetCompletedItems(&mPendingStateResetIndex, mNextPrepareIndex, completedSerial,
                        [device, maySkip](QueueItem &item) {
                            if (!item.hasSubmit)
                            {
                                // No pending state to reset
                                return true;
                            }
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
                                } while (item.isLocked());
                            }
                            item.resetPendingState(device);
                            return true;
                        });
}

void CommandQueue2::flushWaitSemaphores(bool hasProtectedContent,
                                        egl::ContextPriority priority,
                                        std::vector<VkSemaphore> &&waitSemaphores,
                                        std::vector<VkPipelineStageFlags> &&waitSemaphoreStageMasks)
{
    ASSERT(!waitSemaphores.empty());
    ASSERT(waitSemaphores.size() == waitSemaphoreStageMasks.size());

    CmdsState::QueueState &state = mCmdsStateMap[hasProtectedContent].queueStates[priority];

    state.waitSemaphores.insert(state.waitSemaphores.end(), waitSemaphores.begin(),
                                waitSemaphores.end());
    state.waitSemaphoreStageMasks.insert(state.waitSemaphoreStageMasks.end(),
                                         waitSemaphoreStageMasks.begin(),
                                         waitSemaphoreStageMasks.end());

    waitSemaphores.clear();
    waitSemaphoreStageMasks.clear();
}

angle::Result CommandQueue2::flushOutsideRPCommands(
    Context *context,
    bool hasProtectedContent,
    egl::ContextPriority priority,
    OutsideRenderPassCommandBufferHelper **outsideRPCommands)
{
    ASSERT(!context->getRenderer()->isAsyncCommandQueueEnabled());
    onCommandsFlush(hasProtectedContent, priority);
    return doFlushOutsideRPCommands(context, hasProtectedContent, priority, outsideRPCommands);
}

angle::Result CommandQueue2::doFlushOutsideRPCommands(
    Context *context,
    bool hasProtectedContent,
    egl::ContextPriority priority,
    OutsideRenderPassCommandBufferHelper **outsideRPCommands)
{
    auto &buffer = mCmdsStateMap[hasProtectedContent].queueStates[priority].currentBuffer;
    ANGLE_TRY(ensurePrimaryCommandBufferValid(context, hasProtectedContent, &buffer));
    return (*outsideRPCommands)->flushToPrimary(context, &buffer);
}

angle::Result CommandQueue2::flushRenderPassCommands(
    Context *context,
    bool hasProtectedContent,
    egl::ContextPriority priority,
    const RenderPass &renderPass,
    RenderPassCommandBufferHelper **renderPassCommands)
{
    ASSERT(!context->getRenderer()->isAsyncCommandQueueEnabled());
    onCommandsFlush(hasProtectedContent, priority);
    return doFlushRenderPassCommands(context, hasProtectedContent, priority, renderPass,
                                     renderPassCommands);
}

angle::Result CommandQueue2::doFlushRenderPassCommands(
    Context *context,
    bool hasProtectedContent,
    egl::ContextPriority priority,
    const RenderPass &renderPass,
    RenderPassCommandBufferHelper **renderPassCommands)
{
    auto &buffer = mCmdsStateMap[hasProtectedContent].queueStates[priority].currentBuffer;
    ANGLE_TRY(ensurePrimaryCommandBufferValid(context, hasProtectedContent, &buffer));
    return (*renderPassCommands)->flushToPrimary(context, &buffer, &renderPass);
}

angle::Result CommandQueue2::submitCommands(Context *context,
                                            bool hasProtectedContent,
                                            egl::ContextPriority priority,
                                            const VkSemaphore signalSemaphore,
                                            SecondaryCommandBufferList &&commandBuffersToReset,
                                            SecondaryCommandPools *commandPools,
                                            const QueueSerial &submitQueueSerial)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::submitCommands");
    ASSERT(!context->getRenderer()->isAsyncCommandQueueEnabled());

    QueueItemIndex itemIndex = -1;
    ANGLE_TRY(prepareNextSubmit(context, hasProtectedContent, priority, signalSemaphore,
                                submitQueueSerial, &itemIndex));

    ANGLE_TRY(doSubmitCommands(context, std::move(commandBuffersToReset), commandPools, itemIndex));

    return angle::Result::Continue;
}

angle::Result CommandQueue2::queueSubmitOneOff(Context *context,
                                               bool hasProtectedContent,
                                               egl::ContextPriority priority,
                                               VkCommandBuffer commandBufferHandle,
                                               const Semaphore *waitSemaphore,
                                               VkPipelineStageFlags waitSemaphoreStageMask,
                                               const Fence *fence,
                                               SubmitPolicy submitPolicy,
                                               const QueueSerial &submitQueueSerial)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::queueSubmitOneOff");
    ASSERT(!context->getRenderer()->isAsyncCommandQueueEnabled());

    QueueItemIndex itemIndex = -1;
    ANGLE_TRY(prepareNextSubmitOneOff(context, hasProtectedContent, priority, commandBufferHandle,
                                      waitSemaphore, waitSemaphoreStageMask, fence,
                                      submitQueueSerial, &itemIndex));

    ANGLE_TRY(doQueueSubmitOneOff(context, itemIndex));

    return angle::Result::Continue;
}

void CommandQueue2::onCommandsFlush(bool hasProtectedContent, egl::ContextPriority priority)
{
    mCmdsStateMap[hasProtectedContent].queueStates[priority].needSubmit = true;
}

angle::Result CommandQueue2::prepareNextSubmit(Context *context,
                                               bool hasProtectedContent,
                                               egl::ContextPriority priority,
                                               const VkSemaphore signalSemaphore,
                                               const QueueSerial &submitQueueSerial,
                                               QueueItemIndex *itemIndexOut)
{
    CmdsState::QueueState &state = mCmdsStateMap[hasProtectedContent].queueStates[priority];

    ANGLE_TRY(prepareNextSubmit(context, hasProtectedContent, mQueueMap[priority],
                                &mCmdsStateMap[hasProtectedContent].queueStates[priority],
                                state.waitSemaphores, state.waitSemaphoreStageMasks.data(),
                                signalSemaphore, VK_NULL_HANDLE, nullptr, submitQueueSerial,
                                itemIndexOut));

    state.waitSemaphores.clear();
    state.waitSemaphoreStageMasks.clear();

    return angle::Result::Continue;
}

angle::Result CommandQueue2::prepareNextSubmitOneOff(Context *context,
                                                     bool hasProtectedContent,
                                                     egl::ContextPriority priority,
                                                     VkCommandBuffer commandBufferHandle,
                                                     const Semaphore *waitSemaphore,
                                                     VkPipelineStageFlags waitSemaphoreStageMask,
                                                     const Fence *fence,
                                                     const QueueSerial &submitQueueSerial,
                                                     QueueItemIndex *itemIndexOut)
{
    return prepareNextSubmit(context, hasProtectedContent, mQueueMap[priority], nullptr,
                             waitSemaphore, &waitSemaphoreStageMask, VK_NULL_HANDLE,
                             commandBufferHandle, fence, submitQueueSerial, itemIndexOut);
}

angle::Result CommandQueue2::prepareNextSubmit(Context *context,
                                               bool hasProtectedContent,
                                               VkQueue queue,
                                               CmdsState::QueueState *queueState,
                                               VkSemaphores waitSemaphores,
                                               const VkPipelineStageFlags *waitSemaphoreStageMasks,
                                               const VkSemaphore signalSemaphore,
                                               VkCommandBuffer oneOffCommandBuffer,
                                               const Fence *oneOffFence,
                                               const QueueSerial &submitQueueSerial,
                                               QueueItemIndex *itemIndexOut)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::prepareNextSubmit");

    ++mPerfCounters.commandQueueSubmitCallsTotal;
    ++mPerfCounters.commandQueueSubmitCallsPerFrame;

    ANGLE_TRY(throttlePendingItemQueue(context));

    QueueItem &item = mItemQueue[mNextPrepareIndex];
    ANGLE_TRY(item.waitIdle(context->getRenderer()->getMaxFenceWaitTimeNs()));
    ASSERT(!item.hasFence());
    ASSERT(!item.commandBuffer.valid());
    ASSERT(item.commandPools == nullptr &&
           item.commandBuffersToReset.outsideRenderPassCommandBuffers.empty() &&
           item.commandBuffersToReset.renderPassCommandBuffers.empty());
    item.serial              = mSerialFactory.generate();
    item.queueSerial         = submitQueueSerial;
    item.queue               = queue;
    item.queueState          = queueState;
    item.hasProtectedContent = hasProtectedContent;

    if (queueState)
    {
        ASSERT(!oneOffCommandBuffer && !oneOffFence);
        CmdsState::QueueState &state = *queueState;
        item.hasSubmit               = state.needSubmit;
        state.needSubmit             = false;
    }
    else
    {
        ASSERT(oneOffCommandBuffer || oneOffFence || waitSemaphores.count > 0);
        item.hasSubmit = true;
    }

    item.submitInfo       = {};
    item.submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    if (waitSemaphores.count > 0)
    {
        item.waitSemaphores.assign(waitSemaphores.items,
                                   waitSemaphores.items + waitSemaphores.count);
        item.waitSemaphoreStageMasks.assign(waitSemaphoreStageMasks,
                                            waitSemaphoreStageMasks + waitSemaphores.count);
        item.submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.count);
        item.submitInfo.pWaitSemaphores    = item.waitSemaphores.data();
        item.submitInfo.pWaitDstStageMask  = item.waitSemaphoreStageMasks.data();
        item.hasSubmit                     = true;
    }
    else
    {
        item.waitSemaphores.clear();
        item.waitSemaphoreStageMasks.clear();
    }

    item.signalSemaphore = signalSemaphore;
    if (signalSemaphore != VK_NULL_HANDLE)
    {
        item.submitInfo.signalSemaphoreCount = 1;
        item.submitInfo.pSignalSemaphores    = &item.signalSemaphore;
        item.hasSubmit                       = true;
    }

    item.oneOffCommandBuffer = oneOffCommandBuffer;
    if (oneOffCommandBuffer != VK_NULL_HANDLE)
    {
        item.submitInfo.commandBufferCount = 1;
        item.submitInfo.pCommandBuffers    = &item.oneOffCommandBuffer;
    }
    item.oneOffFence = oneOffFence ? oneOffFence->getHandle() : VK_NULL_HANDLE;

    if (item.hasSubmit)
    {
        // Assign here to avoid threading problems with "asyncCommandQueue".
        ++mPerfCounters.vkQueueSubmitCallsTotal;
        ++mPerfCounters.vkQueueSubmitCallsPerFrame;
    }
    else
    {
        item.resetState(QueueItem::State::Finished);
    }

    *itemIndexOut     = mNextPrepareIndex;
    mNextPrepareIndex = IncIndex(mNextPrepareIndex);
    ASSERT(getNumPendingItems() <= kInFlightCommandsLimit);

    // It is not actually submitted, but already can be used for waiting
    mLastSubmittedSerials.setQueueSerial(submitQueueSerial);

    return angle::Result::Continue;
}

angle::Result CommandQueue2::throttlePendingItemQueue(Context *context)
{
    static_assert(kActiveGCThreshold <= kInFlightCommandsLimit, "BAD kActiveGCThreshold");
    static_assert(CommandQueue2::kMaxQueueSize >= kInFlightCommandsLimit,
                  "BAD CommandQueue2::kMaxQueueSize");

    RendererVk *renderer = context->getRenderer();

    const QueueSize numPendingItems = getNumPendingItems();
    if (ANGLE_UNLIKELY(numPendingItems >= kActiveGCThreshold))
    {
        ASSERT(numPendingItems <= kInFlightCommandsLimit);
        if (ANGLE_UNLIKELY(numPendingItems >= kInFlightCommandsLimit))
        {
            // CPU should be throttled to avoid mItemQueue from growing too fast.
            // Important for off-screen scenarios.
            const QueueItemIndex waitIndex = DecIndex(mNextPrepareIndex, kInFlightCommandsLimit);
            ANGLE_TRY(waitAndResetPendingItems(context, mItemQueue[waitIndex],
                                               renderer->getMaxFenceWaitTimeNs()));
        }
        else
        {
            // Reset any items that was already completed (maySkip)
            resetPendingItems(renderer->getDevice(), mLastCompletedSerial.getSerial(), true);
        }
    }

    // CPU should be throttled to avoid accumulating too much memory garbage waiting to be
    // destroyed. This is important to keep peak memory usage at check when game launched and a lot
    // of staging buffers used for textures upload and then gets released. But if there is only one
    // command buffer in flight, we do not wait here to ensure we keep GPU busy.
    while (renderer->getSuballocationGarbageSize() > kMaxBufferSuballocationGarbageSize &&
           getNumPendingItems() > 1)
    {
        const QueueItem &waitItem = mItemQueue[mPendingStateResetIndex];
        ANGLE_TRY(waitAndResetPendingItems(context, waitItem, renderer->getMaxFenceWaitTimeNs()));
        cleanupGarbage(renderer, waitItem.serial);
    }

    return angle::Result::Continue;
}

CommandQueue2::QueueSize CommandQueue2::getNumPendingItems() const
{
    return (kQueueCapacity + mNextPrepareIndex - mPendingStateResetIndex) % kQueueCapacity;
}

angle::Result CommandQueue2::doSubmitCommands(Context *context,
                                              SecondaryCommandBufferList &&commandBuffersToReset,
                                              SecondaryCommandPools *commandPools,
                                              QueueItemIndex itemIndex)
{
    QueueItem::SubmitScope itemSubmitScope(context, this, itemIndex);
    QueueItem &item = itemSubmitScope.getItem();

    ASSERT(item.queueState);
    auto &buffer = item.queueState->currentBuffer;

    ASSERT(item.commandPools == nullptr &&
           item.commandBuffersToReset.outsideRenderPassCommandBuffers.empty() &&
           item.commandBuffersToReset.renderPassCommandBuffers.empty());
    if (!OutsideRenderPassCommandBuffer::ExecutesInline() ||
        !RenderPassCommandBuffer::ExecutesInline())
    {
        ASSERT(buffer.valid() || (commandBuffersToReset.outsideRenderPassCommandBuffers.empty() &&
                                  commandBuffersToReset.renderPassCommandBuffers.empty()));
        item.commandBuffersToReset = std::move(commandBuffersToReset);
        item.commandPools          = commandPools;
    }
    ASSERT(commandBuffersToReset.outsideRenderPassCommandBuffers.empty() &&
           commandBuffersToReset.renderPassCommandBuffers.empty());

    if (buffer.valid())
    {
        ASSERT(item.hasSubmit);
        ANGLE_VK_TRY(context, buffer.end());

        ASSERT(!item.commandBuffer.valid());
        item.commandBuffer = std::move(buffer);

        ASSERT(item.submitInfo.commandBufferCount == 0);
        ASSERT(item.submitInfo.pCommandBuffers == nullptr);
        item.submitInfo.commandBufferCount = 1;
        item.submitInfo.pCommandBuffers    = item.commandBuffer.ptr();
    }

    if (mUseSubmitThread)
    {
        mSubmitThreadTaskQueue.enqueue(
            [this, context, itemSubmitScope = std::move(itemSubmitScope)]() mutable {
                ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::doSubmitCommandsJob");
                itemSubmitScope.setInSubmitThread();
                return doSubmitCommandsJob(context, itemSubmitScope);
            });
        resetCompletedBuffers(context);
    }
    else
    {
        ANGLE_TRY(doSubmitCommandsJob(context, itemSubmitScope));
    }

    return angle::Result::Continue;
}

angle::Result CommandQueue2::ensurePrimaryCommandBufferValid(
    Context *context,
    bool hasProtectedContent,
    PrimaryCommandBuffer *primaryCommandBufferInOut)
{
    if (!primaryCommandBufferInOut->valid())
    {
        CmdsState &state = mCmdsStateMap[hasProtectedContent];

        ANGLE_TRY(state.pool.allocate(context, primaryCommandBufferInOut));

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        ANGLE_VK_TRY(context, primaryCommandBufferInOut->begin(beginInfo));
    }

    return angle::Result::Continue;
}

void CommandQueue2::resetCompletedBuffers(Context *context)
{
    {
        std::lock_guard<std::mutex> lock(mCompletedBuffersMutex);

        for (CmdsState &state : mCmdsStateMap)
        {
            ASSERT(state.completedBuffersPop.empty());
            if (!state.completedBuffersPush.empty())
            {
                swap(state.completedBuffersPush, state.completedBuffersPop);
            }
        }
    }

    for (CmdsState &state : mCmdsStateMap)
    {
        if (!state.completedBuffersPop.empty())
        {
            for (vk::PrimaryCommandBuffer &buffer : state.completedBuffersPop)
            {
                const angle::Result result = state.pool.collect(context, std::move(buffer));
                ASSERT(result == angle::Result::Continue);
            }
            state.completedBuffersPop.clear();
        }
    }
}

angle::Result CommandQueue2::doSubmitCommandsJob(Context *context,
                                                 QueueItem::SubmitScope &itemSubmitScope)
{
    QueueItem &item                       = itemSubmitScope.getItem();
    const Serial submitSerial             = item.serial;
    const Serial lastCleanupGarbageSerial = mLastCleanupGarbageSerial.getSerial();

    if (item.hasSubmit)
    {
        ASSERT(submitSerial > lastCleanupGarbageSerial);
        ANGLE_TRY(queueSubmit(context, itemSubmitScope));
    }
    else
    {
        itemSubmitScope.finish();
    }

    if (ANGLE_UNLIKELY(submitSerial.getValue() >
                       lastCleanupGarbageSerial.getValue() + kActiveGCThreshold))
    {
        cleanupGarbage(context->getRenderer(), mLastCompletedSerial.getSerial());
    }

    return angle::Result::Continue;
}

angle::Result CommandQueue2::doQueueSubmitOneOff(Context *context, QueueItemIndex itemIndex)
{
    QueueItem::SubmitScope itemSubmitScope(context, this, itemIndex);

    if (mUseSubmitThread)
    {
        mSubmitThreadTaskQueue.enqueue(
            [this, context, itemSubmitScope = std::move(itemSubmitScope)]() mutable {
                ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::doQueueSubmitOneOffJob");
                itemSubmitScope.setInSubmitThread();
                return queueSubmit(context, itemSubmitScope);
            });
        resetCompletedBuffers(context);
    }
    else
    {
        ANGLE_TRY(queueSubmit(context, itemSubmitScope));
    }

    return angle::Result::Continue;
}

angle::Result CommandQueue2::queueSubmit(Context *context, QueueItem::SubmitScope &itemSubmitScope)
{
    QueueItem &item = itemSubmitScope.getItem();

    if (kOutputVmaStatsString)
    {
        context->getRenderer()->outputVmaStatString();
    }

    VkProtectedSubmitInfo protectedSubmitInfo = {};
    if (item.hasProtectedContent && item.submitInfo.commandBufferCount > 0)
    {
        protectedSubmitInfo.sType           = VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO;
        protectedSubmitInfo.protectedSubmit = VK_TRUE;
        protectedSubmitInfo.pNext           = item.submitInfo.pNext;
        item.submitInfo.pNext               = &protectedSubmitInfo;
    }

    ASSERT(!item.fence);
    ANGLE_VK_TRY(context, item.fence.init(context->getDevice(), &mFenceRecycler));

    if (item.oneOffFence == VK_NULL_HANDLE)
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "vkQueueSubmit");
        ANGLE_VK_TRY(context,
                     vkQueueSubmit(item.queue, 1, &item.submitInfo, item.fence.get().getHandle()));
    }
    else
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "vkQueueSubmit");
        ANGLE_VK_TRY(context, vkQueueSubmit(item.queue, 1, &item.submitInfo, item.oneOffFence));

        VkSubmitInfo fenceSubmitInfo = {};
        fenceSubmitInfo.sType        = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        ANGLE_VK_TRY(context,
                     vkQueueSubmit(item.queue, 1, &fenceSubmitInfo, item.fence.get().getHandle()));
    }

    itemSubmitScope.finish();

    return angle::Result::Continue;
}

void CommandQueue2::advanceSubmitIndex(QueueItemIndex submitItemIndex)
{
    ASSERT(submitItemIndex == mNextSubmitIndex);
    mNextSubmitIndex = IncIndex(mNextSubmitIndex);
}

void CommandQueue2::checkAndCollectCommandBuffers(Context *context)
{
    const VkDevice device = context->getDevice();

    Serial completedSerial;
    (void)checkCompletedItems(
        context, mCommandBufferResetIndex, mNextSubmitIndex,
        [device](QueueItem &item) { return item.getFenceStatus(device, VK_SUCCESS); },
        &completedSerial);

    collectCommandBuffers(context, completedSerial);
    resetSubmittedItems();
}

void CommandQueue2::collectCommandBuffers(Context *context, Serial completedSerial)
{
    resetCompletedItems(&mCommandBufferResetIndex, mNextSubmitIndex, completedSerial,
                        [context, this](QueueItem &item) {
                            item.collectCommandBuffer(context, this);
                            return true;
                        });
}

void CommandQueue2::resetSubmittedItems()
{
    while (mSubmittedStateResetIndex != mCommandBufferResetIndex &&
           mItemQueue[mSubmittedStateResetIndex].acquireState() == QueueItem::State::Finished)
    {
        mItemQueue[mSubmittedStateResetIndex].resetSubmittedState();
        mSubmittedStateResetIndex = IncIndex(mSubmittedStateResetIndex);
    }
}

VkResult CommandQueue2::queuePresent(Context *context,
                                     egl::ContextPriority priority,
                                     const VkPresentInfoKHR &presentInfo,
                                     SwapchainStatus *swapchainStatus)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::queuePresent");
    ASSERT(!context->getRenderer()->isAsyncCommandQueueEnabled());

    VkResult result = VK_ERROR_DEVICE_LOST;

    doQueuePresent(context, priority, presentInfo,
                   [&result](const VkPresentInfoKHR &presentInfo, VkResult presentResult) {
                       result = presentResult;
                   });

    if (swapchainStatus)
    {
        // Mutex lock is not required.
        ASSERT(!swapchainStatus->isPending);
        // Assigned "lastPresentResult" is not used, but assigned for consistency anyway.
        swapchainStatus->lastPresentResult = result;
    }

    return result;
}

template <class OnPresentResult>
void CommandQueue2::doQueuePresent(Context *context,
                                   egl::ContextPriority priority,
                                   const VkPresentInfoKHR &presentInfo,
                                   OnPresentResult &&onPresentResult)
{
    if (mUseSubmitThread)
    {
        ASSERT(presentInfo.swapchainCount == 1);
        ASSERT(presentInfo.waitSemaphoreCount == 1);
        ASSERT(presentInfo.pResults == nullptr);

        std::vector<VkRectLayerKHR> vkRects;
        const void *pNext = presentInfo.pNext;
        while (pNext)
        {
            const VkStructureType sType = *static_cast<const VkStructureType *>(pNext);
            if (sType == VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR)
            {
                const auto &regions = *static_cast<const VkPresentRegionsKHR *>(pNext);
                ASSERT(regions.swapchainCount == 1);
                const VkPresentRegionKHR &region = regions.pRegions[0];
                vkRects.resize(region.rectangleCount);
                std::copy(region.pRectangles, region.pRectangles + region.rectangleCount,
                          vkRects.begin());
                pNext = regions.pNext;
            }
            else
            {
                ERR() << "Unknown sType: " << sType << " in VkPresentInfoKHR.pNext chain";
                UNREACHABLE();
            }
        }

        mSubmitThreadTaskQueue.enqueue(
            [this, context, priority, swapchain = presentInfo.pSwapchains[0],
             imageIndex    = presentInfo.pImageIndices[0],
             waitSemaphore = presentInfo.pWaitSemaphores[0], vkRects = std::move(vkRects),
             onPresentResult = std::forward<OnPresentResult>(onPresentResult)]() mutable {
                ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::doQueuePresentJob");
                VkPresentInfoKHR presentInfo       = {};
                presentInfo.sType                  = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
                presentInfo.swapchainCount         = 1;
                presentInfo.pSwapchains            = &swapchain;
                presentInfo.pImageIndices          = &imageIndex;
                presentInfo.waitSemaphoreCount     = 1;
                presentInfo.pWaitSemaphores        = &waitSemaphore;
                VkPresentRegionsKHR presentRegions = {};
                VkPresentRegionKHR presentRegion   = {};
                if (!vkRects.empty())
                {
                    presentRegions.sType          = VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR;
                    presentRegions.pNext          = presentInfo.pNext;
                    presentRegions.swapchainCount = 1;
                    presentRegions.pRegions       = &presentRegion;
                    presentRegion.pRectangles     = vkRects.data();
                    presentRegion.rectangleCount  = static_cast<uint32_t>(vkRects.size());
                    presentInfo.pNext             = &presentRegions;
                }
                doQueuePresentJob(context, priority, presentInfo, std::move(onPresentResult));
                return angle::Result::Continue;
            });
    }
    else
    {
        doQueuePresentJob(context, priority, presentInfo,
                          std::forward<OnPresentResult>(onPresentResult));
    }
}

template <class OnPresentResult>
void CommandQueue2::doQueuePresentJob(Context *context,
                                      egl::ContextPriority priority,
                                      const VkPresentInfoKHR &presentInfo,
                                      OnPresentResult &&onPresentResult)
{
    const VkResult result = vkQueuePresentKHR(mQueueMap[priority], &presentInfo);
    onPresentResult(presentInfo, result);

    // vkQueuePresentKHR() may wait for GPU previous frame.
    // There is high chance for successful garbage cleanup.
    checkAndCollectCommandBuffers(context);
    cleanupGarbage(context->getRenderer(), mLastCompletedSerial.getSerial());
}

void CommandQueue2::cleanupGarbage(RendererVk *renderer, Serial completedSerial)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandQueue2::cleanupGarbage");

    renderer->cleanupGarbage();

    mLastCleanupGarbageSerial = completedSerial;
}

void CommandQueue2::updateLastCompletedSerials(
    const angle::FastMap<Serial, kMaxFastQueueSerials> &serials,
    Serial serial)
{
    // Update "mLastCompletedSerials" before "mLastCompletedSerial".
    for (SerialIndex index = 0; index < serials.size(); ++index)
    {
        if (serials[index] != kZeroSerial)
        {
            mLastCompletedSerials.trySetQueueSerial(index, serials[index]);
        }
    }

    // Update (mo_release) "mLastCompletedSerial" after "mLastCompletedSerials".
    mLastCompletedSerial.updateIfIncrease(serial);
}

CommandQueue2::DynamicTimeout::DynamicTimeout(uint64_t original)
    : mOriginal(original), mStartTime(std::numeric_limits<double>::max())
{}

uint64_t CommandQueue2::DynamicTimeout::getRemaining()
{
    if (mStartTime == std::numeric_limits<double>::max())
    {
        mStartTime = angle::GetCurrentSystemTime();
        return mOriginal;
    }
    return mOriginal -
           std::min(mOriginal, uint64_t((angle::GetCurrentSystemTime() - mStartTime) * 1e+9));
}

CommandQueue2::QueueItem::SubmitScope::SubmitScope(Context *context,
                                                   CommandQueue2 *owner,
                                                   QueueItemIndex itemIndex)
    : mContext(context), mOwner(owner), mItemIndex(itemIndex), mItem(owner->mItemQueue + itemIndex)
{}

CommandQueue2::QueueItem::SubmitScope::SubmitScope(SubmitScope &&other)
    : mContext(other.mContext),
      mOwner(other.mOwner),
      mItemIndex(other.mItemIndex),
      mItem(other.mItem)
{
    other.mOwner = nullptr;
    other.mItem  = nullptr;
}

CommandQueue2::QueueItem::SubmitScope::~SubmitScope()
{
    if (!mOwner)
    {
        return;
    }

    if (ANGLE_UNLIKELY(mItem))  // Submit failed
    {
        if (!mIsInSubmitThread)
        {
            // If failed not in the Submit thread, need to wait before cleanup
            mOwner->waitSubmitThreadIdle();
        }

        mOwner->advanceSubmitIndex(mItemIndex);

        ASSERT(mItem->getState() == State::Idle);
        mItem->collectCommandBuffer(mContext, mOwner);
        mItem->resetFence();
        mItem->setStateAndNotify(State::Error);
    }
    else
    {
        ASSERT(!mOwner->mUseSubmitThread || mIsInSubmitThread);
    }
}

void CommandQueue2::QueueItem::SubmitScope::setInSubmitThread()
{
    ASSERT(mOwner->mUseSubmitThread && !mIsInSubmitThread);
    mIsInSubmitThread = true;
}

void CommandQueue2::QueueItem::SubmitScope::finish()
{
    ASSERT(!mOwner->mUseSubmitThread || mIsInSubmitThread);

    mOwner->advanceSubmitIndex(mItemIndex);

    ASSERT(mItem != nullptr);
    if (mItem->hasSubmit)
    {
        ASSERT(mItem->hasFence());
        ASSERT(mItem->getState() == State::Idle);
        mItem->setStateAndNotify(State::Submitted);
    }
    else
    {
        ASSERT(mItem->getState() == State::Finished);
    }
    mItem = nullptr;

    mOwner->checkAndCollectCommandBuffers(mContext);
}

angle::Result CommandQueue2::QueueItem::waitIdle(uint64_t timeout) const
{
    DynamicTimeout dynamicTimeout{timeout};
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

    if (!OutsideRenderPassCommandBuffer::ExecutesInline() ||
        !RenderPassCommandBuffer::ExecutesInline())
    {
        resetSecondaryCommands(device);
    }

    setState(State::Finished);
}

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

VkResult CommandQueue2::QueueItem::getPendingFenceStatus(VkDevice device,
                                                         VkResult errorStateResult) const
{
    const State state = acquireState();
    if (state == State::Idle)
    {
        return VK_NOT_READY;
    }
    ASSERT(state == State::Submitted || state == State::Error);
    return getFenceStatus(device, errorStateResult);
}

VkResult CommandQueue2::QueueItem::getFenceStatus(VkDevice device, VkResult noFenceResult) const
{
    ASSERT((getState() == State::Submitted && hasFence()) ||
           (getState() == State::Error && !hasFence()) || getState() == State::Finished);
    if (fence)
    {
        return fence.get().getStatus(device);
    }
    return noFenceResult;
}

VkResult CommandQueue2::QueueItem::waitPendingFence(VkDevice device,
                                                    DynamicTimeout &timeout,
                                                    VkResult errorStateResult) const
{
    const State state = waitSubmitted(timeout);
    if (ANGLE_UNLIKELY(state != State::Submitted))
    {
        ASSERT(state == State::Idle || state == State::Error);
        return (state == State::Idle) ? VK_TIMEOUT : errorStateResult;
    }
    return waitFence(device, timeout);
}

CommandQueue2::QueueItem::State CommandQueue2::QueueItem::waitSubmitted(
    DynamicTimeout &timeout) const
{
    const State state = waitState(
        timeout, [](State state) { return (state == State::Submitted || state == State::Error); });
    ASSERT(state == State::Submitted || state == State::Idle || state == State::Error);
    return state;
}

VkResult CommandQueue2::QueueItem::waitFence(VkDevice device, DynamicTimeout &timeout) const
{
    ASSERT(getState() == State::Submitted && hasFence());
    return fence.get().wait(device, timeout.getRemaining());
}

void CommandQueue2::QueueItem::collectCommandBuffer(Context *context, CommandQueue2 *owner)
{
    ASSERT(getState() != State::Submitted ||
           (getFenceStatus(context->getDevice(), VK_INCOMPLETE) == VK_SUCCESS));

    if (commandBuffer.valid())
    {
        CmdsState &state = owner->mCmdsStateMap[hasProtectedContent];
        if (owner->mUseSubmitThread)
        {
            std::lock_guard<std::mutex> lock(owner->mCompletedBuffersMutex);
            state.completedBuffersPush.emplace_back(std::move(commandBuffer));
        }
        else
        {
            const angle::Result result = state.pool.collect(context, std::move(commandBuffer));
            ASSERT(result == angle::Result::Continue);
        }
    }

    if (!OutsideRenderPassCommandBuffer::ExecutesInline() ||
        !RenderPassCommandBuffer::ExecutesInline())
    {
        resetSecondaryCommands(context->getDevice());
    }
}

void CommandQueue2::QueueItem::resetSubmittedState()
{
    ASSERT(getState() == State::Finished);

    resetFence();

    setStateAndNotify(State::Idle);
}

void CommandQueue2::QueueItem::resetFence()
{
    fence.release();
}

void CommandQueue2::QueueItem::resetSecondaryCommands(VkDevice device)
{
    if (commandPools)
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "Secondary command buffer recycling");
        ResetSecondaryCommandBuffers(device, &commandPools->outsideRenderPassPool,
                                     &commandBuffersToReset.outsideRenderPassCommandBuffers);
        ResetSecondaryCommandBuffers(device, &commandPools->renderPassPool,
                                     &commandBuffersToReset.renderPassCommandBuffers);
        commandPools = nullptr;
    }
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

template <class IsTargetState>
CommandQueue2::QueueItem::State CommandQueue2::QueueItem::waitState(
    DynamicTimeout &timeout,
    IsTargetState &&isTargetState) const
{
    QueueItem::State state = acquireState();
    if (ANGLE_UNLIKELY(!isTargetState(state)))
    {
        std::unique_lock<std::mutex> lock(mMutex);

        while (true)
        {
            state = getState();
            if (isTargetState(state))
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
    while (true)
    {
        QueueTask task;
        if (!tryPopNextTask(&task))
        {
            break;
        }
        const angle::Result result = task.execute();
        if (result != angle::Result::Continue)
        {
            UNREACHABLE();  // TODO: Investigate this case
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

}  // namespace vk
}  // namespace rx
