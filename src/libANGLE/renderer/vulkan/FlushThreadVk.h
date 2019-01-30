//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FlushThreadVk.h:
//    Defines the class FlushThreadVk, a helper to RendererVk and SurfaceVk that asynchronously
//    performs flush and present operations.
//

#ifndef LIBANGLE_RENDERER_VULKAN_FLUSHTHREADVK_H_
#define LIBANGLE_RENDERER_VULKAN_FLUSHTHREADVK_H_

#include <vulkan/vulkan.h>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "libANGLE/renderer/vulkan/CommandGraph.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{

class FlushThreadVk
{
  public:
    FlushThreadVk();
    ~FlushThreadVk();

    angle::Result initialize(vk::Context *context);
    VkResult onDestroy(vk::Context *context);

    // Returns the swapchain mutex, which could be nullptr if multi-threading is disabled.
    std::mutex *getSwapchainMutex();

    angle::Result flush(vk::Context *context,
                        const vk::CommandBuffer &commandBuffer,
                        const vk::Fence &fence);
    angle::Result flushAndPresent(vk::Context *context,
                                  const vk::CommandBuffer &commandBuffer,
                                  const vk::Fence &fence,
                                  const VkPresentInfoKHR &presentInfo,
                                  std::vector<VkRectLayerKHR> &&presentRegions,
                                  uint32_t swapchainImageIndex);

    // Request a semaphore, that is expected to be signaled externally.  The next submission will
    // wait on it.
    angle::Result allocateSubmitWaitSemaphore(vk::Context *context,
                                              const vk::Semaphore **outSemaphore);

    // Waits for the flush thread to become idle.  This is generally called interally, but is
    // made public for the sake of swapchain resizing implementation, which requires all images to
    // have been returned to the swapchain (i.e. the present operation is already performed).
    VkResult waitForPreviousOperation();

  private:
    // Number of semaphores for external entities to renderer to issue a wait, such as surface's
    // image acquire.
    static constexpr size_t kMaxExternalSemaphores = 64;
    // Total possible number of semaphores a submission can wait on.  +1 is for the semaphore
    // signaled in the last submission.
    static constexpr size_t kMaxWaitSemaphores = kMaxExternalSemaphores + 1;

    using SubmitWaitSemaphores   = angle::FixedVector<vk::SemaphoreHelper, kMaxExternalSemaphores>;
    using SubmitWaitVkSemaphores = angle::FixedVector<VkSemaphore, kMaxWaitSemaphores>;
    using SubmitWaitStageMasks   = angle::FixedVector<VkPipelineStageFlags, kMaxWaitSemaphores>;

    // Implementation of the operations.  If threading is disabled, they are called directly from
    // the functions above.  If threading is enabled, they are called from the thread function.
    VkResult flushImpl();
    VkResult presentImpl();

    // The thread function.  Waits for commands and dispatches them to the handler.
    void threadFunc();

    angle::Result getSubmitSemaphores(vk::Context *context, bool forPresent);

    angle::Result submitFrame(vk::Context *context,
                              const VkSubmitInfo &submitInfo,
                              vk::CommandBuffer &&commandBuffer);

    bool isMainThread() const;
    bool isFlushThread() const;

    enum class SubmitOp
    {
        // Special value to denote that there are no pending jobs
        Done,

        // Operations the thread can perform
        Flush,
        FlushAndPresent,

        // Special operation to make the thread exit, used for cleanup.
        Exit,
    };

    struct SubmitData
    {
        SubmitData();
        ~SubmitData();

        SubmitOp op;

        VkCommandBuffer commandBuffer;
        SubmitWaitVkSemaphores waitSemaphores;
        VkSemaphore signalSemaphore;
        VkFence fence;

        // For Present
        VkPresentInfoKHR presentInfo;
        std::vector<VkRectLayerKHR> presentRegions;
        uint32_t swapchainImageIndex;

        // Return value from the operation
        VkResult result;
    };

    VkDevice mDevice;
    VkQueue mQueue;

    std::thread mSubmitThread;
    // The submit thread does not queue
    // operations to avoid it from falling behind the main thread.  Therefore, a single object is
    // used to schedule a submit-related operation (flush, flush+present).
    SubmitData mSubmitData;
    // Synchronization primitives used for passing parameters to the flush thread and getting
    // the result.  SubmitData::op is used by the main thread to notify the flush thread that a
    // job is available.  It's also used by the flush thread to notify the main thread that the
    // job has completed.  This variable is protected by a mutex.  The condition variable is used by
    // each thread to wake the other thread up when this value changes.  Each thread changes this
    // variable only after setting the other members of mSubmitData, which means those other members
    // don't need to be mutex protected.
    std::condition_variable mSubmitConditionVariable;
    std::mutex mSubmitMutex;

    // Mutex used to synchronize the swapchain between the flush thread calls and the main thread's
    // acquire next image.
    std::mutex mSwapchainMutex;

    // mSubmitWaitSemaphores is a list of specifically requested semaphores to be waited on before a
    // command buffer submission, for example, semaphores signaled by vkAcquireNextImageKHR.
    // After first use, the list is automatically cleared.  This is a vector to support concurrent
    // rendering to multiple surfaces.
    //
    // Note that with multiple contexts present, this may result in a context waiting on image
    // acquisition even if it doesn't render to that surface.  If CommandGraphs are separated by
    // context or share group for example, this could be moved to the one that actually uses the
    // image.
    //
    // Created on the main thread, and handed off to the flush thread on every flush.
    SubmitWaitSemaphores mSubmitWaitSemaphores;
    // mSubmitLastSignaledSemaphore shows which semaphore was last signaled by submission.  This can
    // be set to nullptr if retrieved to be waited on outside RendererVk, such
    // as by the surface before presentation.  Each submission waits on the
    // previously signaled semaphore (as well as any in mSubmitWaitSemaphores)
    // and allocates a new semaphore to signal.
    //
    // Created on the main thread, and handed off to the flush thread on every flush.
    vk::SemaphoreHelper mSubmitLastSignaledSemaphore;

    // Pool of semaphores used to support the aforementioned mid-frame submissions.
    //
    // Entirely manipulated by the main thread.
    vk::DynamicSemaphorePool mSubmitSemaphorePool;

    // The main and flush thread ids used to assert that certain functions are called from the right
    // thread.
    std::thread::id mMainThreadId;
    std::thread::id mFlushThreadId;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_FLUSHTHREADVK_H_
