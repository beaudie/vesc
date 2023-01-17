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
enum class SubmitPolicy
{
    AllowDeferred,
    EnsureSubmitted,
};

class FenceRecycler;
// This is a RAII class manages refcounted vkfence object with auto-release and recycling.
class SharedFence final
{
  public:
    SharedFence();
    SharedFence(const SharedFence &other);
    SharedFence(SharedFence &&other);
    ~SharedFence();
    // Copy assignment will add reference count to the underline object
    SharedFence &operator=(const SharedFence &other);
    // Move assignment will move reference count from other to this object
    SharedFence &operator=(SharedFence &&other);

    // Initialize it with a new vkFence either from recycler or create a new one.
    VkResult init(VkDevice device, FenceRecycler *recycler);
    // Destroy it immediately (will not recycle).
    void destroy(VkDevice device);
    // Release the vkFence (to recycler)
    void release();
    // Return true if underline VkFence is valid
    operator bool() const;
    const Fence &get() const
    {
        ASSERT(mRefCountedFence != nullptr && mRefCountedFence->isReferenced());
        return mRefCountedFence->get();
    }

    // The following three APIs can call without lock. Since fence is refcounted and this object has
    // a refcount to VkFence, No one is able to come in and destroy the VkFence.
    VkResult getStatus(VkDevice device) const;
    VkResult wait(VkDevice device, uint64_t timeout) const;

  private:
    RefCounted<Fence> *mRefCountedFence;
    FenceRecycler *mRecycler;
};

class FenceRecycler
{
  public:
    FenceRecycler() {}
    ~FenceRecycler() {}
    void destroy(Context *context);

    void fetch(VkDevice device, Fence *fenceOut);
    void recycle(Fence &&fence);

  private:
    std::mutex mMutex;
    Recycler<Fence> mRecyler;
};

struct SwapchainStatus
{
    mutable std::mutex mutex;
    mutable CondVarHelper condVar;
    bool isPending = false;

    VkResult lastPresentResult = VK_NOT_READY;
};

enum class CustomTask
{
    Invalid = 0,
    // Process SecondaryCommandBuffer commands into the primary CommandBuffer.
    ProcessOutsideRenderPassCommands,
    ProcessRenderPassCommands,
    // End the current command buffer and submit commands to the queue
    FlushAndQueueSubmit,
    // Submit custom command buffer, excludes some state management
    OneOffQueueSubmit,
    // Execute QueuePresent
    Present,
    // Exit the command processor thread
    Exit,
};

// CommandProcessorTask interface
class CommandProcessorTask
{
  public:
    CommandProcessorTask() { initTask(); }

    void initTask();

    void initTask(CustomTask command) { mTask = command; }

    void initOutsideRenderPassProcessCommands(bool hasProtectedContent,
                                              egl::ContextPriority priority,
                                              OutsideRenderPassCommandBufferHelper *commandBuffer);

    void initRenderPassProcessCommands(bool hasProtectedContent,
                                       egl::ContextPriority priority,
                                       RenderPassCommandBufferHelper *commandBuffer,
                                       const RenderPass *renderPass);

    void initPresent(egl::ContextPriority priority,
                     const VkPresentInfoKHR &presentInfo,
                     SwapchainStatus *swapchainStatus);

    void initFlushAndQueueSubmit(const VkSemaphore semaphore,
                                 bool hasProtectedContent,
                                 egl::ContextPriority priority,
                                 SecondaryCommandPools *commandPools,
                                 SecondaryCommandBufferList &&commandBuffersToReset,
                                 const QueueSerial &submitQueueSerial);

    void initOneOffQueueSubmit(VkCommandBuffer commandBufferHandle,
                               bool hasProtectedContent,
                               egl::ContextPriority priority,
                               const Semaphore *waitSemaphore,
                               VkPipelineStageFlags waitSemaphoreStageMask,
                               const Fence *fence,
                               const QueueSerial &submitQueueSerial);

    CommandProcessorTask &operator=(CommandProcessorTask &&rhs);

    CommandProcessorTask(CommandProcessorTask &&other) : CommandProcessorTask()
    {
        *this = std::move(other);
    }

    const QueueSerial &getSubmitQueueSerial() const { return mSubmitQueueSerial; }
    CustomTask getTaskCommand() { return mTask; }
    VkSemaphore getSemaphore() { return mSemaphore; }
    SecondaryCommandBufferList &&getCommandBuffersToReset()
    {
        return std::move(mCommandBuffersToReset);
    }
    egl::ContextPriority getPriority() const { return mPriority; }
    bool hasProtectedContent() const { return mHasProtectedContent; }
    VkCommandBuffer getOneOffCommandBufferVk() const { return mOneOffCommandBufferVk; }
    const Semaphore *getOneOffWaitSemaphore() { return mOneOffWaitSemaphore; }
    VkPipelineStageFlags getOneOffWaitSemaphoreStageMask() { return mOneOffWaitSemaphoreStageMask; }
    const Fence *getOneOffFence() { return mOneOffFence; }
    const VkPresentInfoKHR &getPresentInfo() const { return mPresentInfo; }
    SwapchainStatus *getSwapchainStatus() const { return mSwapchainStatus; }
    const RenderPass *getRenderPass() const { return mRenderPass; }
    OutsideRenderPassCommandBufferHelper *getOutsideRenderPassCommandBuffer() const
    {
        return mOutsideRenderPassCommandBuffer;
    }
    RenderPassCommandBufferHelper *getRenderPassCommandBuffer() const
    {
        return mRenderPassCommandBuffer;
    }
    SecondaryCommandPools *getCommandPools() const { return mCommandPools; }

  private:
    void copyPresentInfo(const VkPresentInfoKHR &other);

    CustomTask mTask;

    // ProcessCommands
    OutsideRenderPassCommandBufferHelper *mOutsideRenderPassCommandBuffer;
    RenderPassCommandBufferHelper *mRenderPassCommandBuffer;
    const RenderPass *mRenderPass;

    // Flush data
    VkSemaphore mSemaphore;
    SecondaryCommandPools *mCommandPools;
    SecondaryCommandBufferList mCommandBuffersToReset;

    // Flush command data
    QueueSerial mSubmitQueueSerial;

