//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FlushThreadVk.cpp:
//    Implements the class methods for FlushThreadVk.
//

#include "libANGLE/renderer/vulkan/FlushThreadVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "third_party/trace_event/trace_event.h"

namespace rx
{

namespace
{
constexpr bool kEnableThreading           = true;
constexpr const char *kTraceEventCagetory = kEnableThreading ? "gpu.angle.flush" : "gpu.angle";
}  // anonymous namespace

FlushThreadVk::FlushThreadVk() : mDevice(VK_NULL_HANDLE), mQueue(VK_NULL_HANDLE) {}

FlushThreadVk::~FlushThreadVk()
{
    // onDestory() should have already cleaned everything up.
    ASSERT(!mSubmitThread.joinable());
}

angle::Result FlushThreadVk::initialize(vk::Context *context)
{
    mDevice        = context->getRenderer()->getDevice();
    mQueue         = context->getRenderer()->getQueue();
    mMainThreadId  = std::this_thread::get_id();
    mFlushThreadId = mMainThreadId;

    // Initialize the submission semaphore pool.
    ANGLE_TRY(mSubmitSemaphorePool.init(context, vk::kDefaultSemaphorePoolSize));

    if (kEnableThreading)
    {
        // Create the submission thread now that everything else is initialized.
        mSubmitThread  = std::thread(&FlushThreadVk::threadFunc, this);
        mFlushThreadId = mSubmitThread.get_id();
    }

    return angle::Result::Continue;
}

VkResult FlushThreadVk::onDestroy(vk::Context *context)
{
    VkResult result = VK_SUCCESS;

    if (mSubmitThread.joinable())
    {
        // Remember the result of the last operation to return it in the end.
        result = waitForPreviousOperation();

        // Tell the thread it should exit.
        mSubmitMutex.lock();
        mSubmitData.op = SubmitOp::Exit;
        mSubmitMutex.unlock();

        mSubmitConditionVariable.notify_one();

        // Wait for it to actually exit.  There cannot be an error.
        VkResult exitResult = waitForPreviousOperation();
        ASSERT(exitResult == VK_SUCCESS);

        // Clean it up.
        mSubmitThread.join();
    }

    mSubmitSemaphorePool.destroy(mDevice);

    return result;
}

std::mutex *FlushThreadVk::getSwapchainMutex()
{
    return kEnableThreading ? &mSwapchainMutex : nullptr;
}

bool FlushThreadVk::isMainThread() const
{
    return std::this_thread::get_id() == mMainThreadId;
}

bool FlushThreadVk::isFlushThread() const
{
    return std::this_thread::get_id() == mFlushThreadId;
}

VkResult FlushThreadVk::waitForPreviousOperation()
{
    if (!kEnableThreading)
    {
        return VK_SUCCESS;
    }

    {
        // Wait until op becomes Done, signifying that the thread has finished executing the
        // operation.
        std::unique_lock<std::mutex> lock(mSubmitMutex);
        while (mSubmitData.op != SubmitOp::Done)
        {
            mSubmitConditionVariable.wait(lock);
        }
    }

    // Return the result of operation from the thread.
    return mSubmitData.result;
}

// Each of the functions below, which corresponds to an operation the flush thread can handle, has
// the following form:
//
// - Wait for previous op to complete (if threading).
// - Set up the parameters for the operation.
// - If not threading, call the Impl directly.
// - Set mSubmitData.op to the appropriate value under mutex lock.  Note that the mutex includes the
//   memory barrier that makes the changes in this and the previous step visible to the thread.
// - Notify the thread.
//
// onDestroy has a similar behavior, setting mSubmitData.op to Exit.

angle::Result FlushThreadVk::flush(vk::Context *context,
                                   const vk::CommandBuffer &commandBuffer,
                                   const vk::Fence &fence)
{
    ANGLE_VK_TRY(context, waitForPreviousOperation());

    mSubmitData.commandBuffer = commandBuffer.getHandle();
    ANGLE_TRY(getSubmitSemaphores(context, false));
    mSubmitData.fence = fence.getHandle();

    if (!kEnableThreading)
    {
        ANGLE_VK_TRY(context, flushImpl());
        return angle::Result::Continue;
    }

    mSubmitMutex.lock();
    mSubmitData.op = SubmitOp::Flush;
    mSubmitMutex.unlock();

    mSubmitConditionVariable.notify_one();

    return angle::Result::Continue;
}

angle::Result FlushThreadVk::flushAndPresent(vk::Context *context,
                                             const vk::CommandBuffer &commandBuffer,
                                             const vk::Fence &fence,
                                             const VkPresentInfoKHR &presentInfo,
                                             std::vector<VkRectLayerKHR> &&presentRegions,
                                             uint32_t swapchainImageIndex)
{
    ANGLE_VK_TRY(context, waitForPreviousOperation());

    mSubmitData.commandBuffer = commandBuffer.getHandle();
    ANGLE_TRY(getSubmitSemaphores(context, true));
    mSubmitData.fence               = fence.getHandle();
    mSubmitData.presentInfo         = presentInfo;
    mSubmitData.presentRegions      = std::move(presentRegions);
    mSubmitData.swapchainImageIndex = swapchainImageIndex;

    if (!kEnableThreading)
    {
        ANGLE_VK_TRY(context, flushImpl());
        ANGLE_VK_TRY(context, presentImpl());
        return angle::Result::Continue;
    }

    mSubmitMutex.lock();
    mSubmitData.op = SubmitOp::FlushAndPresent;
    mSubmitMutex.unlock();

    mSubmitConditionVariable.notify_one();

    return angle::Result::Continue;
}

VkResult FlushThreadVk::flushImpl()
{
    ASSERT(isFlushThread());

    TRACE_EVENT0(kTraceEventCagetory, "FlushThreadVk::flush");

    SubmitWaitStageMasks waitStageMasks(mSubmitData.waitSemaphores.size(),
                                        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

    VkSubmitInfo submitInfo         = {};
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount   = static_cast<uint32_t>(mSubmitData.waitSemaphores.size());
    submitInfo.pWaitSemaphores      = mSubmitData.waitSemaphores.data();
    submitInfo.pWaitDstStageMask    = waitStageMasks.data();
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &mSubmitData.commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &mSubmitData.signalSemaphore;

    return vkQueueSubmit(mQueue, 1, &submitInfo, mSubmitData.fence);
}

VkResult FlushThreadVk::presentImpl()
{
    ASSERT(isFlushThread());

    TRACE_EVENT0(kTraceEventCagetory, "FlushThreadVk::present");

    // Make present wait on the last signaled semaphore.
    ASSERT(mSubmitData.presentInfo.waitSemaphoreCount == 0);
    mSubmitData.presentInfo.waitSemaphoreCount = 1;
    mSubmitData.presentInfo.pWaitSemaphores    = &mSubmitData.signalSemaphore;

    // Specify the image index
    mSubmitData.presentInfo.pImageIndices = &mSubmitData.swapchainImageIndex;

    // Chain the present regions if it's provided.
    VkPresentRegionsKHR regions = {};
    if (mSubmitData.presentRegions.size() > 0)
    {
        VkPresentRegionKHR region = {};
        region.rectangleCount     = static_cast<uint32_t>(mSubmitData.presentRegions.size());
        region.pRectangles        = mSubmitData.presentRegions.data();

        regions.sType          = VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR;
        regions.pNext          = nullptr;
        regions.swapchainCount = 1;
        regions.pRegions       = &region;

        mSubmitData.presentInfo.pNext = &regions;
    }

    if (kEnableThreading)
    {
        mSwapchainMutex.lock();
    }

    VkResult result = vkQueuePresentKHR(mQueue, &mSubmitData.presentInfo);

    if (kEnableThreading)
    {
        mSwapchainMutex.unlock();
    }

    return result;
}

void FlushThreadVk::threadFunc()
{
    while (true)
    {
        {
            // Wait until op becomes non-Done, signifying that there is a request to process.
            std::unique_lock<std::mutex> lock(mSubmitMutex);
            while (mSubmitData.op == SubmitOp::Done)
            {
                mSubmitConditionVariable.wait(lock);
            }
        }

        mSubmitData.result = VK_SUCCESS;

        bool isExit = mSubmitData.op == SubmitOp::Exit;
        bool isFlush =
            mSubmitData.op == SubmitOp::Flush || mSubmitData.op == SubmitOp::FlushAndPresent;
        bool isPresent = mSubmitData.op == SubmitOp::FlushAndPresent;

        if (isFlush)
        {
            mSubmitData.result = flushImpl();
        }

        if (mSubmitData.result == VK_SUCCESS && isPresent)
        {
            mSubmitData.result = presentImpl();
        }

        // Notify the main thread that the operation is now complete
        mSubmitMutex.lock();
        mSubmitData.op = SubmitOp::Done;
        mSubmitMutex.unlock();

        mSubmitConditionVariable.notify_one();

        if (isExit)
        {
            break;
        }
    }
}

angle::Result FlushThreadVk::allocateSubmitWaitSemaphore(vk::Context *context,
                                                         const vk::Semaphore **outSemaphore)
{
    ASSERT(isMainThread());

    ASSERT(mSubmitWaitSemaphores.size() < mSubmitWaitSemaphores.max_size());

    vk::SemaphoreHelper semaphore;
    ANGLE_TRY(mSubmitSemaphorePool.allocateSemaphore(context, &semaphore));

    mSubmitWaitSemaphores.push_back(std::move(semaphore));
    *outSemaphore = mSubmitWaitSemaphores.back().getSemaphore();

    return angle::Result::Continue;
}

angle::Result FlushThreadVk::getSubmitSemaphores(vk::Context *context, bool forPresent)
{
    ASSERT(isMainThread());

    mSubmitData.waitSemaphores.clear();

    if (mSubmitLastSignaledSemaphore.getSemaphore())
    {
        mSubmitData.waitSemaphores.push_back(
            mSubmitLastSignaledSemaphore.getSemaphore()->getHandle());

        // Return the semaphore to the pool (which will remain valid and unused until the
        // queue it's about to be waited on has finished execution).
        mSubmitSemaphorePool.freeSemaphore(context, &mSubmitLastSignaledSemaphore);
    }

    for (vk::SemaphoreHelper &semaphore : mSubmitWaitSemaphores)
    {
        mSubmitData.waitSemaphores.push_back(semaphore.getSemaphore()->getHandle());

        // Similarly, return the semaphore to the pool.
        mSubmitSemaphorePool.freeSemaphore(context, &semaphore);
    }
    mSubmitWaitSemaphores.clear();

    // On every flush, create a semaphore to be signaled.  On the next submission, this semaphore
    // will be waited on.
    ANGLE_TRY(mSubmitSemaphorePool.allocateSemaphore(context, &mSubmitLastSignaledSemaphore));

    mSubmitData.signalSemaphore = mSubmitLastSignaledSemaphore.getSemaphore()->getHandle();

    if (forPresent)
    {
        // Return the semaphore to the pool (which will remain valid and unused until the
        // queue it's about to be waited on has finished execution).  The presentation engine
        // will wait on it, so we have to make sure the next flush doesn't.
        mSubmitSemaphorePool.freeSemaphore(context, &mSubmitLastSignaledSemaphore);
    }

    return angle::Result::Continue;
}

FlushThreadVk::SubmitData::SubmitData()
    : op(SubmitOp::Done),
      commandBuffer(VK_NULL_HANDLE),
      signalSemaphore(VK_NULL_HANDLE),
      presentInfo{},
      result(VK_SUCCESS)
{}
FlushThreadVk::SubmitData::~SubmitData() {}

}  // namespace rx
