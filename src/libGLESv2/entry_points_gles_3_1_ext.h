//
// Copyright(c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// entry_points_gles_3_1_ext.h : Defines the GLES 3.1 extension entry points.

#ifndef LIBGLESV2_ENTRYPOINTGLES31EXT_H_
#define LIBGLESV2_ENTRYPOINTGLES31EXT_H_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <export.h>

namespace gl
{

// GL_EXT_geometry_shader
ANGLE_EXPORT void GL_APIENTRY FramebufferTextureEXT(GLenum target,
                                                    GLenum attachment,
                                                    GLuint texture,
                                                    GLint level);

}  // namespace gl

#endif  // LIBGLESV2_ENTRYPOINTGLES31EXT_H_