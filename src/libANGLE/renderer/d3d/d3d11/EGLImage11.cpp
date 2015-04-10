//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLImage11.h: Implements the rx::EGLImage11 class, the D3D11 implementation of EGL images

#include "libANGLE/renderer/d3d/d3d11/EGLImage11.h"

#include "common/debug.h"
#include "common/utilities.h"
#include "libANGLE/AttributeMap.h"
#include "libANGLE/Texture.h"
#include "libANGLE/renderer/d3d/TextureD3D.h"

#include <EGL/eglext.h>

namespace rx
{

EGLImage11::EGLImage11(EGLenum target, gl::Texture *buffer, const egl::AttributeMap &attribs)
    : mBuffer(nullptr),
      mTarget(egl_gl::EGLImageTargetToGLTextureTarget(target)),
      mMipLevel(attribs.get(EGL_GL_TEXTURE_LEVEL_KHR, 0)),
      mLayer(attribs.get(EGL_GL_TEXTURE_ZOFFSET_KHR, 0)),
      mRTV(nullptr),
      mSRV(nullptr)
{
    ASSERT(mTarget == GL_TEXTURE_2D);

    mBuffer = GetImplAs<TextureD3D>(buffer);
}

EGLImage11::~EGLImage11()
{

}

egl::Error EGLImage11::initialize()
{
    TextureStorage *storage = nullptr;
    gl::Error error = mBuffer->getNativeTexture(&storage);
    if (error.isError())
    {
        ASSERT(error.getCode() == GL_OUT_OF_MEMORY);
        return egl::Error(EGL_BAD_ALLOC, error.getMessage().c_str());
    }

    return egl::Error(EGL_SUCCESS);
}

void EGLImage11::reference(gl::Texture *sibling)
{

}

void EGLImage11::orphan(gl::Texture *sibling)
{

}

}
