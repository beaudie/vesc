//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RendererEGL.h: Renderer class for GL on EGL platforms.  Owns an EGL context object.

#ifndef LIBANGLE_RENDERER_GL_EGL_RENDEREREGL_H_
#define LIBANGLE_RENDERER_GL_EGL_RENDEREREGL_H_

#include "libANGLE/renderer/gl/RendererGL.h"
#include "libANGLE/renderer/gl/egl/FunctionsEGL.h"

namespace rx
{
class DisplayEGL;

class RendererEGL : public RendererGL
{
  public:
    RendererEGL(const FunctionsGL *functionsGL,
                const egl::AttributeMap &attribMap,
                DisplayEGL *display,
                EGLContext context);
    ~RendererEGL() override;

    EGLContext getContext() const;

  private:
    DisplayEGL *mDisplay;
    EGLContext mContext;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_EGL_RENDEREREGL_H_
