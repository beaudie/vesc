#ifndef __gl3ext_h_
#define __gl3ext_h_ 1

#ifdef __cplusplus
extern "C" {
#endif
//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// gl3ext_angle.h: ANGLE extensions for gles 3.0.
//

#include "gl3.h"

#ifndef GL_ANGLE_TEXTURE_MULTISAMPLE
#define GL_ANGLE_TEXTURE_MULTISAMPLE 1
#define GL_SAMPLE_POSITION_ANGLE 0x8E50
#define GL_SAMPLE_MASK_ANGLE 0x8E51
#define GL_SAMPLE_MASK_VALUE_ANGLE 0x8E52
#define GL_TEXTURE_2D_MULTISAMPLE_ANGLE 0x9100
#define GL_MAX_SAMPLE_MASK_WORDS_ANGLE 0x8E59
#define GL_MAX_COLOR_TEXTURE_SAMPLES_ANGLE 0x910E
#define GL_MAX_DEPTH_TEXTURE_SAMPLES_ANGLE 0x910F
#define GL_MAX_INTEGER_SAMPLES_ANGLE 0x9110
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE_ANGLE 0x9104
#define GL_TEXTURE_SAMPLES_ANGLE 0x9106
#define GL_TEXTURE_FIXED_SAMPLE_LOCATIONS_ANGLE 0x9107
typedef void(GL_APIENTRYP PFNGLTEXSTORAGE2DMULTISAMPLEANGLE)(GLenum target,
                                                             GLsizei samples,
                                                             GLenum internalformat,
                                                             GLsizei width,
                                                             GLsizei height,
                                                             GLboolean fixedsamplelocations);
typedef void(GL_APIENTRYP PFNGLGETMULTISAMPLEFVANGLE)(GLenum pname, GLuint index, GLfloat *val);
typedef void(GL_APIENTRYP PFNGLSAMPLEMASKIANGLE)(GLuint maskNumber, GLbitfield mask);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glTexStorage2DMultisampleANGLE(GLenum target,
                                                           GLsizei samples,
                                                           GLenum internalformat,
                                                           GLsizei width,
                                                           GLsizei height,
                                                           GLboolean fixedsamplelocations);
GL_APICALL void GL_APIENTRY glGetMultisamplefvANGLE(GLenum pname, GLuint index, GLfloat *val);
GL_APICALL void GL_APIENTRY glSampleMaskiANGLE(GLuint maskNumber, GLbitfield mask);
#endif
#endif  // !GL_ANGLE_MULTISAMPLE_TEXTURE

#ifdef __cplusplus
}
#endif

#endif