    // Present command data
    VkPresentInfoKHR mPresentInfo;
    VkSwapchainKHR mSwapchain;
    VkSemaphore mWaitSemaphore;
    uint32_t mImageIndex;
    // Used by Present if supportsIncrementalPresent is enabled
    VkPresentRegionKHR mPresentRegion;
    VkPresentRegionsKHR mPresentRegions;
    std::vector<VkRectLayerKHR> mRects;

    VkSwapchainPresentFenceInfoEXT mPresentFenceInfo;
    VkFence mPresentFence;

    SwapchainStatus *mSwapchainStatus;

    // Used by OneOffQueueSubmit
    VkCommandBuffer mOneOffCommandBufferVk;
    const Semaphore *mOneOffWaitSemaphore;
    VkPipelineStageFlags mOneOffWaitSemaphoreStageMask;
    const Fence *mOneOffFence;

    // Flush, Present & QueueWaitIdle data
    egl::ContextPriority mPriority;
    bool mHasProtectedContent;
};

struct CommandBatch final : angle::NonCopyable
{
    CommandBatch();
    ~CommandBatch();
    CommandBatch(CommandBatch &&other);
    CommandBatch &operator=(CommandBatch &&other);

    void destroy(VkDevice device);
    void resetSecondaryCommandBuffers(VkDevice device);

    PrimaryCommandBuffer primaryCommands;
    // commandPools is for secondary CommandBuffer allocation
    SecondaryCommandPools *commandPools;
    SecondaryCommandBufferList commandBuffersToReset;
    SharedFence fence;
    QueueSerial queueSerial;
    bool hasProtectedContent;
};

class DeviceQueueMap;

class QueueFamily final : angle::NonCopyable
{
  public:
    static const uint32_t kInvalidIndex = std::numeric_limits<uint32_t>::max();

    static uint32_t FindIndex(const std::vector<VkQueueFamilyProperties> &queueFamilyProperties,
                              VkQueueFlags flags,
                              int32_t matchNumber,  // 0 = first match, 1 = second match ...
                              uint32_t *matchCount);
    static const uint32_t kQueueCount = static_cast<uint32_t>(egl::ContextPriority::EnumCount);
    static const float kQueuePriorities[static_cast<uint32_t>(egl::ContextPriority::EnumCount)];

    QueueFamily() : mProperties{}, mIndex(kInvalidIndex) {}
    ~QueueFamily() {}

    void initialize(const VkQueueFamilyProperties &queueFamilyProperties, uint32_t index);
    bool valid() const { return (mIndex != kInvalidIndex); }
    uint32_t getIndex() const { return mIndex; }
    const VkQueueFamilyProperties *getProperties() const { return &mProperties; }
    bool isGraphics() const { return ((mProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) > 0); }
    bool isCompute() const { return ((mProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) > 0); }
    bool supportsProtected() const
    {
        return ((mProperties.queueFlags & VK_QUEUE_PROTECTED_BIT) > 0);
    }
    uint32_t getDeviceQueueCount() const { return mProperties.queueCount; }

    DeviceQueueMap initializeQueueMap(VkDevice device,
                                      bool makeProtected,
                                      uint32_t queueIndex,
                                      uint32_t queueCount);

  private:
    VkQueueFamilyProperties mProperties;
    uint32_t mIndex;

    void getDeviceQueue(VkDevice device, bool makeProtected, uint32_t queueIndex, VkQueue *queue);
};

class DeviceQueueMap : public angle::PackedEnumMap<egl::ContextPriority, VkQueue>
{
    friend QueueFamily;

  public:
    DeviceQueueMap() : mIndex(QueueFamily::kInvalidIndex), mIsProtected(false) {}
    DeviceQueueMap(uint32_t queueFamilyIndex, bool isProtected)
        : mIndex(queueFamilyIndex), mIsProtected(isProtected)
    {}
    DeviceQueueMap(const DeviceQueueMap &other) = default;
    ~DeviceQueueMap();
    DeviceQueueMap &operator=(const DeviceQueueMap &other);

    bool valid() const { return (mIndex != QueueFamily::kInvalidIndex); }
    uint32_t getIndex() const { return mIndex; }
    bool isProtected() const { return mIsProtected; }
    egl::ContextPriority getDevicePriority(egl::ContextPriority priority) const;

  private:
    uint32_t mIndex;
    bool mIsProtected;
    angle::PackedEnumMap<egl::ContextPriority, egl::ContextPriority> mPriorities;
};

class CommandQueue : angle::NonCopyable
{
  public:
    // These public APIs are inherently thread safe. Thread unsafe methods must be protected methods
    // that are only accessed via ThreadSafeCommandQueue API.
    egl::ContextPriority getDriverPriority(egl::ContextPriority priority) const
    {
        return mQueueMap.getDevicePriority(priority);
    }
    uint32_t getDeviceQueueIndex() const { return mQueueMap.getIndex(); }

    VkQueue getQueue(egl::ContextPriority priority) const { return mQueueMap[priority]; }

    // The ResourceUse still have unfinished queue serial by ANGLE or vulkan.
    bool hasUnfinishedUse(const ResourceUse &use) const;
    // The ResourceUse still have queue serial not yet submitted to vulkan.
    bool hasUnsubmittedUse(const ResourceUse &use) const;
    Serial getLastSubmittedSerial(SerialIndex index) const { return mLastSubmittedSerials[index]; }

  protected:
    // These are accessed by ThreadSafeCommandQueue only
    CommandQueue();
    ~CommandQueue();

    angle::Result init(Context *context, const DeviceQueueMap &queueMap);
    void destroy(Context *context);

    void handleDeviceLost(RendererVk *renderer);

    angle::Result submitCommands(Context *context,
                                 bool hasProtectedContent,
                                 egl::ContextPriority priority,
                                 const VkSemaphore signalSemaphore,
                                 SecondaryCommandBufferList &&commandBuffersToReset,
                                 SecondaryCommandPools *commandPools,
                                 const QueueSerial &submitQueueSerial);

    angle::Result queueSubmitOneOff(Context *context,
                                    bool hasProtectedContent,
                                    egl::ContextPriority contextPriority,
                                    VkCommandBuffer commandBufferHandle,
                                    const Semaphore *waitSemaphore,
                                    VkPipelineStageFlags waitSemaphoreStageMask,
                                    const Fence *fence,
                                    SubmitPolicy submitPolicy,
                                    const QueueSerial &submitQueueSerial);

