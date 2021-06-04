//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vma_allocator_wrapper.cpp:
//    Hides VMA functions so we can use separate warning sets.
//

#include "vk_mem_alloc_wrapper.h"

#include <vk_mem_alloc.h>

#include <algorithm>

namespace vma
{
VmaPool DeviceMemorySmallPools[16] = {};
VmaPool DeviceMemoryLargePools[16] = {};
std::atomic<VkDeviceSize> totalDeviceMemorySize;
std::atomic<VkDeviceSize> peakDeviceMemorySize;

/// Callback function called after successful vkAllocateMemory.
void VKAPI_PTR allocateDeviceMemoryCallback(VmaAllocator VMA_NOT_NULL allocator,
                                            uint32_t memoryType,
                                            VkDeviceMemory VMA_NOT_NULL_NON_DISPATCHABLE memory,
                                            VkDeviceSize size,
                                            void *VMA_NULLABLE pUserData)
{
    totalDeviceMemorySize += size;
    // This is race prone, but don't want to eat extra cost of lock just because of this
    peakDeviceMemorySize.store(std::max(peakDeviceMemorySize, totalDeviceMemorySize),
                               std::memory_order_relaxed);
}
/// Callback function called before vkFreeMemory.
void VKAPI_PTR freeDeviceMemoryCallback(VmaAllocator VMA_NOT_NULL allocator,
                                        uint32_t memoryType,
                                        VkDeviceMemory VMA_NOT_NULL_NON_DISPATCHABLE memory,
                                        VkDeviceSize size,
                                        void *VMA_NULLABLE pUserData)
{
    totalDeviceMemorySize -= size;
}

VkResult InitAllocator(VkPhysicalDevice physicalDevice,
                       VkDevice device,
                       VkInstance instance,
                       uint32_t apiVersion,
                       VkDeviceSize preferredLargeHeapBlockSize,
                       VmaAllocator *pAllocator)
{
    VmaVulkanFunctions funcs                  = {};
    funcs.vkGetPhysicalDeviceProperties       = vkGetPhysicalDeviceProperties;
    funcs.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    funcs.vkAllocateMemory                    = vkAllocateMemory;
    funcs.vkFreeMemory                        = vkFreeMemory;
    funcs.vkMapMemory                         = vkMapMemory;
    funcs.vkUnmapMemory                       = vkUnmapMemory;
    funcs.vkFlushMappedMemoryRanges           = vkFlushMappedMemoryRanges;
    funcs.vkInvalidateMappedMemoryRanges      = vkInvalidateMappedMemoryRanges;
    funcs.vkBindBufferMemory                  = vkBindBufferMemory;
    funcs.vkBindImageMemory                   = vkBindImageMemory;
    funcs.vkGetBufferMemoryRequirements       = vkGetBufferMemoryRequirements;
    funcs.vkGetImageMemoryRequirements        = vkGetImageMemoryRequirements;
    funcs.vkCreateBuffer                      = vkCreateBuffer;
    funcs.vkDestroyBuffer                     = vkDestroyBuffer;
    funcs.vkCreateImage                       = vkCreateImage;
    funcs.vkDestroyImage                      = vkDestroyImage;
    funcs.vkCmdCopyBuffer                     = vkCmdCopyBuffer;
    {
#if !defined(ANGLE_SHARED_LIBVULKAN)
        // When the vulkan-loader is statically linked, we need to use the extension
        // functions defined in ANGLE's rx namespace. When it's dynamically linked
        // with volk, this will default to the function definitions with no namespace
        using rx::vkBindBufferMemory2KHR;
        using rx::vkBindImageMemory2KHR;
        using rx::vkGetBufferMemoryRequirements2KHR;
        using rx::vkGetImageMemoryRequirements2KHR;
        using rx::vkGetPhysicalDeviceMemoryProperties2KHR;
#endif  // !defined(ANGLE_SHARED_LIBVULKAN)
        funcs.vkGetBufferMemoryRequirements2KHR       = vkGetBufferMemoryRequirements2KHR;
        funcs.vkGetImageMemoryRequirements2KHR        = vkGetImageMemoryRequirements2KHR;
        funcs.vkBindBufferMemory2KHR                  = vkBindBufferMemory2KHR;
        funcs.vkBindImageMemory2KHR                   = vkBindImageMemory2KHR;
        funcs.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR;
    }

    VmaAllocatorCreateInfo allocatorInfo      = {};
    allocatorInfo.physicalDevice              = physicalDevice;
    allocatorInfo.device                      = device;
    allocatorInfo.instance                    = instance;
    allocatorInfo.pVulkanFunctions            = &funcs;
    allocatorInfo.vulkanApiVersion            = apiVersion;
    allocatorInfo.preferredLargeHeapBlockSize = preferredLargeHeapBlockSize;

    VmaDeviceMemoryCallbacks deviceMemoryCallbacks = {};
    deviceMemoryCallbacks.pfnAllocate              = allocateDeviceMemoryCallback;
    deviceMemoryCallbacks.pfnFree                  = freeDeviceMemoryCallback;
    allocatorInfo.pDeviceMemoryCallbacks           = &deviceMemoryCallbacks;

    totalDeviceMemorySize = 0;
    peakDeviceMemorySize  = 0;

    return vmaCreateAllocator(&allocatorInfo, pAllocator);
}

void DestroyAllocator(VmaAllocator allocator)
{
    for (int i = 0; i < 16; i++)
    {
        if (DeviceMemorySmallPools[i])
        {
            vmaDestroyPool(allocator, DeviceMemorySmallPools[i]);
            DeviceMemorySmallPools[i] = nullptr;
        }
    }
    for (int i = 0; i < 16; i++)
    {
        if (DeviceMemoryLargePools[i])
        {
            vmaDestroyPool(allocator, DeviceMemoryLargePools[i]);
            DeviceMemoryLargePools[i] = nullptr;
        }
    }
    vmaDestroyAllocator(allocator);
}

void FreeMemory(VmaAllocator allocator, VmaAllocation allocation)
{
    vmaFreeMemory(allocator, allocation);
}

VkResult CreateBuffer(VmaAllocator allocator,
                      const VkBufferCreateInfo *pBufferCreateInfo,
                      VkMemoryPropertyFlags requiredFlags,
                      VkMemoryPropertyFlags preferredFlags,
                      bool persistentlyMappedBuffers,
                      uint32_t *pMemoryTypeIndexOut,
                      VkBuffer *pBuffer,
                      VmaAllocation *pAllocation)
{
    VkResult result;
    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.requiredFlags           = requiredFlags;
    allocationCreateInfo.preferredFlags          = preferredFlags;
    allocationCreateInfo.flags = (persistentlyMappedBuffers) ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0;
    VmaAllocationInfo allocationInfo = {};

    uint32_t memoryTypeIndex = *pMemoryTypeIndexOut;
    if (pBufferCreateInfo->size <= 4096)
    {
        if (!DeviceMemorySmallPools[memoryTypeIndex])
        {
            // Create a custom pool.
            VmaPoolCreateInfo poolCreateInfo = {};
            poolCreateInfo.memoryTypeIndex   = memoryTypeIndex;
            poolCreateInfo.flags             = VMA_POOL_CREATE_IGNORE_BUFFER_IMAGE_GRANULARITY_BIT |
                                   VMA_POOL_CREATE_BUDDY_ALGORITHM_BIT;
            poolCreateInfo.blockSize     = 128ull * 1024;  // 128K
            poolCreateInfo.maxBlockCount = -1;

            VmaPool pool;
            result = vmaCreatePool(allocator, &poolCreateInfo, &pool);
            if (VK_SUCCESS != result)
            {
                return result;
            }
            DeviceMemorySmallPools[*pMemoryTypeIndexOut] = pool;
        }
        allocationCreateInfo.pool = DeviceMemorySmallPools[*pMemoryTypeIndexOut];
    }
    else if (pBufferCreateInfo->size <= 1ull * 1024 * 1024)
    {
        if (!DeviceMemoryLargePools[memoryTypeIndex])
        {
            // Create a custom pool.
            VmaPoolCreateInfo poolCreateInfo = {};
            poolCreateInfo.memoryTypeIndex   = memoryTypeIndex;
            poolCreateInfo.flags             = VMA_POOL_CREATE_IGNORE_BUFFER_IMAGE_GRANULARITY_BIT;
            poolCreateInfo.blockSize         = 4ull * 1024 * 1024;  // 4M
            poolCreateInfo.maxBlockCount     = -1;

            VmaPool pool;
            result = vmaCreatePool(allocator, &poolCreateInfo, &pool);
            if (VK_SUCCESS != result)
            {
                return result;
            }
            DeviceMemoryLargePools[*pMemoryTypeIndexOut] = pool;
        }
        allocationCreateInfo.pool = DeviceMemoryLargePools[*pMemoryTypeIndexOut];
    }
    else
    {}

    result = vmaCreateBuffer(allocator, pBufferCreateInfo, &allocationCreateInfo, pBuffer,
                             pAllocation, &allocationInfo);
    *pMemoryTypeIndexOut = allocationInfo.memoryType;

    return result;
}

VkResult FindMemoryTypeIndexForBufferInfo(VmaAllocator allocator,
                                          const VkBufferCreateInfo *pBufferCreateInfo,
                                          VkMemoryPropertyFlags requiredFlags,
                                          VkMemoryPropertyFlags preferredFlags,
                                          bool persistentlyMappedBuffers,
                                          uint32_t *pMemoryTypeIndexOut)
{
    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.requiredFlags           = requiredFlags;
    allocationCreateInfo.preferredFlags          = preferredFlags;
    allocationCreateInfo.flags = (persistentlyMappedBuffers) ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0;

    return vmaFindMemoryTypeIndexForBufferInfo(allocator, pBufferCreateInfo, &allocationCreateInfo,
                                               pMemoryTypeIndexOut);
}

void GetMemoryTypeProperties(VmaAllocator allocator,
                             uint32_t memoryTypeIndex,
                             VkMemoryPropertyFlags *pFlags)
{
    vmaGetMemoryTypeProperties(allocator, memoryTypeIndex, pFlags);
}

VkResult MapMemory(VmaAllocator allocator, VmaAllocation allocation, void **ppData)
{
    return vmaMapMemory(allocator, allocation, ppData);
}

void UnmapMemory(VmaAllocator allocator, VmaAllocation allocation)
{
    return vmaUnmapMemory(allocator, allocation);
}

void FlushAllocation(VmaAllocator allocator,
                     VmaAllocation allocation,
                     VkDeviceSize offset,
                     VkDeviceSize size)
{
    vmaFlushAllocation(allocator, allocation, offset, size);
}

void InvalidateAllocation(VmaAllocator allocator,
                          VmaAllocation allocation,
                          VkDeviceSize offset,
                          VkDeviceSize size)
{
    vmaInvalidateAllocation(allocator, allocation, offset, size);
}

void BuildStatsString(VmaAllocator allocator, char **statsString, VkBool32 detailedMap)
{
    vmaBuildStatsString(allocator, statsString, detailedMap);
}

void FreeStatsString(VmaAllocator allocator, char *statsString)
{
    vmaFreeStatsString(allocator, statsString);
}
}  // namespace vma
