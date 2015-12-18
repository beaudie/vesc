//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WindowSurfaceCros.h: surfaceless implementation of egl::Surface for windows

#ifndef LIBANGLE_RENDERER_GL_CROS_WINDOWSURFACECROS_H_
#define LIBANGLE_RENDERER_GL_CROS_WINDOWSURFACECROS_H_

#include "libANGLE/renderer/gl/SurfaceGL.h"
#include "libANGLE/renderer/gl/cros/DisplayDRM.h"

namespace rx
{

class FunctionsEGL;
class StateManagerGL;
struct WorkaroundsGL;

class WindowSurfaceCros : public SurfaceGL
{
  public:
    WindowSurfaceCros(RendererGL *renderer, DisplayDRM::Buffer *buffer);
    ~WindowSurfaceCros() override;

    FramebufferImpl *createDefaultFramebuffer(const gl::Framebuffer::Data &data) override;

    egl::Error initialize() override;
    egl::Error makeCurrent() override;

    egl::Error swap() override;
    egl::Error postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height) override;
    egl::Error querySurfacePointerANGLE(EGLint attribute, void **value) override;
    egl::Error bindTexImage(gl::Texture *texture, EGLint buffer) override;
    egl::Error releaseTexImage(EGLint buffer) override;
    void setSwapInterval(EGLint interval) override;

    EGLint getWidth() const override;
    EGLint getHeight() const override;

    EGLint isPostSubBufferSupported() const override;
    EGLint getSwapBehavior() const override;

  private:
    DisplayDRM::Buffer *mBuffer;
    SwapControlData mSwapControl;
};

}

#endif // LIBANGLE_RENDERER_GL_CROS_WINDOWSURFACECROS_H_
