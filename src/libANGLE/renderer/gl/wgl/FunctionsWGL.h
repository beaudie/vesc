// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FunctionsWGL.h: Defines a WGL function loader class.

#ifndef LIBANGLE_RENDERER_GL_WGL_FUNCTIONSWGL_H_
#define LIBANGLE_RENDERER_GL_WGL_FUNCTIONSWGL_H_

#include "common/platform.h"

namespace rx
{

class FunctionsWGL
{
  public:
    FunctionsWGL();
    ~FunctionsWGL();

    PFNWGLSWAPINTERVALEXTPROC SwapIntervalEXT;
    PFNWGLGETPIXELFORMATATTRIBIVARBPROC GetPixelFormatAttribivARB;
    PFNWGLGETEXTENSIONSSTRINGEXTPROC GetExtensionsStringEXT;
    PFNWGLGETEXTENSIONSSTRINGARBPROC GetExtensionsStringARB;
    PFNWGLCREATECONTEXTATTRIBSARBPROC CreateContextAttribsARB;

  private:
    HMODULE mOpenGLModule;
};

}

#endif // LIBANGLE_RENDERER_GL_WGL_FUNCTIONSWGL_H_
