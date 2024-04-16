//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLMemoryVk.cpp: Implements the class methods for CLMemoryVk.

#include "libANGLE/renderer/vulkan/CLMemoryVk.h"
#include "libANGLE/renderer/vulkan/CLContextVk.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

#include "libANGLE/CLBuffer.h"
#include "libANGLE/CLContext.h"
#include "libANGLE/CLMemory.h"
#include "libANGLE/cl_utils.h"

namespace rx
{

CLMemoryVk::CLMemoryVk(const cl::Memory &memory)
    : CLMemoryImpl(memory),
      mContext(&memory.getContext().getImpl<CLContextVk>()),
      mRenderer(mContext->getRenderer()),
      mMappedMemory(nullptr),
      mParent(nullptr)
{}

CLMemoryVk::~CLMemoryVk()
{
    mContext->mAssociatedObjects->mMemories.erase(mMemory.getNative());
}

angle::Result CLMemoryVk::createSubBuffer(const cl::Buffer &buffer,
                                          cl::MemFlags flags,
                                          size_t size,
                                          CLMemoryImpl::Ptr *subBufferOut)
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

VkBufferUsageFlags CLMemoryVk::getVkUsageFlags()
{
    VkBufferUsageFlags usageFlags =
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    if (mMemory.getFlags().isSet(CL_MEM_WRITE_ONLY))
    {
        usageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    else if (mMemory.getFlags().isSet(CL_MEM_READ_ONLY))
    {
        usageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    else
    {
        usageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }

    return usageFlags;
}

VkMemoryPropertyFlags CLMemoryVk::getVkMemPropertyFlags()
{
    // TODO: http://anglebug.com/8588
    VkMemoryPropertyFlags propFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

    if (mMemory.getFlags().isSet(CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR |
                                 CL_MEM_COPY_HOST_PTR))
    {
        propFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }

    return propFlags;
}

angle::Result CLMemoryVk::copyTo(void *dst, size_t srcOffset, size_t size)
{
    uint8_t *src = nullptr;
    ANGLE_TRY(map(src, srcOffset));
    std::memcpy(dst, src, size);
    unmap();
    return angle::Result::Continue;
}

angle::Result CLMemoryVk::copyTo(CLMemoryVk *dst, size_t srcOffset, size_t dstOffset, size_t size)
{
    uint8_t *dstPtr = nullptr;
    ANGLE_TRY(dst->map(dstPtr, dstOffset));
    ANGLE_TRY(copyTo(dstPtr, srcOffset, size));
    dst->unmap();
    return angle::Result::Continue;
}

angle::Result CLMemoryVk::copyFrom(const void *src, size_t srcOffset, size_t size)
{
    uint8_t *dst = nullptr;
    ANGLE_TRY(map(dst, srcOffset));
    std::memcpy(dst, src, size);
    unmap();
    return angle::Result::Continue;
}

CLBufferVk::CLBufferVk(const cl::Buffer &buffer) : CLMemoryVk(buffer)
{
    if (buffer.isSubBuffer())
    {
        mParent = &buffer.getParent()->getImpl<CLBufferVk>();
    }
    mDefaultBufferCreateInfo             = {};
    mDefaultBufferCreateInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    mDefaultBufferCreateInfo.size        = buffer.getSize();
    mDefaultBufferCreateInfo.usage       = getVkUsageFlags();
    mDefaultBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
}

CLBufferVk::~CLBufferVk()
{
    if (isMapped())
    {
        unmap();
    }
    mBuffer.destroy(mRenderer);
}

angle::Result CLBufferVk::createSubBuffer(const cl::Buffer &buffer,
                                          cl::MemFlags flags,
                                          size_t size,
                                          CLMemoryImpl::Ptr *subBufferOut)
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

angle::Result CLBufferVk::create(void *hostPtr)
{
    if (!isSubBuffer())
    {
        VkBufferCreateInfo createInfo  = mDefaultBufferCreateInfo;
        createInfo.size                = getSize();
        VkMemoryPropertyFlags memFlags = getVkMemPropertyFlags();
        if (IsError(mBuffer.init(mContext, createInfo, memFlags)))
        {
            ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
        }
        if (mMemory.getFlags().isSet(CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR))
        {
            ASSERT(hostPtr);
            ANGLE_CL_IMPL_TRY_ERROR(setDataImpl(static_cast<uint8_t *>(hostPtr), getSize(), 0),
                                    CL_OUT_OF_RESOURCES);
        }
    }
    return angle::Result::Continue;
}

angle::Result CLBufferVk::createStagingBuffer(size_t size)
{
    VkBufferCreateInfo createInfo = mDefaultBufferCreateInfo;
    createInfo.size               = size;

    if (IsError(mStagingBuffer.init(mContext, createInfo, getVkMemPropertyFlags())))
    {
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
    }

    return angle::Result::Continue;
}

angle::Result CLBufferVk::copyStagingTo(void *ptr, size_t offset, size_t size)
{
    uint8_t *ptrOut;
    ANGLE_TRY(getStagingBuffer().map(mContext, &ptrOut));
    std::memcpy(ptr, ptrOut + offset, size);
    getStagingBuffer().unmap(mContext->getRenderer());
    return angle::Result::Continue;
}

angle::Result CLBufferVk::mapImpl()
{
    ASSERT(!isMapped());

    if (isSubBuffer())
    {
        ANGLE_TRY(mParent->map(mMappedMemory, static_cast<size_t>(mBuffer.getOffset())));
        return angle::Result::Continue;
    }

    ANGLE_TRY(mBuffer.map(mContext, &mMappedMemory));
    return angle::Result::Continue;
}

void CLBufferVk::unmapImpl()
{
    if (isSubBuffer())
    {
        mMappedMemory = nullptr;
    }
    mBuffer.unmap(mRenderer);
    mMappedMemory = nullptr;
}

angle::Result CLMemoryVk::map(uint8_t *&ptrOut, size_t offset)
{
    if (!isMapped())
    {
        ANGLE_TRY(mapImpl());
    }
    ptrOut = mMappedMemory + offset;
    return angle::Result::Continue;
}

angle::Result CLBufferVk::setRect(const void *data,
                                  const cl::BufferRect &srcRect,
                                  const cl::BufferRect &rect)
{
    ASSERT(srcRect.valid() && rect.valid());
    ASSERT(srcRect.mSize == rect.mSize);

    uint8_t *mapPtr = nullptr;
    ANGLE_TRY(map(mapPtr));
    const uint8_t *srcData = reinterpret_cast<const uint8_t *>(data);
    for (int slice = 0; slice < rect.mSize.depth; slice++)
    {
        for (int row = 0; row < rect.mSize.height; row++)
        {
            const uint8_t *src = srcData + srcRect.getRowOffset(slice, row);
            uint8_t *dst       = mapPtr + rect.getRowOffset(slice, row);

            memcpy(dst, src, srcRect.mSize.width * srcRect.mElementSize);
        }
    }

    return angle::Result::Continue;
}

angle::Result CLBufferVk::getRect(const cl::BufferRect &srcRect,
                                  const cl::BufferRect &outRect,
                                  void *outData)
{
    ASSERT(srcRect.valid() && outRect.valid());
    ASSERT(srcRect.mSize == outRect.mSize);

    uint8_t *mapPtr = nullptr;
    ANGLE_TRY(map(mapPtr));
    uint8_t *dstData = reinterpret_cast<uint8_t *>(outData);
    for (int slice = 0; slice < srcRect.mSize.depth; slice++)
    {
        for (int row = 0; row < srcRect.mSize.height; row++)
        {
            const uint8_t *src = mapPtr + srcRect.getRowOffset(slice, row);
            uint8_t *dst       = dstData + outRect.getRowOffset(slice, row);

            memcpy(dst, src, srcRect.mSize.width * srcRect.mElementSize);
        }
    }

    return angle::Result::Continue;
}

// offset is for mapped pointer
angle::Result CLBufferVk::setDataImpl(const uint8_t *data, size_t size, size_t offset)
{
    // buffer cannot be in use state
    ASSERT(mBuffer.valid());
    ASSERT(!isCurrentlyInUse());
    ASSERT(size + offset <= getSize());
    ASSERT(data != nullptr);

    // Assuming host visible buffers for now
    // TODO: http://anglebug.com/8589
    if (!mBuffer.isHostVisible())
    {
        UNIMPLEMENTED();
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
    }

    uint8_t *mapPointer = nullptr;
    ANGLE_TRY(map(mapPointer));

    std::memcpy(mapPointer, data, size);
    mBuffer.unmap(mRenderer);

    return angle::Result::Continue;
}

bool CLBufferVk::isCurrentlyInUse() const
{
    return !mRenderer->hasResourceUseFinished(mBuffer.getResourceUse());
}

}  // namespace rx
