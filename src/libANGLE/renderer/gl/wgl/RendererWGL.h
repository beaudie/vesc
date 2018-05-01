//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RendererWGL.h: Renderer class for GL on Windows.  Owns a WGL context object.

#ifndef LIBANGLE_RENDERER_GL_WGL_RENDERERWGL_H_
#define LIBANGLE_RENDERER_GL_WGL_RENDERERWGL_H_

#include "libANGLE/renderer/gl/RendererGL.h"
#include "libANGLE/renderer/gl/wgl/FunctionsWGL.h"

namespace rx
{
class RendererWGL : public RendererGL
{
  public:
    RendererWGL(const FunctionsGL *functionsGL,
                const egl::AttributeMap &attribMap,
                const FunctionsWGL *functionsWGL,
                HGLRC context);
    ~RendererWGL() override;

    HGLRC getContext() const;

  private:
    const FunctionsWGL *mFunctionsWGL;
    HGLRC mContext;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_WGL_RENDERERWGL_H_
