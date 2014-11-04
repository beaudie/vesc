//
// Copyright (c) 2012-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderTarget9.cpp: Implements a D3D9-specific wrapper for IDirect3DSurface9
// pointers retained by renderbuffers.

#include "libGLESv2/renderer/d3d/d3d9/RenderTarget9.h"
#include "libGLESv2/renderer/d3d/d3d9/Renderer9.h"
#include "libGLESv2/renderer/d3d/d3d9/renderer9_utils.h"
#include "libGLESv2/renderer/d3d/d3d9/SwapChain9.h"
#include "libGLESv2/renderer/d3d/d3d9/formatutils9.h"
#include "libGLESv2/main.h"

namespace rx
{

RenderTarget9::RenderTarget9()
{
}

RenderTarget9::~RenderTarget9()
{
}

RenderTarget9 *RenderTarget9::makeRenderTarget9(RenderTarget *target)
{
    ASSERT(HAS_DYNAMIC_TYPE(rx::RenderTarget9*, target));
    return static_cast<rx::RenderTarget9*>(target);
}

void RenderTarget9::invalidate(GLint x, GLint y, GLsizei width, GLsizei height)
{
        // Currently a no-op
}

// TODO: AddRef the incoming surface to take ownership instead of expecting that its ref is being given.
SurfaceRenderTarget9::SurfaceRenderTarget9(IDirect3DSurface9 *surface, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth,
                                           GLsizei samples)
    : mWidth(width),
      mHeight(height),
      mDepth(1),
      mInternalFormat(internalFormat),
      mActualFormat(internalFormat),
      mSamples(samples),
      mRenderTarget(surface)
{
    if (mRenderTarget)
    {
        D3DSURFACE_DESC description;
        mRenderTarget->GetDesc(&description);

        const d3d9::D3DFormat &d3dFormatInfo = d3d9::GetD3DFormatInfo(description.Format);
        mActualFormat = d3dFormatInfo.internalFormat;
    }
}

SurfaceRenderTarget9::~SurfaceRenderTarget9()
{
    SafeRelease(mRenderTarget);
}

IDirect3DSurface9 *SurfaceRenderTarget9::getSurface()
{
    // Caller is responsible for releasing the returned surface reference.
    // TODO: remove the AddRef to match RenderTarget11
    if (mRenderTarget)
    {
        mRenderTarget->AddRef();
    }

    return mRenderTarget;
}


SwapChainRenderTarget9::SwapChainRenderTarget9(SwapChain9 *swapChain, bool depth)
    : mSwapChain(swapChain),
      mDepth(depth)
{
}

SwapChainRenderTarget9::~SwapChainRenderTarget9()
{
}

GLsizei SwapChainRenderTarget9::getWidth() const
{
    return mSwapChain->getWidth();
}

GLsizei SwapChainRenderTarget9::getHeight() const
{
    return mSwapChain->getHeight();
}

GLsizei SwapChainRenderTarget9::getDepth() const
{
    return 1;
}

GLenum SwapChainRenderTarget9::getInternalFormat() const
{
    return (mDepth ? mSwapChain->GetDepthBufferInternalFormat() : mSwapChain->GetBackBufferInternalFormat());
}

GLenum SwapChainRenderTarget9::getActualFormat() const
{
    return d3d9::GetD3DFormatInfo(d3d9::GetTextureFormatInfo(getInternalFormat()).texFormat).internalFormat;
}

GLsizei SwapChainRenderTarget9::getSamples() const
{
    return 0;
}

IDirect3DSurface9 *SwapChainRenderTarget9::getSurface()
{
    return (mDepth ? mSwapChain->getDepthStencil() : mSwapChain->getRenderTarget());
}

}
