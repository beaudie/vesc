//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BufferVk.cpp:
//    Implements the class methods for BufferVk.
//

#include "libANGLE/renderer/vulkan/BufferVk.h"

#include "common/debug.h"
#include "common/mathutil.h"
#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/trace.h"

namespace rx
{

namespace
{
// Vertex attribute buffers are used as storage buffers for conversion in compute, where access to
// the buffer is made in 4-byte chunks.  Assume the size of the buffer is 4k+n where n is in [0, 3).
// On some hardware, reading 4 bytes from address 4k returns 0, making it impossible to read the
// last n bytes.  By rounding up the buffer sizes to a multiple of 4, the problem is alleviated.
constexpr size_t kBufferSizeGranularity = 4;
static_assert(gl::isPow2(kBufferSizeGranularity), "use as alignment, must be power of two");

// Start with a fairly small buffer size. We can increase this dynamically as we convert more data.
constexpr size_t kConvertedArrayBufferInitialSize = 1024 * 8;

// Base size for all staging buffers
constexpr size_t kStagingBufferBaseSize = 1024;
// Fix the staging buffer size multiplier for unpack buffers, for now
constexpr size_t kUnpackBufferStagingBufferMultiplier = 1024;

size_t CalculateStagingBufferSize(gl::BufferBinding target, size_t size, size_t alignment)
{
    size_t alignedSize = rx::roundUp(size, alignment);
    int multiplier     = std::max(gl::log2(alignedSize), 1);

    switch (target)
    {
        case gl::BufferBinding::Array:
        case gl::BufferBinding::DrawIndirect:
        case gl::BufferBinding::ElementArray:
        case gl::BufferBinding::Uniform:
            return kStagingBufferBaseSize * multiplier;

        case gl::BufferBinding::PixelUnpack:
            return std::max(alignedSize,
                            (kStagingBufferBaseSize * kUnpackBufferStagingBufferMultiplier));

        default:
            return kStagingBufferBaseSize;
    }
}

// Buffers that have a static usage pattern will be allocated in
// device local memory to speed up access to and from the GPU.
// Dynamic usage patterns or that are frequently mapped
// will now request host cached memory to speed up access from the CPU.
ANGLE_INLINE VkMemoryPropertyFlags GetPreferredMemoryType(gl::BufferBinding target,
                                                          gl::BufferUsage usage)
{
    constexpr VkMemoryPropertyFlags kDeviceLocalFlags =
        (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    constexpr VkMemoryPropertyFlags kHostCachedFlags =
        (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
         VK_MEMORY_PROPERTY_HOST_CACHED_BIT);

    if (target == gl::BufferBinding::PixelUnpack)
    {
        return kHostCachedFlags;
    }

    switch (usage)
    {
        case gl::BufferUsage::StaticCopy:
        case gl::BufferUsage::StaticDraw:
        case gl::BufferUsage::StaticRead:
            // For static usage, request a device local memory
            return kDeviceLocalFlags;
        default:
            // For non-static usage, request a host cached memory
            return kHostCachedFlags;
    }
}
}  // namespace

// ConversionBuffer implementation.
ConversionBuffer::ConversionBuffer(RendererVk *renderer,
                                   VkBufferUsageFlags usageFlags,
                                   size_t initialSize,
                                   size_t alignment,
                                   bool hostVisible)
    : dirty(true), lastAllocationOffset(0)
{
    data.init(renderer, usageFlags, alignment, initialSize, hostVisible);
}

ConversionBuffer::~ConversionBuffer() = default;

ConversionBuffer::ConversionBuffer(ConversionBuffer &&other) = default;

// BufferVk::VertexConversionBuffer implementation.
BufferVk::VertexConversionBuffer::VertexConversionBuffer(RendererVk *renderer,
                                                         angle::FormatID formatIDIn,
                                                         GLuint strideIn,
                                                         size_t offsetIn,
                                                         bool hostVisible)
    : ConversionBuffer(renderer,
                       vk::kVertexBufferUsageFlags,
                       kConvertedArrayBufferInitialSize,
                       vk::kVertexBufferAlignment,
                       hostVisible),
      formatID(formatIDIn),
      stride(strideIn),
      offset(offsetIn)
{}

BufferVk::VertexConversionBuffer::VertexConversionBuffer(VertexConversionBuffer &&other) = default;

BufferVk::VertexConversionBuffer::~VertexConversionBuffer() = default;

// BufferVk implementation.
BufferVk::BufferVk(const gl::BufferState &state) : BufferImpl(state) {}

BufferVk::~BufferVk() {}

void BufferVk::destroy(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);

