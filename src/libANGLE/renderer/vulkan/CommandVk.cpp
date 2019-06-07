//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CommandVk.cpp:
//    Implements the class methods for PersistantCommandPool and PersistantCommandBuffer
//

#include "libANGLE/renderer/vulkan/CommandVk.h"

namespace rx
{

namespace vk
{

PersistantCommandBuffer::~PersistantCommandBuffer()
{
    ASSERT(!allocated());
}

bool PersistantCommandBuffer::allocated() const
{
    return priv::CommandBuffer::valid() && mPoolAllocateId != kNullPollAllocatedId &&
           mOwnerPool != nullptr;
}

void PersistantCommandBuffer::destroy(VkDevice device)
{
    if (mInflight)
    {
        ASSERT(mOwnerPool);
        mOwnerPool->collect(std::move(*this));
    }
    priv::CommandBuffer::destroy(device);
}

PersistantCommandPool::PersistantCommandPool()
    : mAllocatedBufferCount(0),
      // The allocate number could be further adjusted to have better performance
      kInitBufferNum(3),
      kIncreaseBufferNum(1)
{}

PersistantCommandPool::~PersistantCommandPool()
{
    ASSERT(mHandle == VK_NULL_HANDLE && mFreeBuffers.empty());
}

VkResult PersistantCommandPool::init(VkDevice device, uint32_t queueFamilyIndex)
{
    ASSERT(!valid());

    // Initialize the command pool now that we know the queue family index.
    VkCommandPoolCreateInfo commandPoolInfo = {};
    commandPoolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolInfo.queueFamilyIndex        = queueFamilyIndex;
    VkResult res = vkCreateCommandPool(device, &commandPoolInfo, nullptr, &mHandle);
    if (res != VK_SUCCESS)
    {
        return res;
    }

    allocateCommandBufferArray(device, kInitBufferNum);
    return res;
}

void PersistantCommandPool::destroy(VkDevice device)
{
    if (valid())
    {
        for (CommandBufferArrayHandle &bufferArrayHandle : mAllocatedBufferArrays)
        {
            vkFreeCommandBuffers(device, this->getHandle(), bufferArrayHandle.size,
                                 bufferArrayHandle.array);
            delete[] bufferArrayHandle.array;
            bufferArrayHandle.size  = 0;
            bufferArrayHandle.array = nullptr;
        }
        mAllocatedBufferArrays.clear();
        mAllocatedBufferCount = 0;

        vkDestroyCommandPool(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
    for (PersistantCommandBuffer &cmdBuf : mFreeBuffers)
    {
        cmdBuf.destroy(device);
    }
    mFreeBuffers.clear();
}

angle::Result PersistantCommandPool::alloc(VkDevice device, PersistantCommandBuffer *bufferOutput)
{
    if (mFreeBuffers.empty())
    {
        allocateBatch(device);
        ASSERT(!mFreeBuffers.empty());
    }

    *bufferOutput = std::move(mFreeBuffers.back());
    mFreeBuffers.pop_back();

    bufferOutput->mInflight = true;
    return angle::Result::Continue;
}

void PersistantCommandPool::collect(PersistantCommandBuffer &&buffer)
{
    ASSERT(buffer.mOwnerPool == this);
    ASSERT(buffer.mInflight == true);
    ASSERT(buffer.mPoolAllocateId != PersistantCommandBuffer::kNullPollAllocatedId);
    ASSERT(buffer.mPoolAllocateId < (int32_t)mAllocatedBufferCount);

    buffer.mInflight = false;
    // VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT NOT set, CommandBuffer
    // still hold the memory resource
    vkResetCommandBuffer(buffer.mHandle, 0);

    mFreeBuffers.emplace_back(std::move(buffer));
}

void PersistantCommandPool::allocateCommandBufferArray(VkDevice device, uint32_t count)
{
    VkCommandBuffer *pCmdBuffers = new VkCommandBuffer[count];
    {
        VkCommandBufferAllocateInfo commandBufferInfo = {};
        commandBufferInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferInfo.commandPool = this->getHandle();
        // Used only for primary CommandBuffer allocation
        commandBufferInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferInfo.commandBufferCount = count;
        ASSERT(vkAllocateCommandBuffers(device, &commandBufferInfo, pCmdBuffers) == VK_SUCCESS);

        mAllocatedBufferArrays.emplace_back(CommandBufferArrayHandle{
            .array = pCmdBuffers,
            .size  = count,
        });
    }

    for (uint32_t i = 0; i < count; i++)
    {
        uint32_t id = mAllocatedBufferCount + i;
        mFreeBuffers.emplace_back(PersistantCommandBuffer(this, id, pCmdBuffers[i]));
    }

    mAllocatedBufferCount += count;
    // If we allocate too many, it mostly like would be a bug
    ASSERT(mAllocatedBufferCount < 100);
}

}  // namespace vk

}  // namespace rx