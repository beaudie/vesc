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

#include "common/utilities.h"

namespace gl
{
unsigned int RenderbufferStorage::mCurrentSerial = 1;

Renderbuffer::Renderbuffer(GLuint id, RenderbufferStorage *newStorage)
  : RefCountObject(id),
    mStorage(newStorage)
{
    ASSERT(mStorage);
}

Renderbuffer::~Renderbuffer()
{
    SafeDelete(mStorage);
}

void Renderbuffer::setStorage(RenderbufferStorage *newStorage)
{
    ASSERT(newStorage);

    SafeDelete(mStorage);
    mStorage = newStorage;
}

RenderbufferStorage *Renderbuffer::getStorage()
{
    ASSERT(mStorage);
    return mStorage;
}

GLsizei Renderbuffer::getWidth() const
{
    ASSERT(mStorage);
    return mStorage->getWidth();
}

GLsizei Renderbuffer::getHeight() const
{
    ASSERT(mStorage);
    return mStorage->getHeight();
}

GLenum Renderbuffer::getInternalFormat() const
{
    ASSERT(mStorage);
    return mStorage->getInternalFormat();
}

GLenum Renderbuffer::getActualFormat() const
{
    ASSERT(mStorage);
    return mStorage->getActualFormat();
}

GLsizei Renderbuffer::getSamples() const
{
    ASSERT(mStorage);
    return mStorage->getSamples();
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

RenderbufferStorage::RenderbufferStorage(rx::RenderTarget *renderTarget)
    : mSerial(issueSerials(1)),
      mRenderTarget(renderTarget)
{
    ASSERT(mRenderTarget);
}

RenderbufferStorage::~RenderbufferStorage()
{
    SafeDelete(mRenderTarget);
}

rx::RenderTarget *RenderbufferStorage::getRenderTarget()
{
    return mRenderTarget;
}

GLsizei RenderbufferStorage::getWidth() const
{
    return mRenderTarget->getWidth();
}

GLsizei RenderbufferStorage::getHeight() const
{
    return mRenderTarget->getHeight();
}

GLenum RenderbufferStorage::getInternalFormat() const
{
    return mRenderTarget->getInternalFormat();
}

GLenum RenderbufferStorage::getActualFormat() const
{
    return mRenderTarget->getActualFormat();
}

GLsizei RenderbufferStorage::getSamples() const
{
    return mRenderTarget->getSamples();
}

unsigned int RenderbufferStorage::getSerial() const
{
    return mSerial;
}

unsigned int RenderbufferStorage::issueSerials(unsigned int count)
{
    unsigned int firstSerial = mCurrentSerial;
    mCurrentSerial += count;
    return firstSerial;
}

bool RenderbufferStorage::isTexture() const
{
    return false;
}

unsigned int RenderbufferStorage::getTextureSerial() const
{
    return -1;
}

}
