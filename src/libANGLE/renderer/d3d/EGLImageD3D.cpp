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
#include "libANGLE/renderer/d3d/d3d11/RenderTarget11.h"

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

EGLImageD3D::EGLImageD3D(RendererD3D *renderer, EGLenum target, gl::Texture *buffer, const egl::AttributeMap &attribs)
    : mRenderer(renderer),
      mTextureBuffer(GetImplAs<TextureD3D>(buffer)),
      mTextureImageIndex(getImageIndex(egl_gl::EGLImageTargetToGLTextureTarget(target),
                                       attribs.get(EGL_GL_TEXTURE_LEVEL_KHR, 0),
                                       attribs.get(EGL_GL_TEXTURE_ZOFFSET_KHR, 0))),
      mRenderbufferBuffer(nullptr),
      mRenderTarget(nullptr)
{
}

EGLImageD3D::~EGLImageD3D()
{
}

egl::Error EGLImageD3D::initialize()
{
    return egl::Error(EGL_SUCCESS);
}

void EGLImageD3D::reference(gl::Texture *sibling)
{
}

void EGLImageD3D::orphan(gl::Texture *sibling)
{
    TextureD3D *siblingTextureD3D = GetImplAs<TextureD3D>(sibling);
    if (siblingTextureD3D == mTextureBuffer)
    {
        copyToLocalRendertarget();
    }
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
        *outRT = mRenderTarget;
        return gl::Error(GL_NO_ERROR);
    }
}

gl::Error EGLImageD3D::copyToLocalRendertarget()
{
    ASSERT(mTextureBuffer != nullptr || mRenderbufferBuffer != nullptr);
    ASSERT(mRenderTarget == nullptr);

    RenderTargetD3D *curRenderTarget = nullptr;
    gl::Error error = getRenderTarget(&curRenderTarget);
    if (error.isError())
    {
        return error;
    }

    return mRenderer->createRenderTargetCopy(curRenderTarget, &mRenderTarget);
}

}
