//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLImageD3D.h: Defines the rx::EGLImageD3D class, the D3D implementation of EGL images

#ifndef LIBANGLE_RENDERER_D3D_EGLIMAGED3D_H_
#define LIBANGLE_RENDERER_D3D_EGLIMAGED3D_H_

#include "libANGLE/ImageIndex.h"
#include "libANGLE/renderer/ImageImpl.h"

namespace egl
{
class AttributeMap;
}

namespace rx
{
class TextureD3D;
class RenderbufferD3D;
class RendererD3D;
class RenderTargetD3D;

class EGLImageD3D final : public ImageImpl
{
  public:
    EGLImageD3D(RendererD3D *renderer,
                EGLenum target,
                egl::ImageSibling *buffer,
                const egl::AttributeMap &attribs);
    virtual ~EGLImageD3D();

    egl::Error initialize() override;

    gl::Error orphan(egl::ImageSibling *sibling) override;

    gl::Error getRenderTarget(RenderTargetD3D **outRT) const;

  private:
    gl::Error copyToLocalRendertarget();

    RendererD3D *mRenderer;

    egl::ImageSibling *mBuffer;

    TextureD3D *mTextureBuffer;
    gl::ImageIndex mTextureImageIndex;

    RenderbufferD3D *mRenderbufferBuffer;

    RenderTargetD3D *mRenderTarget;
};
}

#endif  // LIBANGLE_RENDERER_D3D_EGLIMAGED3D_H_