    VkResult queuePresent(egl::ContextPriority contextPriority,
                          const VkPresentInfoKHR &presentInfo,
                          SwapchainStatus *swapchainStatus);

    angle::Result checkCompletedCommands(Context *context);

    void flushWaitSemaphores(bool hasProtectedContent,
                             egl::ContextPriority priority,
                             std::vector<VkSemaphore> &&waitSemaphores,
                             std::vector<VkPipelineStageFlags> &&waitSemaphoreStageMasks);
    angle::Result flushOutsideRPCommands(Context *context,
                                         bool hasProtectedContent,
                                         egl::ContextPriority priority,
                                         OutsideRenderPassCommandBufferHelper **outsideRPCommands);
    angle::Result flushRenderPassCommands(Context *context,
                                          bool hasProtectedContent,
                                          egl::ContextPriority priority,
                                          const RenderPass &renderPass,
                                          RenderPassCommandBufferHelper **renderPassCommands);

    angle::Result queueSubmit(Context *context,
                              egl::ContextPriority contextPriority,
                              const VkSubmitInfo &submitInfo,
                              const Fence *fence,
                              const QueueSerial &submitQueueSerial);

    const angle::VulkanPerfCounters &getPerfCounters() const { return mPerfCounters; }
    void resetPerFramePerfCounters();

    void releaseToCommandBatch(bool hasProtectedContent,
                               PrimaryCommandBuffer &&commandBuffer,
                               SecondaryCommandPools *commandPools,
                               CommandBatch *batch);
    angle::Result retireFinishedCommands(Context *context, size_t finishedCount);
    angle::Result retireFinishedCommandsAndCleanupGarbage(Context *context, size_t finishedCount);
    angle::Result ensurePrimaryCommandBufferValid(Context *context,
                                                  bool hasProtectedContent,
                                                  egl::ContextPriority priority);
    // Returns number of CommandBatchs that are smaller than serials
    size_t getBatchCountUpToSerials(RendererVk *renderer, const Serials &serials);
    // Returns the last valid SharedFence of the first "count" CommandBatchs in mInflightCommands.
    const SharedFence &getSharedFenceToWait(size_t count);

    // For validation only. Should only be called with ASSERT macro.
    bool allInFlightCommandsAreAfterSerials(const Serials &serials);

    PrimaryCommandBuffer &getCommandBuffer(bool hasProtectedContent, egl::ContextPriority priority)
    {
        return mCmdsStateMap[hasProtectedContent].queueStates[priority].commandBuffer;
    }

    PersistentCommandPool &getCommandPool(bool hasProtectedContent)
    {
        return mCmdsStateMap[hasProtectedContent].commandPool;
    }

    std::vector<CommandBatch> mInFlightCommands;

    struct CmdsState
    {
        struct QueueState
        {
            PrimaryCommandBuffer commandBuffer;
            std::vector<VkSemaphore> waitSemaphores;
            std::vector<VkPipelineStageFlags> waitSemaphoreStageMasks;
        };
        angle::PackedEnumMap<egl::ContextPriority, QueueState> queueStates;
        // Keeps a free list of reusable primary command buffers.
        PersistentCommandPool commandPool;
    };
    // Array index: 0 - normal Contexts; 1 - hasProtectedContent
    CmdsState mCmdsStateMap[2];

    // Queue serial management.
    AtomicQueueSerialFixedArray mLastSubmittedSerials;
    // This queue serial can be read/write from different threads, so we need to use atomic
    // operations to access the underline value. Since we only do load/store on this value, it
    // should be just a normal uint64_t load/store on most platforms.
    AtomicQueueSerialFixedArray mLastCompletedSerials;

    // QueueMap
    DeviceQueueMap mQueueMap;

    FenceRecycler mFenceRecycler;

    angle::VulkanPerfCounters mPerfCounters;

  private:
    angle::Result finishQueueSerial(Context *context,
                                    const QueueSerial &queueSerial,
                                    uint64_t timeout);
    angle::Result finishResourceUse(Context *context, const ResourceUse &use, uint64_t timeout);
};

// Replacement for the CommandQueue
// Main differences:
// - Supports concurrent checking/waiting for commands from Context thread, while
//   flushing/submitting in async thread (does not require a mutex).
// - Allows concurrent waiting for QueueSerial submission/completion, while still allows
//   flushing/submitting from other Context threads (unlocks mutex).
// - Implemented additional submitting thread. Helps process command buffers when driver blocks.
// - Removed forced Garbage Cleanup from checking/waiting functions (better wait timings).
// - Garbage Cleanup performed only as part of submit operations or explicitly.
// - Added explicit cleanupAllCompletedGarbage() function to use in glFinish() to release resources.
// - Fixed "Finish to Serial" logic in cases, when multiple VkQueues used at the same time
//   (need to wait for a VkFence from each VkQueue).
// - Fixed bug when "OneOff" submits can not be explicitly waited (with external fence).
// - Fixed race-condition with "VulkanPerfCounters" when using async thread.
// - Better "kInFlightCommandsLimit" handling when using async thread.
// - Better "kMaxBufferSuballocationGarbageSize" handling when using async thread.
//
// Each thread may run in parallel (no mutex):
// - Context thread   - any thread from which GL/EGL API is called (Some client application thread)
// - Execution thread - additional thread or "Context thread" (CommandProcessor thread)
// - Submit thread    - additional thread or "Execution thread" (mSubmitThreadTaskQueue)
class CommandQueue2 : angle::NonCopyable
{
  public:
    using QueueItemIndex = uint32_t;

  public:
    CommandQueue2(std::mutex &mutex);

    angle::Result init(Context *context, const DeviceQueueMap &queueMap);
    void destroy(Context *context);

    // Multiple threads

    egl::ContextPriority getDriverPriority(egl::ContextPriority priority) const
    {
        return mQueueMap.getDevicePriority(priority);
    }
    uint32_t getDeviceQueueIndex() const { return mQueueMap.getIndex(); }
    VkQueue getQueue(egl::ContextPriority priority) const { return mQueueMap[priority]; }

