//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderTarget11.h: Defines a DX11-specific wrapper for ID3D11View pointers
// retained by Renderbuffers.

#ifndef LIBGLESV2_RENDERER_RENDERTARGET11_H_
#define LIBGLESV2_RENDERER_RENDERTARGET11_H_

#include "libGLESv2/renderer/RenderTarget.h"

namespace rx
{
class SwapChain11;

class RenderTarget11 : public RenderTarget
{
  public:
    RenderTarget11();
    virtual ~RenderTarget11();

    static RenderTarget11 *makeRenderTarget11(RenderTarget *renderTarget);

    virtual ID3D11Resource *getTexture() const = 0;
    virtual ID3D11RenderTargetView *getRenderTargetView() const = 0;
    virtual ID3D11DepthStencilView *getDepthStencilView() const = 0;
    virtual ID3D11ShaderResourceView *getShaderResourceView() const = 0;

    virtual unsigned int getSubresourceIndex() const = 0;

  private:
    DISALLOW_COPY_AND_ASSIGN(RenderTarget11);
};

class ResourceRenderTarget11 : public RenderTarget11
{
  public:
    // ResourceRenderTarget11 takes ownership of any D3D11 resources it is given and will AddRef them
    ResourceRenderTarget11(ID3D11RenderTargetView *rtv, ID3D11Resource *resource, ID3D11ShaderResourceView *srv,
                           GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLsizei samples);
    ResourceRenderTarget11(ID3D11DepthStencilView *dsv, ID3D11Resource *resource, ID3D11ShaderResourceView *srv,
                           GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLsizei samples);

    virtual ~ResourceRenderTarget11();

    virtual GLsizei getWidth() const { return mWidth; }
    virtual GLsizei getHeight() const { return mHeight; }
    virtual GLsizei getDepth() const { return mDepth; }
    virtual GLenum getInternalFormat() const { return mInternalFormat; }
    virtual GLenum getActualFormat() const { return mActualFormat; }
    virtual GLsizei getSamples() const { return mSamples; }

    virtual void invalidate(GLint x, GLint y, GLsizei width, GLsizei height);

    virtual ID3D11Resource *getTexture() const;
    virtual ID3D11RenderTargetView *getRenderTargetView() const;
    virtual ID3D11DepthStencilView *getDepthStencilView() const;
    virtual ID3D11ShaderResourceView *getShaderResourceView() const;

    virtual unsigned int getSubresourceIndex() const;

  private:
    DISALLOW_COPY_AND_ASSIGN(ResourceRenderTarget11);

    GLsizei mWidth;
    GLsizei mHeight;
    GLsizei mDepth;
    GLenum mInternalFormat;
    GLenum mActualFormat;
    GLsizei mSamples;

    unsigned int mSubresourceIndex;
    ID3D11Resource *mTexture;
    ID3D11RenderTargetView *mRenderTarget;
    ID3D11DepthStencilView *mDepthStencil;
    ID3D11ShaderResourceView *mShaderResource;
};

class SwapChainRenderTarget11 : public RenderTarget11
{
  public:
    SwapChainRenderTarget11(SwapChain11 *swapChain, bool depth);

    virtual ~SwapChainRenderTarget11();

    virtual GLsizei getWidth() const;
    virtual GLsizei getHeight() const;
    virtual GLsizei getDepth() const;
    virtual GLenum getInternalFormat() const;
    virtual GLenum getActualFormat() const;
    virtual GLsizei getSamples() const;

    virtual void invalidate(GLint x, GLint y, GLsizei width, GLsizei height);

    virtual ID3D11Resource *getTexture() const;
    virtual ID3D11RenderTargetView *getRenderTargetView() const;
    virtual ID3D11DepthStencilView *getDepthStencilView() const;
    virtual ID3D11ShaderResourceView *getShaderResourceView() const;

    virtual unsigned int getSubresourceIndex() const;

  private:
    DISALLOW_COPY_AND_ASSIGN(SwapChainRenderTarget11);

    SwapChain11 *mSwapChain;
    bool mDepth;
};

}

#endif // LIBGLESV2_RENDERER_RENDERTARGET11_H_
