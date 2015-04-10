//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLImageD3D.h: Defines the rx::EGLImageD3D class, the D3D implementation of EGL images

#ifndef LIBANGLE_RENDERER_D3D_EGLIMAGED3D_H_
#define LIBANGLE_RENDERER_D3D_EGLIMAGED3D_H_

#include "libANGLE/renderer/ImageImpl.h"

namespace rx
{
class RenderTargetD3D;

class EGLImageD3D : public ImageImpl
{
  public:
    virtual ~EGLImageD3D();

    virtual RenderTargetD3D *getRenderTarget() const = 0;
};

}

#endif // LIBANGLE_RENDERER_D3D_EGLIMAGED3D_H_
