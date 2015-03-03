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
}

namespace rx
{
class FunctionsGL;

namespace nativegl
{

void GetGLVersion(PFNGLGETSTRINGPROC getStringFunction, GLuint *outMajorVersion, GLuint *outMinorVersion, bool *outIsES);

// Reads the GL extensions by calling glGetString(GL_EXTENSIONS).  This method is deprecated in OpenGL core profile.
std::vector<std::string> GetNonIndexedExtensions(PFNGLGETSTRINGPROC getStringFunction);

// Read the GL extensions by calling glGetInteger(GL_NUM_EXTENSIONS) and glGetStringi(GL_EXTENSIONS).  This method
// is available as of OpenGL 3.0 and OpenGL ES 3.0.
std::vector<std::string> GetIndexedExtensions(PFNGLGETINTEGERVPROC getIntegerFunction, PFNGLGETSTRINGIPROC getStringIFunction);

// Reads the GL extensions by determining the best GL functions to call.
std::vector<std::string> GetGLExtensions(const FunctionsGL *functions);


}

namespace nativegl_gl
{

void GenerateCaps(const FunctionsGL *functions, gl::Caps *caps, gl::TextureCapsMap *textureCapsMap,
                  gl::Extensions *extensions);

}

}

#endif // LIBANGLE_RENDERER_GL_RENDERERGLUTILS_H_
