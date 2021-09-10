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

#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
#    include "common/system_utils.h"
#endif
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

class FenceRecycler
{
  public:
    FenceRecycler() {}
    ~FenceRecycler() {}
    void destroy(vk::Context *context);

    angle::Result newSharedFence(vk::Context *context, vk::Shared<vk::Fence> *sharedFenceOut);
    inline void resetSharedFence(vk::Shared<vk::Fence> *sharedFenceIn)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        sharedFenceIn->resetAndRecycle(&mRecyler);
    }

  private:
    std::mutex mMutex;
    vk::Recycler<vk::Fence> mRecyler;
};

enum class CustomTask
{
    Invalid = 0,
    // Process SecondaryCommandBuffer commands into the primary CommandBuffer.
    ProcessCommands,
    // End the current command buffer and submit commands to the queue
    FlushAndQueueSubmit,
    // Submit custom command buffer, excludes some state management
    OneOffQueueSubmit,
#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    // Finish queue commands up to given serial value, process garbage
    FinishToSerial,
#endif
    // Execute QueuePresent
    Present,
#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    // do cleanup processing on completed commands
    // TODO: https://issuetracker.google.com/170312581 - should be able to remove
    // checkCompletedCommands command with fence refactor.
    CheckCompletedCommands,
#endif
#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    // Clears completed Commands and Garbage
    CleanupAllGarbage,
#endif
    // Exit the command processor thread
    Exit,
};

// CommandProcessorTask interface
class CommandProcessorTask
{
  public:
    CommandProcessorTask() { initTask(); }
// SVDT: Fixed CRASH when "asyncCommandQueue" feature is enabled.
#if SVDT_GLOBAL_CHANGES
    ~CommandProcessorTask() { mRenderPass.release(); }
#endif

    void initTask();

    void initTask(CustomTask command) { mTask = command; }

    void initProcessCommands(bool hasProtectedContent,
                             CommandBufferHelper *commandBuffer,
                             const RenderPass *renderPass);

    void initPresent(egl::ContextPriority priority, const VkPresentInfoKHR &presentInfo);

#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    void initFlushAndQueueSubmit(GarbageList &&currentGarbage, uint32_t submitItemIndex);

    void initOneOffQueueSubmit(uint32_t submitItemIndex);

#elif !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    void initFinishToSerial(Serial serial);

    void initFlushAndQueueSubmit(const std::vector<VkSemaphore> &waitSemaphores,
                                 const std::vector<VkPipelineStageFlags> &waitSemaphoreStageMasks,
                                 const Semaphore *semaphore,
                                 bool hasProtectedContent,
                                 egl::ContextPriority priority,
                                 GarbageList &&currentGarbage,
                                 Serial submitQueueSerial);

    void initOneOffQueueSubmit(VkCommandBuffer commandBufferHandle,
                               bool hasProtectedContent,
                               egl::ContextPriority priority,
                               const Fence *fence,
                               Serial submitQueueSerial);
#endif // !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2

    CommandProcessorTask &operator=(CommandProcessorTask &&rhs);

    CommandProcessorTask(CommandProcessorTask &&other) : CommandProcessorTask()
    {
        *this = std::move(other);
    }

#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    uint32_t getSubmitItemIndex() const { return mSubmitItemIndex; }
#endif

#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    void setQueueSerial(Serial serial) { mSerial = serial; }
    Serial getQueueSerial() const { return mSerial; }
#endif
    CustomTask getTaskCommand() { return mTask; }
#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    std::vector<VkSemaphore> &getWaitSemaphores() { return mWaitSemaphores; }
    std::vector<VkPipelineStageFlags> &getWaitSemaphoreStageMasks()
    {
        return mWaitSemaphoreStageMasks;
    }
    const Semaphore *getSemaphore() { return mSemaphore; }
#endif
    GarbageList &getGarbage() { return mGarbage; }
    egl::ContextPriority getPriority() const { return mPriority; }
    bool hasProtectedContent() const { return mHasProtectedContent; }
#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    VkCommandBuffer getOneOffCommandBufferVk() const { return mOneOffCommandBufferVk; }
    const Fence *getOneOffFence() { return mOneOffFence; }
#endif
    const VkPresentInfoKHR &getPresentInfo() const { return mPresentInfo; }
// SVDT: Fixed CRASH when "asyncCommandQueue" feature is enabled.
#if SVDT_GLOBAL_CHANGES
    const RenderPass &getRenderPass() const { return mRenderPass; }
#else
    const RenderPass *getRenderPass() const { return mRenderPass; }
#endif
    CommandBufferHelper *getCommandBuffer() const { return mCommandBuffer; }

