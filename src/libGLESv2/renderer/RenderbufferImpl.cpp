//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderbufferImpl.h: Implements the shared methods of the abstract class gl::RenderbufferImpl

#include "libGLESv2/renderer/RenderbufferImpl.h"

namespace rx
{
unsigned int RenderbufferImpl::mCurrentSerial = 1;

RenderbufferImpl::RenderbufferImpl() : mSerial(issueSerials(1))
{
    mWidth = 0;
    mHeight = 0;
    mInternalFormat = GL_RGBA4;
    mActualFormat = GL_RGBA8_OES;
    mSamples = 0;
}

RenderbufferImpl::~RenderbufferImpl()
{
}

rx::RenderTarget *RenderbufferImpl::getRenderTarget()
{
    return NULL;
}

GLsizei RenderbufferImpl::getWidth() const
{
    return mWidth;
}

GLsizei RenderbufferImpl::getHeight() const
{
    return mHeight;
}

GLenum RenderbufferImpl::getInternalFormat() const
{
    return mInternalFormat;
}

GLenum RenderbufferImpl::getActualFormat() const
{
    return mActualFormat;
}

GLsizei RenderbufferImpl::getSamples() const
{
    return mSamples;
}

unsigned int RenderbufferImpl::getSerial() const
{
    return mSerial;
}

unsigned int RenderbufferImpl::issueSerials(unsigned int count)
{
    unsigned int firstSerial = mCurrentSerial;
    mCurrentSerial += count;
    return firstSerial;
}

bool RenderbufferImpl::isTexture() const
{
    return false;
}

unsigned int RenderbufferImpl::getTextureSerial() const
{
    return -1;
}
}
