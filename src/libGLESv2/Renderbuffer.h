//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Renderbuffer.h: Defines the wrapper class gl::Renderbuffer, as well as the
// class hierarchy used to store its contents: RenderbufferStorage, Colorbuffer,
// DepthStencilbuffer, Depthbuffer and Stencilbuffer. Implements GL renderbuffer
// objects and related functionality. [OpenGL ES 2.0.24] section 4.4.3 page 108.

#ifndef LIBGLESV2_RENDERBUFFER_H_
#define LIBGLESV2_RENDERBUFFER_H_

#include "angle_gl.h"

#include "common/angleutils.h"
#include "common/RefCountObject.h"
#include "libGLESv2/renderer/RenderbufferImpl.h"

namespace rx
{
class Renderer;
class SwapChain;
class RenderTarget;
class TextureStorage;
}

namespace gl
{
class FramebufferAttachment;

// A GL renderbuffer object is usually used as a depth or stencil buffer attachment
// for a framebuffer object. The renderbuffer itself is a distinct GL object, see
// FramebufferAttachment and Framebuffer for how they are applied to an FBO via an
// attachment point.

class Renderbuffer : public RefCountObject
{
  public:
    Renderbuffer(rx::RenderbufferImpl *impl, GLuint id);
    virtual ~Renderbuffer();

    void setImpl(rx::RenderbufferImpl *newImpl);
    rx::RenderbufferImpl *getImpl();

    GLsizei getWidth() const;
    GLsizei getHeight() const;
    GLenum getInternalFormat() const;
    GLenum getActualFormat() const;
    GLsizei getSamples() const;
    GLuint getRedSize() const;
    GLuint getGreenSize() const;
    GLuint getBlueSize() const;
    GLuint getAlphaSize() const;
    GLuint getDepthSize() const;
    GLuint getStencilSize() const;

  private:
    rx::RenderbufferImpl *mRenderbuffer;
};

}

#endif   // LIBGLESV2_RENDERBUFFER_H_