    // The ResourceUse still have unfinished queue serial by ANGLE or vulkan.
    bool hasUnfinishedUse(const ResourceUse &use) const { return use > mLastCompletedSerials; }
    // The ResourceUse still have queue serial not yet submitted to vulkan.
    bool hasUnsubmittedUse(const ResourceUse &use) const { return use > mLastSubmittedSerials; }
    Serial getLastSubmittedSerial(SerialIndex index) const { return mLastSubmittedSerials[index]; }

    void waitSubmitThreadIdle();  // Safe to call after destroy()

    // Exclusively single thread (other threads must be suspended)

    void cleanupAllCompletedGarbage(Context *context);
    void handleDeviceLost(RendererVk *renderer);

    // Single Context thread

    angle::Result checkCompletedCommands(Context *context);
    angle::Result finishResourceUse(Context *context, const ResourceUse &use, uint64_t timeout);
    angle::Result finishQueueSerial(Context *context,
                                    const QueueSerial &queueSerial,
                                    uint64_t timeout);
    angle::Result waitIdle(Context *context, uint64_t timeout);
    angle::Result waitForResourceUseToFinishWithUserTimeout(Context *context,
                                                            const ResourceUse &use,
                                                            uint64_t timeout,
                                                            VkResult *result);
    bool isBusy() const;

    const angle::VulkanPerfCounters &getPerfCounters() const { return mPerfCounters; }
    void resetPerFramePerfCounters();

    void flushWaitSemaphores(bool hasProtectedContent,
                             egl::ContextPriority priority,
                             std::vector<VkSemaphore> &&waitSemaphores,
                             std::vector<VkPipelineStageFlags> &&waitSemaphoreStageMasks);

    // Single Context thread (asyncCommandQueue = true)

    void onCommandsFlush(bool hasProtectedContent, egl::ContextPriority priority);

    angle::Result prepareNextSubmit(Context *context,
                                    bool hasProtectedContent,
                                    egl::ContextPriority priority,
                                    const VkSemaphore signalSemaphore,
                                    const QueueSerial &submitQueueSerial,
                                    QueueItemIndex *itemIndexOut);

    angle::Result prepareNextSubmitOneOff(Context *context,
                                          bool hasProtectedContent,
                                          egl::ContextPriority priority,
                                          VkCommandBuffer commandBufferHandle,
                                          const Semaphore *waitSemaphore,
                                          VkPipelineStageFlags waitSemaphoreStageMask,
                                          const Fence *fence,
                                          const QueueSerial &submitQueueSerial,
                                          QueueItemIndex *itemIndexOut);

    angle::Result waitForQueueSerialActuallySubmitted(RendererVk *renderer,
                                                      const QueueSerial &queueSerial);

    // Single Execution thread (asyncCommandQueue = true)

    angle::Result doFlushOutsideRPCommands(
        Context *context,
        bool hasProtectedContent,
        egl::ContextPriority priority,
        OutsideRenderPassCommandBufferHelper **outsideRPCommands);
    angle::Result doFlushRenderPassCommands(Context *context,
                                            bool hasProtectedContent,
                                            egl::ContextPriority priority,
                                            const RenderPass &renderPass,
                                            RenderPassCommandBufferHelper **renderPassCommands);

    angle::Result doSubmitCommands(Context *context,
                                   SecondaryCommandBufferList &&commandBuffersToReset,
                                   SecondaryCommandPools *commandPools,
                                   QueueItemIndex itemIndex);

    angle::Result doQueueSubmitOneOff(Context *context, QueueItemIndex itemIndex);

    template <class OnPresentResult>
    void doQueuePresent(Context *context,
                        egl::ContextPriority priority,
                        const VkPresentInfoKHR &presentInfo,
                        OnPresentResult &&onPresentResult);

  protected:
    // Single Execution (Context) thread (asyncCommandQueue = false)

    angle::Result flushOutsideRPCommands(Context *context,
                                         bool hasProtectedContent,
                                         egl::ContextPriority priority,
                                         OutsideRenderPassCommandBufferHelper **outsideRPCommands);
    angle::Result flushRenderPassCommands(Context *context,
                                          bool hasProtectedContent,
                                          egl::ContextPriority priority,
                                          const RenderPass &renderPass,
                                          RenderPassCommandBufferHelper **renderPassCommands);

    angle::Result submitCommands(Context *context,
                                 bool hasProtectedContent,
                                 egl::ContextPriority priority,
                                 const VkSemaphore signalSemaphore,
                                 SecondaryCommandBufferList &&commandBuffersToReset,
                                 SecondaryCommandPools *commandPools,
                                 const QueueSerial &submitQueueSerial);

    angle::Result queueSubmitOneOff(Context *context,
                                    bool hasProtectedContent,
                                    egl::ContextPriority priority,
                                    VkCommandBuffer commandBufferHandle,
                                    const Semaphore *waitSemaphore,
                                    VkPipelineStageFlags waitSemaphoreStageMask,
                                    const Fence *fence,
                                    SubmitPolicy submitPolicy,
                                    const QueueSerial &submitQueueSerial);

    VkResult queuePresent(Context *context,
                          egl::ContextPriority priority,
                          const VkPresentInfoKHR &presentInfo,
                          SwapchainStatus *swapchainStatus);

  private:
    using QueueSize = QueueItemIndex;

    static constexpr uint32_t kVkQueueCount = QueueFamily::kQueueCount;

    static constexpr QueueSize kQueueCapacity = 64;
    static constexpr QueueSize kQueueSlack    = 2;  // Important value!

    static constexpr QueueItemIndex kInvalidQueueItemIndex = -1;

    static_assert((kQueueCapacity & (kQueueCapacity - 1)) == 0,
                  "kQueueCapacity MUST be the Power of 2");
    static_assert(kQueueSlack >= 2 && kQueueSlack < kQueueCapacity, "BAD kQueueSlack");
    static constexpr QueueSize kMaxQueueSize = kQueueCapacity - kQueueSlack;

    struct VkSemaphores final
    {
        VkSemaphores(std::nullptr_t) {}
        VkSemaphores(const Semaphore *s)
        {
            if (s)
            {
                count = 1;
                items = s->ptr();
            }
        }
        VkSemaphores(const VkSemaphore &s) : count(1), items(&s) {}
        VkSemaphores(const std::vector<VkSemaphore> &v) : count(v.size()), items(v.data()) {}
        size_t count             = 0;
        const VkSemaphore *items = nullptr;
    };

