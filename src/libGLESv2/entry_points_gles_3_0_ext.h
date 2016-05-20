//
// Copyright(c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// entry_points_gles_3_0_ext.h : Dfeines the GLES 3.x extension entry points

#ifndef LIBGLESV2_ENTRYPOINTGLES3EXT_H
#define LIBGLESV2_ENTRYPOINTGLES3EXT_H

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <export.h>

namespace gl
{

// GL_EXT_blend_func_extended
ANGLE_EXPORT void GL_APIENTRY BindFragDataLocationIndexedEXT(GLuint program,
                                                             GLuint colorNumber,
                                                             GLuint index,
                                                             const GLchar* name);
ANGLE_EXPORT GLint GL_APIENTRY GetFragDataIndexEXT(GLuint program, const GLchar* name);
ANGLE_EXPORT void GL_APIENTRY BindFragDataLocationEXT(GLuint program, GLuint colorNumber, const GLchar* name);

} // gl

#endif // LIBGLESV2_ENTRYPOINTGLES3EXT_H