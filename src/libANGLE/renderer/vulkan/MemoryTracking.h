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

#include "common/SimpleMutex.h"
#include "common/angleutils.h"
#include "common/backtrace_utils.h"
#include "common/vulkan/vk_headers.h"
#include "vk_mem_alloc_wrapper.h"

#ifdef ANGLE_ENABLE_MEMORY_ALLOC_CALLBACKS
constexpr bool kMemoryCallbackEnable = true;
#else
constexpr bool kMemoryCallbackEnable = false;
#endif

namespace rx
{
namespace vk
{
class Renderer;

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
    mutable angle::SimpleMutex mMemoryReportMutex;
    VkDeviceSize mCurrentTotalAllocatedMemory;
    VkDeviceSize mMaxTotalAllocatedMemory;
    angle::HashMap<VkObjectType, MemorySizes> mSizesPerType;
    VkDeviceSize mCurrentTotalImportedMemory;
    VkDeviceSize mMaxTotalImportedMemory;
    angle::HashMap<uint64_t, int> mUniqueIDCounts;
};

enum class MemoryAllocationCallbackType
{
    Unknown                = 0,
    Allocator              = 1,
    Buffer                 = 2,
    BufferView             = 3,
    CommandPool            = 4,
    DebugUtils             = 5,
    DescriptorPool         = 6,
    DescriptorSetLayout    = 7,
    Device                 = 8,
    DeviceMemory           = 9,
    Event                  = 10,
    Framebuffer            = 11,
    Image                  = 12,
    ImageView              = 13,
    Instance               = 14,
    Pipeline               = 15,
    PipelineCache          = 16,
    PipelineLayout         = 17,
    QueryPool              = 18,
    RenderPass             = 19,
    Sampler                = 20,
    SamplerYcbcrConversion = 21,
    Semaphore              = 22,
    ShaderModule           = 23,
    Swapchain              = 24,

    InvalidEnum = 25,
    EnumCount   = InvalidEnum,
};

constexpr const char *kMemoryAllocationCallbackTypeMessage[] = {
    "Unknown",        "Allocator",
    "Buffer",         "BufferView",
    "CommandPool",    "DebugUtils",
    "DescriptorPool", "DescriptorSetLayout",
    "Device",         "DeviceMemory",
    "Event",          "Framebuffer",
    "Image",          "ImageView",
    "Instance",       "Pipeline",
    "PipelineCache",  "PipelineLayout",
    "QueryPool",      "RenderPass",
    "Sampler",        "SamplerYcbcrConversion",
    "Semaphore",      "ShaderModule",
    "Swapchain",      "Invalid",
};
constexpr const uint32_t kMemoryAllocationCallbackTypeCount =
    static_cast<uint32_t>(MemoryAllocationCallbackType::EnumCount);

struct MemoryCallbackInfo
{
    MemoryAllocationCallbackType callbackType;
    std::string label;
    // More info can be added.
};

// Define a general function that will be redirected to the general callback functions?
#define DEFINE_CALLBACK(VULKANOBJ)                                                               \
    VKAPI_ATTR static void *VKAPI_CALL onAlloc##VULKANOBJ(                                       \
        void *pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) \
    {                                                                                            \
        MemoryCallbackInfo callbackInfo;                                                         \
        callbackInfo.callbackType = MemoryAllocationCallbackType::VULKANOBJ;                     \
        callbackInfo.label        = #VULKANOBJ;                                                  \
        return static_cast<MemoryAllocationCallback *>(pUserData)->onAllocImpl(                  \
            &callbackInfo, size, alignment, allocationScope);                                    \
    }                                                                                            \
    VKAPI_ATTR static void *VKAPI_CALL onRealloc##VULKANOBJ(                                     \
        void *pUserData, void *pOriginal, size_t size, size_t alignment,                         \
        VkSystemAllocationScope allocationScope)                                                 \
    {                                                                                            \
        MemoryCallbackInfo callbackInfo;                                                         \
        callbackInfo.callbackType = MemoryAllocationCallbackType::VULKANOBJ;                     \
        callbackInfo.label        = #VULKANOBJ;                                                  \
        return static_cast<MemoryAllocationCallback *>(pUserData)->onReallocImpl(                \
            &callbackInfo, pOriginal, size, alignment, allocationScope);                         \
    }                                                                                            \
    VKAPI_ATTR static void VKAPI_CALL onFree##VULKANOBJ(void *pUserData, void *pMemory)          \
    {                                                                                            \
        MemoryCallbackInfo callbackInfo;                                                         \
        callbackInfo.callbackType = MemoryAllocationCallbackType::VULKANOBJ;                     \
        callbackInfo.label        = #VULKANOBJ;                                                  \
        return static_cast<MemoryAllocationCallback *>(pUserData)->onFreeImpl(&callbackInfo,     \
                                                                              pMemory);          \
    }                                                                                            \
    static VkAllocationCallbacks ConstructCallback##VULKANOBJ(                                   \
        const MemoryAllocationCallback *allocCallback)                                           \
    {                                                                                            \
        VkAllocationCallbacks callback;                                                          \
        callback.pUserData             = (void *)allocCallback;                                  \
        callback.pfnAllocation         = &onAlloc##VULKANOBJ;                                    \
        callback.pfnReallocation       = &onRealloc##VULKANOBJ;                                  \
        callback.pfnFree               = &onFree##VULKANOBJ;                                     \
        callback.pfnInternalAllocation = &onInternalAlloc;                                       \
        callback.pfnInternalFree       = &onInternalFree;                                        \
        return callback;                                                                         \
    }

