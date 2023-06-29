//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MemoryTracking.h:
//    Defines the classes used for memory tracking in ANGLE.
//

#ifndef LIBANGLE_RENDERER_VULKAN_MEMORYTRACKING_H_
#define LIBANGLE_RENDERER_VULKAN_MEMORYTRACKING_H_

#include <array>
#include <atomic>
#include <mutex>

#include "common/angleutils.h"
#include "common/backtrace_utils.h"
#include "common/vulkan/vk_headers.h"

#define MEMORY_CALLBACK_ENABLE

#ifdef MEMORY_CALLBACK_ENABLE
constexpr bool kMemoryCallbackEnable = true;
#else
constexpr bool kMemoryCallbackEnable = false;
#endif

namespace rx
{
class RendererVk;

namespace vk
{
// Used to designate memory allocation type for tracking purposes.
enum class MemoryAllocationType
{
    Unspecified                              = 0,
    ImageExternal                            = 1,
    OffscreenSurfaceAttachmentImage          = 2,
    SwapchainMSAAImage                       = 3,
    SwapchainDepthStencilImage               = 4,
    StagingImage                             = 5,
    ImplicitMultisampledRenderToTextureImage = 6,
    TextureImage                             = 7,
    FontImage                                = 8,
    RenderBufferStorageImage                 = 9,
    Buffer                                   = 10,
    BufferExternal                           = 11,

    InvalidEnum = 12,
    EnumCount   = InvalidEnum,
};

constexpr const char *kMemoryAllocationTypeMessage[] = {
    "Unspecified",
    "ImageExternal",
    "OffscreenSurfaceAttachmentImage",
    "SwapchainMSAAImage",
    "SwapchainDepthStencilImage",
    "StagingImage",
    "ImplicitMultisampledRenderToTextureImage",
    "TextureImage",
    "FontImage",
    "RenderBufferStorageImage",
    "Buffer",
    "BufferExternal",
    "Invalid",
};
constexpr const uint32_t kMemoryAllocationTypeCount =
    static_cast<uint32_t>(MemoryAllocationType::EnumCount);

constexpr uint32_t kVkSystemAllocationScopeSize       = 5;
constexpr const char *kSystemAllocationScopeMessage[] = {
    "Command", "Object", "Cache", "Device", "Instance", "Invalid",
};

// Used to select the severity for memory allocation logs.
enum class MemoryLogSeverity
{
    INFO,
    WARN,
};

// Used to store memory allocation information for tracking purposes.
struct MemoryAllocationInfo
{
    MemoryAllocationInfo() = default;
    uint64_t id;
    MemoryAllocationType allocType;
    uint32_t memoryHeapIndex;
    void *handle;
    VkDeviceSize size;
};

class MemoryAllocInfoMapKey
{
  public:
    MemoryAllocInfoMapKey() : handle(nullptr) {}
    MemoryAllocInfoMapKey(void *handle) : handle(handle) {}

    bool operator==(const MemoryAllocInfoMapKey &rhs) const
    {
        return reinterpret_cast<uint64_t>(handle) == reinterpret_cast<uint64_t>(rhs.handle);
    }

    size_t hash() const;

  private:
    void *handle;
};

// Process GPU memory reports
class MemoryReport final : angle::NonCopyable
{
  public:
    MemoryReport();
    void processCallback(const VkDeviceMemoryReportCallbackDataEXT &callbackData, bool logCallback);
    void logMemoryReportStats() const;

  private:
    struct MemorySizes
    {
        VkDeviceSize allocatedMemory;
        VkDeviceSize allocatedMemoryMax;
        VkDeviceSize importedMemory;
        VkDeviceSize importedMemoryMax;
    };
    mutable std::mutex mMemoryReportMutex;
    VkDeviceSize mCurrentTotalAllocatedMemory;
    VkDeviceSize mMaxTotalAllocatedMemory;
    angle::HashMap<VkObjectType, MemorySizes> mSizesPerType;
    VkDeviceSize mCurrentTotalImportedMemory;
    VkDeviceSize mMaxTotalImportedMemory;
    angle::HashMap<uint64_t, int> mUniqueIDCounts;
};

struct MemoryCallbackInfo
{
    std::string label;
    // More info can be added.
};

class MemoryAllocationCallback
{
  public:
    MemoryAllocationCallback()
    {
        mCallbacks.pUserData             = (void *)this;
        mCallbacks.pfnAllocation         = &onAlloc;
        mCallbacks.pfnReallocation       = &onRealloc;
        mCallbacks.pfnFree               = &onFree;
        mCallbacks.pfnInternalAllocation = &onInternalAlloc;
        mCallbacks.pfnInternalFree       = &onInternalFree;

        for (uint32_t i = 0; i < kVkSystemAllocationScopeSize; i++)
        {
            mSystemAllocationSize[i]    = 0;
            mSystemAllocationSizeMax[i] = 0;
        }
    }

