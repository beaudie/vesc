//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// StreamingBufferManager:
//    Create, map and flush buffers as needed to hold data, returning a handle and offset for each
//    chunk.
//

#include "StreamingBufferManager.h"

#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"

namespace rx
{

StreamingBufferManager::StreamingBufferManager(VkBufferUsageFlags usage) : mUsage(usage)
{
}

StreamingBufferManager::~StreamingBufferManager()
{
}

gl::Error StreamingBufferManager::allocate(ContextVk *context,
                                           size_t amount,
                                           uint8_t **ptrOut,
                                           VkBuffer *handleOut,
                                           VkDeviceSize *offsetOut)
{
    RendererVk *renderer = context->getRenderer();
    updateQueueSerial(renderer->getCurrentQueueSerial());
    VkDevice device = context->getDevice();

    if (mOffset + amount > mSize)
    {
        if (mPtr)
        {
            mMemory.unmap(device);
            mPtr = nullptr;
        }
        renderer->releaseResource(*this, &mBuffer);
        renderer->releaseResource(*this, &mMemory);

        VkBufferCreateInfo createInfo;
        createInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext                 = nullptr;
        createInfo.flags                 = 0;
        createInfo.size                  = std::max(amount, static_cast<typeof(amount)>(kMinSize));
        createInfo.usage                 = mUsage;
        createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;
        ANGLE_TRY(mBuffer.init(device, createInfo));
        ANGLE_TRY(vk::AllocateBufferMemory(context, createInfo.size, &mBuffer, &mMemory, &mSize,
                                           (VkMemoryPropertyFlagBits)0));
        ANGLE_TRY(mMemory.map(device, 0, mSize, 0, &mPtr));
        mOffset    = 0;
        mLastFlush = 0;
    }

    ASSERT(mBuffer.valid());
    *handleOut = mBuffer.getHandle();

    ASSERT(mPtr);
    *ptrOut    = mPtr + mOffset;
    *offsetOut = mOffset;
    mOffset += amount;
    ASSERT(mOffset <= mSize);

    return gl::NoError();
}

gl::Error StreamingBufferManager::flush(ContextVk *context)
{
    if (mOffset > mLastFlush)
    {
        VkMappedMemoryRange range;
        range.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.pNext  = nullptr;
        range.memory = mMemory.getHandle();
        range.offset = mLastFlush;
        range.size   = mOffset - mLastFlush;
        ANGLE_VK_TRY(vkFlushMappedMemoryRanges(context->getDevice(), 1, &range));

        mLastFlush = mOffset;
    }
    return gl::NoError();
}

}  // namespace rx