  private:
    void copyPresentInfo(const VkPresentInfoKHR &other);

    CustomTask mTask;

    // ProcessCommands
// SVDT: Fixed CRASH when "asyncCommandQueue" feature is enabled.
#if SVDT_GLOBAL_CHANGES
    RenderPass mRenderPass;
#else
    const RenderPass *mRenderPass;
#endif
    CommandBufferHelper *mCommandBuffer;

    // Flush data
#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    std::vector<VkSemaphore> mWaitSemaphores;
    std::vector<VkPipelineStageFlags> mWaitSemaphoreStageMasks;
    const Semaphore *mSemaphore;
#endif
    GarbageList mGarbage;
#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    uint32_t mSubmitItemIndex;
#endif

#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    // FinishToSerial & Flush command data
    Serial mSerial;
#endif

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
#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    VkCommandBuffer mOneOffCommandBufferVk;
    const Fence *mOneOffFence;
#endif

    // Flush, Present & QueueWaitIdle data
    egl::ContextPriority mPriority;
    bool mHasProtectedContent;
};

#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
struct CommandBatch final : angle::NonCopyable
{
    CommandBatch();
    ~CommandBatch();
    CommandBatch(CommandBatch &&other);
    CommandBatch &operator=(CommandBatch &&other);

    void destroy(VkDevice device);

    PrimaryCommandBuffer primaryCommands;
// SVDT: Removed dead code related to the uninitialized/unused CommandPool.
#if !SVDT_GLOBAL_CHANGES
    // commandPool is for secondary CommandBuffer allocation
    CommandPool commandPool;
#endif
    Shared<Fence> fence;
    Serial serial;
    bool hasProtectedContent;
};
#endif

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
    DeviceQueueMap() : mIndex(vk::QueueFamily::kInvalidIndex), mIsProtected(false) {}
    DeviceQueueMap(uint32_t queueFamilyIndex, bool isProtected)
        : mIndex(queueFamilyIndex), mIsProtected(isProtected)
    {}
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

class CommandQueueInterface : angle::NonCopyable
{
  public:
    virtual ~CommandQueueInterface() {}

    virtual angle::Result init(Context *context, const DeviceQueueMap &queueMap) = 0;
    virtual void destroy(Context *context)                                       = 0;

    virtual void handleDeviceLost(RendererVk *renderer) = 0;

    // Wait until the desired serial has been completed.
    virtual angle::Result finishToSerial(Context *context,
                                         Serial finishSerial,
                                         uint64_t timeout) = 0;
    virtual Serial reserveSubmitSerial()                   = 0;
    virtual angle::Result submitFrame(
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
        Serial submitQueueSerial)                                      = 0;
    virtual angle::Result queueSubmitOneOff(Context *context,
                                            bool hasProtectedContent,
                                            egl::ContextPriority contextPriority,
                                            VkCommandBuffer commandBufferHandle,
                                            const Fence *fence,
                                            SubmitPolicy submitPolicy,
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
                                                 bool hasProtectedContent,
                                                 CommandBufferHelper **outsideRPCommands)   = 0;
    virtual angle::Result flushRenderPassCommands(Context *context,
                                                  bool hasProtectedContent,
                                                  const RenderPass &renderPass,
                                                  CommandBufferHelper **renderPassCommands) = 0;

    virtual Serial getLastSubmittedQueueSerial() const = 0;
    virtual Serial getLastCompletedQueueSerial() const = 0;
    virtual Serial getCurrentQueueSerial() const       = 0;
};

#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
class CommandQueue final : public CommandQueueInterface
{
  public:
    CommandQueue();
    ~CommandQueue() override;

    angle::Result init(Context *context, const DeviceQueueMap &queueMap) override;
    void destroy(Context *context) override;
    void clearAllGarbage(RendererVk *renderer);

    void handleDeviceLost(RendererVk *renderer) override;

    angle::Result finishToSerial(Context *context, Serial finishSerial, uint64_t timeout) override;