    // TODO: The following will be removed in favor of the cleaner version.
    operator VkAllocationCallbacks() const
    {
        VkAllocationCallbacks callback;

        callback.pUserData =
            (void *)this;  // TODO: Can be the class related to that Vulkan object type.
        callback.pfnAllocation         = &onAlloc;
        callback.pfnReallocation       = &onRealloc;
        callback.pfnFree               = &onFree;
        callback.pfnInternalAllocation = &onInternalAlloc;
        callback.pfnInternalFree       = &onInternalFree;

        return callback;
    }

    VkAllocationCallbacks *getCallbacks() { return &mCallbacks; }

    void printSystemAllocationData();

    // Define a general function that will be redirected to the general callback functions?
#define DEFINE_CALLBACK(VULKANOBJ)                                                                 \
    VKAPI_ATTR static void *VKAPI_CALL onAlloc##VULKANOBJ(                                         \
        void *pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)   \
    {                                                                                              \
        MemoryCallbackInfo callbackInfo;                                                           \
        callbackInfo.label = #VULKANOBJ;                                                           \
        return static_cast<MemoryAllocationCallback *>(pUserData)->onAllocImpl(                    \
            &callbackInfo, size, alignment, allocationScope);                                      \
    }                                                                                              \
    VKAPI_ATTR static void *VKAPI_CALL onRealloc##VULKANOBJ(                                       \
        void *pUserData, void *pOriginal, size_t size, size_t alignment,                           \
        VkSystemAllocationScope allocationScope)                                                   \
    {                                                                                              \
        MemoryCallbackInfo callbackInfo;                                                           \
        callbackInfo.label = #VULKANOBJ;                                                           \
        return static_cast<MemoryAllocationCallback *>(pUserData)->onReallocImpl(                  \
            &callbackInfo, pOriginal, size, alignment, allocationScope);                           \
    }                                                                                              \
    VKAPI_ATTR static void VKAPI_CALL onFree##VULKANOBJ(void *pUserData, void *pMemory)            \
    {                                                                                              \
        MemoryCallbackInfo callbackInfo;                                                           \
        callbackInfo.label = #VULKANOBJ;                                                           \
        return static_cast<MemoryAllocationCallback *>(pUserData)->onFreeImpl(&callbackInfo,       \
                                                                              pMemory);            \
    }                                                                                              \
    static VkAllocationCallbacks BuildCallback##VULKANOBJ(const VkAllocationCallbacks *callbackIn) \
    {                                                                                              \
        VkAllocationCallbacks callback;                                                            \
        callback.pUserData             = callbackIn->pUserData;                                    \
        callback.pfnAllocation         = &onAlloc##VULKANOBJ;                                      \
        callback.pfnReallocation       = &onRealloc##VULKANOBJ;                                    \
        callback.pfnFree               = &onFree##VULKANOBJ;                                       \
        callback.pfnInternalAllocation = &onInternalAlloc;                                         \
        callback.pfnInternalFree       = &onInternalFree;                                          \
        return callback;                                                                           \
    }

