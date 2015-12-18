//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PBufferSurfaceDRM.h: DRM implementation of egl::Surface for PBuffers

#ifndef LIBANGLE_RENDERER_GL_DRM_PBUFFERSURFACEDRM_H_
#define LIBANGLE_RENDERER_GL_DRM_PBUFFERSURFACEDRM_H_

#include "libANGLE/renderer/gl/SurfaceGL.h"
#include "libANGLE/renderer/gl/drm/platform_drm.h"

namespace rx
{

class DisplayDRM;
class FunctionsDRM;

class PbufferSurfaceDRM : public SurfaceGL
{
  public:
    PbufferSurfaceDRM(RendererGL *renderer,
                      EGLint width,
                      EGLint height,
                      bool largest,
                      const FunctionsDRM &drm,
                      EGLContext context,
                      EGLConfig fbConfig);
    ~PbufferSurfaceDRM() override;

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
    unsigned mWidth;
    unsigned mHeight;
    bool mLargest;

    const FunctionsDRM &mDRM;
    EGLContext mContext;
    EGLConfig mFBConfig;
    EGLSurface mPbuffer;
};

}

#endif // LIBANGLE_RENDERER_GL_DRM_PBUFFERSURFACEDRM_H_
