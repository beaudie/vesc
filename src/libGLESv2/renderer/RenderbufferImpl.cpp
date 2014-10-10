//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderbufferImpl.h: Implements the shared methods of the abstract class gl::RenderbufferImpl

#include "libGLESv2/renderer/RenderbufferImpl.h"

namespace rx
{
RenderbufferImpl::RenderbufferImpl()
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

}
