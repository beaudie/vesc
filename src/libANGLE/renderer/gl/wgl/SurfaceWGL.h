//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SurfaceWGL.h: WGL implementation of egl::Surface

#ifndef LIBANGLE_RENDERER_GL_WGL_SURFACEWGL_H_
#define LIBANGLE_RENDERER_GL_WGL_SURFACEWGL_H_

#include "libANGLE/renderer/gl/SurfaceGL.h"

namespace rx
{

class SurfaceWGL : public SurfaceGL
{
  public:
    SurfaceWGL(egl::Display *display, const egl::Config *config, EGLint width, EGLint height,
               EGLint fixedSize, EGLint postSubBufferSupported, EGLenum textureFormat,
               EGLenum textureType, EGLClientBuffer shareHandle);
    virtual ~SurfaceWGL();

    egl::Error initialize() override;

    egl::Error swap() override;
    egl::Error postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height) override;
    egl::Error querySurfacePointerANGLE(EGLint attribute, void **value) override;
    egl::Error bindTexImage(EGLint buffer) override;
    egl::Error releaseTexImage(EGLint buffer) override;
    void setSwapInterval(EGLint interval) override;

    EGLNativeWindowType getWindowHandle() const override;
};

}

#endif // LIBANGLE_RENDERER_GL_WGL_SURFACEWGL_H_
