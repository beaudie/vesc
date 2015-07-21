//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLImageD3D.cpp: Implements the rx::EGLImageD3D class, the D3D implementation of EGL images

#include "libANGLE/renderer/d3d/EGLImageD3D.h"

#include "common/debug.h"
#include "common/utilities.h"
#include "libANGLE/AttributeMap.h"
#include "libANGLE/Texture.h"
#include "libANGLE/renderer/d3d/RenderbufferD3D.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"
#include "libANGLE/renderer/d3d/RenderTargetD3D.h"
#include "libANGLE/renderer/d3d/TextureD3D.h"
#include "libANGLE/renderer/d3d/TextureStorage.h"

#include <EGL/eglext.h>

namespace rx
{
static gl::ImageIndex getImageIndex(GLenum target, size_t mip, size_t layer)
{
    if (target == GL_TEXTURE_3D)
    {
        return gl::ImageIndex::Make3D(mip, layer);
    }
    else
    {
        ASSERT(layer == 0);
        return gl::ImageIndex::MakeGeneric(target, mip);
    }
}

EGLImageD3D::EGLImageD3D(RendererD3D *renderer,
                         EGLenum target,
                         egl::ImageSibling *buffer,
                         const egl::AttributeMap &attribs)
    : mRenderer(renderer),
      mBuffer(buffer),
      mTextureBuffer(nullptr),
      mTextureImageIndex(gl::ImageIndex::MakeInvalid()),
      mRenderbufferBuffer(nullptr),
      mRenderTarget(nullptr)
{
    ASSERT(renderer != nullptr);
    ASSERT(buffer != nullptr);

    if (target == EGL_GL_RENDERBUFFER)
    {
        mRenderbufferBuffer = GetImplAs<RenderbufferD3D>(GetAs<gl::Renderbuffer>(buffer));
    }
    else
    {
        mTextureBuffer     = GetImplAs<TextureD3D>(GetAs<gl::Texture>(buffer));
        mTextureImageIndex = getImageIndex(egl_gl::EGLImageTargetToGLTextureTarget(target),
                                           attribs.get(EGL_GL_TEXTURE_LEVEL_KHR, 0),
                                           attribs.get(EGL_GL_TEXTURE_ZOFFSET_KHR, 0));
    }
}

EGLImageD3D::~EGLImageD3D()
{
}

egl::Error EGLImageD3D::initialize()
{
    return egl::Error(EGL_SUCCESS);
}

gl::Error EGLImageD3D::orphan(egl::ImageSibling *sibling)
{
    if (sibling == mBuffer)
    {
        gl::Error error = copyToLocalRendertarget();
        if (error.isError())
        {
            return error;
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error EGLImageD3D::getRenderTarget(RenderTargetD3D **outRT) const
{
    if (mTextureBuffer)
    {
        return mTextureBuffer->getRenderTarget(mTextureImageIndex, outRT);
    }
    else if (mRenderbufferBuffer)
    {
        return mRenderbufferBuffer->getRenderTarget(outRT);
    }
    else
    {
        ASSERT(mRenderTarget);
        *outRT = mRenderTarget;
        return gl::Error(GL_NO_ERROR);
    }
}

gl::Error EGLImageD3D::copyToLocalRendertarget()
{
    ASSERT(mBuffer != nullptr);
    ASSERT(mTextureBuffer != nullptr || mRenderbufferBuffer != nullptr);
    ASSERT(mRenderTarget == nullptr);

    RenderTargetD3D *curRenderTarget = nullptr;
    gl::Error error = getRenderTarget(&curRenderTarget);
    if (error.isError())
    {
        return error;
    }

    // Clear the source image buffers
    mBuffer             = nullptr;
    mTextureBuffer      = nullptr;
    mRenderbufferBuffer = nullptr;

    return mRenderer->createRenderTargetCopy(curRenderTarget, &mRenderTarget);
}
}
