//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FramebufferGL.cpp: Implements the FramebufferGL and DefaultAttachmentGL classes.

#include "libANGLE/renderer/gl/FramebufferGL.h"

namespace rx
{

DefaultAttachmentGL::DefaultAttachmentGL()
{
}

DefaultAttachmentGL::~DefaultAttachmentGL()
{
}

GLsizei DefaultAttachmentGL::getWidth() const
{
    return 0;
}

GLsizei DefaultAttachmentGL::getHeight() const
{
    return 0;
}

GLenum DefaultAttachmentGL::getInternalFormat() const
{
    return 0;
}

GLsizei DefaultAttachmentGL::getSamples() const
{
    return 0;
}

FramebufferGL::FramebufferGL()
{
}

FramebufferGL::~FramebufferGL()
{
}

void FramebufferGL::setColorAttachment(size_t index, const gl::FramebufferAttachment *attachment)
{
}

void FramebufferGL::setDepthttachment(const gl::FramebufferAttachment *attachment)
{
}

void FramebufferGL::setStencilAttachment(const gl::FramebufferAttachment *attachment)
{
}

void FramebufferGL::setDepthStencilAttachment(const gl::FramebufferAttachment *attachment)
{
}

void FramebufferGL::setDrawBuffers(size_t count, const GLenum *buffers)
{
}

void FramebufferGL::setReadBuffer(GLenum buffer)
{
}

gl::Error FramebufferGL::invalidate(size_t count, const GLenum *attachments)
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error FramebufferGL::invalidateSub(size_t count, const GLenum *attachments, const gl::Rectangle &area)
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error FramebufferGL::clear(const gl::State &state, GLbitfield mask)
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error FramebufferGL::clearBufferfv(const gl::State &state, GLenum buffer, GLint drawbuffer, const GLfloat *values)
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error FramebufferGL::clearBufferuiv(const gl::State &state, GLenum buffer, GLint drawbuffer, const GLuint *values)
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error FramebufferGL::clearBufferiv(const gl::State &state, GLenum buffer, GLint drawbuffer, const GLint *values)
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error FramebufferGL::clearBufferfi(const gl::State &state, GLenum buffer, GLint drawbuffer, GLfloat depth,
                                       GLint stencil)
{
    return gl::Error(GL_NO_ERROR);
}

GLenum FramebufferGL::getImplementationColorReadFormat() const
{
    return GL_NONE;
}

GLenum FramebufferGL::getImplementationColorReadType() const
{
    return GL_NONE;
}

gl::Error FramebufferGL::readPixels(const gl::State &state, const gl::Rectangle &area, GLenum format, GLenum type,
                                    GLvoid *pixels) const
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error FramebufferGL::blit(const gl::State &state, const gl::Rectangle &sourceArea, const gl::Rectangle &destArea,
                              GLbitfield mask, GLenum filter, const gl::Framebuffer *sourceFramebuffer)
{
    return gl::Error(GL_NO_ERROR);
}

GLenum FramebufferGL::checkStatus() const
{
    return GL_FRAMEBUFFER_COMPLETE;
}

}
