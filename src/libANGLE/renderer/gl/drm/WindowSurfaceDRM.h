//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WindowSurfaceDRM.h: DRM implementation of egl::Surface for windows

#ifndef LIBANGLE_RENDERER_GL_DRM_WINDOWSURFACEDRM_H_
#define LIBANGLE_RENDERER_GL_DRM_WINDOWSURFACEDRM_H_

#include "libANGLE/renderer/gl/SurfaceGL.h"
#include "libANGLE/renderer/gl/drm/DisplayDRM.h"
#include "libANGLE/renderer/gl/drm/platform_drm.h"

namespace rx
{

class DisplayDRM;
class FunctionsDRM;

class WindowSurfaceDRM : public SurfaceGL
{
  public:
    WindowSurfaceDRM(const FunctionsDRM &drm,
                     DisplayDRM *drmDisplay,
                     RendererGL *renderer,
                     Window window,
                     EGLDisplay display,
                     drm::Context context,
                     drm::FBConfig fbConfig);
    ~WindowSurfaceDRM() override;

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
    bool getWindowDimensions(Window window, unsigned int *width, unsigned int *height) const;

    Window mParent;
    unsigned int mParentWidth, mParentHeight;
    Window mWindow;
    EGLDisplay mDisplay;

    const FunctionsDRM &mDRM;
    DisplayDRM *mDRMDisplay;

    drm::Context mContext;
    drm::FBConfig mFBConfig;
    drm::Window mDRMWindow;

    SwapControlData mSwapControl;
};

}

#endif // LIBANGLE_RENDERER_GL_DRM_WINDOWSURFACEDRM_H_
