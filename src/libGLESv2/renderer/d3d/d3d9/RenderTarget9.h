//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderTarget9.h: Defines a D3D9-specific wrapper for IDirect3DSurface9 pointers
// retained by Renderbuffers.

#ifndef LIBGLESV2_RENDERER_RENDERTARGET9_H_
#define LIBGLESV2_RENDERER_RENDERTARGET9_H_

#include "libGLESv2/renderer/RenderTarget.h"

namespace rx
{
class Renderer;
class Renderer9;
class SwapChain9;

class RenderTarget9 : public RenderTarget
{
  public:
    RenderTarget9();

    virtual ~RenderTarget9();

    static RenderTarget9 *makeRenderTarget9(RenderTarget *renderTarget);

    virtual IDirect3DSurface9 *getSurface() = 0;

  private:
    DISALLOW_COPY_AND_ASSIGN(RenderTarget9);
};

class SurfaceRenderTarget9 : public RenderTarget9
{
  public:
    SurfaceRenderTarget9(IDirect3DSurface9 *surface, GLenum internalFormat);

    virtual ~SurfaceRenderTarget9();

    virtual GLsizei getWidth() const { return mWidth; }
    virtual GLsizei getHeight() const { return mHeight; }
    virtual GLsizei getDepth() const { return mDepth; }
    virtual GLenum getInternalFormat() const { return mInternalFormat; }
    virtual GLenum getActualFormat() const { return mActualFormat; }
    virtual GLsizei getSamples() const { return mSamples; }

    virtual void invalidate(GLint x, GLint y, GLsizei width, GLsizei height);

    virtual IDirect3DSurface9 *getSurface();

  private:
    DISALLOW_COPY_AND_ASSIGN(SurfaceRenderTarget9);

    GLsizei mWidth;
    GLsizei mHeight;
    GLsizei mDepth;
    GLenum mInternalFormat;
    GLenum mActualFormat;
    GLsizei mSamples;

    IDirect3DSurface9 *mRenderTarget;
};

class SwapChainRenderTarget9 : public RenderTarget9
{
  public:
    SwapChainRenderTarget9(SwapChain9 *swapChain, bool depth);

    virtual ~SwapChainRenderTarget9();

    virtual GLsizei getWidth() const;
    virtual GLsizei getHeight() const;
    virtual GLsizei getDepth() const;
    virtual GLenum getInternalFormat() const;
    virtual GLenum getActualFormat() const;
    virtual GLsizei getSamples() const;

    virtual void invalidate(GLint x, GLint y, GLsizei width, GLsizei height);

    virtual IDirect3DSurface9 *getSurface();

  private:
    DISALLOW_COPY_AND_ASSIGN(SwapChainRenderTarget9);

    SwapChain9 *mSwapChain;
    bool mDepth;
};


}

#endif // LIBGLESV2_RENDERER_RENDERTARGET9_H_
