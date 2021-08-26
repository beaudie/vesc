//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayEGLSurfaceless.h: Surfaceless implementation of egl::DisplayEGL

#ifndef LIBANGLE_RENDERER_GL_EGL_DISPLAYEGLSURFACELESS_H_
#define LIBANGLE_RENDERER_GL_EGL_DISPLAYEGLSURFACELESS_H_

#include <string>

#include "libANGLE/renderer/gl/egl/DisplayEGL.h"

namespace rx
{

class DisplayEGLSurfaceless final : public DisplayEGL
{
  public:
    DisplayEGLSurfaceless(const egl::DisplayState &state);
    ~DisplayEGLSurfaceless() override;

    egl::Error initialize(egl::Display *display) override;

    SurfaceImpl *createWindowSurface(const egl::SurfaceState &state,
                                     EGLNativeWindowType window,
                                     const egl::AttributeMap &attribs) override;
    SurfaceImpl *createPbufferSurface(const egl::SurfaceState &state,
                                      const egl::AttributeMap &attribs) override;

    ContextImpl *createContext(const gl::State &state,
                               gl::ErrorSet *errorSet,
                               const egl::Config *configuration,
                               const gl::Context *shareContext,
                               const egl::AttributeMap &attribs) override;

    egl::Error makeCurrent(egl::Display *display,
                           egl::Surface *drawSurface,
                           egl::Surface *readSurface,
                           gl::Context *context) override;

    egl::ConfigSet generateConfigs() override;

  private:
    EGLint fixSurfaceType(EGLint surfaceType) const override;
    bool validateEglConfig(const EGLint *configAttribs);
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_EGL_DISPLAYEGLSURFACELESS_H_
