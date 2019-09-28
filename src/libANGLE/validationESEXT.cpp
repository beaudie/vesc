//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// validationEXT.cpp: Validation functions for OpenGL ES extension entry points.

#include "libANGLE/validationESEXT.h"

namespace gl
{
// GL_QCOM_extended_get
bool ValidateExtGetBufferPointervQCOM(Context *context, GLenum target, void **params)
{
    return true;
}

bool ValidateExtGetBuffersQCOM(Context *context,
                               GLuint *buffers,
                               GLint maxBuffers,
                               GLint *numBuffers)
{
    return true;
}

bool ValidateExtGetFramebuffersQCOM(Context *context,
                                    GLuint *framebuffers,
                                    GLint maxFramebuffers,
                                    GLint *numFramebuffers)
{
    return true;
}

bool ValidateExtGetRenderbuffersQCOM(Context *context,
                                     GLuint *renderbuffers,
                                     GLint maxRenderbuffers,
                                     GLint *numRenderbuffers)
{
    return true;
}

bool ValidateExtGetTexLevelParameterivQCOM(Context *context,
                                           GLuint texture,
                                           GLenum face,
                                           GLint level,
                                           GLenum pname,
                                           GLint *params)
{
    return true;
}

bool ValidateExtGetTexSubImageQCOM(Context *context,
                                   GLenum target,
                                   GLint level,
                                   GLint xoffset,
                                   GLint yoffset,
                                   GLint zoffset,
                                   GLsizei width,
                                   GLsizei height,
                                   GLsizei depth,
                                   GLenum format,
                                   GLenum type,
                                   void *texels)
{
    return true;
}

bool ValidateExtGetTexturesQCOM(Context *context,
                                GLuint *textures,
                                GLint maxTextures,
                                GLint *numTextures)
{
    return true;
}

bool ValidateExtTexObjectStateOverrideiQCOM(Context *context,
                                            GLenum target,
                                            GLenum pname,
                                            GLint param)
{
    return true;
}
}  // namespace gl