    DEFINE_CALLBACK(Allocator)
    DEFINE_CALLBACK(Buffer)
    DEFINE_CALLBACK(BufferView)
    DEFINE_CALLBACK(CommandPool)
    DEFINE_CALLBACK(DebugUtils)
    DEFINE_CALLBACK(DescriptorPool)
    DEFINE_CALLBACK(DescriptorSetLayout)
    DEFINE_CALLBACK(Device)
    DEFINE_CALLBACK(DeviceMemory)
    DEFINE_CALLBACK(Event)
    DEFINE_CALLBACK(Framebuffer)
    DEFINE_CALLBACK(Image)
    DEFINE_CALLBACK(ImageView)
    DEFINE_CALLBACK(Instance)
    DEFINE_CALLBACK(Pipeline)
    DEFINE_CALLBACK(PipelineCache)
    DEFINE_CALLBACK(PipelineLayout)
    DEFINE_CALLBACK(QueryPool)
    DEFINE_CALLBACK(RenderPass)
    DEFINE_CALLBACK(Sampler)
    DEFINE_CALLBACK(SamplerYcbcrConversion)
    DEFINE_CALLBACK(Semaphore)
    DEFINE_CALLBACK(ShaderModule)
    DEFINE_CALLBACK(Swapchain)

  private:
    void recordAlloc(MemoryCallbackInfo *callbackInfo,
                     size_t size,
                     size_t alignment,
                     VkSystemAllocationScope allocationScope,
                     void *pMemory);

    void recordRealloc(MemoryCallbackInfo *callbackInfo,
                       void *pOriginal,
                       size_t size,
                       size_t alignment,
                       VkSystemAllocationScope allocationScope,
                       void *pMemory);

    void recordFree(MemoryCallbackInfo *callbackInfo, void *pMemory);

    // Functions that will be called if an allocation callback is used in a Vulkan API call.
    VKAPI_ATTR static void *VKAPI_CALL onAlloc(void *pUserData,
                                               size_t size,
                                               size_t alignment,
                                               VkSystemAllocationScope allocationScope);
    VKAPI_ATTR static void *VKAPI_CALL onRealloc(void *pUserData,
                                                 void *pOriginal,
                                                 size_t size,
                                                 size_t alignment,
                                                 VkSystemAllocationScope allocationScope);
    VKAPI_ATTR static void VKAPI_CALL onFree(void *pUserData, void *pMemory);
    VKAPI_ATTR static void VKAPI_CALL onInternalAlloc(void *pUserData,
                                                      size_t size,
                                                      VkInternalAllocationType allocationType,
                                                      VkSystemAllocationScope allocationScope);
    VKAPI_ATTR static void VKAPI_CALL onInternalFree(void *pUserData,
                                                     size_t size,
                                                     VkInternalAllocationType allocationType,
                                                     VkSystemAllocationScope allocationScope);

    void *onAllocImpl(MemoryCallbackInfo *callbackInfo,
                      size_t size,
                      size_t alignment,
                      VkSystemAllocationScope allocationScope);
    void *onReallocImpl(MemoryCallbackInfo *callbackInfo,
                        void *pOriginal,
                        size_t size,
                        size_t alignment,
                        VkSystemAllocationScope allocationScope);
    void onFreeImpl(MemoryCallbackInfo *callbackInfo, void *pMemory);

    // Implementation of CPU allocations used in allocation callbacks.
    static void *allocateImpl(size_t alignedSize, size_t alignment);
    static void *reallocateImpl(void *pOriginal, size_t alignedSize, size_t alignment);
    static void freeImpl(void *pMemory);

    // Keeping track of the system allocations.
    std::mutex mMutex;
    VkAllocationCallbacks mCallbacks;
    using SizeScopePair = std::pair<VkDeviceSize, VkSystemAllocationScope>;
    std::unordered_map<void *, SizeScopePair> mMemoryPropertyMap;

    std::array<VkDeviceSize, kVkSystemAllocationScopeSize> mSystemAllocationSize;
    std::array<VkDeviceSize, kVkSystemAllocationScopeSize> mSystemAllocationSizeMax;
};
}  // namespace vk

// Memory tracker for allocations and deallocations, which is used in RendererVk.
class MemoryAllocationTracker : angle::NonCopyable
{
  public:
    MemoryAllocationTracker(RendererVk *renderer);
    void initMemoryTrackers();
    void onDeviceInit();
    void onDestroy();

    // Memory statistics are logged when handling a context error.
    void logMemoryStatsOnError();

    // Collect information regarding memory allocations and deallocations.
    void onMemoryAllocImpl(vk::MemoryAllocationType allocType,
                           VkDeviceSize size,
                           uint32_t memoryTypeIndex,
                           void *handle);
    void onMemoryDeallocImpl(vk::MemoryAllocationType allocType,
                             VkDeviceSize size,
                             uint32_t memoryTypeIndex,
                             void *handle);