    release(contextVk);
}

void BufferVk::release(ContextVk *contextVk)
{
    RendererVk *renderer = contextVk->getRenderer();
    mBuffer.release(renderer);
    mStagingBuffer.release(renderer);
    mCpuOnlyBuffer.release();

    for (ConversionBuffer &buffer : mVertexConversionBuffers)
    {
        buffer.data.release(renderer);
    }
}

void BufferVk::initializeStagingBuffer(ContextVk *contextVk, gl::BufferBinding target, size_t size)
{
    RendererVk *rendererVk = contextVk->getRenderer();

    constexpr VkImageUsageFlags kBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    size_t alignment =
        static_cast<size_t>(rendererVk->getPhysicalDeviceProperties().limits.minMemoryMapAlignment);
    size_t stagingBufferSize = CalculateStagingBufferSize(target, size, alignment);

    mStagingBuffer.init(rendererVk, kBufferUsageFlags, alignment, stagingBufferSize, true);
}

void BufferVk::initializeCpuOnlyBuffer(ContextVk *contextVk, gl::BufferBinding target, size_t size)
{
    // For now, enable CPU only buffers only for pixel unpack buffers.
    // If usecases present themselves, we can enable them for other buffer types.
    if (target == gl::BufferBinding::PixelUnpack)
    {
        // Initialize CPU only buffer
        mCpuOnlyBuffer.init(size);

        // Allocate required memory
        (void)mCpuOnlyBuffer.allocate(size);
    }
}

void BufferVk::updateCpuOnlyBuffer(const uint8_t *data, size_t size, size_t offset)
{
    if (mCpuOnlyBuffer.isEnabled())
    {
        mCpuOnlyBuffer.updateData(data, size, offset);
    }
}

angle::Result BufferVk::setData(const gl::Context *context,
                                gl::BufferBinding target,
                                const void *data,
                                size_t size,
                                gl::BufferUsage usage)
{
    ContextVk *contextVk = vk::GetImpl(context);

    if (size > static_cast<size_t>(mState.getSize()))
    {
        // Release and re-create the memory and buffer.
        release(contextVk);

        // We could potentially use multiple backing buffers for different usages.
        // For now keep a single buffer with all relevant usage flags.
        VkImageUsageFlags usageFlags =
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

        if (contextVk->getFeatures().supportsTransformFeedbackExtension.enabled)
        {
            usageFlags |= VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
        }

        VkBufferCreateInfo createInfo    = {};
        createInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.flags                 = 0;
        createInfo.size                  = roundUpPow2(size, kBufferSizeGranularity);
        createInfo.usage                 = usageFlags;
        createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;

        // Assume host visible/coherent memory available.
        VkMemoryPropertyFlags memoryPropertyFlags = GetPreferredMemoryType(target, usage);

        ANGLE_TRY(mBuffer.init(contextVk, createInfo, memoryPropertyFlags));

        // Initialize the staging buffer
        initializeStagingBuffer(contextVk, target, size);

        // Initialize the CPU only buffer if needed
        initializeCpuOnlyBuffer(contextVk, target, size);
    }

    if (data && size > 0)
    {
        ANGLE_TRY(setDataImpl(contextVk, static_cast<const uint8_t *>(data), size, 0));
    }

    return angle::Result::Continue;
}

angle::Result BufferVk::setSubData(const gl::Context *context,
                                   gl::BufferBinding target,
                                   const void *data,
                                   size_t size,
                                   size_t offset)
{
    ASSERT(mBuffer.valid());

    ContextVk *contextVk = vk::GetImpl(context);
    ANGLE_TRY(setDataImpl(contextVk, static_cast<const uint8_t *>(data), size, offset));

    return angle::Result::Continue;
}

angle::Result BufferVk::waitForIdle(ContextVk *contextVk)
{
    // If there are pending commands for the buffer, flush them.
    if (mBuffer.usedInRecordedCommands())
    {
        ANGLE_TRY(contextVk->flushImpl(nullptr));
    }

    // Make sure the driver is done with the buffer.
    if (mBuffer.usedInRunningCommands(contextVk->getLastCompletedQueueSerial()))
    {
        ANGLE_TRY(mBuffer.finishRunningCommands(contextVk));
    }

    ASSERT(!mBuffer.isCurrentlyInUse(contextVk->getLastCompletedQueueSerial()));

    return angle::Result::Continue;
}

angle::Result BufferVk::copySubData(const gl::Context *context,
                                    BufferImpl *source,
                                    GLintptr sourceOffset,
                                    GLintptr destOffset,
                                    GLsizeiptr size)
{
    ASSERT(mBuffer.valid());

    ContextVk *contextVk = vk::GetImpl(context);
    auto *sourceBuffer   = GetAs<BufferVk>(source);
    ASSERT(sourceBuffer->getBuffer().valid());

    // If there is an active CPU only buffer then we need to update that
    // as well. This will require us to complete all recorded and running
    // commands for the source buffer. Do this only if the dest buffer
    // has a CPU only buffer.
    if (mCpuOnlyBuffer.isEnabled())
    {
        ANGLE_TRY(sourceBuffer->waitForIdle(contextVk));

        // Update the CPU only buffer
        uint8_t *srcPtr;
        ANGLE_VK_TRY(contextVk, sourceBuffer->getBuffer().getDeviceMemory().map(
                                    contextVk->getDevice(), sourceOffset, size, 0, &srcPtr));

        updateCpuOnlyBuffer(srcPtr, size, destOffset);

        // Unmap the source buffer
        sourceBuffer->getBuffer().getDeviceMemory().unmap(contextVk->getDevice());
    }

    vk::CommandBuffer *commandBuffer = nullptr;

    ANGLE_TRY(contextVk->onBufferRead(VK_ACCESS_TRANSFER_READ_BIT, &sourceBuffer->getBuffer()));
    ANGLE_TRY(contextVk->onBufferWrite(VK_ACCESS_TRANSFER_WRITE_BIT, &mBuffer));
    ANGLE_TRY(contextVk->endRenderPassAndGetCommandBuffer(&commandBuffer));

    // Enqueue a copy command on the GPU.
    const VkBufferCopy copyRegion = {static_cast<VkDeviceSize>(sourceOffset),
                                     static_cast<VkDeviceSize>(destOffset),
                                     static_cast<VkDeviceSize>(size)};

    commandBuffer->copyBuffer(sourceBuffer->getBuffer().getBuffer(), mBuffer.getBuffer(), 1,
                              &copyRegion);

    // The new destination buffer data may require a conversion for the next draw, so mark it dirty.
    onDataChanged();

    return angle::Result::Continue;
}

angle::Result BufferVk::map(const gl::Context *context, GLenum access, void **mapPtr)
{
    ASSERT(mBuffer.valid());

    return mapImpl(vk::GetImpl(context), mapPtr);
}

angle::Result BufferVk::mapRange(const gl::Context *context,
                                 size_t offset,
                                 size_t length,
                                 GLbitfield access,
                                 void **mapPtr)
{
    return mapRangeImpl(vk::GetImpl(context), offset, length, access, mapPtr);
}

angle::Result BufferVk::mapImpl(ContextVk *contextVk, void **mapPtr)
{
    return mapRangeImpl(contextVk, 0, static_cast<VkDeviceSize>(mState.getSize()), 0, mapPtr);
}

angle::Result BufferVk::mapRangeImpl(ContextVk *contextVk,
                                     VkDeviceSize offset,
                                     VkDeviceSize length,
                                     GLbitfield access,
                                     void **mapPtr)
{
    const bool unsynchronizedMap = ((access & GL_MAP_UNSYNCHRONIZED_BIT) != 0);

    if (mCpuOnlyBuffer.isEnabled() == false)
    {
        ASSERT(mBuffer.valid());

        if (unsynchronizedMap == false)
        {
            ANGLE_TRY(waitForIdle(contextVk));
        }

        ANGLE_VK_TRY(contextVk,
                     mBuffer.getDeviceMemory().map(contextVk->getDevice(), offset, length, 0,
                                                   reinterpret_cast<uint8_t **>(mapPtr)));
    }
    else
    {
        mCpuOnlyBuffer.onMapBuffer(unsynchronizedMap, ((access & GL_MAP_WRITE_BIT) != 0),
                                   static_cast<size_t>(offset), static_cast<size_t>(length));

        // If app requested the GL_MAP_UNSYNCHRONIZED_BIT bit, even though we have a
        // CPU only buffer return the, possibly stale, GPU buffer.
        if (unsynchronizedMap)
        {
            ANGLE_VK_TRY(contextVk,
                         mBuffer.getDeviceMemory().map(contextVk->getDevice(), offset, length, 0,
                                                       reinterpret_cast<uint8_t **>(mapPtr)));
        }
        else
        {
            *mapPtr = (mCpuOnlyBuffer.getCurrentBuffer() + offset);
        }
    }

    return angle::Result::Continue;
}

angle::Result BufferVk::unmap(const gl::Context *context, GLboolean *result)
{
    unmapImpl(vk::GetImpl(context));

    // This should be false if the contents have been corrupted through external means.  Vulkan
    // doesn't provide such information.
    *result = true;

    return angle::Result::Continue;
}

void BufferVk::unmapImpl(ContextVk *contextVk)
{
    ASSERT(mBuffer.valid());

    if (mCpuOnlyBuffer.isEnabled() == false)
    {
        mBuffer.getDeviceMemory().unmap(contextVk->getDevice());
        mBuffer.onExternalWrite(VK_ACCESS_HOST_WRITE_BIT);
    }
    else
    {
        bool unsynchronizedMap = false;
        bool writeOperation    = false;
        size_t offset          = 0;
        size_t size            = 0;

        mCpuOnlyBuffer.onUnmapBuffer(&unsynchronizedMap, &writeOperation, &offset, &size);

        // Need to unmap the GPU buffer if it was a GL_MAP_UNSYNCHRONIZED_BIT access
        if (unsynchronizedMap)
        {
            mBuffer.getDeviceMemory().unmap(contextVk->getDevice());
            mBuffer.onExternalWrite(VK_ACCESS_HOST_WRITE_BIT);
        }

        // If it was a write operation we need to update either the GPU buffer
        // or the CPU only buffer.
        if (writeOperation)
        {
            // If it was an unsynchronized map, the GPU buffer is upto date. Refresh contents
            // of the CPU only buffer
            if (unsynchronizedMap)
            {
                uint8_t *mapPtr = nullptr;
                (void)mBuffer.getDeviceMemory().map(contextVk->getDevice(), offset, size, 0,
                                                    &mapPtr);
                ASSERT(mapPtr);
                mCpuOnlyBuffer.updateData(mapPtr, size, offset);
                mBuffer.getDeviceMemory().unmap(contextVk->getDevice());
            }
            else
            {
                // We do not yet know if thie data will ever be used. Perform a staged
                // update which will get flushed only when and if necessary.
                const uint8_t *data = (mCpuOnlyBuffer.getCurrentBuffer() + offset);
                (void)stagedUpdate(contextVk, data, size, offset);
            }
        }
    }

    markConversionBuffersDirty();
}

angle::Result BufferVk::getIndexRange(const gl::Context *context,
                                      gl::DrawElementsType type,
                                      size_t offset,
                                      size_t count,
                                      bool primitiveRestartEnabled,
                                      gl::IndexRange *outRange)
{
    ContextVk *contextVk = vk::GetImpl(context);
    RendererVk *renderer = contextVk->getRenderer();

    // This is a workaround for the mock ICD not implementing buffer memory state.
    // Could be removed if https://github.com/KhronosGroup/Vulkan-Tools/issues/84 is fixed.
    if (renderer->isMockICDEnabled())
    {
        outRange->start = 0;
        outRange->end   = 0;
        return angle::Result::Continue;
    }

    ANGLE_TRACE_EVENT0("gpu.angle", "BufferVk::getIndexRange");

    uint8_t *mapPointer = nullptr;

    if (mCpuOnlyBuffer.isEnabled() == false)
    {
        // Needed before reading buffer or we could get stale data.
        ANGLE_TRY(mBuffer.finishRunningCommands(contextVk));

        ASSERT(mBuffer.valid());

        const GLuint &typeBytes = gl::GetDrawElementsTypeSize(type);

        ANGLE_VK_TRY(contextVk, mBuffer.getDeviceMemory().map(contextVk->getDevice(), offset,
                                                              typeBytes * count, 0, &mapPointer));
    }
    else
    {
        mapPointer = (mCpuOnlyBuffer.getCurrentBuffer() + offset);
    }

    *outRange = gl::ComputeIndexRange(type, mapPointer, count, primitiveRestartEnabled);

    mBuffer.getDeviceMemory().unmap(contextVk->getDevice());
    return angle::Result::Continue;
}

angle::Result BufferVk::directUpdate(ContextVk *contextVk,
                                     const uint8_t *data,
                                     size_t size,
                                     size_t offset)
{
    VkDevice device     = contextVk->getDevice();
    uint8_t *mapPointer = nullptr;

    ANGLE_VK_TRY(contextVk, mBuffer.getDeviceMemory().map(device, offset, size, 0, &mapPointer));
    ASSERT(mapPointer);

    memcpy(mapPointer, data, size);

    mBuffer.getDeviceMemory().unmap(device);
    mBuffer.onExternalWrite(VK_ACCESS_HOST_WRITE_BIT);

    return angle::Result::Continue;
}

angle::Result BufferVk::stagedUpdate(ContextVk *contextVk,
                                     const uint8_t *data,
                                     size_t size,
                                     size_t offset)
{
    // Acquire a "new" staging buffer
    bool needToReleasePreviousBuffers = false;
    uint8_t *mapPointer               = nullptr;
    VkDeviceSize stagingBufferOffset  = 0;

    ANGLE_TRY(mStagingBuffer.allocate(contextVk, size, &mapPointer, nullptr, &stagingBufferOffset,
                                      &needToReleasePreviousBuffers));
    if (needToReleasePreviousBuffers)
    {
        // Release previous staging buffers
        mStagingBuffer.releaseInFlightBuffers(contextVk);
    }
    ASSERT(mapPointer);

    memcpy(mapPointer, data, size);

    // Enqueue a copy command on the GPU.
    VkBufferCopy copyRegion = {stagingBufferOffset, offset, size};
    ANGLE_TRY(mBuffer.copyFromBuffer(contextVk, mStagingBuffer.getCurrentBuffer(),
                                     VK_ACCESS_HOST_WRITE_BIT, copyRegion));
    mStagingBuffer.getCurrentBuffer()->retain(&contextVk->getResourceUseList());

    return angle::Result::Continue;
}

angle::Result BufferVk::setDataImpl(ContextVk *contextVk,
                                    const uint8_t *data,
                                    size_t size,
                                    size_t offset)
{
    // Update CPU only buffer
    updateCpuOnlyBuffer(data, size, offset);

    // Use map when available.
    if (mBuffer.isCurrentlyInUse(contextVk->getLastCompletedQueueSerial()))
    {
        ANGLE_TRY(stagedUpdate(contextVk, data, size, offset));
    }
    else
    {
        ANGLE_TRY(directUpdate(contextVk, data, size, offset));
    }

    // Update conversions
    markConversionBuffersDirty();

    return angle::Result::Continue;
}

angle::Result BufferVk::copyToBufferHelper(ContextVk *contextVk,
                                           vk::BufferHelper *destBuffer,
                                           uint32_t copyCount,
                                           const VkBufferCopy *copies)
{
    vk::CommandBuffer *commandBuffer;
    ANGLE_TRY(contextVk->onBufferWrite(VK_ACCESS_TRANSFER_WRITE_BIT, destBuffer));
    ANGLE_TRY(contextVk->onBufferRead(VK_ACCESS_TRANSFER_READ_BIT, &mBuffer));
    ANGLE_TRY(contextVk->endRenderPassAndGetCommandBuffer(&commandBuffer));

    commandBuffer->copyBuffer(mBuffer.getBuffer(), destBuffer->getBuffer(), copyCount, copies);

    return angle::Result::Continue;
}

ConversionBuffer *BufferVk::getVertexConversionBuffer(RendererVk *renderer,
                                                      angle::FormatID formatID,
                                                      GLuint stride,
                                                      size_t offset,
                                                      bool hostVisible)
{
    for (VertexConversionBuffer &buffer : mVertexConversionBuffers)
    {
        if (buffer.formatID == formatID && buffer.stride == stride && buffer.offset == offset)
        {
            return &buffer;
        }
    }

    mVertexConversionBuffers.emplace_back(renderer, formatID, stride, offset, hostVisible);
    return &mVertexConversionBuffers.back();
}

void BufferVk::markConversionBuffersDirty()
{
    for (VertexConversionBuffer &buffer : mVertexConversionBuffers)
    {
        buffer.dirty = true;
    }
}

void BufferVk::onDataChanged()
{
    markConversionBuffersDirty();
}

}  // namespace rx
