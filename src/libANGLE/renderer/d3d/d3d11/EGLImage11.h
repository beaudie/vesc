//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLImage11.h: Defines the rx::EGLImage11 class, the D3D11 implementation of EGL images

#ifndef LIBANGLE_RENDERER_D3D_D3D11_EGLIMAGE11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_EGLIMAGE11_H_

#include "libANGLE/renderer/d3d/EGLImageD3D.h"

namespace egl
{
class AttributeMap;
}

namespace rx
{
class TextureD3D;

class EGLImage11 : public EGLImageD3D
{
  public:
    EGLImage11(EGLenum target, gl::Texture *buffer, const egl::AttributeMap &attribs);
    virtual ~EGLImage11();

    egl::Error initialize() override;

    void reference(gl::Texture *sibling) override;
    void orphan(gl::Texture *sibling) override;

    gl::Error getRenderTarget(RenderTargetD3D **outRT) const override;

  private:
    TextureD3D *mBuffer;
    GLenum mTarget;
    size_t mMipLevel;
    size_t mLayer;

    RenderTargetD3D *mRenderTarget;
};

}

#endif // LIBANGLE_RENDERER_D3D_D3D11_EGLIMAGE11_H_
