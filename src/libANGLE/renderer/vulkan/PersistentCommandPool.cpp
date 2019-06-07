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
    ASSERT(!mCommandPool.valid() && mFreeBuffers.empty());
}

angle::Result PersistentCommandPool::init(vk::Context *context, uint32_t queueFamilyIndex)
{
    ASSERT(!mCommandPool.valid());

    // Initialize the command pool now that we know the queue family index.
    VkCommandPoolCreateInfo commandPoolInfo = {};
    commandPoolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolInfo.queueFamilyIndex        = queueFamilyIndex;

    ANGLE_VK_TRY(context, mCommandPool.init(context->getDevice(), commandPoolInfo));

    ANGLE_TRY(allocateCommandBuffers(context, kInitBufferNum));
    return angle::Result::Continue;
}

void PersistentCommandPool::destroy(const vk::Context *context)
{
    ASSERT(mCommandPool.valid());
    ASSERT(mAllocatedBufferCount == mFreeBuffers.size());
    mAllocatedBufferCount = 0;

    VkDevice device = context->getDevice();
    for (vk::PrimaryCommandBuffer &cmdBuf : mFreeBuffers)
    {
        VkCommandBuffer cmdBufHandles[] = {cmdBuf.getHandle()};
        vkFreeCommandBuffers(device, mCommandPool.getHandle(), ArraySize(cmdBufHandles),
                             cmdBufHandles);
        cmdBuf.destroy(device);
    }
    mFreeBuffers.clear();

    mCommandPool.destroy(device);
}

angle::Result PersistentCommandPool::alloc(vk::Context *context,
                                           vk::PrimaryCommandBuffer *bufferOutput)
{
    if (mFreeBuffers.empty())
    {
        ANGLE_TRY(allocateCommandBuffers(context, kIncreaseBufferNum));
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
    VkResult res = vkResetCommandBuffer(buffer.getHandle(), 0);
    ASSERT(res == VK_SUCCESS);

    mFreeBuffers.emplace_back(std::move(buffer));
}

angle::Result PersistentCommandPool::allocateCommandBuffers(vk::Context *context, uint32_t count)
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

        ANGLE_VK_TRY(context, vkAllocateCommandBuffers(context->getDevice(), &commandBufferInfo,
                                                       allocatedBatch.data()));
    }

    for (VkCommandBuffer &cmdBufferHandle : allocatedBatch)
    {
        mFreeBuffers.emplace_back(vk::PrimaryCommandBuffer(std::move(cmdBufferHandle)));
    }

    mAllocatedBufferCount += count;
    // If we allocate too many, it mostly like would be a bug
    ASSERT(mAllocatedBufferCount < 15);

    return angle::Result::Continue;
}

}  // namespace vk

}  // namespace rx