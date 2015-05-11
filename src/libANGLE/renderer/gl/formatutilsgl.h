//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// formatutilsgl.h: Queries for GL image formats and their translations to native
// GL formats.

#ifndef LIBANGLE_RENDERER_GL_FORMATUTILSGL_H_
#define LIBANGLE_RENDERER_GL_FORMATUTILSGL_H_

#include <string>
#include <vector>

#include "angle_gl.h"

namespace rx
{

namespace nativegl
{

struct SupportRequirement
{
    SupportRequirement();

    GLuint majorVersion;
    GLuint minorVersion;
    std::vector<std::string> extensions;
};

struct InternalFormatRequirements
{
    InternalFormatRequirements();

    SupportRequirement texture;
    SupportRequirement filter;
    SupportRequirement renderbuffer;
    SupportRequirement framebufferAttachment;
};

struct InternalFormat
{
    InternalFormat();

    InternalFormatRequirements glSupport;
    InternalFormatRequirements glesSupport;

};
const InternalFormat &GetInternalFormatInfo(GLenum internalFormat);

}

}

#endif // LIBANGLE_RENDERER_GL_FORMATUTILSGL_H_
