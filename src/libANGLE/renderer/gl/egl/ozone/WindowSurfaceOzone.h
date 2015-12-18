//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WindowSurfaceOzone.h: Ozone implementation of egl::Surface for windows

#ifndef LIBANGLE_RENDERER_GL_OZONE_WINDOWSURFACEOZONE_H_
#define LIBANGLE_RENDERER_GL_OZONE_WINDOWSURFACEOZONE_H_

#include "libANGLE/renderer/gl/SurfaceGL.h"
#include "libANGLE/renderer/gl/egl/ozone/DisplayOzone.h"

namespace rx
{

class WindowSurfaceOzone : public SurfaceGL
{
  public:
    WindowSurfaceOzone(RendererGL *renderer, DisplayOzone::Buffer *buffer);
    ~WindowSurfaceOzone() override;

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
    DisplayOzone::Buffer *mBuffer;

    // TODO(fjhenigman) Implement swap control.  This will be used for that.
    SwapControlData mSwapControl;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_OZONE_WINDOWSURFACEOZONE_H_
