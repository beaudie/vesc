//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderbufferD3d.cpp: Implements the RenderbufferD3D class, a specialization of RenderbufferImpl


#include "libGLESv2/renderer/d3d/RenderbufferD3D.h"

#include "libGLESv2/renderer/Renderer.h"
#include "libGLESv2/renderer/RenderTarget.h"

namespace rx
{
unsigned int RenderbufferD3D::mCurrentSerial = 1;

RenderbufferD3D::RenderbufferD3D(Renderer *renderer) : mSerial(issueSerials(1)), mRenderer(renderer)
{
    mRenderTarget = NULL;
}

RenderbufferD3D::~RenderbufferD3D()
{
    SafeDelete(mRenderTarget);
}

RenderbufferD3D *RenderbufferD3D::makeRenderbufferD3D(RenderbufferImpl *renderbuffer)
{
    ASSERT(HAS_DYNAMIC_TYPE(RenderbufferD3D*, renderbuffer));
    return static_cast<RenderbufferD3D*>(renderbuffer);
}

void RenderbufferD3D::setStorage(GLsizei width, GLsizei height, GLenum internalformat, GLsizei samples)
{
    // If the renderbuffer parameters are queried, the calling function
    // will expect one of the valid renderbuffer formats for use in
    // glRenderbufferStorage, but we should create depth and stencil buffers
    // as DEPTH24_STENCIL8
    GLenum creationFormat = internalformat;
    if (internalformat == GL_DEPTH_COMPONENT16 || internalformat == GL_STENCIL_INDEX8)
    {
        creationFormat = GL_DEPTH24_STENCIL8_OES;
    }

    RenderTarget *newRT = mRenderer->createRenderTarget(width, height, creationFormat, samples);
    SafeDelete(mRenderTarget);
    mRenderTarget = newRT;
}

void RenderbufferD3D::setStorage(SwapChain *swapChain, bool depth)
{
    RenderTarget *newRT = mRenderer->createRenderTarget(swapChain, depth);
    SafeDelete(mRenderTarget);
    mRenderTarget = newRT;
}

GLsizei RenderbufferD3D::getWidth() const
{
    return mRenderTarget->getWidth();
}

GLsizei RenderbufferD3D::getHeight() const
{
    return mRenderTarget->getHeight();
}

GLenum RenderbufferD3D::getInternalFormat() const
{
    return mRenderTarget->getInternalFormat();
}

GLenum RenderbufferD3D::getActualFormat() const
{
    return mRenderTarget->getActualFormat();
}

GLsizei RenderbufferD3D::getSamples() const
{
    return mRenderTarget->getSamples();
}

RenderTarget *RenderbufferD3D::getRenderTarget()
{
    return mRenderTarget;
}

unsigned int RenderbufferD3D::getSerial() const
{
    return mSerial;
}

unsigned int RenderbufferD3D::issueSerials(unsigned int count)
{
    unsigned int firstSerial = mCurrentSerial;
    mCurrentSerial += count;
    return firstSerial;
}

}
