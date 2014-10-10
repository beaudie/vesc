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

RenderbufferD3D::RenderbufferD3D(RenderTarget *renderTarget, GLenum requestedFormat) : mSerial(issueSerials(1))
{
    mRenderTarget = renderTarget;

    if (mRenderTarget)
    {
        mWidth = mRenderTarget->getWidth();
        mHeight = mRenderTarget->getHeight();
        mInternalFormat = requestedFormat;
        mActualFormat = mRenderTarget->getActualFormat();
        mSamples = mRenderTarget->getSamples();
    }
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
