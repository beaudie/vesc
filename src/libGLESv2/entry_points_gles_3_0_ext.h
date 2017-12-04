//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// entry_points_gles_3_0_ext.h:
//   Defines the GLES 3.0 extention entry points.

#ifndef LIBGLESV2_ENTRYPOINTGLES30EXT_H_
#define LIBGLESV2_ENTRYPOINTGLES30EXT_H_

#include <GLES3/gl3.h>
#include <GLES3/gl3ext_angle.h>
#include <export.h>

#include "common/platform.h"

namespace gl
{
ANGLE_EXPORT void GL_APIENTRY TexStorage2DMultisampleANGLE(GLenum target,
                                                           GLsizei samples,
                                                           GLenum internalformat,
                                                           GLsizei width,
                                                           GLsizei height,
                                                           GLboolean fixedsamplelocations);
ANGLE_EXPORT void GL_APIENTRY GetMultisamplefvANGLE(GLenum pname, GLuint index, GLfloat *val);
ANGLE_EXPORT void GL_APIENTRY SampleMaskiANGLE(GLuint maskNumber, GLbitfield mask);
}  // namespace gl

#endif  // LIBGLESV2_ENTRYPOINTGLES30EXT_H_
