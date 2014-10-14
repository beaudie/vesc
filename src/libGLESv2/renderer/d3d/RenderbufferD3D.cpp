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
RenderbufferD3D::RenderbufferD3D(RenderTarget *renderTarget, GLenum requestedFormat)
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
    if (mRenderTarget)
    {
        delete mRenderTarget;
    }
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

void RenderbufferD3D::invalidate(GLint x, GLint y, GLsizei width, GLsizei height)
{
    ASSERT(mRenderTarget);
    mRenderTarget->invalidate(x, y, width, height);
}

}