    struct CmdsState
    {
        PersistentCommandPool pool;
        struct QueueState
        {
            // Single Execution thread
            PrimaryCommandBuffer currentBuffer;
            // Single Context thread
            std::vector<VkSemaphore> waitSemaphores;
            std::vector<VkPipelineStageFlags> waitSemaphoreStageMasks;
            bool needSubmit = false;
        };
        angle::PackedEnumMap<egl::ContextPriority, QueueState> queueStates;
        std::vector<vk::PrimaryCommandBuffer> completedBuffersPush;
        std::vector<vk::PrimaryCommandBuffer> completedBuffersPop;
    };

    class DynamicTimeout final : angle::NonCopyable
    {
      public:
        explicit DynamicTimeout(uint64_t original);
        uint64_t getRemaining();

      private:
        const uint64_t mOriginal;
        double mStartTime;
    };

    class QueueItem final : angle::NonCopyable
    {
      public:
        enum class State
        {
            Idle      = 0,
            Submitted = 1,
            Error     = 2,
            Finished  = 3,
        };

        class SubmitScope final : angle::NonCopyable
        {
          public:
            SubmitScope(Context *context, CommandQueue2 *owner, QueueItemIndex itemIndex);
            SubmitScope(SubmitScope &&other);
            ~SubmitScope();

            QueueItem &getItem() const
            {
                ASSERT(mItem != nullptr);
                return *mItem;
            }

            void setInSubmitThread();
            void finish();

          private:
            Context *const mContext;
            CommandQueue2 *mOwner;
            const QueueItemIndex mItemIndex;
            QueueItem *mItem;
            bool mIsInSubmitThread = false;
        };

      public:
        Serial serial;
        QueueSerial queueSerial;
        VkQueue queue                     = VK_NULL_HANDLE;
        CmdsState::QueueState *queueState = nullptr;
        bool hasProtectedContent          = false;
        bool hasSubmit                    = false;

        VkSubmitInfo submitInfo = {};
        std::vector<VkSemaphore> waitSemaphores;
        std::vector<VkPipelineStageFlags> waitSemaphoreStageMasks;
        VkSemaphore signalSemaphore = VK_NULL_HANDLE;

        VkCommandBuffer oneOffCommandBuffer = VK_NULL_HANDLE;
        VkFence oneOffFence                 = VK_NULL_HANDLE;

        SharedFence fence;
        PrimaryCommandBuffer commandBuffer;

        // commandPools is for secondary CommandBuffer allocation
        SecondaryCommandPools *commandPools = nullptr;
        SecondaryCommandBufferList commandBuffersToReset;

      public:
        void resetState(State s = State::Idle) { mState.store(s, std::memory_order_relaxed); }
        State getState() const { return mState.load(std::memory_order_relaxed); }
        State acquireState() const { return mState.load(std::memory_order_acquire); }

        // Single Context thread
        angle::Result waitIdle(uint64_t timeout) const;
        void resetPendingState(VkDevice device);
        void resetSecondaryCommands(VkDevice device);

        void acquireLock();     // Call inside "mCommandQueueMutex"
        void releaseLock();     // Call outside "mCommandQueueMutex"
        bool isLocked() const;  // Call inside "mCommandQueueMutex"

        bool hasFence() const { return fence; }
        VkResult getPendingFenceStatus(VkDevice device, VkResult errorStateResult) const;
        VkResult getFenceStatus(VkDevice device, VkResult noFenceResult) const;
        VkResult waitPendingFence(VkDevice device,
                                  DynamicTimeout &timeout,
                                  VkResult errorStateResult) const;
        State waitSubmitted(DynamicTimeout &timeout) const;
        VkResult waitFence(VkDevice device, DynamicTimeout &timeout) const;

        // Single Submit thread
        void collectCommandBuffer(Context *context, CommandQueue2 *owner);
        void resetSubmittedState();

      private:
        void resetFence();

        void setState(State newState);
        void setStateAndNotify(State newState);
        template <class IsTargetState>
        State waitState(DynamicTimeout &timeout, IsTargetState &&isTargetState) const;

      private:
        std::atomic<State> mState{State::Idle};
        std::atomic<int> mLockCounter{0};
        mutable std::mutex mMutex;
        mutable CondVarHelper mCondVar;
    };

    class QueueTask final
    {
      public:
        QueueTask() = default;

        template <class F>
        explicit QueueTask(F &&functor)
        {
            mPtr.reset(new Wrapper<std::remove_reference_t<F>>(std::forward<F>(functor)));
        }

        angle::Result execute() { return mPtr->execute(); }

      private:
        class Interface
        {
          public:
            virtual ~Interface()            = default;
            virtual angle::Result execute() = 0;
        };

        template <class T>
        class Wrapper final : public Interface
        {
          public:
            template <class F>
            Wrapper(F &&functor) : mFunctor(std::forward<F>(functor))
            {}
            virtual angle::Result execute() final override { return mFunctor(); }

          private:
            T mFunctor;
        };

      private:
        std::unique_ptr<Interface> mPtr;
    };

    class SubmitThreadTaskQueue final : angle::NonCopyable
    {
      public:
        ~SubmitThreadTaskQueue();

        void init();
        void destroy();

        template <class F>
        void enqueue(F &&functor);
        void waitIdle();

        // Only for ASSERT()
        bool isIdle() const { return !mIsBusy; }

      private:
        void processTaskQueue();
        bool tryPopNextTask(QueueTask *taskOut);

      private:
        std::deque<QueueTask> mTaskQueue;
        std::thread mSubmitThread;
        std::mutex mMutex;
        CondVarHelper mCondVar;
        bool mIsBusy   = false;
        bool mNeedExit = false;
    };

  private:
    static QueueItemIndex IncIndex(QueueItemIndex index, QueueSize value = 1);
    static QueueItemIndex DecIndex(QueueItemIndex index, QueueSize value = 1);

    template <class GetFenceStatus>
    angle::Result checkCompletedItems(Context *context,
                                      QueueItemIndex beginIndex,
                                      QueueItemIndex endIndex,
                                      GetFenceStatus &&getFenceStatus,
                                      Serial *completedSerialOut);

