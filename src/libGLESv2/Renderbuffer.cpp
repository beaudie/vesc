//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Renderbuffer.cpp: the gl::Renderbuffer class and its derived classes
// Colorbuffer, Depthbuffer and Stencilbuffer. Implements GL renderbuffer
// objects and related functionality. [OpenGL ES 2.0.24] section 4.4.3 page 108.

#include "libGLESv2/Renderbuffer.h"
#include "libGLESv2/Texture.h"
#include "libGLESv2/formatutils.h"
#include "libGLESv2/FramebufferAttachment.h"
#include "libGLESv2/renderer/Renderer.h"
#include "libGLESv2/renderer/RenderTarget.h"
#include "libGLESv2/renderer/RenderbufferImpl.h"

#include "common/utilities.h"

namespace gl
{
Renderbuffer::Renderbuffer(rx::RenderbufferImpl *impl, GLuint id)
  : RefCountObject(id),
    mRenderbuffer(impl)
{
    ASSERT(mRenderbuffer);
}

Renderbuffer::~Renderbuffer()
{
    SafeDelete(mRenderbuffer);
}

void Renderbuffer::setImpl(rx::RenderbufferImpl *newImpl)
{
    ASSERT(newImpl);

    SafeDelete(mRenderbuffer);
    mRenderbuffer = newImpl;
}

rx::RenderbufferImpl *Renderbuffer::getImpl()
{
    ASSERT(mRenderbuffer);
    return mRenderbuffer;
}

GLsizei Renderbuffer::getWidth() const
{
    ASSERT(mRenderbuffer);
    return mRenderbuffer->getWidth();
}

GLsizei Renderbuffer::getHeight() const
{
    ASSERT(mRenderbuffer);
    return mRenderbuffer->getHeight();
}

GLenum Renderbuffer::getInternalFormat() const
{
    ASSERT(mRenderbuffer);
    return mRenderbuffer->getInternalFormat();
}

GLenum Renderbuffer::getActualFormat() const
{
    ASSERT(mRenderbuffer);
    return mRenderbuffer->getActualFormat();
}

GLsizei Renderbuffer::getSamples() const
{
    ASSERT(mRenderbuffer);
    return mRenderbuffer->getSamples();
}

GLuint Renderbuffer::getRedSize() const
{
    return GetInternalFormatInfo(getActualFormat()).redBits;
}

GLuint Renderbuffer::getGreenSize() const
{
    return GetInternalFormatInfo(getActualFormat()).greenBits;
}

GLuint Renderbuffer::getBlueSize() const
{
    return GetInternalFormatInfo(getActualFormat()).blueBits;
}

GLuint Renderbuffer::getAlphaSize() const
{
    return GetInternalFormatInfo(getActualFormat()).alphaBits;
}

GLuint Renderbuffer::getDepthSize() const
{
    return GetInternalFormatInfo(getActualFormat()).depthBits;
}

GLuint Renderbuffer::getStencilSize() const
{
    return GetInternalFormatInfo(getActualFormat()).stencilBits;
}

}
