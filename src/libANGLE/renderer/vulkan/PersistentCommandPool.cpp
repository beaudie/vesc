//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PersistentCommandPool.cpp:
//    Implements the class methods for PersistentCommandPool and PersistentCommandBuffer
//

#include "libANGLE/renderer/vulkan/PersistentCommandPool.h"

namespace rx
{

namespace vk
{

PersistentCommandPool::PersistentCommandPool() : mAllocatedBufferCount(0) {}

PersistentCommandPool::~PersistentCommandPool()
{
    ASSERT(!mCommandPool.valid() && mAllocatedBufferBatches.empty() && mFreeBuffers.empty());
}

VkResult PersistentCommandPool::init(VkDevice device, uint32_t queueFamilyIndex)
{
    ASSERT(!mCommandPool.valid());

    // Initialize the command pool now that we know the queue family index.
    VkCommandPoolCreateInfo commandPoolInfo = {};
    commandPoolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolInfo.queueFamilyIndex        = queueFamilyIndex;

    VkResult res = mCommandPool.init(device, commandPoolInfo);
    if (res != VK_SUCCESS)
    {
        return res;
    }

    allocateCommandBuffers(device, kInitBufferNum);
    return res;
}

void PersistentCommandPool::destroy(VkDevice device)
{
    ASSERT(mCommandPool.valid());
    for (std::vector<VkCommandBuffer> &allocatedBatch : mAllocatedBufferBatches)
    {
        vkFreeCommandBuffers(device, mCommandPool.getHandle(), allocatedBatch.size(),
                             allocatedBatch.data());
        allocatedBatch.clear();
    }
    mAllocatedBufferBatches.clear();
    mAllocatedBufferCount = 0;

    for (vk::PrimaryCommandBuffer &cmdBuf : mFreeBuffers)
    {
        cmdBuf.destroy(device);
    }
    mFreeBuffers.clear();

    mCommandPool.destroy(device);
}

angle::Result PersistentCommandPool::alloc(VkDevice device, vk::PrimaryCommandBuffer *bufferOutput)
{
    if (mFreeBuffers.empty())
    {
        allocateCommandBuffers(device, kIncreaseBufferNum);
        ASSERT(!mFreeBuffers.empty());
    }

    *bufferOutput = std::move(mFreeBuffers.back());
    mFreeBuffers.pop_back();

    return angle::Result::Continue;
}

void PersistentCommandPool::collect(vk::PrimaryCommandBuffer &&buffer)
{
    // VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT NOT set, CommandBuffer
    // still hold the memory resource
    vkResetCommandBuffer(buffer.getHandle(), 0);

    mFreeBuffers.emplace_back(std::move(buffer));
}

void PersistentCommandPool::allocateCommandBuffers(VkDevice device, uint32_t count)
{
    std::vector<VkCommandBuffer> allocatedBatch;
    {
        allocatedBatch.resize(count);

        VkCommandBufferAllocateInfo commandBufferInfo = {};
        commandBufferInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferInfo.commandPool = mCommandPool.getHandle();
        // Used only for primary CommandBuffer allocation
        commandBufferInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferInfo.commandBufferCount = count;

        VkResult res = vkAllocateCommandBuffers(device, &commandBufferInfo, allocatedBatch.data());
        ASSERT(res == VK_SUCCESS);
    }

    for (VkCommandBuffer &cmdBufferHandle : allocatedBatch)
    {
        mFreeBuffers.emplace_back(vk::PrimaryCommandBuffer(cmdBufferHandle));
    }

    mAllocatedBufferBatches.emplace_back(std::move(allocatedBatch));
    mAllocatedBufferCount += allocatedBatch.size();
    // If we allocate too many, it mostly like would be a bug
    ASSERT(mAllocatedBufferCount < 15);
}

}  // namespace vk

}  // namespace rx