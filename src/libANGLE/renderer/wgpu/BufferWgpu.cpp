//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BufferWgpu.cpp:
//    Implements the class methods for BufferWgpu.
//

#include "libANGLE/renderer/wgpu/BufferWgpu.h"

#include "common/debug.h"
#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/wgpu/ContextWgpu.h"
#include "libANGLE/renderer/wgpu/wgpu_utils.h"

namespace rx
{
namespace
{
// Based on a buffer binding target, guess the WebGPU buffer usage.
WebGPUBufferUsage BindingTargetToWebGPUBufferUsage(gl::BufferBinding binding)
{
    switch (binding)
    {
        case gl::BufferBinding::Array:
        case gl::BufferBinding::ElementArray:
            return WebGPUBufferUsage::VertexOrIndex;

        case gl::BufferBinding::Uniform:
            return WebGPUBufferUsage::Uniform;

        case gl::BufferBinding::PixelPack:
            return WebGPUBufferUsage::MapRead;

        case gl::BufferBinding::PixelUnpack:
            return WebGPUBufferUsage::MapWrite;

        case gl::BufferBinding::CopyRead:
        case gl::BufferBinding::CopyWrite:
        case gl::BufferBinding::ShaderStorage:
        case gl::BufferBinding::Texture:
        case gl::BufferBinding::TransformFeedback:
        case gl::BufferBinding::DispatchIndirect:
        case gl::BufferBinding::DrawIndirect:
        case gl::BufferBinding::AtomicCounter:
            UNIMPLEMENTED();
            return WebGPUBufferUsage::Count;

        default:
            UNREACHABLE();
            return WebGPUBufferUsage::Count;
    }
}

wgpu::BufferUsage WebGPUBufferUsageToUsageFlags(WebGPUBufferUsage usage)
{
    switch (usage)
    {
        case WebGPUBufferUsage::VertexOrIndex:
            return wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Index |
                   wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;

        case WebGPUBufferUsage::Uniform:
            return wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopySrc |
                   wgpu::BufferUsage::CopyDst;

        case WebGPUBufferUsage::MapRead:
            return wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;

        case WebGPUBufferUsage::MapWrite:
            return wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;

        default:
            UNREACHABLE();
            return wgpu::BufferUsage::None;
    }
}
}  // namespace

BufferWgpu::BufferWgpu(const gl::BufferState &state) : BufferImpl(state)
{
    // Initialize usage flags of the buffer array
    for (size_t usageIndex = 0; usageIndex < static_cast<size_t>(WebGPUBufferUsage::Count);
         ++usageIndex)
    {
        mBuffers[usageIndex].usage = static_cast<WebGPUBufferUsage>(usageIndex);
    }
}

BufferWgpu::~BufferWgpu() {}

angle::Result BufferWgpu::setData(const gl::Context *context,
                                  gl::BufferBinding target,
                                  const void *data,
                                  size_t size,
                                  gl::BufferUsage usage)
{
    ContextWgpu *contextWgpu = webgpu::GetImpl(context);
    wgpu::Device device      = webgpu::GetDevice(context);

    // Delete all existing buffers that don't match the new size
    for (PerUsageBuffer &perUsageBuffer : mBuffers)
    {
        if (perUsageBuffer.buffer.size() != size)
        {
            perUsageBuffer.buffer.reset();
        }
    }

    WebGPUBufferUsage usageForBinding = BindingTargetToWebGPUBufferUsage(target);

    mCurrentSerial++;
    mSize = size;

    if (data && size > 0)
    {
        // Try to find a buffer to upload to. Prefer the latest one if it can be mapped for write.
        // Otherwise use the one associated with this binding point. If nothing can be mapped,
        // create a new buffer to replace the one at this binding point.
        PerUsageBuffer *perUsageBuffer = getLatestBuffer();
        if (perUsageBuffer && !perUsageBuffer->buffer.canMapForWrite())
        {
            // The latest buffer cannot be used for this upload.
            perUsageBuffer = nullptr;
        }

        if (!perUsageBuffer)
        {
            perUsageBuffer = &getBuffer(usageForBinding);
        }

        ASSERT(perUsageBuffer);
        if (perUsageBuffer->buffer.valid() && !perUsageBuffer->buffer.canMapForWrite())
        {
            // If the current buffer cannot be mapped, delete it so a new one can be created.
            perUsageBuffer->buffer.reset();
        }

        if (!perUsageBuffer->buffer.valid())
        {
            // Initialize the buffer if it doesn't exist. Make it mapped at creation.
            ANGLE_TRY(perUsageBuffer->buffer.initBuffer(
                device, size, WebGPUBufferUsageToUsageFlags(perUsageBuffer->usage), true));
        }

        if (!perUsageBuffer->buffer.getMappedState().has_value())
        {
            ANGLE_TRY(
                perUsageBuffer->buffer.mapImmediate(contextWgpu, wgpu::MapMode::Write, 0, size));
        }

        uint8_t *mappedData = perUsageBuffer->buffer.getMapWritePointer(0, size);
        memcpy(mappedData, data, size);

        // Mark this usage/buffer as having the latest data.
        perUsageBuffer->serial = mCurrentSerial;
    }
    else
    {
        // Even though no buffer was created and no data was uploaded, mark this this usage as the
        // latest so that subsequent operations like glBufferSubData will "remember" the usage
        // unless another is used.
        PerUsageBuffer &perUsageBuffer = getBuffer(usageForBinding);
        perUsageBuffer.serial          = mCurrentSerial;
    }

    return angle::Result::Continue;
}

angle::Result BufferWgpu::setSubData(const gl::Context *context,
                                     gl::BufferBinding target,
                                     const void *data,
                                     size_t size,
                                     size_t offset)
{
    ContextWgpu *contextWgpu = webgpu::GetImpl(context);
    wgpu::Device device      = webgpu::GetDevice(context);

    PerUsageBuffer *perUsageBuffer = getLatestBuffer();

    if (!perUsageBuffer->buffer.valid())
    {
        ANGLE_TRY(perUsageBuffer->buffer.initBuffer(
            device, size, WebGPUBufferUsageToUsageFlags(perUsageBuffer->usage), true));
    }

    if (perUsageBuffer->buffer.canMapForWrite())
    {
        if (!perUsageBuffer->buffer.getMappedState().has_value())
        {
            ANGLE_TRY(perUsageBuffer->buffer.mapImmediate(contextWgpu, wgpu::MapMode::Write, offset,
                                                          size));
        }

        uint8_t *mappedData = perUsageBuffer->buffer.getMapWritePointer(offset, size);
        memcpy(mappedData, data, size);
    }
    else
    {
        webgpu::BufferHelper stagingBuffer;
        ANGLE_TRY(stagingBuffer.initBuffer(
            device, size, wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc, true));
        uint8_t *mappedData = perUsageBuffer->buffer.getMapWritePointer(0, size);
        memcpy(mappedData, data, size);
        ANGLE_TRY(stagingBuffer.unmap());

        // TODO: get a command encoder and call CopyBufferToBuffer(stagingBuffer, 0,
        // perUsageBuffer->buffer.getBuffer(), offset, size);
        UNREACHABLE();
    }

    return angle::Result::Continue;
}

angle::Result BufferWgpu::copySubData(const gl::Context *context,
                                      BufferImpl *source,
                                      GLintptr sourceOffset,
                                      GLintptr destOffset,
                                      GLsizeiptr size)
{
    return angle::Result::Continue;
}

angle::Result BufferWgpu::map(const gl::Context *context, GLenum access, void **mapPtr)
{
    return angle::Result::Continue;
}

angle::Result BufferWgpu::mapRange(const gl::Context *context,
                                   size_t offset,
                                   size_t length,
                                   GLbitfield access,
                                   void **mapPtr)
{
    return angle::Result::Continue;
}

angle::Result BufferWgpu::unmap(const gl::Context *context, GLboolean *result)
{
    *result = GL_TRUE;
    return angle::Result::Continue;
}

angle::Result BufferWgpu::getIndexRange(const gl::Context *context,
                                        gl::DrawElementsType type,
                                        size_t offset,
                                        size_t count,
                                        bool primitiveRestartEnabled,
                                        gl::IndexRange *outRange)
{
    return angle::Result::Continue;
}

BufferWgpu::PerUsageBuffer &BufferWgpu::getBuffer(WebGPUBufferUsage usage)
{
    return mBuffers[static_cast<size_t>(usage)];
}

BufferWgpu::PerUsageBuffer *BufferWgpu::getLatestBuffer()
{
    for (PerUsageBuffer &perUsageBuffer : mBuffers)
    {
        if (perUsageBuffer.serial == mCurrentSerial)
        {
            return &perUsageBuffer;
        }
    }

    return nullptr;
}
}  // namespace rx
