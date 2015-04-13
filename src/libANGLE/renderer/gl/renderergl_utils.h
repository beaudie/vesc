//
// Copyright (c) 2012-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// renderer11_utils.h: Conversion functions and other utility routines
// specific to the OpenGL renderer.

#ifndef LIBANGLE_RENDERER_GL_RENDERERGLUTILS_H_
#define LIBANGLE_RENDERER_GL_RENDERERGLUTILS_H_

#include "libANGLE/renderer/gl/functionsgl_typedefs.h"

#include <vector>

namespace gl
{
struct Caps;
class TextureCapsMap;
struct Extensions;
class Error;
}

namespace rx
{
class FunctionsGL;

namespace nativegl
{

void GetGLVersion(PFNGLGETSTRINGPROC getStringFunction, GLuint *outMajorVersion, GLuint *outMinorVersion, bool *outIsES);
std::vector<std::string> GetGLExtensions(PFNGLGETSTRINGPROC getStringFunction);

// Check for GL_OUT_OF_MEMORY errors only, should be called after GL functions that may allocate memory
inline gl::Error CheckForGLOutOfMemoryError(const FunctionsGL *functions);

}

namespace nativegl_gl
{

void GenerateCaps(const FunctionsGL *functions, gl::Caps *caps, gl::TextureCapsMap *textureCapsMap,
                  gl::Extensions *extensions);

}

}

#include "renderergl_utils.inl"

#endif // LIBANGLE_RENDERER_GL_RENDERERGLUTILS_H_
