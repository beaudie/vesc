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

namespace rx
{

BufferWgpu::BufferWgpu(const gl::BufferState &state, AllocationTrackerWgpu *allocationTracker)
    : BufferImpl(state), mAllocationTracker(allocationTracker)
{
    ASSERT(mAllocationTracker != nullptr);
}

BufferWgpu::~BufferWgpu()
{
    bool memoryReleaseResult = mAllocationTracker->updateMemoryAllocation(mData.size(), 0);
    ASSERT(memoryReleaseResult);
}

angle::Result BufferWgpu::setDataWithUsageFlags(const gl::Context *context,
                                                gl::BufferBinding target,
                                                GLeglClientBufferEXT clientBuffer,
                                                const void *data,
                                                size_t size,
                                                gl::BufferUsage usage,
                                                GLbitfield flags)
{
    ANGLE_CHECK_GL_ALLOC(GetImplAs<ContextWgpu>(context),
                         mAllocationTracker->updateMemoryAllocation(mData.size(), size));

    mData.resize(size, 0);
    if (size > 0 && data != nullptr)
    {
        memcpy(mData.data(), data, size);
    }
    return angle::Result::Continue;
}

angle::Result BufferWgpu::setData(const gl::Context *context,
                                  gl::BufferBinding target,
                                  const void *data,
                                  size_t size,
                                  gl::BufferUsage usage)
{
    ANGLE_CHECK_GL_ALLOC(GetImplAs<ContextWgpu>(context),
                         mAllocationTracker->updateMemoryAllocation(mData.size(), size));

    mData.resize(size, 0);
    if (size > 0 && data != nullptr)
    {
        memcpy(mData.data(), data, size);
    }
    return angle::Result::Continue;
}

angle::Result BufferWgpu::setSubData(const gl::Context *context,
                                     gl::BufferBinding target,
                                     const void *data,
                                     size_t size,
                                     size_t offset)
{
    if (size > 0)
    {
        memcpy(mData.data() + offset, data, size);
    }
    return angle::Result::Continue;
}

angle::Result BufferWgpu::copySubData(const gl::Context *context,
                                      BufferImpl *source,
                                      GLintptr sourceOffset,
                                      GLintptr destOffset,
                                      GLsizeiptr size)
{
    BufferWgpu *sourceWgpu = GetAs<BufferWgpu>(source);
    if (size > 0)
    {
        memcpy(mData.data() + destOffset, sourceWgpu->mData.data() + sourceOffset, size);
    }
    return angle::Result::Continue;
}

angle::Result BufferWgpu::map(const gl::Context *context, GLenum access, void **mapPtr)
{
    *mapPtr = mData.data();
    return angle::Result::Continue;
}

angle::Result BufferWgpu::mapRange(const gl::Context *context,
                                   size_t offset,
                                   size_t length,
                                   GLbitfield access,
                                   void **mapPtr)
{
    *mapPtr = mData.data() + offset;
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
    *outRange = gl::ComputeIndexRange(type, mData.data() + offset, count, primitiveRestartEnabled);
    return angle::Result::Continue;
}

uint8_t *BufferWgpu::getDataPtr()
{
    return mData.data();
}

const uint8_t *BufferWgpu::getDataPtr() const
{
    return mData.data();
}

}  // namespace rx
