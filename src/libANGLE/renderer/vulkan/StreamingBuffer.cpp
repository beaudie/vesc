//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// StreamingBuffer:
//    Create, map and flush buffers as needed to hold data, returning a handle and offset for each
//    chunk.
//

#include "StreamingBuffer.h"

#include "anglebase/numerics/safe_math.h"

#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"

namespace rx
{
StreamingBuffer::StreamingBuffer(VkBufferUsageFlags usage, size_t minSize)
    : mUsage(usage),
      mMinSize(minSize),
      mNextWriteOffset(0),
      mLastFlushOffset(0),
      mSize(0),
      mMappedMemory(nullptr)
{
}

StreamingBuffer::~StreamingBuffer()
{
}

gl::Error StreamingBuffer::allocate(ContextVk *context,
                                    VkMemoryPropertyFlags memoryPropertyFlags,
                                    size_t sizeInBytes,
                                    uint8_t **ptrOut,
                                    VkBuffer *handleOut,
                                    VkDeviceSize *offsetOut)
{
    // Users should not pass in HOST_COHERENT_BIT for performance purposes.
    ASSERT((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0);

    RendererVk *renderer = context->getRenderer();

    // TODO(fjhenigman): Update this when we have buffers that need to
    // persist longer than one frame.
    updateQueueSerial(renderer->getCurrentQueueSerial());

    angle::base::CheckedNumeric<size_t> checkedNextWriteOffset = mNextWriteOffset;
    checkedNextWriteOffset += sizeInBytes;

    if (!checkedNextWriteOffset.IsValid() || checkedNextWriteOffset.ValueOrDie() > mSize)
    {
        VkDevice device = context->getDevice();

        if (mMappedMemory)
        {
            mMemory.unmap(device);
            mMappedMemory = nullptr;
        }
        renderer->releaseResource(*this, &mBuffer);
        renderer->releaseResource(*this, &mMemory);

        VkBufferCreateInfo createInfo;
        createInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext                 = nullptr;
        createInfo.flags                 = 0;
        createInfo.size                  = std::max(sizeInBytes, mMinSize);
        createInfo.usage                 = mUsage;
        createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;
        ANGLE_TRY(mBuffer.init(device, createInfo));

        // Remove the host coherent bit if it was passed, we know we can deal without it with this
        // streaming buffer implementation.
        ANGLE_TRY(
            vk::AllocateBufferMemory(renderer, memoryPropertyFlags, &mBuffer, &mMemory, &mSize));
        ANGLE_TRY(mMemory.map(device, 0, mSize, 0, &mMappedMemory));
        mNextWriteOffset = 0;
        mLastFlushOffset = 0;
    }

    ASSERT(mBuffer.valid());
    *handleOut = mBuffer.getHandle();

    ASSERT(mMappedMemory);
    *ptrOut    = mMappedMemory + mNextWriteOffset;
    *offsetOut = mNextWriteOffset;
    mNextWriteOffset += sizeInBytes;

    return gl::NoError();
}

gl::Error StreamingBuffer::flush(ContextVk *context)
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

void StreamingBuffer::destroy(VkDevice device)
{
    mBuffer.destroy(device);
    mMemory.destroy(device);
}

}  // namespace rx