    Serial reserveSubmitSerial() override;

    angle::Result submitFrame(Context *context,
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
                              Serial submitQueueSerial) override;

    angle::Result queueSubmitOneOff(Context *context,
                                    bool hasProtectedContent,
                                    egl::ContextPriority contextPriority,
                                    VkCommandBuffer commandBufferHandle,
                                    const Fence *fence,
                                    SubmitPolicy submitPolicy,
                                    Serial submitQueueSerial) override;

    VkResult queuePresent(egl::ContextPriority contextPriority,
                          const VkPresentInfoKHR &presentInfo) override;

    angle::Result waitForSerialWithUserTimeout(vk::Context *context,
                                               Serial serial,
                                               uint64_t timeout,
                                               VkResult *result) override;

    angle::Result checkCompletedCommands(Context *context) override;

    angle::Result flushOutsideRPCommands(Context *context,
                                         bool hasProtectedContent,
                                         CommandBufferHelper **outsideRPCommands) override;
    angle::Result flushRenderPassCommands(Context *context,
                                          bool hasProtectedContent,
                                          const RenderPass &renderPass,
                                          CommandBufferHelper **renderPassCommands) override;

    Serial getLastSubmittedQueueSerial() const override;
    Serial getLastCompletedQueueSerial() const override;
    Serial getCurrentQueueSerial() const override;

#if SVDT_ENABLE_VULKAN_ASYNC_COMMAND_QUEUE_LAST_SUBMITTED_SERIAL_WA
    void setLastSubmittedQueueSerial(Context *context, Serial submitQueueSerial);
#endif

    angle::Result queueSubmit(Context *context,
                              egl::ContextPriority contextPriority,
                              const VkSubmitInfo &submitInfo,
                              const Fence *fence,
                              Serial submitQueueSerial);

    egl::ContextPriority getDriverPriority(egl::ContextPriority priority)
    {
// SVDT: CRITICAL bug fixed in "rx::RendererVk" class when "asyncCommandQueue" is enabled.
#if SVDT_GLOBAL_CHANGES
        ASSERT(mQueueMap.valid());
#endif
        return mQueueMap.getDevicePriority(priority);
    }

  private:
// SVDT: Removed dead code related to the uninitialized/unused CommandPool.
#if !SVDT_GLOBAL_CHANGES
    angle::Result releaseToCommandBatch(Context *context,
                                        bool hasProtectedContent,
                                        PrimaryCommandBuffer &&commandBuffer,
                                        CommandPool *commandPool,
                                        CommandBatch *batch);
#endif
    angle::Result retireFinishedCommands(Context *context, size_t finishedCount);
    angle::Result ensurePrimaryCommandBufferValid(Context *context, bool hasProtectedContent);

    bool allInFlightCommandsAreAfterSerial(Serial serial);

    VkQueue getQueue(egl::ContextPriority priority) { return mQueueMap[priority]; }

    PrimaryCommandBuffer &getCommandBuffer(bool hasProtectedContent)
    {
        if (hasProtectedContent)
        {
            return mProtectedCommands;
        }
        else
        {
            return mPrimaryCommands;
        }
    }

    PersistentCommandPool &getCommandPool(bool hasProtectedContent)
    {
        if (hasProtectedContent)
        {
            return mProtectedCommandPool;
        }
        else
        {
            return mPrimaryCommandPool;
        }
    }

    GarbageQueue mGarbageQueue;

    std::vector<CommandBatch> mInFlightCommands;

    // Keeps a free list of reusable primary command buffers.
    PrimaryCommandBuffer mPrimaryCommands;
    PersistentCommandPool mPrimaryCommandPool;
    PrimaryCommandBuffer mProtectedCommands;
    PersistentCommandPool mProtectedCommandPool;

    // Queue serial management.
    AtomicSerialFactory mQueueSerialFactory;
    Serial mLastCompletedQueueSerial;
    Serial mLastSubmittedQueueSerial;
    Serial mCurrentQueueSerial;

    // QueueMap
    DeviceQueueMap mQueueMap;

    FenceRecycler mFenceRecycler;
};
#elif SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2