    template <class TryResetItem>
    void resetCompletedItems(QueueItemIndex *beginIndex,
                             QueueItemIndex endIndex,
                             Serial completedSerial,
                             TryResetItem &&tryResetItem);

    // Single Context thread

    angle::Result prepareNextSubmit(Context *context,
                                    bool hasProtectedContent,
                                    VkQueue queue,
                                    CmdsState::QueueState *queueState,
                                    VkSemaphores waitSemaphores,
                                    const VkPipelineStageFlags *waitSemaphoreStageMasks,
                                    const VkSemaphore signalSemaphore,
                                    VkCommandBuffer oneOffCommandBuffer,
                                    const Fence *oneOffFence,
                                    const QueueSerial &submitQueueSerial,
                                    QueueItemIndex *itemIndexOut);

    angle::Result throttlePendingItemQueue(Context *context);  // Call before enqueue
    QueueSize getNumPendingItems() const;

    template <class OnEachItem>
    bool forEachUncompletedItem(Serial completedSerial,
                                const Serials &serials,
                                OnEachItem &&onEachItem,
                                QueueItemIndex *waitIndexOut);
    angle::Result waitAndResetPendingItems(Context *context,
                                           const QueueItem &waitItem,
                                           uint64_t timeout,
                                           bool mayUnlock = false);
    angle::Result waitAndResetPendingItems(Context *context,
                                           const ResourceUse &use,
                                           Serial waitSerial,
                                           uint64_t timeout,
                                           bool mayUnlock = false);
    angle::Result waitPendingItems(Context *context,
                                   Serial completedSerial,
                                   QueueItemIndex waitIndex,
                                   uint64_t timeout,
                                   bool mayUnlock = false);
    void resetPendingItems(VkDevice device, Serial completedSerial, bool maySkip = false);

    // Single Execution thread

    angle::Result ensurePrimaryCommandBufferValid(Context *context,
                                                  bool hasProtectedContent,
                                                  PrimaryCommandBuffer *primaryCommandBufferInOut);
    void resetCompletedBuffers(Context *context);

    // Single Submit thread

    angle::Result doSubmitCommandsJob(Context *context, QueueItem::SubmitScope &itemSubmitScope);
    angle::Result queueSubmit(Context *context, QueueItem::SubmitScope &itemSubmitScope);

    template <class OnPresentResult>
    void doQueuePresentJob(Context *context,
                           egl::ContextPriority priority,
                           const VkPresentInfoKHR &presentInfo,
                           OnPresentResult &&onPresentResult);

    void advanceSubmitIndex(QueueItemIndex submitItemIndex);
    void checkAndCollectCommandBuffers(Context *context);
    void collectCommandBuffers(Context *context, Serial completedSerial);
    void resetSubmittedItems();

    // Multiple threads

    void cleanupGarbage(RendererVk *renderer, Serial completedSerial);

    void updateLastCompletedSerials(const angle::FastMap<Serial, kMaxFastQueueSerials> &serials,
                                    Serial serial);

  private:
    std::mutex &mMutex;

    DeviceQueueMap mQueueMap;
    CmdsState mCmdsStateMap[2];  // 0 - normal; 1 - hasProtectedContent

    FenceRecycler mFenceRecycler;  // Multiple threads

    QueueItem mItemQueue[kQueueCapacity];
    // Single Context thread
    QueueItemIndex mNextPrepareIndex       = 0;
    QueueItemIndex mPendingStateResetIndex = 0;
    // Single Submit thread
    QueueItemIndex mNextSubmitIndex          = 0;
    QueueItemIndex mCommandBufferResetIndex  = 0;
    QueueItemIndex mSubmittedStateResetIndex = 0;

    // Multiple threads
    AtomicQueueSerial mLastCleanupGarbageSerial;

    SerialFactory mSerialFactory;  // Single Context thread
    AtomicQueueSerialFixedArray mLastSubmittedSerials;
    AtomicQueueSerialFixedArray mLastCompletedSerials;
    AtomicQueueSerial mLastCompletedSerial;

    angle::VulkanPerfCounters mPerfCounters = {};  // Single Submit thread

    SubmitThreadTaskQueue mSubmitThreadTaskQueue;
    std::mutex mCompletedBuffersMutex;
    bool mUseSubmitThread = false;
};

