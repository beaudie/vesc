//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BufferWgpu.h:
//    Defines the class interface for BufferWgpu, implementing BufferImpl.
//

#ifndef LIBANGLE_RENDERER_WGPU_BUFFERWGPU_H_
#define LIBANGLE_RENDERER_WGPU_BUFFERWGPU_H_

#include "libANGLE/renderer/BufferImpl.h"

#include "libANGLE/renderer/wgpu/wgpu_helpers.h"

#include <dawn/webgpu_cpp.h>

namespace rx
{

enum class WebGPUBufferUsage
{
    VertexOrIndex,
    Uniform,
    MapRead,
    MapWrite,

    Count,
};

class BufferWgpu : public BufferImpl
{
  public:
    BufferWgpu(const gl::BufferState &state);
    ~BufferWgpu() override;

    angle::Result setData(const gl::Context *context,
                          gl::BufferBinding target,
                          const void *data,
                          size_t size,
                          gl::BufferUsage usage) override;
    angle::Result setSubData(const gl::Context *context,
                             gl::BufferBinding target,
                             const void *data,
                             size_t size,
                             size_t offset) override;
    angle::Result copySubData(const gl::Context *context,
                              BufferImpl *source,
                              GLintptr sourceOffset,
                              GLintptr destOffset,
                              GLsizeiptr size) override;
    angle::Result map(const gl::Context *context, GLenum access, void **mapPtr) override;
    angle::Result mapRange(const gl::Context *context,
                           size_t offset,
                           size_t length,
                           GLbitfield access,
                           void **mapPtr) override;
    angle::Result unmap(const gl::Context *context, GLboolean *result) override;

    angle::Result getIndexRange(const gl::Context *context,
                                gl::DrawElementsType type,
                                size_t offset,
                                size_t count,
                                bool primitiveRestartEnabled,
                                gl::IndexRange *outRange) override;

  private:
    struct PerUsageBuffer
    {
        webgpu::BufferHelper buffer;
        WebGPUBufferUsage usage;
        size_t serial = 0;
    };

    PerUsageBuffer &getBuffer(WebGPUBufferUsage usage);

    // Get the buffer with the latest data. May return null if there are no buffers allocated yet.
    PerUsageBuffer *getLatestBuffer();

    std::array<PerUsageBuffer, static_cast<size_t>(WebGPUBufferUsage::Count)> mBuffers;
    size_t mCurrentSerial = 1;

    size_t mSize = 0;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_BUFFERWGPU_H_