// Replacement for the CommandQueue
// Main differences:
// - Allows checking/waiting for commands from Context thread, while flushing/submitting in other thread.
//   (Does not require CommandProcessorTask and CommandProcessor::waitForWorkComplete() call)
// - Removed forced Garbage Cleanup from checking/waiting functions.
// - Garbage Cleanup performed only as part of submit operations or explicitly.
// - Added explicit cleanupAllGarbage() function to use in glFinish() to release resources.
// - getLastSubmittedQueueSerial() will be updated in Context thread, while submitting in other thread.
// - Fixed "Finish to Serial" logic in cases, when multiple VkQueues used at the same time.
// - Fixed bug when "OneOff" submits can not be explicitly waited.
// - Better "kInFlightCommandsLimit" handling when using threads.
//
// Threads:
// - Context thread - any thread from which GL/EGL API is called (Some client application thread)
// - Execution thread - additional thread or "Context thread" (CommandProcessor thread)
// - Submit thread - additional thread or "Execution thread" (mSubmitThreadTaskQueue)
class CommandQueue2 final : angle::NonCopyable
{
  public:
    using QueuePriority = egl::ContextPriority;
    using QueueSize = uint32_t;
    using QueueItemIndex = QueueSize;

    static constexpr uint32_t kVkQueueCount = QueueFamily::kQueueCount;

    static constexpr QueueSize kQueueCapacity = 128;
    static constexpr QueueSize kQueueSlack = 2; // Important value!

    static_assert((kQueueCapacity & (kQueueCapacity - 1)) == 0, "kQueueCapacity MUST be the Power of 2");
    static_assert(kQueueSlack >= 2 && kQueueSlack < kQueueCapacity, "BAD kQueueSlack");
    static constexpr QueueSize kMaxQueueSize = kQueueCapacity - kQueueSlack;

  public:
    angle::Result init(Context *context, const DeviceQueueMap &queueMap);
    void destroy(Context *context);

    // Single Context thread

    void handleDeviceLost(RendererVk *renderer);

    Serial reserveSubmitSerial();

    angle::Result checkCompletedCommands(Context *context);
    angle::Result finishToSerial(Context *context, Serial finishSerial, uint64_t timeout);
    angle::Result waitForSerialWithUserTimeout(
            Context *context, Serial serial, uint64_t timeout, VkResult *result);

    egl::ContextPriority getDriverPriority(egl::ContextPriority priority)
    {
        return mQueueMap.getDevicePriority(priority);
    }

    // Single Execution thread

    angle::Result flushOutsideRPCommands(
            Context *context, bool hasProtectedContent,
            CommandBufferHelper **outsideRPCommands);
    angle::Result flushRenderPassCommands(
            Context *context, bool hasProtectedContent,
            const RenderPass &renderPass, CommandBufferHelper **renderPassCommands);

    angle::Result submitFrame(
            Context *context,
            bool hasProtectedContent,
            QueuePriority priority,
            const std::vector<VkSemaphore> &waitSemaphores,
            const std::vector<VkPipelineStageFlags> &waitSemaphoreStageMasks,
            const Semaphore *signalSemaphore,
            GarbageList &&currentGarbage,
            Serial submitQueueSerial);

    angle::Result queueSubmitOneOff(
            Context *context,
            bool hasProtectedContent,
            QueuePriority priority,
            VkCommandBuffer commandBufferHandle,
            const Fence *fence,
            SubmitPolicy submitPolicy,
            Serial submitQueueSerial);

    VkResult queuePresent(Context *context, QueuePriority priority, const VkPresentInfoKHR &presentInfo);

    angle::Result cleanupAllGarbage(Context *context);

    // Multiple threads

    Serial getLastSubmittedQueueSerial() const;
    Serial getLastCompletedQueueSerial() const;
    Serial getCurrentQueueSerial() const;

  public: // Alternative methods for use from multiple threads (ex: CommandProcessor class)
    struct VkSemaphores final
    {
        VkSemaphores(std::nullptr_t) {}
        VkSemaphores(const Semaphore *s) { if (s) { count = 1; items = s->ptr(); } }
        VkSemaphores(const VkSemaphore &s) : count(1), items(&s) {}
        VkSemaphores(const std::vector<VkSemaphore> &v) : count(v.size()), items(v.data()) {}
        size_t count = 0;
        const VkSemaphore *items = nullptr;
    };

  public:
    // Single Context thread

#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
    void waitSubmitThreadIdle(); // Safe to call after destroy()
#endif

