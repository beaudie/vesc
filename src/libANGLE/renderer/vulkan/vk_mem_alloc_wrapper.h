//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vma_allocator_wrapper.h:
//    Hides VMA functions so we can use separate warning sets.
//

#ifndef LIBANGLE_RENDERER_VULKAN_VK_MEM_ALLOC_WRAPPER_H_
#define LIBANGLE_RENDERER_VULKAN_VK_MEM_ALLOC_WRAPPER_H_

#include "common/vulkan/vk_headers.h"

VK_DEFINE_HANDLE(VmaAllocator)
VK_DEFINE_HANDLE(VmaAllocation)
VK_DEFINE_HANDLE(VmaPool)

namespace vma
{
enum class VmaPoolCreateFlagBits : uint32_t
{
    VMA_POOL_CREATE_IGNORE_BUFFER_IMAGE_GRANULARITY_BIT = 2,
    VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT                = 4,
    VMA_POOL_CREATE_BUDDY_ALGORITHM_BIT                 = 8,
};

enum class VmaAllocationCreateFlagBits : uint32_t
{
    VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT = 1,
    VMA_ALLOCATION_CREATE_MAPPED_BIT           = 4,
};

VkResult InitAllocator(VkPhysicalDevice physicalDevice,
                       VkDevice device,
                       VkInstance instance,
                       uint32_t apiVersion,
                       VkDeviceSize preferredLargeHeapBlockSize,
                       VmaAllocator *pAllocator);

void DestroyAllocator(VmaAllocator allocator);

VkResult CreatePool(VmaAllocator allocator,
                    uint32_t memoryTypeIndex,
                    VmaPoolCreateFlagBits flags,
                    VkDeviceSize blockSize,
                    VmaPool *pPool);
void DestroyPool(VmaAllocator allocator, VmaPool pool);

VkResult AllocateMemory(VmaAllocator allocator,
                        const VkMemoryRequirements *pVkMemoryRequirements,
                        VkMemoryPropertyFlags requiredFlags,
                        VkMemoryPropertyFlags preferredFlags,
                        VmaAllocationCreateFlagBits flags,
                        VmaPool customPool,
                        uint32_t *pMemoryTypeIndexOut,
                        VmaAllocation *pAllocation,
                        VkDeviceSize *sizeOut);
void FreeMemory(VmaAllocator allocator, VmaAllocation allocation);

VkResult BindBufferMemory(VmaAllocator allocator, VmaAllocation allocation, VkBuffer buffer);

VkResult CreateBuffer(VmaAllocator allocator,
                      const VkBufferCreateInfo *pBufferCreateInfo,
                      VkMemoryPropertyFlags requiredFlags,
                      VkMemoryPropertyFlags preferredFlags,
                      bool persistentlyMappedBuffers,
                      uint32_t *pMemoryTypeIndexOut,
                      VkBuffer *pBuffer,
                      VmaAllocation *pAllocation);

VkResult FindMemoryTypeIndexForBufferInfo(VmaAllocator allocator,
                                          const VkBufferCreateInfo *pBufferCreateInfo,
                                          VkMemoryPropertyFlags requiredFlags,
                                          VkMemoryPropertyFlags preferredFlags,
                                          bool persistentlyMappedBuffers,
                                          uint32_t *pMemoryTypeIndexOut);

VkResult FindMemoryTypeIndex(VmaAllocator allocator,
                             uint32_t memoryTypeBits,
                             VkMemoryPropertyFlags requiredFlags,
                             VkMemoryPropertyFlags preferredFlags,
                             bool persistentlyMappedBuffers,
                             uint32_t *pMemoryTypeIndexOut);

void GetMemoryTypeProperties(VmaAllocator allocator,
                             uint32_t memoryTypeIndex,
                             VkMemoryPropertyFlags *pFlags);

VkResult MapMemory(VmaAllocator allocator, VmaAllocation allocation, void **ppData);

void UnmapMemory(VmaAllocator allocator, VmaAllocation allocation);

void FlushAllocation(VmaAllocator allocator,
                     VmaAllocation allocation,
                     VkDeviceSize offset,
                     VkDeviceSize size);

void InvalidateAllocation(VmaAllocator allocator,
                          VmaAllocation allocation,
                          VkDeviceSize offset,
                          VkDeviceSize size);

void BuildStatsString(VmaAllocator allocator, char **statsString, VkBool32 detailedMap);
void FreeStatsString(VmaAllocator allocator, char *statsString);

}  // namespace vma

#endif  // LIBANGLE_RENDERER_VULKAN_VK_MEM_ALLOC_WRAPPER_H_