#define POPULATE_CALLBACK(VULKANOBJ, CALLBACKPTR, CALLBACKMAP)     \
    do                                                             \
    {                                                              \
        if (kMemoryCallbackEnable)                                 \
        {                                                          \
            CALLBACKMAP[MemoryAllocationCallbackType::VULKANOBJ] = \
                ConstructCallback##VULKANOBJ(CALLBACKPTR);         \
        }                                                          \
    } while (0)

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

        for (uint32_t i = 0; i < kMemoryAllocationCallbackTypeCount; i++)
        {
            mPerTypeSystemAllocationSize[i]    = 0;
            mPerTypeSystemAllocationSizeMax[i] = 0;
        }

        POPULATE_CALLBACK(Unknown, this, mCallbackMap);
        POPULATE_CALLBACK(Allocator, this, mCallbackMap);
        POPULATE_CALLBACK(Buffer, this, mCallbackMap);
        POPULATE_CALLBACK(BufferView, this, mCallbackMap);
        POPULATE_CALLBACK(CommandPool, this, mCallbackMap);
        POPULATE_CALLBACK(DebugUtils, this, mCallbackMap);
        POPULATE_CALLBACK(DescriptorPool, this, mCallbackMap);
        POPULATE_CALLBACK(DescriptorSetLayout, this, mCallbackMap);
        POPULATE_CALLBACK(Device, this, mCallbackMap);
        POPULATE_CALLBACK(DeviceMemory, this, mCallbackMap);
        POPULATE_CALLBACK(Event, this, mCallbackMap);
        POPULATE_CALLBACK(Framebuffer, this, mCallbackMap);
        POPULATE_CALLBACK(Image, this, mCallbackMap);
        POPULATE_CALLBACK(ImageView, this, mCallbackMap);
        POPULATE_CALLBACK(Instance, this, mCallbackMap);
        POPULATE_CALLBACK(Pipeline, this, mCallbackMap);
        POPULATE_CALLBACK(PipelineCache, this, mCallbackMap);
        POPULATE_CALLBACK(PipelineLayout, this, mCallbackMap);
        POPULATE_CALLBACK(QueryPool, this, mCallbackMap);
        POPULATE_CALLBACK(RenderPass, this, mCallbackMap);
        POPULATE_CALLBACK(Sampler, this, mCallbackMap);
        POPULATE_CALLBACK(SamplerYcbcrConversion, this, mCallbackMap);
        POPULATE_CALLBACK(Semaphore, this, mCallbackMap);
        POPULATE_CALLBACK(ShaderModule, this, mCallbackMap);
        POPULATE_CALLBACK(Swapchain, this, mCallbackMap);

        // Device memory callback
        mDeviceMemoryCallbacks.pUserData   = (void *)this;
        mDeviceMemoryCallbacks.pfnAllocate = onDeviceAllocate;
        mDeviceMemoryCallbacks.pfnFree     = onDeviceFree;
    }

    vma::DeviceMemoryCallbacks *getDeviceMemoryCallback()
    {
        if (!kMemoryCallbackEnable)
        {
            return nullptr;
        }

        return &mDeviceMemoryCallbacks;
    }

    VkAllocationCallbacks *getAllocationCallback(MemoryAllocationCallbackType callbackType)
    {
        if (!kMemoryCallbackEnable)
        {
            return nullptr;
        }

        return &mCallbackMap[callbackType];
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

    DEFINE_CALLBACK(Unknown)
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

    // VMA Device memory callbacks (Using VmaDeviceAllocationCallbacks)
    static VKAPI_ATTR void VKAPI_PTR onDeviceAllocate(VmaAllocator allocator,
                                                      uint32_t memoryType,
                                                      VkDeviceMemory memory,
                                                      VkDeviceSize size,
                                                      void *pUserData)
    {
        INFO() << "onDeviceAllocate(): " << size << " | " << memory;
    }

    static VKAPI_ATTR void VKAPI_PTR onDeviceFree(VmaAllocator allocator,
                                                  uint32_t memoryType,
                                                  VkDeviceMemory memory,
                                                  VkDeviceSize size,
                                                  void *pUserData)
    {
        INFO() << "onDeviceFree(): " << size << " | " << memory;
    }

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
    vma::DeviceMemoryCallbacks mDeviceMemoryCallbacks;
    using SizeScopePair = std::pair<VkDeviceSize, VkSystemAllocationScope>;
    std::unordered_map<void *, SizeScopePair> mMemoryPropertyMap;

    std::array<VkDeviceSize, kVkSystemAllocationScopeSize> mSystemAllocationSize;
    std::array<VkDeviceSize, kVkSystemAllocationScopeSize> mSystemAllocationSizeMax;

    std::array<VkDeviceSize, kMemoryAllocationCallbackTypeCount> mPerTypeSystemAllocationSize;
    std::array<VkDeviceSize, kMemoryAllocationCallbackTypeCount> mPerTypeSystemAllocationSizeMax;

    std::unordered_map<MemoryAllocationCallbackType, VkAllocationCallbacks> mCallbackMap;
};
}  // namespace vk

// Memory tracker for allocations and deallocations, which is used in vk::Renderer.
class MemoryAllocationTracker : angle::NonCopyable
{
  public:
    MemoryAllocationTracker(vk::Renderer *renderer);
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

    vk::MemoryAllocationCallback *getAllocationCallbacks() { return &mAllocationCallbacks; }
    VkAllocationCallbacks *getAllocationCallback(vk::MemoryAllocationCallbackType callbackType);
    vma::DeviceMemoryCallbacks *getDeviceMemoryCallback();
    VkAllocationCallbacks *getCallbacks();

  private:
    // Pointer to parent renderer object.
    vk::Renderer *const mRenderer;

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
    angle::SimpleMutex mMemoryAllocationMutex;

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
