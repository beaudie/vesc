//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// gl3ext_angle.h: ANGLE extensions for gles 3.0.
//

#ifndef INCLUDE_GLES3_GLES3EXT_ANGLE_H_
#define INCLUDE_GLES3_GLES3EXT_ANGLE_H_

#ifndef GL_ANGLE_texture_multisample
#define GL_ANGLE_texture_multisample 1
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
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glTexStorage2DMultisampleANGLE(GLenum target,
                                                           GLsizei samples,
                                                           GLenum internalformat,
                                                           GLsizei width,
                                                           GLsizei height,
                                                           GLboolean fixedsamplelocations);
#endif
#endif  // !GL_ANGLE_texture_multisample

#endif  // INCLUDE_GLES3_GLES3EXT_ANGLE_H_
