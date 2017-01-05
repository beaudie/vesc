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

StreamingBufferManager::StreamingBufferManager(VkBufferUsageFlags usage, size_t minSize)
    : mUsage(usage), mMinSize(minSize)
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

    if (mNextWriteOffset + amount > mSize)
    {
        if (mMapping)
        {
            mMemory.unmap(device);
            mMapping = nullptr;
        }
        renderer->releaseResource(*this, &mBuffer);
        renderer->releaseResource(*this, &mMemory);

        VkBufferCreateInfo createInfo;
        createInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext                 = nullptr;
        createInfo.flags                 = 0;
        createInfo.size                  = std::max(amount, mMinSize);
        createInfo.usage                 = mUsage;
        createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;
        ANGLE_TRY(mBuffer.init(device, createInfo));
        ANGLE_TRY(vk::AllocateBufferMemory(context, createInfo.size, &mBuffer,
                                           0 /* memory property flags */, &mMemory, &mSize,
                                           &mAlignment));
        ANGLE_TRY(mMemory.map(device, 0, mSize, 0, &mMapping));
        mNextWriteOffset = 0;
        mLastFlushOffset = 0;
    }

    ASSERT(mBuffer.valid());
    *handleOut = mBuffer.getHandle();

    ASSERT(mMapping);
    *ptrOut          = mMapping + mNextWriteOffset;
    *offsetOut       = mNextWriteOffset;
    mNextWriteOffset = roundUp(mNextWriteOffset + amount, mAlignment);

    return gl::NoError();
}

gl::Error StreamingBufferManager::flush(ContextVk *context)
{
    if (mNextWriteOffset > mLastFlushOffset)
    {
        VkMappedMemoryRange range;
        range.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.pNext  = nullptr;
        range.memory = mMemory.getHandle();
        range.offset = mLastFlushOffset;
        range.size   = mNextWriteOffset - mLastFlushOffset;
        ANGLE_VK_TRY(vkFlushMappedMemoryRanges(context->getDevice(), 1, &range));

        mLastFlushOffset = mNextWriteOffset;
    }
    return gl::NoError();
}

}  // namespace rx
