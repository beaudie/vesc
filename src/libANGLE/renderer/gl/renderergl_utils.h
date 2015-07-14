//
// Copyright (c) 2012-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// renderergl_utils.h: Conversion functions and other utility routines
// specific to the OpenGL renderer.

#ifndef LIBANGLE_RENDERER_GL_RENDERERGLUTILS_H_
#define LIBANGLE_RENDERER_GL_RENDERERGLUTILS_H_

#include "libANGLE/renderer/gl/functionsgl_typedefs.h"

#include <string>
#include <vector>

namespace gl
{
struct Caps;
class TextureCapsMap;
struct Extensions;
struct Version;
}

namespace rx
{
class FunctionsGL;
struct WorkaroundsGL;

namespace nativegl_gl
{

void GenerateCaps(const FunctionsGL *functions, gl::Caps *caps, gl::TextureCapsMap *textureCapsMap,
                  gl::Extensions *extensions, gl::Version *maxSupportedESVersion);

void GenerateWorkarounds(const FunctionsGL *functions, WorkaroundsGL *workarounds);
}

namespace nativegl
{
bool AbleToReadbackBufferData(const FunctionsGL *functions);
bool AbleToUseVertexArrayObjects(const FunctionsGL *functions);

uint8_t *MapBuffer(const FunctionsGL *functions, GLenum binding, size_t offset, size_t size, GLbitfield usageBits);
bool UnmapBuffer(const FunctionsGL *functions, GLenum binding);
}

}

#endif // LIBANGLE_RENDERER_GL_RENDERERGLUTILS_H_
