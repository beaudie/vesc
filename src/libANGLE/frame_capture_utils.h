//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// frame_capture_utils.h:
//   ANGLE frame capture utils interface.
//
#ifndef FRAME_CAPTURE_UTILS_H_
#define FRAME_CAPTURE_UTILS_H_

#include <vector>

#include "common/Color.h"
#include "libANGLE/BinaryStream.h"
#include "libANGLE/Error.h"

namespace gl
{
class Buffer;
class BufferState;
class Context;
class Framebuffer;
class FramebufferAttachment;
class FramebufferState;
class ImageIndex;
class Sampler;
class SamplerState;
}  // namespace gl

typedef unsigned int GLenum;

namespace angle
{
class MemoryBuffer;
class ScratchBuffer;

Result SerializeContext(gl::BinaryOutputStream *bos, const gl::Context *context);

Result SerializeFramebuffer(const gl::Context *context,
                            gl::BinaryOutputStream *bos,
                            ScratchBuffer *scratchBuffer,
                            gl::Framebuffer *framebuffer);

Result SerializeFramebufferState(const gl::Context *context,
                                 gl::BinaryOutputStream *bos,
                                 ScratchBuffer *scratchBuffer,
                                 gl::Framebuffer *framebuffer,
                                 const gl::FramebufferState &framebufferState);

Result SerializeFramebufferAttachment(const gl::Context *context,
                                      gl::BinaryOutputStream *bos,
                                      ScratchBuffer *scratchBuffer,
                                      gl::Framebuffer *framebuffer,
                                      const gl::FramebufferAttachment &framebufferAttachment);

void SerializeImageIndex(gl::BinaryOutputStream *bos, const gl::ImageIndex &imageIndex);

Result SerializeBuffer(const gl::Context *context,
                       gl::BinaryOutputStream *bos,
                       ScratchBuffer *scratchBuffer,
                       gl::Buffer *buffer);

void SerializeBufferState(gl::BinaryOutputStream *bos, const gl::BufferState &bufferState);

void SerializeSampler(gl::BinaryOutputStream *bos, gl::Sampler *sampler);

void SerializeSamplerState(gl::BinaryOutputStream *bos, const gl::SamplerState &samplerState);

void SerializeColorGeneric(gl::BinaryOutputStream *bos, const ColorGeneric &colorGeneric);

template <typename T>
void SerializeColor(gl::BinaryOutputStream *bos, const Color<T> &color)
{
    bos->writeInt(color.red);
    bos->writeInt(color.green);
    bos->writeInt(color.blue);
    bos->writeInt(color.alpha);
}
}  // namespace angle
#endif  // FRAME_CAPTURE_UTILS_H_