    angle::Result prepareNextSubmit(
            Context *context,
            bool hasProtectedContent,
            QueuePriority priority,
            VkSemaphores waitSemaphores,
            const VkPipelineStageFlags *waitSemaphoreStageMasks,
            const Semaphore *signalSemaphore,
            VkCommandBuffer oneOffCommandBuffer,
            const Fence *oneOffFence,
            Serial submitQueueSerial,
            QueueItemIndex *itemIndexOut);

    // Single Execution thread

    angle::Result doSubmitFrame(
            Context *context, GarbageList &&currentGarbage, QueueItemIndex itemIndex);

    angle::Result doQueueSubmitOneOff(Context *context, QueueItemIndex itemIndex);

    template <class ON_PRESENT_RESULT>
    void doQueuePresent(
            Context *context, QueuePriority priority, const VkPresentInfoKHR &presentInfo,
            ON_PRESENT_RESULT &&onPresentResult);

  private:
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

            QueueItem &getItem() const { ASSERT(mItem != nullptr); return *mItem; }

#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
            void setInSubmitThread();
#endif
            void setSubmitOK();

          private:
            Context *const mContext;
            CommandQueue2 *mOwner;
            const QueueItemIndex mItemIndex;
            QueueItem *mItem;
#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
            bool mIsInSubmitThread = false;
#endif
        };

      public:
        bool hasProtectedContent = false;
        Serial serial;
        VkQueue queue = VK_NULL_HANDLE;

        VkSubmitInfo submitInfo = {};
        std::vector<VkSemaphore> waitSemaphores;
        std::vector<VkPipelineStageFlags> waitSemaphoreStageMasks;
        VkSemaphore signalSemaphore = VK_NULL_HANDLE;

        VkCommandBuffer oneOffCommandBuffer = VK_NULL_HANDLE;
        VkFence oneOffFence = VK_NULL_HANDLE;

        Shared<Fence> fence;
        PrimaryCommandBuffer commandBuffer;

      public:
        void resetState() { mState.store(State::Idle, std::memory_order_relaxed); }
        State getState() const { return mState.load(std::memory_order_relaxed); }
        State acquireState() const { return mState.load(std::memory_order_acquire); }

        // Single Context thread
        angle::Result waitIdle(uint64_t timeout) const;
        void resetPendingState(VkDevice device);
#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_CONCURRENT_WAIT
        void acquireLock();  // Call inside "mCommandQueueMutex"
        void releaseLock();  // Call outside "mCommandQueueMutex"
        bool isLocked() const;  // Call inside "mCommandQueueMutex"
#endif
        VkResult getPendingFenceStatus(VkDevice device, VkResult errorStateResult) const;
        VkResult getFenceStatus(VkDevice device) const;
        VkResult waitPendingFence(VkDevice device, DynamicTimeout &timeout, VkResult errorStateResult) const;
        State waitSubmitted(DynamicTimeout &timeout) const;
        VkResult waitFence(VkDevice device, DynamicTimeout &timeout) const;

        // Single Submit thread
        void collectCommandBuffer(Context *context, CommandQueue2 *owner);
        void resetSubmittedState(CommandQueue2 *owner);

      private:
        void resetFence(CommandQueue2 *owner);

        void setState(State newState);
        void setStateAndNotify(State newState);
        template <class PRED>
        State waitState(DynamicTimeout &timeout, PRED &&pred) const;

      private:
        std::atomic<State> mState{ State::Idle };
#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_CONCURRENT_WAIT
        std::atomic<int> mLockCounter{ 0 };
#endif
        mutable std::mutex mMutex;
        mutable CondVarHelper mCondVar;
    };

