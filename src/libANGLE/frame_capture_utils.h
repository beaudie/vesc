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

namespace gl
{
class BinaryOutputStream;
class Context;
class Framebuffer;
class FramebufferAttachment;
class FramebufferState;
class ImageIndex;
}  // namespace gl

typedef unsigned int GLenum;

namespace angle
{
class MemoryBuffer;
class ScratchBuffer;

void SerializeContext(gl::BinaryOutputStream *bos, gl::Context *context);

void SerializeFramebuffer(gl::Context *context,
                          gl::BinaryOutputStream *bos,
                          gl::Framebuffer *framebuffer);

void SerializeFramebufferState(gl::Context *context,
                               gl::BinaryOutputStream *bos,
                               gl::Framebuffer *framebuffer,
                               const gl::FramebufferState &framebufferState);

void SerializeFramebufferAttachment(gl::Context *context,
                                    gl::BinaryOutputStream *bos,
                                    ScratchBuffer &scratchBuffer,
                                    gl::Framebuffer *framebuffer,
                                    const gl::FramebufferAttachment &framebufferAttachment);

void SerializeImageIndex(gl::BinaryOutputStream *bos, const gl::ImageIndex &imageIndex);

}  // namespace angle
#endif  // FRAME_CAPTURE_UTILS_H_
