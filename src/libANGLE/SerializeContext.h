// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SerializeContext.h:
//   ANGLE serialize context interface.
//
#ifndef LIBANGLE_SERIALIZE_CONTEXT_H_
#define LIBANGLE_SERIALIZE_CONTEXT_H_

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

void SerializeContext(gl::BinaryOutputStream *bos, gl::Context *context);

void SerializeFramebuffer(gl::BinaryOutputStream *bos,
                          gl::Context *context,
                          gl::Framebuffer *framebuffer);

void SerializeFramebufferState(gl::BinaryOutputStream *bos,
                               gl::Context *context,
                               gl::FramebufferState *framebufferState);

void SerializeFramebufferAttachment(gl::BinaryOutputStream *bos,
                                    gl::Context *context,
                                    GLenum mode,
                                    const gl::FramebufferAttachment *framebufferAttachment);

void SerializeImageIndex(gl::BinaryOutputStream *bos, const gl::ImageIndex &imageIndex);

std::vector<uint8_t> ReadPixelsFromAttachment(
    gl::Context *context,
    GLenum mode,
    const gl::FramebufferAttachment *framebufferAttachment);

}  // namespace angle
#endif  // LIBANGLE_SERIALIZE_CONTEXT_H_