#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
    class QueueTask final
    {
    public:
        QueueTask() = default;

        template <class F>
        explicit QueueTask(F &&functor)
        {
            mPtr.reset(new Wrapper<std::remove_reference_t<F>>(std::forward<F>(functor)));
        }

        angle::Result execute()
        {
            return mPtr->execute();
        }

    private:
        class Interface
        {
        public:
            virtual ~Interface() = default;
            virtual angle::Result execute() = 0;
        };

        template <class T>
        class Wrapper final : public Interface
        {
        public:
            template <class F>
            Wrapper(F &&functor) : mFunctor(std::forward<F>(functor)) {}
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

      private:
        void processTaskQueue();
        bool tryPopNextTask(QueueTask *taskOut);

      private:
        std::deque<QueueTask> mTaskQueue;
        std::thread mSubmitThread;
        std::mutex mMutex;
        CondVarHelper mCondVar;
        bool mIsBusy = false;
        bool mNeedExit = false;
    };
#endif // SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE

    struct CmdsState
    {
        PersistentCommandPool pool;
        PrimaryCommandBuffer currentBuffer;
#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
        std::vector<vk::PrimaryCommandBuffer> completedBuffersPush;
        std::vector<vk::PrimaryCommandBuffer> completedBuffersPop;
#endif
    };

  private:
    static QueueItemIndex incIndex(QueueItemIndex index, QueueSize value = 1);
    static QueueItemIndex decIndex(QueueItemIndex index, QueueSize value = 1);

    template <class GetFenceStatus, class ResetItem>
    angle::Result checkAndResetCompletedItems(
            Context *context, QueueItemIndex *beginIndex, const QueueItemIndex endIndex,
            GetFenceStatus &&getFenceStatus, ResetItem &&resetItem);

    template <class TryResetItem>
    void resetCompletedItems(
            QueueItemIndex *beginIndex, const QueueItemIndex endIndex,
            Serial completedSerial, TryResetItem &&tryResetItem);

    bool itemsHasValidOrdering(QueueItemIndex beginIndex, const QueueItemIndex endIndex) const;

    // Single Context thread

    angle::Result throttlePendingItemQueue(Context *context); // Call before enqueue
    QueueSize getNumPendingItems() const;

    angle::Result waitAndResetPendingItems(Context *context, Serial waitSerial,
                                           uint64_t timeout, bool mayUnlock = false);
    angle::Result waitPendingItems(Context *context, Serial completedSerial, Serial waitSerial,
                                   uint64_t timeout, bool mayUnlock = false);
    void resetPendingItems(Context *context, Serial completedSerial, bool maySkip = false);

    // Single Execution thread

    angle::Result getValidPrimaryCommandBuffer(
            Context *context, bool hasProtectedContent, PrimaryCommandBuffer **primaryCommandBufferOut);
#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
    void resetCompletedBuffers(Context *context);
#endif

    // Single Submit thread

    angle::Result doSubmitFrameJob(Context *context, QueueItem::SubmitScope &&itemSubmitScope);
    angle::Result queueSubmit(Context *context, QueueItem::SubmitScope &&itemSubmitScope);

    template <class ON_PRESENT_RESULT>
    void doQueuePresentJob(
            Context *context, QueuePriority priority, const VkPresentInfoKHR &presentInfo,
            ON_PRESENT_RESULT &&onPresentResult);

    void advanceSubmitIndex(QueueItemIndex submitItemIndex);
    void checkAndCollectCommandBuffers(Context *context);
    void collectCommandBuffers(Context *context, Serial completedSerial);
    void resetSubmittedItems();

    angle::Result clearGarbage(Context *context, Serial completedSerial);
    void clearGarbageQueue(Context *context, Serial completedSerial);

    // Multiple threads

    void collectGarbage(GarbageList &&garbage, Serial serial);

    void updateLastSubmittedQueueSerial(Serial newSerial);
    void updateLastCompletedQueueSerial(Serial oldSerial, Serial newSerialCandidate);

  private:
    DeviceQueueMap mQueueMap;
    CmdsState mCmdsStateMap[2]; // 0 - normal; 1 - hasProtectedContent

    FenceRecycler mFenceRecycler; // Multiple threads

    QueueItem mItemQueue[kQueueCapacity];
    // Single Context thread
    QueueItemIndex mNextPrepareIndex = 0;
    QueueItemIndex mPendingStateResetIndex = 0;
    // Single Submit thread
    QueueItemIndex mNextSubmitIndex = 0;
    QueueItemIndex mCommandBufferResetIndex = 0;
    QueueItemIndex mSubmittedStateResetIndex = 0;

    GarbageQueue mGarbageQueue; // Multiple threads
    std::mutex mGarbageQueueMutex;
    Serial mLastClearGarbageSerial; // Single Submit thread

    SerialFactory mQueueSerialFactory; // Single Context thread
    std::atomic<Serial> mLastCompletedQueueSerial{ Serial() }; // Multiple threads
    std::atomic<Serial> mLastSubmittedQueueSerial{ Serial() }; // Multiple threads
    std::atomic<Serial> mCurrentQueueSerial{ mQueueSerialFactory.generate() }; // Multiple threads

#if SVDT_ENABLE_VULKAN_SUBMIT_THREAD_TASK_QUEUE
    SubmitThreadTaskQueue mSubmitThreadTaskQueue;
    std::mutex mCompletedBuffersMutex;
    bool mUseSubmitThread = false;
#endif
};
#endif // SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2

