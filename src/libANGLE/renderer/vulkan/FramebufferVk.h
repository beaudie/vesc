//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FramebufferVk.h: Defines the class interface for FramebufferVk.

#ifndef LIBANGLE_RENDERER_VULKAN_FRAMEBUFFERVK_H_
#define LIBANGLE_RENDERER_VULKAN_FRAMEBUFFERVK_H_

#include "libANGLE/renderer/FramebufferImpl.h"

namespace rx
{

class FramebufferVk : public FramebufferImpl
{
  public:
    FramebufferVk(const gl::Framebuffer::Data &data);
    ~FramebufferVk() override;

    gl::Error discard(size_t count, const GLenum *attachments) override;
    gl::Error invalidate(size_t count, const GLenum *attachments) override;
    gl::Error invalidateSub(size_t count,
                            const GLenum *attachments,
                            const gl::Rectangle &area) override;

    gl::Error clear(const gl::Data &data, GLbitfield mask) override;
    gl::Error clearBufferfv(const gl::Data &data,
                            GLenum buffer,
                            GLint drawbuffer,
                            const GLfloat *values) override;
    gl::Error clearBufferuiv(const gl::Data &data,
                             GLenum buffer,
                             GLint drawbuffer,
                             const GLuint *values) override;
    gl::Error clearBufferiv(const gl::Data &data,
                            GLenum buffer,
                            GLint drawbuffer,
                            const GLint *values) override;
    gl::Error clearBufferfi(const gl::Data &data,
                            GLenum buffer,
                            GLint drawbuffer,
                            GLfloat depth,
                            GLint stencil) override;

    GLenum getImplementationColorReadFormat() const override;
    GLenum getImplementationColorReadType() const override;
    gl::Error readPixels(const gl::State &state,
                         const gl::Rectangle &area,
                         GLenum format,
                         GLenum type,
                         GLvoid *pixels) const override;

    gl::Error blit(const gl::State &state,
                   const gl::Rectangle &sourceArea,
                   const gl::Rectangle &destArea,
                   GLbitfield mask,
                   GLenum filter,
                   const gl::Framebuffer *sourceFramebuffer) override;

    bool checkStatus() const override;

    void syncState(const gl::Framebuffer::DirtyBits &dirtyBits) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_FRAMEBUFFERVK_H_