class ThreadSafeCommandQueue : public CommandQueue
{
  public:
    angle::Result init(Context *context, const DeviceQueueMap &queueMap)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return CommandQueue::init(context, queueMap);
    }
    void destroy(Context *context)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        CommandQueue::destroy(context);
    }

    void handleDeviceLost(RendererVk *renderer)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        CommandQueue::handleDeviceLost(renderer);
    }

    // Wait until the desired serial has been completed.
    angle::Result finishResourceUse(Context *context, const ResourceUse &use, uint64_t timeout);
    angle::Result finishQueueSerial(Context *context,
                                    const QueueSerial &queueSerial,
                                    uint64_t timeout);
    angle::Result waitIdle(Context *context, uint64_t timeout);
    angle::Result waitForResourceUseToFinishWithUserTimeout(Context *context,
                                                            const ResourceUse &use,
                                                            uint64_t timeout,
                                                            VkResult *result);
    bool isBusy(RendererVk *renderer) const;

    angle::Result submitCommands(Context *context,
                                 bool hasProtectedContent,
                                 egl::ContextPriority priority,
                                 const VkSemaphore signalSemaphore,
                                 SecondaryCommandBufferList &&commandBuffersToReset,
                                 SecondaryCommandPools *commandPools,
                                 const QueueSerial &submitQueueSerial)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return CommandQueue::submitCommands(context, hasProtectedContent, priority, signalSemaphore,
                                            std::move(commandBuffersToReset), commandPools,
                                            submitQueueSerial);
    }
    angle::Result queueSubmitOneOff(Context *context,
                                    bool hasProtectedContent,
                                    egl::ContextPriority contextPriority,
                                    VkCommandBuffer commandBufferHandle,
                                    const Semaphore *waitSemaphore,
                                    VkPipelineStageFlags waitSemaphoreStageMask,
                                    const Fence *fence,
                                    SubmitPolicy submitPolicy,
                                    const QueueSerial &submitQueueSerial)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return CommandQueue::queueSubmitOneOff(
            context, hasProtectedContent, contextPriority, commandBufferHandle, waitSemaphore,
            waitSemaphoreStageMask, fence, submitPolicy, submitQueueSerial);
    }
    VkResult queuePresent(egl::ContextPriority contextPriority,
                          const VkPresentInfoKHR &presentInfo,
                          SwapchainStatus *swapchainStatus)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return CommandQueue::queuePresent(contextPriority, presentInfo, swapchainStatus);
    }

    // Check to see which batches have finished completion (forward progress for
    // the last completed serial, for example for when the application busy waits on a query
    // result). It would be nice if we didn't have to expose this for QueryVk::getResult.
    angle::Result checkCompletedCommands(Context *context)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return CommandQueue::checkCompletedCommands(context);
    }
    void flushWaitSemaphores(bool hasProtectedContent,
                             egl::ContextPriority priority,
                             std::vector<VkSemaphore> &&waitSemaphores,
                             std::vector<VkPipelineStageFlags> &&waitSemaphoreStageMasks)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return CommandQueue::flushWaitSemaphores(hasProtectedContent, priority,
                                                 std::move(waitSemaphores),
                                                 std::move(waitSemaphoreStageMasks));
    }
    angle::Result flushOutsideRPCommands(Context *context,
                                         bool hasProtectedContent,
                                         egl::ContextPriority priority,
                                         OutsideRenderPassCommandBufferHelper **outsideRPCommands)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return CommandQueue::flushOutsideRPCommands(context, hasProtectedContent, priority,
                                                    outsideRPCommands);
    }
    angle::Result flushRenderPassCommands(Context *context,
                                          bool hasProtectedContent,
                                          egl::ContextPriority priority,
                                          const RenderPass &renderPass,
                                          RenderPassCommandBufferHelper **renderPassCommands)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return CommandQueue::flushRenderPassCommands(context, hasProtectedContent, priority,
                                                     renderPass, renderPassCommands);
    }

    const angle::VulkanPerfCounters getPerfCounters() const
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return CommandQueue::getPerfCounters();
    }
    void resetPerFramePerfCounters()
    {
        std::unique_lock<std::mutex> lock(mMutex);
        CommandQueue::resetPerFramePerfCounters();
    }

  private:
    mutable std::mutex mMutex;
};

// CommandProcessor is used to dispatch work to the GPU when the asyncCommandQueue feature is
// enabled. Issuing the |destroy| command will cause the worker thread to clean up it's resources
// and shut down. This command is sent when the renderer instance shuts down. Tasks are defined by
// the CommandQueue interface.

class CommandProcessor : public Context
{
  public:
    CommandProcessor(RendererVk *renderer);
    ~CommandProcessor() override;

    // Context
    void handleError(VkResult result,
                     const char *file,
                     const char *function,
                     unsigned int line) override;

    angle::Result init(Context *context, const DeviceQueueMap &queueMap);

    void destroy(Context *context);

    void handleDeviceLost(RendererVk *renderer);

    angle::Result submitCommands(Context *context,
                                 bool hasProtectedContent,
                                 egl::ContextPriority priority,
                                 const VkSemaphore signalSemaphore,
                                 SecondaryCommandBufferList &&commandBuffersToReset,
                                 SecondaryCommandPools *commandPools,
                                 const QueueSerial &submitQueueSerial);

    angle::Result queueSubmitOneOff(Context *context,
                                    bool hasProtectedContent,
                                    egl::ContextPriority contextPriority,
                                    VkCommandBuffer commandBufferHandle,
                                    const Semaphore *waitSemaphore,
                                    VkPipelineStageFlags waitSemaphoreStageMask,
                                    const Fence *fence,
                                    SubmitPolicy submitPolicy,
                                    const QueueSerial &submitQueueSerial);
    VkResult queuePresent(egl::ContextPriority contextPriority,
                          const VkPresentInfoKHR &presentInfo,
                          SwapchainStatus *swapchainStatus);

    angle::Result checkCompletedCommands(Context *context);

    void flushWaitSemaphores(bool hasProtectedContent,
                             egl::ContextPriority priority,
                             std::vector<VkSemaphore> &&waitSemaphores,
                             std::vector<VkPipelineStageFlags> &&waitSemaphoreStageMasks)
    {
        return mCommandQueue.flushWaitSemaphores(hasProtectedContent, priority,
                                                 std::move(waitSemaphores),
                                                 std::move(waitSemaphoreStageMasks));
    }
    angle::Result flushOutsideRPCommands(Context *context,
                                         bool hasProtectedContent,
                                         egl::ContextPriority priority,
                                         OutsideRenderPassCommandBufferHelper **outsideRPCommands);
    angle::Result flushRenderPassCommands(Context *context,
                                          bool hasProtectedContent,
                                          egl::ContextPriority priority,
                                          const RenderPass &renderPass,
                                          RenderPassCommandBufferHelper **renderPassCommands);

    egl::ContextPriority getDriverPriority(egl::ContextPriority priority)
    {
        return mCommandQueue.getDriverPriority(priority);
    }
    uint32_t getDeviceQueueIndex() const { return mCommandQueue.getDeviceQueueIndex(); }
    VkQueue getQueue(egl::ContextPriority priority) { return mCommandQueue.getQueue(priority); }

    // Note that due to inheritance from Context, this class has a set of perf counters as well,
    // but currently only the counters in the member command queue are of interest.
    const angle::VulkanPerfCounters getPerfCounters() const
    {
        return mCommandQueue.getPerfCounters();
    }
    void resetPerFramePerfCounters() { mCommandQueue.resetPerFramePerfCounters(); }

    ANGLE_INLINE bool hasUnfinishedUse(const ResourceUse &use) const
    {
        return mCommandQueue.hasUnfinishedUse(use);
    }

    bool hasUnsubmittedUse(const ResourceUse &use) const;
    Serial getLastSubmittedSerial(SerialIndex index) const { return mLastSubmittedSerials[index]; }

  protected:
    bool hasPendingError() const
    {
        std::lock_guard<std::mutex> queueLock(mErrorMutex);
        return !mErrors.empty();
    }
    angle::Result checkAndPopPendingError(Context *errorHandlingContext);

    // Entry point for command processor thread, calls processTasksImpl to do the
    // work. called by RendererVk::initializeDevice on main thread
    void processTasks();

    // Called asynchronously from main thread to queue work that is then processed by the worker
    // thread
    void queueCommand(CommandProcessorTask &&task);

