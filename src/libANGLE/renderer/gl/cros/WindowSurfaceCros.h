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

class DisplayDRM;
class FunctionsEGL;
class StateManagerGL;
struct WorkaroundsGL;

class WindowSurfaceCros : public SurfaceGL
{
  public:
    WindowSurfaceCros(const FunctionsEGL &egl,
                     DisplayDRM *drmDisplay,
                     RendererGL *renderer,
                     int width,
                     int height,
                     EGLImageKHR eglImage,
                     const FunctionsGL *functionsGL,
                     EGLDisplay display,
                     EGLContext context,
                     EGLConfig fbConfig);
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
#ifdef XXX
    bool getWindowDimensions(unsigned window, unsigned int *width, unsigned int *height) const;

    unsigned mParent;
    unsigned int mParentWidth, mParentHeight;
    unsigned mWindow;
#endif
    int mWidth, mHeight;
    EGLDisplay mDisplay;

    const FunctionsEGL &mEGL;
    DisplayDRM *mDRMDisplay;

    EGLContext mContext;
    EGLConfig mFBConfig;
    EGLImageKHR mEGLImage;
    const FunctionsGL *mFunctions;
    StateManagerGL *mStateManager;
    const WorkaroundsGL &mWorkarounds;
    GLuint mFramebuffer;

    SwapControlData mSwapControl;
};

}

#endif // LIBANGLE_RENDERER_GL_CROS_WINDOWSURFACECROS_H_