// CommandProcessor is used to dispatch work to the GPU when the asyncCommandQueue feature is
// enabled. Issuing the |destroy| command will cause the worker thread to clean up it's resources
// and shut down. This command is sent when the renderer instance shuts down. Tasks are defined by
// the CommandQueue interface.

class CommandProcessor : public Context, public CommandQueueInterface
{
  public:
    CommandProcessor(RendererVk *renderer);
    ~CommandProcessor() override;

// SVDT: CRITICAL bug fixed in "rx::RendererVk" class when "asyncCommandQueue" is enabled.
#if SVDT_GLOBAL_CHANGES
    egl::ContextPriority getDriverPriority(egl::ContextPriority priority)
    {
        return mCommandQueue.getDriverPriority(priority);
    }
#endif

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
    angle::Result init(Context *context, const DeviceQueueMap &qeueMap) override;

    void destroy(Context *context) override;

    void handleDeviceLost(RendererVk *renderer) override;

    angle::Result finishToSerial(Context *context, Serial finishSerial, uint64_t timeout) override;

    Serial reserveSubmitSerial() override;

    angle::Result submitFrame(Context *context,
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
                              Serial submitQueueSerial) override;

    angle::Result queueSubmitOneOff(Context *context,
                                    bool hasProtectedContent,
                                    egl::ContextPriority contextPriority,
                                    VkCommandBuffer commandBufferHandle,
                                    const Fence *fence,
                                    SubmitPolicy submitPolicy,
                                    Serial submitQueueSerial) override;
    VkResult queuePresent(egl::ContextPriority contextPriority,
                          const VkPresentInfoKHR &presentInfo) override;

    angle::Result waitForSerialWithUserTimeout(vk::Context *context,
                                               Serial serial,
                                               uint64_t timeout,
                                               VkResult *result) override;

    angle::Result checkCompletedCommands(Context *context) override;

    angle::Result flushOutsideRPCommands(Context *context,
                                         bool hasProtectedContent,
                                         CommandBufferHelper **outsideRPCommands) override;
    angle::Result flushRenderPassCommands(Context *context,
                                          bool hasProtectedContent,
                                          const RenderPass &renderPass,
                                          CommandBufferHelper **renderPassCommands) override;

#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    angle::Result cleanupAllGarbage(Context *context);
#endif

    Serial getLastSubmittedQueueSerial() const override;
    Serial getLastCompletedQueueSerial() const override;
    Serial getCurrentQueueSerial() const override;

  private:
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

    VkResult getLastAndClearPresentResult(VkSwapchainKHR swapchain);
#if !SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    VkResult present(egl::ContextPriority priority, const VkPresentInfoKHR &presentInfo);
#endif

    std::queue<CommandProcessorTask> mTasks;
    mutable std::mutex mWorkerMutex;
    // Signal worker thread when work is available
#if SVDT_ENABLE_VULKAN_OPTIMIZED_ASYNC_COMMAND_ENQUEUE
    CondVarHelper mWorkAvailableCondition;
#else
    std::condition_variable mWorkAvailableCondition;
#endif
    // Signal main thread when all work completed
    mutable std::condition_variable mWorkerIdleCondition;
    // Track worker thread Idle state for assertion purposes
    bool mWorkerThreadIdle;
// SVDT: Removed dead code related to the uninitialized/unused CommandPool.
#if !SVDT_GLOBAL_CHANGES
    // Command pool to allocate processor thread primary command buffers from
    CommandPool mCommandPool;
#endif
#if SVDT_ENABLE_VULKAN_COMMAND_QUEUE_2
    CommandQueue2 mCommandQueue;
#else
    CommandQueue mCommandQueue;

    mutable std::mutex mQueueSerialMutex;
#endif

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