    // Command processor thread, called by processTasks. The loop waits for work to
    // be submitted from a separate thread.
    angle::Result processTasksImpl(bool *exitThread);

    // Command processor thread, process a task
    angle::Result processTask(CommandProcessorTask *task);

    VkResult present(egl::ContextPriority priority,
                     const VkPresentInfoKHR &presentInfo,
                     SwapchainStatus *swapchainStatus);
    void updateSwapchainStatus(SwapchainStatus *swapchainStatus, VkResult presentResult);

    // Used by main thread to wait for worker thread to complete all outstanding work.
    angle::Result waitForWorkComplete(Context *context);

    std::queue<CommandProcessorTask> mTasks;
    mutable std::mutex mWorkerMutex;
    // Signal worker thread when work is available
    std::condition_variable mWorkAvailableCondition;
    // Signal main thread when all work completed
    mutable std::condition_variable mWorkerIdleCondition;
    // Track worker thread Idle state for assertion purposes
    bool mWorkerThreadIdle;
    ThreadSafeCommandQueue mCommandQueue;

    // Tracks last serial that was submitted to command processor. Note: this maybe different from
    // mLastSubmittedQueueSerial in CommandQueue since submission from CommandProcessor to
    // CommandQueue occur in a separate thread.
    AtomicQueueSerialFixedArray mLastSubmittedSerials;

    mutable std::mutex mErrorMutex;
    std::queue<Error> mErrors;

    // Command queue worker thread.
    std::thread mTaskThread;
};

class ThreadSafeCommandProcessor : public CommandProcessor
{
  public:
    ThreadSafeCommandProcessor(RendererVk *renderer) : CommandProcessor(renderer) {}

    angle::Result init(Context *context, const DeviceQueueMap &queueMap)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return CommandProcessor::init(context, queueMap);
    }
    void destroy(Context *context)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        CommandProcessor::destroy(context);
    }

    void handleDeviceLost(RendererVk *renderer)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        CommandProcessor::handleDeviceLost(renderer);
    }

    // Wait until the desired serial has been submitted.
    angle::Result waitForQueueSerialToBeSubmitted(vk::Context *context,
                                                  const QueueSerial &queueSerial);
    angle::Result waitForResourceUseToBeSubmitted(vk::Context *context, const ResourceUse &use);

    // Wait until the desired serial has been completed.
    angle::Result finishResourceUse(Context *context, const ResourceUse &use, uint64_t timeout);
    angle::Result finishQueueSerial(Context *context,
                                    const QueueSerial &queueSerial,
                                    uint64_t timeout);
    angle::Result waitIdle(Context *context, uint64_t timeout);
    angle::Result waitForResourceUseToFinishWithUserTimeout(Context *context,
                                                            const ResourceUse &use,
                                                            uint64_t timeout,
                                                            VkResult *result);
    bool isBusy(RendererVk *renderer) const;

    angle::Result submitCommands(Context *context,
                                 bool hasProtectedContent,
                                 egl::ContextPriority priority,
                                 const VkSemaphore signalSemaphore,
                                 SecondaryCommandBufferList &&commandBuffersToReset,
                                 SecondaryCommandPools *commandPools,
                                 const QueueSerial &submitQueueSerial)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return CommandProcessor::submitCommands(context, hasProtectedContent, priority,
                                                signalSemaphore, std::move(commandBuffersToReset),
                                                commandPools, submitQueueSerial);
    }
    angle::Result queueSubmitOneOff(Context *context,
                                    bool hasProtectedContent,
                                    egl::ContextPriority contextPriority,
                                    VkCommandBuffer commandBufferHandle,
                                    const Semaphore *waitSemaphore,
                                    VkPipelineStageFlags waitSemaphoreStageMask,
                                    const Fence *fence,
                                    SubmitPolicy submitPolicy,
                                    const QueueSerial &submitQueueSerial)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return CommandProcessor::queueSubmitOneOff(
            context, hasProtectedContent, contextPriority, commandBufferHandle, waitSemaphore,
            waitSemaphoreStageMask, fence, submitPolicy, submitQueueSerial);
    }
    VkResult queuePresent(egl::ContextPriority contextPriority,
                          const VkPresentInfoKHR &presentInfo,
                          SwapchainStatus *swapchainStatus)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return CommandProcessor::queuePresent(contextPriority, presentInfo, swapchainStatus);
    }

    // Check to see which batches have finished completion (forward progress for
    // the last completed serial, for example for when the application busy waits on a query
    // result). It would be nice if we didn't have to expose this for QueryVk::getResult.
    angle::Result checkCompletedCommands(Context *context)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return CommandProcessor::checkCompletedCommands(context);
    }
    void flushWaitSemaphores(bool hasProtectedContent,
                             egl::ContextPriority priority,
                             std::vector<VkSemaphore> &&waitSemaphores,
                             std::vector<VkPipelineStageFlags> &&waitSemaphoreStageMasks)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return CommandProcessor::flushWaitSemaphores(hasProtectedContent, priority,
                                                     std::move(waitSemaphores),
                                                     std::move(waitSemaphoreStageMasks));
    }
    angle::Result flushOutsideRPCommands(Context *context,
                                         bool hasProtectedContent,
                                         egl::ContextPriority priority,
                                         OutsideRenderPassCommandBufferHelper **outsideRPCommands)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return CommandProcessor::flushOutsideRPCommands(context, hasProtectedContent, priority,
                                                        outsideRPCommands);
    }
    angle::Result flushRenderPassCommands(Context *context,
                                          bool hasProtectedContent,
                                          egl::ContextPriority priority,
                                          const RenderPass &renderPass,
                                          RenderPassCommandBufferHelper **renderPassCommands)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return CommandProcessor::flushRenderPassCommands(context, hasProtectedContent, priority,
                                                         renderPass, renderPassCommands);
    }

    const angle::VulkanPerfCounters getPerfCounters() const
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return CommandProcessor::getPerfCounters();
    }
    void resetPerFramePerfCounters()
    {
        std::unique_lock<std::mutex> lock(mMutex);
        CommandProcessor::resetPerFramePerfCounters();
    }

  private:
    mutable std::mutex mMutex;
};
}  // namespace vk

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_COMMAND_PROCESSOR_H_
