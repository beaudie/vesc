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
struct BlendState;
class Buffer;
class BufferState;
class Context;
struct Extensions;
class Framebuffer;
class FramebufferAttachment;
class FramebufferState;
class ImageIndex;
struct Limitations;
struct RasterizerState;
struct Rectangle;
class State;
struct Version;
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

template <typename T>
void SerializeColor(gl::BinaryOutputStream *bos, const Color<T> &color)
{
    bos->writeInt(color.red);
    bos->writeInt(color.green);
    bos->writeInt(color.blue);
    bos->writeInt(color.alpha);
}

void SerializeGLGlobalStates(gl::BinaryOutputStream *bos, const gl::State &state);

void SerializeExtensions(gl::BinaryOutputStream *bos, const gl::Extensions &extensions);

void SerializeLimitations(gl::BinaryOutputStream *bos, const gl::Limitations &limitations);

void SerializeRasterizerState(gl::BinaryOutputStream *bos,
                              const gl::RasterizerState &rasterizerState);

void SerializeRectangle(gl::BinaryOutputStream *bos, const gl::Rectangle &rectangle);

void SerializeBlendState(gl::BinaryOutputStream *bos, const gl::BlendState &blendState);

}  // namespace angle
#endif  // FRAME_CAPTURE_UTILS_H_