    // Memory allocation statistics functions.
    VkDeviceSize getActiveMemoryAllocationsSize(uint32_t allocTypeIndex) const;
    VkDeviceSize getActiveHeapMemoryAllocationsSize(uint32_t allocTypeIndex,
                                                    uint32_t heapIndex) const;

    uint64_t getActiveMemoryAllocationsCount(uint32_t allocTypeIndex) const;
    uint64_t getActiveHeapMemoryAllocationsCount(uint32_t allocTypeIndex, uint32_t heapIndex) const;

    // Compare the expected flags with the flags of the allocated memory.
    void compareExpectedFlagsWithAllocatedFlags(VkMemoryPropertyFlags requiredFlags,
                                                VkMemoryPropertyFlags preferredFlags,
                                                VkMemoryPropertyFlags allocatedFlags,
                                                void *handle);

    // Pending memory allocation information is used for logging in case of an unsuccessful
    // allocation. It is cleared in onMemoryAlloc().
    VkDeviceSize getPendingMemoryAllocationSize() const;
    vk::MemoryAllocationType getPendingMemoryAllocationType() const;
    uint32_t getPendingMemoryTypeIndex() const;

    void resetPendingMemoryAlloc();
    void setPendingMemoryAlloc(vk::MemoryAllocationType allocType,
                               VkDeviceSize size,
                               uint32_t memoryTypeIndex);

    vk::MemoryAllocationCallback *getAllocationCallback() { return &mAllocationCallbacks; }
    VkAllocationCallbacks *getCallbacks();

  private:
    // Pointer to parent renderer object.
    RendererVk *const mRenderer;

    // For tracking the overall memory allocation sizes and counts per memory allocation type.
    std::array<std::atomic<VkDeviceSize>, vk::kMemoryAllocationTypeCount>
        mActiveMemoryAllocationsSize;
    std::array<std::atomic<uint64_t>, vk::kMemoryAllocationTypeCount> mActiveMemoryAllocationsCount;

    // Memory allocation data per memory heap.
    using PerHeapMemoryAllocationSizeArray =
        std::array<std::atomic<VkDeviceSize>, VK_MAX_MEMORY_HEAPS>;
    using PerHeapMemoryAllocationCountArray =
        std::array<std::atomic<uint64_t>, VK_MAX_MEMORY_HEAPS>;

    std::array<PerHeapMemoryAllocationSizeArray, vk::kMemoryAllocationTypeCount>
        mActivePerHeapMemoryAllocationsSize;
    std::array<PerHeapMemoryAllocationCountArray, vk::kMemoryAllocationTypeCount>
        mActivePerHeapMemoryAllocationsCount;

    // Pending memory allocation information is used for logging in case of an allocation error.
    // It includes the size and type of the last attempted allocation, which are cleared after
    // the allocation is successful.
    std::atomic<VkDeviceSize> mPendingMemoryAllocationSize;
    std::atomic<vk::MemoryAllocationType> mPendingMemoryAllocationType;
    std::atomic<uint32_t> mPendingMemoryTypeIndex;

    // Mutex is used to update the data when debug layers are enabled.
    std::mutex mMemoryAllocationMutex;

    // Additional information regarding memory allocation with debug layers enabled, including
    // allocation ID and a record of all active allocations.
    uint64_t mMemoryAllocationID;
    using MemoryAllocInfoMap = angle::HashMap<vk::MemoryAllocInfoMapKey, vk::MemoryAllocationInfo>;
    std::unordered_map<angle::BacktraceInfo, MemoryAllocInfoMap> mMemoryAllocationRecord;

    // Allocation callback object.
    vk::MemoryAllocationCallback mAllocationCallbacks;
};
}  // namespace rx

// Introduce std::hash for MemoryAllocInfoMapKey.
namespace std
{
template <>
struct hash<rx::vk::MemoryAllocInfoMapKey>
{
    size_t operator()(const rx::vk::MemoryAllocInfoMapKey &key) const { return key.hash(); }
};
}  // namespace std

#endif  // LIBANGLE_RENDERER_VULKAN_MEMORYTRACKING_H_
