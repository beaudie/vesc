//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// gl2ext_angle.h: ANGLE modifications to the gl2ext.h header file.
//   Currently we don't include this file directly, we patch gl2ext.h
//   to include it implicitly so it is visible throughout our code.

#ifndef INCLUDE_GLES2_GL2EXT_ANGLE_H_
#define INCLUDE_GLES2_GL2EXT_ANGLE_H_

// clang-format off

#ifndef GL_ANGLE_client_arrays
#define GL_ANGLE_client_arrays 1
#define GL_CLIENT_ARRAYS_ANGLE 0x93AA
#endif /* GL_ANGLE_client_arrays */

#ifndef GL_ANGLE_request_extension
#define GL_ANGLE_request_extension 1
#define GL_REQUESTABLE_EXTENSIONS_ANGLE   0x93A8
#define GL_NUM_REQUESTABLE_EXTENSIONS_ANGLE 0x93A8
typedef void (GL_APIENTRYP PFNGLREQUESTEXTENSIONANGLEPROC) (const GLchar *name);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glRequestExtensionANGLE (const GLchar *name);
#endif
#endif /* GL_ANGLE_webgl_compatibility */

#ifndef GL_ANGLE_robust_resource_initialization
#define GL_ANGLE_robust_resource_initialization 1
#define GL_ROBUST_RESOURCE_INITIALIZATION_ANGLE 0x93AB
#endif /* GL_ANGLE_robust_resource_initialization */

#ifndef GL_CHROMIUM_framebuffer_mixed_samples
#define GL_CHROMIUM_frambuffer_mixed_samples 1
#define GL_COVERAGE_MODULATION_CHROMIUM 0x9332
typedef void (GL_APIENTRYP PFNGLCOVERAGEMODULATIONCHROMIUMPROC) (GLenum components);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glCoverageModulationCHROMIUM(GLenum components);
#endif
#endif /* GL_CHROMIUM_framebuffer_mixed_samples */

#ifndef GL_CHROMIUM_bind_generates_resource
#define GL_CHROMIUM_bind_generates_resource 1
#define GL_BIND_GENERATES_RESOURCE_CHROMIUM 0x9244
#endif /* GL_CHROMIUM_bind_generates_resource */

// needed by NV_path_rendering (and thus CHROMIUM_path_rendering)
// but CHROMIUM_path_rendering only needs MatrixLoadfEXT, MatrixLoadIdentityEXT
#ifndef GL_EXT_direct_state_access
#define GL_EXT_direct_state_access 1
typedef void(GL_APIENTRYP PFNGLMATRIXLOADFEXTPROC)(GLenum matrixMode, const GLfloat *m);
typedef void(GL_APIENTRYP PFNGLMATRIXLOADIDENTITYEXTPROC)(GLenum matrixMode);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glMatrixLoadfEXT(GLenum matrixMode, const GLfloat *m);
GL_APICALL void GL_APIENTRY glMatrixLoadIdentityEXT(GLenum matrixMode);
#endif
#endif /* GL_EXT_direct_state_access */

#ifndef GL_CHROMIUM_path_rendering
#define GL_CHROMIUM_path_rendering 1
#define GL_PATH_MODELVIEW_CHROMIUM 0x1700
#define GL_PATH_PROJECTION_CHROMIUM 0x1701
#define GL_CLOSE_PATH_CHROMIUM 0x00
#define GL_MOVE_TO_CHROMIUM 0x02
#define GL_LINE_TO_CHROMIUM 0x04
#define GL_QUADRATIC_CURVE_TO_CHROMIUM 0x0A
#define GL_CUBIC_CURVE_TO_CHROMIUM 0x0C
#define GL_CONIC_CURVE_TO_CHROMIUM 0x1A
#define GL_PATH_MODELVIEW_MATRIX_CHROMIUM 0x0BA6
#define GL_PATH_PROJECTION_MATRIX_CHROMIUM 0x0BA7
#define GL_PATH_STROKE_WIDTH_CHROMIUM 0x9075
#define GL_PATH_END_CAPS_CHROMIUM 0x9076
#define GL_PATH_JOIN_STYLE_CHROMIUM 0x9079
#define GL_PATH_MITER_LIMIT_CHROMIUM 0x907a
#define GL_PATH_STROKE_BOUND_CHROMIUM 0x9086
#define GL_FLAT_CHROMIUM 0x1D00
#define GL_SQUARE_CHROMIUM 0x90a3
#define GL_ROUND_CHROMIUM 0x90a4
#define GL_BEVEL_CHROMIUM 0x90A6
#define GL_MITER_REVERT_CHROMIUM 0x90A7
#define GL_COUNT_UP_CHROMIUM 0x9088
#define GL_COUNT_DOWN_CHROMIUM 0x9089
#define GL_CONVEX_HULL_CHROMIUM 0x908B
#define GL_BOUNDING_BOX_CHROMIUM 0x908D
#define GL_BOUNDING_BOX_OF_BOUNDING_BOXES_CHROMIUM 0x909C
#define GL_EYE_LINEAR_CHROMIUM 0x2400
#define GL_OBJECT_LINEAR_CHROMIUM 0x2401
#define GL_CONSTANT_CHROMIUM 0x8576
#define GL_TRANSLATE_X_CHROMIUM 0x908E
#define GL_TRANSLATE_Y_CHROMIUM 0x908F
#define GL_TRANSLATE_2D_CHROMIUM 0x9090
#define GL_TRANSLATE_3D_CHROMIUM 0x9091
#define GL_AFFINE_2D_CHROMIUM 0x9092
#define GL_AFFINE_3D_CHROMIUM 0x9094
#define GL_TRANSPOSE_AFFINE_2D_CHROMIUM 0x9096
#define GL_TRANSPOSE_AFFINE_3D_CHROMIUM 0x9098
typedef void(GL_APIENTRYP PFNGLMATRIXLOADFCHROMIUMPROC)(GLenum matrixMode, const GLfloat *m);
typedef void(GL_APIENTRYP PFNGLMATRIXLOADIDENTITYCHROMIUMPROC)(GLenum matrixMode);
typedef GLuint(GL_APIENTRYP PFNGLGENPATHSCHROMIUMPROC)(GLsizei range);
typedef void(GL_APIENTRYP PFNGLDELETEPATHSCHROMIUMPROC)(GLuint path, GLsizei range);
typedef GLboolean(GL_APIENTRYP PFNGLISPATHCHROMIUMPROC)(GLuint path);
typedef void(GL_APIENTRYP PFNGLPATHCOMMANDSCHROMIUMPROC)(GLuint path,
                                                         GLsizei numCommands,
                                                         const GLubyte *commands,
                                                         GLsizei numCoords,
                                                         GLenum coordType,
                                                         const void *coords);
typedef void(GL_APIENTRYP PFNGLPATHPARAMETERICHROMIUMPROC)(GLuint path, GLenum pname, GLint value);
typedef void(GL_APIENTRYP PFNGLPATHPARAMETERFCHROMIUMPROC)(GLuint path,
                                                           GLenum pname,
                                                           GLfloat value);
typedef void(GL_APIENTRYP PFNGLGETPATHPARAMETERIVCHROMIUMPROC)(GLuint path,
                                                               GLenum pname,
                                                               GLint *value);
typedef void(GL_APIENTRYP PFNGLGETPATHPARAMETERFVCHROMIUMPROC)(GLuint path,
                                                               GLenum pname,
                                                               GLfloat *value);
typedef void(GL_APIENTRYP PFNGLPATHSTENCILFUNCCHROMIUMPROC)(GLenum func, GLint ref, GLuint mask);
typedef void(GL_APIENTRYP PFNGLSTENCILFILLPATHCHROMIUMPROC)(GLuint path,
                                                            GLenum fillMode,
                                                            GLuint mask);
typedef void(GL_APIENTRYP PFNGLSTENCILSTROKEPATHCHROMIUMPROC)(GLuint path,
                                                              GLint reference,
                                                              GLuint mask);
typedef void(GL_APIENTRYP PFNGLCOVERFILLPATHCHROMIUMPROC)(GLuint path, GLenum coverMode);
typedef void(GL_APIENTRYP PFNGLCOVERSTROKEPATHCHROMIUMPROC)(GLuint path, GLenum coverMode);
typedef void(GL_APIENTRYP PFNGLSTENCILTHENCOVERFILLPATHCHROMIUMPROC)(GLuint path,
                                                                     GLenum fillMode,
                                                                     GLuint mask,
                                                                     GLenum coverMode);
typedef void(GL_APIENTRYP PFNGLSTENCILTHENCOVERSTROKEPATHCHROMIUMPROC)(GLuint path,
                                                                       GLint reference,
                                                                       GLuint mask,
                                                                       GLenum coverMode);
typedef void(GL_APIENTRYP PFNGLCOVERFILLPATHINSTANCEDCHROMIUMPROC)(GLsizei numPaths,
                                                                   GLenum pathNameType,
                                                                   const void *paths,
                                                                   GLuint pathBase,
                                                                   GLenum coverMode,
                                                                   GLenum transformType,
                                                                   const GLfloat *transformValues);
typedef void(GL_APIENTRYP PFNGLCOVERSTROKEPATHINSTANCEDCHROMIUMPROC)(
    GLsizei numPaths,
    GLenum pathNameType,
    const void *paths,
    GLuint pathBase,
    GLenum coverMode,
    GLenum transformType,
    const GLfloat *transformValues);
typedef void(GL_APIENTRYP PFNGLSTENCILFILLPATHINSTANCEDCHROMIUMPROC)(
    GLsizei numPaths,
    GLenum pathNameType,
    const void *paths,
    GLuint pathBase,
    GLenum fillMode,
    GLuint mask,
    GLenum transformType,
    const GLfloat *transformValues);
typedef void(GL_APIENTRYP PFNGLSTENCILSTROKEPATHINSTANCEDCHROMIUMPROC)(
    GLsizei numPaths,
    GLenum pathNameType,
    const void *paths,
    GLuint pathBase,
    GLint reference,
    GLuint mask,
    GLenum transformType,
    const GLfloat *transformValues);
typedef void(GL_APIENTRYP PFNGLSTENCILTHENCOVERFILLPATHINSTANCEDCHROMIUMPROC)(
    GLsizei numPaths,
    GLenum pathNameType,
    const void *paths,
    GLuint pathBase,
    GLenum fillMode,
    GLuint mask,
    GLenum coverMode,
    GLenum transformType,
    const GLfloat *transformValues);
typedef void(GL_APIENTRYP PFNGLSTENCILTHENCOVERSTROKEPATHINSTANCEDCHROMIUMPROC)(
    GLsizei numPaths,
    GLenum pathNameType,
    const void *paths,
    GLuint pathBase,
    GLint reference,
    GLuint mask,
    GLenum coverMode,
    GLenum transformType,
    const GLfloat *transformValues);

typedef void(GL_APIENTRY PFNGLBINDFRAGMENTINPUTLOCATIONCHROMIUMPROC)(GLuint program,
                                                                     GLint location,
                                                                     const GLchar *name);
typedef void(GL_APIENTRYP PFNGLPROGRAMPATHFRAGMENTINPUTGENCHROMIUMPROC)(GLuint program,
                                                                        GLint location,
                                                                        GLenum genMode,
                                                                        GLint components,
                                                                        const GLfloat *coeffs);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glMatrixLoadfCHROMIUM(GLenum matrixMode, const GLfloat *m);
GL_APICALL void GL_APIENTRY glMatrixLoadIdentityCHROMIUM(GLenum matrixMode);
GL_APICALL GLuint GL_APIENTRY glGenPathsCHROMIUM(GLsizei range);
GL_APICALL void GL_APIENTRY glDeletePathsCHROMIUM(GLuint path, GLsizei range);
GL_APICALL GLboolean GL_APIENTRY glIsPathCHROMIUM(GLuint path);
GL_APICALL void GL_APIENTRY glPathCommandsCHROMIUM(GLuint path,
                                                   GLsizei numCommands,
                                                   const GLubyte *commands,
                                                   GLsizei numCoords,
                                                   GLenum coordType,
                                                   const void *coords);
GL_APICALL void GL_APIENTRY glPathParameteriCHROMIUM(GLuint path, GLenum pname, GLint value);
GL_APICALL void GL_APIENTRY glPathParameterfCHROMIUM(GLuint path, GLenum pname, GLfloat value);
GL_APICALL void GL_APIENTRY glGetPathParameterivCHROMIUM(GLuint path, GLenum pname, GLint *value);
GL_APICALL void GL_APIENTRY glGetPathParameterfvCHROMIUM(GLuint path, GLenum pname, GLfloat *value);
GL_APICALL void GL_APIENTRY glPathStencilFuncCHROMIUM(GLenum func, GLint ref, GLuint mask);
GL_APICALL void GL_APIENTRY glStencilFillPathCHROMIUM(GLuint path, GLenum fillMode, GLuint mask);
GL_APICALL void GL_APIENTRY glStencilStrokePathCHROMIUM(GLuint path, GLint reference, GLuint mask);
GL_APICALL void GL_APIENTRY glCoverFillPathCHROMIUM(GLuint path, GLenum coverMode);
GL_APICALL void GL_APIENTRY glCoverStrokePathCHROMIUM(GLuint path, GLenum coverMode);
GL_APICALL void GL_APIENTRY glStencilThenCoverFillPathCHROMIUM(GLuint path,
                                                               GLenum fillMode,
                                                               GLuint mask,
                                                               GLenum coverMode);
GL_APICALL void GL_APIENTRY glStencilThenCoverStrokePathCHROMIUM(GLuint path,
                                                                 GLint reference,
                                                                 GLuint mask,
                                                                 GLenum coverMode);
GL_APICALL void GL_APIENTRY glCoverFillPathInstancedCHROMIUM(GLsizei numPaths,
                                                             GLenum pathNameType,
                                                             const void *paths,
                                                             GLuint pathBase,
                                                             GLenum coverMode,
                                                             GLenum transformType,
                                                             const GLfloat *transformValues);
GL_APICALL void GL_APIENTRY glCoverStrokePathInstancedCHROMIUM(GLsizei numPaths,
                                                               GLenum pathNameType,
                                                               const void *paths,
                                                               GLuint pathBase,
                                                               GLenum coverMode,
                                                               GLenum transformType,
                                                               const GLfloat *transformValues);
GL_APICALL void GL_APIENTRY glStencilFillPathInstancedCHROMIUM(GLsizei numPaths,
                                                               GLenum pathNameType,
                                                               const void *paths,
                                                               GLuint pathBase,
                                                               GLenum fillMode,
                                                               GLuint mask,
                                                               GLenum transformType,
                                                               const GLfloat *transformValues);
GL_APICALL void GL_APIENTRY glStencilStrokePathInstancedCHROMIUM(GLsizei numPaths,
                                                                 GLenum pathNameType,
                                                                 const void *paths,
                                                                 GLuint pathBase,
                                                                 GLint reference,
                                                                 GLuint mask,
                                                                 GLenum transformType,
                                                                 const GLfloat *transformValues);
GL_APICALL void GL_APIENTRY
glStencilThenCoverFillPathInstancedCHROMIUM(GLsizei numPaths,
                                            GLenum pathNameType,
                                            const void *paths,
                                            GLuint pathBase,
                                            GLenum fillMode,
                                            GLuint mask,
                                            GLenum coverMode,
                                            GLenum transformType,
                                            const GLfloat *transformValues);

GL_APICALL void GL_APIENTRY
glStencilThenCoverStrokePathInstancedCHROMIUM(GLsizei numPaths,
                                              GLenum pathNameType,
                                              const void *paths,
                                              GLuint pathBase,
                                              GLint reference,
                                              GLuint mask,
                                              GLenum coverMode,
                                              GLenum transformType,
                                              const GLfloat *transformValues);

GL_APICALL void GL_APIENTRY glBindFragmentInputLocationCHROMIUM(GLuint program,
                                                                GLint location,
                                                                const GLchar *name);
GL_APICALL void GL_APIENTRY glProgramPathFragmentInputGenCHROMIUM(GLuint program,
                                                                  GLint location,
                                                                  GLenum genMode,
                                                                  GLint components,
                                                                  const GLfloat *coeffs);

#endif
#endif /* GL_CHROMIUM_path_rendering */

#ifndef GL_CHROMIUM_copy_texture
#define GL_CHROMIUM_copy_texture 1
typedef void(GL_APIENTRYP PFNGLCOPYTEXTURECHROMIUMPROC)(GLuint sourceId,
                                                        GLint sourceLevel,
                                                        GLenum destTarget,
                                                        GLuint destId,
                                                        GLint destLevel,
                                                        GLint internalFormat,
                                                        GLenum destType,
                                                        GLboolean unpackFlipY,
                                                        GLboolean unpackPremultiplyAlpha,
                                                        GLboolean unpackUnmultiplyAlpha);
typedef void(GL_APIENTRYP PFNGLCOPYSUBTEXTURECHROMIUMPROC)(GLuint sourceId,
                                                           GLint sourceLevel,
                                                           GLenum destTarget,
                                                           GLuint destId,
                                                           GLint destLevel,
                                                           GLint xoffset,
                                                           GLint yoffset,
                                                           GLint x,
                                                           GLint y,
                                                           GLsizei width,
                                                           GLsizei height,
                                                           GLboolean unpackFlipY,
                                                           GLboolean unpackPremultiplyAlpha,
                                                           GLboolean unpackUnmultiplyAlpha);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glCopyTextureCHROMIUM(GLuint sourceId,
                                                  GLint sourceLevel,
                                                  GLenum destTarget,
                                                  GLuint destId,
                                                  GLint destLevel,
                                                  GLint internalFormat,
                                                  GLenum destType,
                                                  GLboolean unpackFlipY,
                                                  GLboolean unpackPremultiplyAlpha,
                                                  GLboolean unpackUnmultiplyAlpha);
GL_APICALL void GL_APIENTRY glCopySubTextureCHROMIUM(GLuint sourceId,
                                                     GLint sourceLevel,
                                                     GLenum destTarget,
                                                     GLuint destId,
                                                     GLint destLevel,
                                                     GLint xoffset,
                                                     GLint yoffset,
                                                     GLint x,
                                                     GLint y,
                                                     GLsizei width,
                                                     GLsizei height,
                                                     GLboolean unpackFlipY,
                                                     GLboolean unpackPremultiplyAlpha,
                                                     GLboolean unpackUnmultiplyAlpha);
#endif
#endif /* GL_CHROMIUM_copy_texture */

#ifndef GL_CHROMIUM_compressed_copy_texture
#define GL_CHROMIUM_compressed_copy_texture 1
typedef void(GL_APIENTRYP PFNGLCOMPRESSEDCOPYTEXTURECHROMIUMPROC)(GLuint sourceId, GLuint destId);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glCompressedCopyTextureCHROMIUM(GLuint sourceId, GLuint destId);
#endif
#endif /* GL_CHROMIUM_compressed_copy_texture */

#ifndef GL_CHROMIUM_sync_query
#define GL_CHROMIUM_sync_query 1
#define GL_COMMANDS_COMPLETED_CHROMIUM    0x84F7
#endif  /* GL_CHROMIUM_sync_query */

#ifndef GL_EXT_texture_compression_s3tc_srgb
#define GL_EXT_texture_compression_s3tc_srgb 1
#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT  0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F
#endif /* GL_EXT_texture_compression_s3tc_srgb */

#ifndef GL_ANGLE_lossy_etc_decode
#define GL_ANGLE_lossy_etc_decode 1
#define GL_ETC1_RGB8_LOSSY_DECODE_ANGLE                                  0x9690
#define GL_COMPRESSED_R11_LOSSY_DECODE_EAC_ANGLE                         0x9691
#define GL_COMPRESSED_SIGNED_R11_LOSSY_DECODE_EAC_ANGLE                  0x9692
#define GL_COMPRESSED_RG11_LOSSY_DECODE_EAC_ANGLE                        0x9693
#define GL_COMPRESSED_SIGNED_RG11_LOSSY_DECODE_EAC_ANGLE                 0x9694
#define GL_COMPRESSED_RGB8_LOSSY_DECODE_ETC2_ANGLE                       0x9695
#define GL_COMPRESSED_SRGB8_LOSSY_DECODE_ETC2_ANGLE                      0x9696
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE   0x9697
#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE  0x9698
#define GL_COMPRESSED_RGBA8_LOSSY_DECODE_ETC2_EAC_ANGLE                  0x9699
#define GL_COMPRESSED_SRGB8_ALPHA8_LOSSY_DECODE_ETC2_EAC_ANGLE           0x969A
#endif /* GL_ANGLE_lossy_etc_decode */

#ifndef GL_ANGLE_robust_client_memory
#define GL_ANGLE_robust_client_memory 1
typedef void (GL_APIENTRYP PFNGLGETBOOLEANVROBUSTANGLE) (GLenum pname, GLsizei bufSize, GLsizei *length, GLboolean *data);
typedef void (GL_APIENTRYP PFNGLGETBUFFERPARAMETERIVROBUSTANGLE) (GLenum target, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETFLOATVROBUSTANGLE) (GLenum pname, GLsizei bufSize, GLsizei *length, GLfloat *data);
typedef void (GL_APIENTRYP PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVROBUSTANGLE) (GLenum target, GLenum attachment, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETINTEGERVROBUSTANGLE) (GLenum pname, GLsizei bufSize, GLsizei *length, GLint *data);
typedef void (GL_APIENTRYP PFNGLGETPROGRAMIVROBUSTANGLE) (GLuint program, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETRENDERBUFFERPARAMETERIVROBUSTANGLE) (GLenum target, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETSHADERIVROBUSTANGLE) (GLuint shader, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETTEXPARAMETERFVROBUSTANGLE) (GLenum target, GLenum pname, GLsizei bufSize, GLsizei *length, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETTEXPARAMETERIVROBUSTANGLE) (GLenum target, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETUNIFORMFVROBUSTANGLE) (GLuint program, GLint location, GLsizei bufSize, GLsizei *length, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETUNIFORMIVROBUSTANGLE) (GLuint program, GLint location, GLsizei bufSize, GLsizei *length, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETVERTEXATTRIBFVROBUSTANGLE) (GLuint index, GLenum pname, GLsizei bufSize, GLsizei *length, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETVERTEXATTRIBIVROBUSTANGLE) (GLuint index, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETVERTEXATTRIBPOINTERVROBUSTANGLE) (GLuint index, GLenum pname, GLsizei bufSize, GLsizei *length, void **pointer);
typedef void (GL_APIENTRYP PFNGLREADPIXELSROBUSTANGLE) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLsizei *length, GLsizei *columns, GLsizei *rows, void *pixels);
typedef void (GL_APIENTRYP PFNGLTEXIMAGE2DROBUSTANGLE) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, GLsizei bufSize, const void *pixels);
typedef void (GL_APIENTRYP PFNGLTEXPARAMETERFVROBUSTANGLE) (GLenum target, GLenum pname, GLsizei bufSize, const GLfloat *params);
typedef void (GL_APIENTRYP PFNGLTEXPARAMETERIVROBUSTANGLE) (GLenum target, GLenum pname, GLsizei bufSize, const GLint *params);
typedef void (GL_APIENTRYP PFNGLTEXSUBIMAGE2DROBUSTANGLE) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, const void *pixels);
typedef void (GL_APIENTRYP PFNGLTEXIMAGE3DROBUSTANGLE) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, GLsizei bufSize, const void *pixels);
typedef void (GL_APIENTRYP PFNGLTEXSUBIMAGE3DROBUSTANGLE) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, const void *pixels);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXIMAGE2DROBUSTANGLE) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, GLsizei bufSize, const void *data);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXSUBIMAGE2DROBUSTANGLE) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, GLsizei bufSize, const void *data);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXIMAGE3DROBUSTANGLE) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, GLsizei bufSize, const void *data);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXSUBIMAGE3DROBUSTANGLE) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, GLsizei bufSize, const void *data);
typedef void (GL_APIENTRYP PFNGLGETQUERYIVROBUSTANGLE) (GLenum target, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTUIVROBUSTANGLE) (GLuint id, GLenum pname, GLsizei bufSize, GLsizei *length, GLuint *params);
typedef void (GL_APIENTRYP PFNGLGETBUFFERPOINTERVROBUSTANGLE) (GLenum target, GLenum pname, GLsizei bufSize, GLsizei *length, void **params);
typedef void (GL_APIENTRYP PFNGLGETINTEGERI_VROBUSTANGLE) (GLenum target, GLuint index, GLsizei bufSize, GLsizei *length, GLint *data);
typedef void (GL_APIENTRYP PFNGETINTERNALFORMATIVROBUSTANGLE) (GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETVERTEXATTRIBIIVROBUSTANGLE) (GLuint index, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETVERTEXATTRIBIUIVROBUSTANGLE) (GLuint index, GLenum pname, GLsizei bufSize, GLsizei *length, GLuint *params);
typedef void (GL_APIENTRYP PFNGLGETUNIFORMUIVROBUSTANGLE) (GLuint program, GLint location, GLsizei bufSize, GLsizei *length, GLuint *params);
typedef void (GL_APIENTRYP PFNGLGETACTIVEUNIFORMBLOCKIVROBUSTANGLE) (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETINTEGER64VROBUSTANGLE) (GLenum pname, GLsizei bufSize, GLsizei *length, GLint64 *data);
typedef void (GL_APIENTRYP PFNGLGETINTEGER64I_VROBUSTANGLE) (GLenum target, GLuint index, GLsizei bufSize, GLsizei *length, GLint64 *data);
typedef void (GL_APIENTRYP PFNGLGETBUFFERPARAMETERI64VROBUSTANGLE) (GLenum target, GLenum pname, GLsizei bufSize, GLsizei *length, GLint64 *params);
typedef void (GL_APIENTRYP PFNGLSAMPLERPARAMETERIVROBUSTANGLE) (GLuint sampler, GLenum pname, GLsizei bufSize, const GLint *param);
typedef void (GL_APIENTRYP PFNGLSAMPLERPARAMETERFVROBUSTANGLE) (GLuint sampler, GLenum pname, GLsizei bufSize, const GLfloat *param);
typedef void (GL_APIENTRYP PFNGLGETSAMPLERPARAMETERIVROBUSTANGLE) (GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETSAMPLERPARAMETERFVROBUSTANGLE) (GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei *length, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETFRAMEBUFFERPARAMETERIVROBUSTANGLE) (GLenum target, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETPROGRAMINTERFACEIVROBUSTANGLE) (GLuint program, GLenum programInterface, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETBOOLEANI_VROBUSTANGLE) (GLenum target, GLuint index, GLsizei bufSize, GLsizei *length, GLboolean *data);
typedef void (GL_APIENTRYP PFNGLGETMULTISAMPLEFVROBUSTANGLE) (GLenum pname, GLuint index, GLsizei bufSize, GLsizei *length, GLfloat *val);
typedef void (GL_APIENTRYP PFNGLGETTEXLEVELPARAMETERIVROBUSTANGLE) (GLenum target, GLint level, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETTEXLEVELPARAMETERFVROBUSTANGLE) (GLenum target, GLint level, GLenum pname, GLsizei bufSize, GLsizei *length, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETPOINTERVROBUSTANGLEROBUSTANGLE) (GLenum pname, GLsizei bufSize, GLsizei *length, void **params);
typedef void (GL_APIENTRYP PFNGLREADNPIXELSROBUSTANGLE) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLsizei *length, GLsizei *columns, GLsizei *rows, void *data);
typedef void (GL_APIENTRYP PFNGLGETNUNIFORMFVROBUSTANGLE) (GLuint program, GLint location, GLsizei bufSize, GLsizei *length, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETNUNIFORMIVROBUSTANGLE) (GLuint program, GLint location, GLsizei bufSize, GLsizei *length, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETNUNIFORMUIVROBUSTANGLE) (GLuint program, GLint location, GLsizei bufSize, GLsizei *length, GLuint *params);
typedef void (GL_APIENTRYP PFNGLTEXPARAMETERIIVROBUSTANGLE) (GLenum target, GLenum pname, GLsizei bufSize, const GLint *params);
typedef void (GL_APIENTRYP PFNGLTEXPARAMETERIUIVROBUSTANGLE) (GLenum target, GLenum pname, GLsizei bufSize, const GLuint *params);
typedef void (GL_APIENTRYP PFNGLGETTEXPARAMETERIIVROBUSTANGLE) (GLenum target, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETTEXPARAMETERIUIVROBUSTANGLE) (GLenum target, GLenum pname, GLsizei bufSize, GLsizei *length, GLuint *params);
typedef void (GL_APIENTRYP PFNGLSAMPLERPARAMETERIIVROBUSTANGLE) (GLuint sampler, GLenum pname, GLsizei bufSize, const GLint *param);
typedef void (GL_APIENTRYP PFNGLSAMPLERPARAMETERIUIVROBUSTANGLE) (GLuint sampler, GLenum pname, GLsizei bufSize, const GLuint *param);
typedef void (GL_APIENTRYP PFNGLGETSAMPLERPARAMETERIIVROBUSTANGLE) (GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETSAMPLERPARAMETERIUIVROBUSTANGLE) (GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei *length, GLuint *params);
typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTIVROBUSTANGLE)(GLuint id, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTI64VROBUSTANGLE)(GLuint id, GLenum pname, GLsizei bufSize, GLsizei *length, GLint64 *params);
typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTUI64VROBUSTANGLE)(GLuint id, GLenum pname, GLsizei bufSize, GLsizei *length, GLuint64 *params);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glGetBooleanvRobustANGLE (GLenum pname, GLsizei bufSize, GLsizei *length, GLboolean *data);
GL_APICALL void GL_APIENTRY glGetBufferParameterivRobustANGLE (GLenum target, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL void GL_APIENTRY glGetFloatvRobustANGLE (GLenum pname, GLsizei bufSize, GLsizei *length, GLfloat *data);
GL_APICALL void GL_APIENTRY glGetFramebufferAttachmentParameterivRobustANGLE (GLenum target, GLenum attachment, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL void GL_APIENTRY glGetIntegervRobustANGLE (GLenum pname, GLsizei bufSize, GLsizei *length, GLint *data);
GL_APICALL void GL_APIENTRY glGetProgramivRobustANGLE (GLuint program, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL void GL_APIENTRY glGetRenderbufferParameterivRobustANGLE (GLenum target, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL void GL_APIENTRY glGetShaderivRobustANGLE (GLuint shader, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL void GL_APIENTRY glGetTexParameterfvRobustANGLE (GLenum target, GLenum pname, GLsizei bufSize, GLsizei *length, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetTexParameterivRobustANGLE (GLenum target, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL void GL_APIENTRY glGetUniformfvRobustANGLE (GLuint program, GLint location, GLsizei bufSize, GLsizei *length, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetUniformivRobustANGLE (GLuint program, GLint location, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL void GL_APIENTRY glGetVertexAttribfvRobustANGLE (GLuint index, GLenum pname, GLsizei bufSize, GLsizei *length, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetVertexAttribivRobustANGLE (GLuint index, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL void GL_APIENTRY glGetVertexAttribPointervRobustANGLE (GLuint index, GLenum pname, GLsizei bufSize, GLsizei *length, void **pointer);
GL_APICALL void GL_APIENTRY glReadPixelsRobustANGLE (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLsizei *length, GLsizei *columns, GLsizei *rows, void *pixels);
GL_APICALL void GL_APIENTRY glTexImage2DRobustANGLE (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, GLsizei bufSize, const void *pixels);
GL_APICALL void GL_APIENTRY glTexParameterfvRobustANGLE (GLenum target, GLenum pname, GLsizei bufSize, const GLfloat *params);
GL_APICALL void GL_APIENTRY glTexParameterivRobustANGLE (GLenum target, GLenum pname, GLsizei bufSize, const GLint *params);
GL_APICALL void GL_APIENTRY glTexSubImage2DRobustANGLE (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, const void *pixels);
GL_APICALL void GL_APIENTRY glTexImage3DRobustANGLE (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, GLsizei bufSize, const void *pixels);
GL_APICALL void GL_APIENTRY glTexSubImage3DRobustANGLE (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, const void *pixels);
GL_APICALL void GL_APIENTRY glCompressedTexImage2DRobustANGLE(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, GLsizei bufSize, const void *data);
GL_APICALL void GL_APIENTRY glCompressedTexSubImage2DRobustANGLE(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, GLsizei bufSize, const void *data);
GL_APICALL void GL_APIENTRY glCompressedTexImage3DRobustANGLE(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, GLsizei bufSize, const void *data);
GL_APICALL void GL_APIENTRY glCompressedTexSubImage3DRobustANGLE(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, GLsizei bufSize, const void *data);
GL_APICALL void GL_APIENTRY glGetQueryivRobustANGLE (GLenum target, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL void GL_APIENTRY glGetQueryObjectuivRobustANGLE (GLuint id, GLenum pname, GLsizei bufSize, GLsizei *length, GLuint *params);
GL_APICALL void GL_APIENTRY glGetBufferPointervRobustANGLE (GLenum target, GLenum pname, GLsizei bufSize, GLsizei *length, void **params);
GL_APICALL void GL_APIENTRY glGetIntegeri_vRobustANGLE (GLenum target, GLuint index, GLsizei bufSize, GLsizei *length, GLint *data);
GL_APICALL void GL_APIENTRY glGetInternalformativRobustANGLE (GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL void GL_APIENTRY glGetVertexAttribIivRobustANGLE (GLuint index, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL void GL_APIENTRY glGetVertexAttribIuivRobustANGLE (GLuint index, GLenum pname, GLsizei bufSize, GLsizei *length, GLuint *params);
GL_APICALL void GL_APIENTRY glGetUniformuivRobustANGLE (GLuint program, GLint location, GLsizei bufSize, GLsizei *length, GLuint *params);
GL_APICALL void GL_APIENTRY glGetActiveUniformBlockivRobustANGLE (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL void GL_APIENTRY glGetInteger64vRobustANGLE (GLenum pname, GLsizei bufSize, GLsizei *length, GLint64 *data);
GL_APICALL void GL_APIENTRY glGetInteger64i_vRobustANGLE (GLenum target, GLuint index, GLsizei bufSize, GLsizei *length, GLint64 *data);
GL_APICALL void GL_APIENTRY glGetBufferParameteri64vRobustANGLE (GLenum target, GLenum pname, GLsizei bufSize, GLsizei *length, GLint64 *params);
GL_APICALL void GL_APIENTRY glSamplerParameterivRobustANGLE (GLuint sampler, GLenum pname, GLsizei bufSize, const GLint *param);
GL_APICALL void GL_APIENTRY glSamplerParameterfvRobustANGLE (GLuint sampler, GLenum pname, GLsizei bufSize, const GLfloat *param);
GL_APICALL void GL_APIENTRY glGetSamplerParameterivRobustANGLE (GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL void GL_APIENTRY glGetSamplerParameterfvRobustANGLE (GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei *length, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetFramebufferParameterivRobustANGLE (GLenum target, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL void GL_APIENTRY glGetProgramInterfaceivRobustANGLE (GLuint program, GLenum programInterface, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL void GL_APIENTRY glGetBooleani_vRobustANGLE (GLenum target, GLuint index, GLsizei bufSize, GLsizei *length, GLboolean *data);
GL_APICALL void GL_APIENTRY glGetMultisamplefvRobustANGLE (GLenum pname, GLuint index, GLsizei bufSize, GLsizei *length, GLfloat *val);
GL_APICALL void GL_APIENTRY glGetTexLevelParameterivRobustANGLE (GLenum target, GLint level, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL void GL_APIENTRY glGetTexLevelParameterfvRobustANGLE (GLenum target, GLint level, GLenum pname, GLsizei bufSize, GLsizei *length, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetPointervRobustANGLERobustANGLE (GLenum pname, GLsizei bufSize, GLsizei *length, void **params);
GL_APICALL void GL_APIENTRY glReadnPixelsRobustANGLE (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLsizei *length, GLsizei *columns, GLsizei *rows, void *data);
GL_APICALL void GL_APIENTRY glGetnUniformfvRobustANGLE (GLuint program, GLint location, GLsizei bufSize, GLsizei *length, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetnUniformivRobustANGLE (GLuint program, GLint location, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL void GL_APIENTRY glGetnUniformuivRobustANGLE (GLuint program, GLint location, GLsizei bufSize, GLsizei *length, GLuint *params);
GL_APICALL void GL_APIENTRY glTexParameterIivRobustANGLE (GLenum target, GLenum pname, GLsizei bufSize, const GLint *params);
GL_APICALL void GL_APIENTRY glTexParameterIuivRobustANGLE (GLenum target, GLenum pname, GLsizei bufSize, const GLuint *params);
GL_APICALL void GL_APIENTRY glGetTexParameterIivRobustANGLE (GLenum target, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL void GL_APIENTRY glGetTexParameterIuivRobustANGLE (GLenum target, GLenum pname, GLsizei bufSize, GLsizei *length, GLuint *params);
GL_APICALL void GL_APIENTRY glSamplerParameterIivRobustANGLE (GLuint sampler, GLenum pname, GLsizei bufSize, const GLint *param);
GL_APICALL void GL_APIENTRY glSamplerParameterIuivRobustANGLE (GLuint sampler, GLenum pname, GLsizei bufSize, const GLuint *param);
GL_APICALL void GL_APIENTRY glGetSamplerParameterIivRobustANGLE (GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL void GL_APIENTRY glGetSamplerParameterIuivRobustANGLE (GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei *length, GLuint *params);
GL_APICALL void GL_APIENTRY glGetQueryObjectivRobustANGLE(GLuint id, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL void GL_APIENTRY glGetQueryObjecti64vRobustANGLE(GLuint id, GLenum pname, GLsizei bufSize, GLsizei *length, GLint64 *params);
GL_APICALL void GL_APIENTRY glGetQueryObjectui64vRobustANGLE(GLuint id, GLenum pname, GLsizei bufSize, GLsizei *length, GLuint64 *params);
#endif
#endif /* GL_ANGLE_robust_client_memory */

#ifndef GL_ANGLE_program_cache_control
#define GL_ANGLE_program_cache_control 1
#define GL_PROGRAM_CACHE_ENABLED_ANGLE 0x93AC
#endif  /* GL_ANGLE_program_cache_control */

#ifndef GL_ANGLE_multiview
#define GL_ANGLE_multiview 1
// The next four enums coincide with the enums introduced by the GL_OVR_multiview extension and use the values reserved by that extension.
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_NUM_VIEWS_ANGLE 0x9630
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_BASE_VIEW_INDEX_ANGLE 0x9632
#define GL_MAX_VIEWS_ANGLE 0x9631
#define GL_FRAMEBUFFER_INCOMPLETE_VIEW_TARGETS_ANGLE 0x9633
// The next four enums are reserved specifically for ANGLE.
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_VIEWPORT_OFFSETS_ANGLE 0x969B
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_MULTIVIEW_LAYOUT_ANGLE 0x969C
#define GL_FRAMEBUFFER_MULTIVIEW_SIDE_BY_SIDE_ANGLE 0x969D
#define GL_FRAMEBUFFER_MULTIVIEW_LAYERED_ANGLE 0x969E
typedef void(GL_APIENTRYP PFNGLFRAMEBUFFERTEXTUREMULTIVIEWLAYEREDANGLE)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews);
typedef void(GL_APIENTRYP PFNGLFRAMEBUFFERTEXTUREMULTIVIEWSIDEBYSIDEANGLE)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLsizei numViews, const GLint *viewportOffsets);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glFramebufferTextureMultiviewLayeredANGLE(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews);
GL_APICALL void GL_APIENTRY glFramebufferTextureMultiviewSideBySideANGLE(GLenum target, GLenum attachment, GLuint texture, GLint level, GLsizei numViews, const GLint *viewportOffsets);
#endif
#endif /* GL_ANGLE_multiview */

#ifndef GL_ANGLE_texture_rectangle
#define GL_ANGLE_texture_rectangle 1
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_ANGLE 0x84F8
#define GL_TEXTURE_RECTANGLE_ANGLE 0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE_ANGLE 0x84F6
#define GL_SAMPLER_2D_RECT_ANGLE 0x8B63
#endif /* GL_ANGLE_texture_rectangle */

#ifdef EGL_ANGLE_explicit_context
typedef void *GLeglContext;
// OpenGL ES 2.0
typedef void (GL_APIENTRYP PFNGLACTIVETEXTURECONTEXTANGLE)(GLeglContext ctx, GLenum texture);
typedef void (GL_APIENTRYP PFNGLATTACHSHADERCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLuint shader);
typedef void (GL_APIENTRYP PFNGLBINDATTRIBLOCATIONCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLuint index, const GLchar *name);
typedef void (GL_APIENTRYP PFNGLBINDBUFFERCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLuint buffer);
typedef void (GL_APIENTRYP PFNGLBINDFRAMEBUFFERCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLuint framebuffer);
typedef void (GL_APIENTRYP PFNGLBINDRENDERBUFFERCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLuint renderbuffer);
typedef void (GL_APIENTRYP PFNGLBINDTEXTURECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLuint texture);
typedef void (GL_APIENTRYP PFNGLBLENDCOLORCONTEXTANGLE)(GLeglContext ctx, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (GL_APIENTRYP PFNGLBLENDEQUATIONCONTEXTANGLE)(GLeglContext ctx, GLenum mode);
typedef void (GL_APIENTRYP PFNGLBLENDEQUATIONSEPARATECONTEXTANGLE)(GLeglContext ctx, GLenum modeRGB, GLenum modeAlpha);
typedef void (GL_APIENTRYP PFNGLBLENDFUNCCONTEXTANGLE)(GLeglContext ctx, GLenum sfactor, GLenum dfactor);
typedef void (GL_APIENTRYP PFNGLBLENDFUNCSEPARATECONTEXTANGLE)(GLeglContext ctx, GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef void (GL_APIENTRYP PFNGLBUFFERDATACONTEXTANGLE)(GLeglContext ctx, GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void (GL_APIENTRYP PFNGLBUFFERSUBDATACONTEXTANGLE)(GLeglContext ctx, GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
typedef void (GL_APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSCONTEXTANGLE)(GLeglContext ctx, GLenum target);
typedef void (GL_APIENTRYP PFNGLCLEARCONTEXTANGLE)(GLeglContext ctx, GLbitfield mask);
typedef void (GL_APIENTRYP PFNGLCLEARCOLORCONTEXTANGLE)(GLeglContext ctx, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (GL_APIENTRYP PFNGLCLEARDEPTHFCONTEXTANGLE)(GLeglContext ctx, GLfloat d);
typedef void (GL_APIENTRYP PFNGLCLEARSTENCILCONTEXTANGLE)(GLeglContext ctx, GLint s);
typedef void (GL_APIENTRYP PFNGLCOLORMASKCONTEXTANGLE)(GLeglContext ctx, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
typedef void (GL_APIENTRYP PFNGLCOMPILESHADERCONTEXTANGLE)(GLeglContext ctx, GLuint shader);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXIMAGE2DCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXSUBIMAGE2DCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
typedef void (GL_APIENTRYP PFNGLCOPYTEXIMAGE2DCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void (GL_APIENTRYP PFNGLCOPYTEXSUBIMAGE2DCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GL_APIENTRYP PFNGLCREATEPROGRAMCONTEXTANGLE)(GLeglContext ctx);
typedef void (GL_APIENTRYP PFNGLCREATESHADERCONTEXTANGLE)(GLeglContext ctx, GLenum type);
typedef void (GL_APIENTRYP PFNGLCULLFACECONTEXTANGLE)(GLeglContext ctx, GLenum mode);
typedef void (GL_APIENTRYP PFNGLDELETEBUFFERSCONTEXTANGLE)(GLeglContext ctx, GLsizei n, const GLuint *buffers);
typedef void (GL_APIENTRYP PFNGLDELETEFRAMEBUFFERSCONTEXTANGLE)(GLeglContext ctx, GLsizei n, const GLuint *framebuffers);
typedef void (GL_APIENTRYP PFNGLDELETEPROGRAMCONTEXTANGLE)(GLeglContext ctx, GLuint program);
typedef void (GL_APIENTRYP PFNGLDELETERENDERBUFFERSCONTEXTANGLE)(GLeglContext ctx, GLsizei n, const GLuint *renderbuffers);
typedef void (GL_APIENTRYP PFNGLDELETESHADERCONTEXTANGLE)(GLeglContext ctx, GLuint shader);
typedef void (GL_APIENTRYP PFNGLDELETETEXTURESCONTEXTANGLE)(GLeglContext ctx, GLsizei n, const GLuint *textures);
typedef void (GL_APIENTRYP PFNGLDEPTHFUNCCONTEXTANGLE)(GLeglContext ctx, GLenum func);
typedef void (GL_APIENTRYP PFNGLDEPTHMASKCONTEXTANGLE)(GLeglContext ctx, GLboolean flag);
typedef void (GL_APIENTRYP PFNGLDEPTHRANGEFCONTEXTANGLE)(GLeglContext ctx, GLfloat n, GLfloat f);
typedef void (GL_APIENTRYP PFNGLDETACHSHADERCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLuint shader);
typedef void (GL_APIENTRYP PFNGLDISABLECONTEXTANGLE)(GLeglContext ctx, GLenum cap);
typedef void (GL_APIENTRYP PFNGLDISABLEVERTEXATTRIBARRAYCONTEXTANGLE)(GLeglContext ctx, GLuint index);
typedef void (GL_APIENTRYP PFNGLDRAWARRAYSCONTEXTANGLE)(GLeglContext ctx, GLenum mode, GLint first, GLsizei count);
typedef void (GL_APIENTRYP PFNGLDRAWELEMENTSCONTEXTANGLE)(GLeglContext ctx, GLenum mode, GLsizei count, GLenum type, const void *indices);
typedef void (GL_APIENTRYP PFNGLENABLECONTEXTANGLE)(GLeglContext ctx, GLenum cap);
typedef void (GL_APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYCONTEXTANGLE)(GLeglContext ctx, GLuint index);
typedef void (GL_APIENTRYP PFNGLFINISHCONTEXTANGLE)(GLeglContext ctx);
typedef void (GL_APIENTRYP PFNGLFLUSHCONTEXTANGLE)(GLeglContext ctx);
typedef void (GL_APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFERCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (GL_APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (GL_APIENTRYP PFNGLFRONTFACECONTEXTANGLE)(GLeglContext ctx, GLenum mode);
typedef void (GL_APIENTRYP PFNGLGENBUFFERSCONTEXTANGLE)(GLeglContext ctx, GLsizei n, GLuint *buffers);
typedef void (GL_APIENTRYP PFNGLGENFRAMEBUFFERSCONTEXTANGLE)(GLeglContext ctx, GLsizei n, GLuint *framebuffers);
typedef void (GL_APIENTRYP PFNGLGENRENDERBUFFERSCONTEXTANGLE)(GLeglContext ctx, GLsizei n, GLuint *renderbuffers);
typedef void (GL_APIENTRYP PFNGLGENTEXTURESCONTEXTANGLE)(GLeglContext ctx, GLsizei n, GLuint *textures);
typedef void (GL_APIENTRYP PFNGLGENERATEMIPMAPCONTEXTANGLE)(GLeglContext ctx, GLenum target);
typedef void (GL_APIENTRYP PFNGLGETACTIVEATTRIBCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
typedef void (GL_APIENTRYP PFNGLGETACTIVEUNIFORMCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
typedef void (GL_APIENTRYP PFNGLGETATTACHEDSHADERSCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
typedef void (GL_APIENTRYP PFNGLGETATTRIBLOCATIONCONTEXTANGLE)(GLeglContext ctx, GLuint program, const GLchar *name);
typedef void (GL_APIENTRYP PFNGLGETBOOLEANVCONTEXTANGLE)(GLeglContext ctx, GLenum pname, GLboolean *data);
typedef void (GL_APIENTRYP PFNGLGETBUFFERPARAMETERIVCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETERRORCONTEXTANGLE)(GLeglContext ctx);
typedef void (GL_APIENTRYP PFNGLGETFLOATVCONTEXTANGLE)(GLeglContext ctx, GLenum pname, GLfloat *data);
typedef void (GL_APIENTRYP PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum attachment, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETINTEGERVCONTEXTANGLE)(GLeglContext ctx, GLenum pname, GLint *data);
typedef void (GL_APIENTRYP PFNGLGETPROGRAMINFOLOGCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (GL_APIENTRYP PFNGLGETPROGRAMIVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETRENDERBUFFERPARAMETERIVCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETSHADERINFOLOGCONTEXTANGLE)(GLeglContext ctx, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (GL_APIENTRYP PFNGLGETSHADERPRECISIONFORMATCONTEXTANGLE)(GLeglContext ctx, GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
typedef void (GL_APIENTRYP PFNGLGETSHADERSOURCECONTEXTANGLE)(GLeglContext ctx, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
typedef void (GL_APIENTRYP PFNGLGETSHADERIVCONTEXTANGLE)(GLeglContext ctx, GLuint shader, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETSTRINGCONTEXTANGLE)(GLeglContext ctx, GLenum name);
typedef void (GL_APIENTRYP PFNGLGETTEXPARAMETERFVCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETTEXPARAMETERIVCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETUNIFORMLOCATIONCONTEXTANGLE)(GLeglContext ctx, GLuint program, const GLchar *name);
typedef void (GL_APIENTRYP PFNGLGETUNIFORMFVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETUNIFORMIVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETVERTEXATTRIBPOINTERVCONTEXTANGLE)(GLeglContext ctx, GLuint index, GLenum pname, void **pointer);
typedef void (GL_APIENTRYP PFNGLGETVERTEXATTRIBFVCONTEXTANGLE)(GLeglContext ctx, GLuint index, GLenum pname, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETVERTEXATTRIBIVCONTEXTANGLE)(GLeglContext ctx, GLuint index, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLHINTCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum mode);
typedef void (GL_APIENTRYP PFNGLISBUFFERCONTEXTANGLE)(GLeglContext ctx, GLuint buffer);
typedef void (GL_APIENTRYP PFNGLISENABLEDCONTEXTANGLE)(GLeglContext ctx, GLenum cap);
typedef void (GL_APIENTRYP PFNGLISFRAMEBUFFERCONTEXTANGLE)(GLeglContext ctx, GLuint framebuffer);
typedef void (GL_APIENTRYP PFNGLISPROGRAMCONTEXTANGLE)(GLeglContext ctx, GLuint program);
typedef void (GL_APIENTRYP PFNGLISRENDERBUFFERCONTEXTANGLE)(GLeglContext ctx, GLuint renderbuffer);
typedef void (GL_APIENTRYP PFNGLISSHADERCONTEXTANGLE)(GLeglContext ctx, GLuint shader);
typedef void (GL_APIENTRYP PFNGLISTEXTURECONTEXTANGLE)(GLeglContext ctx, GLuint texture);
typedef void (GL_APIENTRYP PFNGLLINEWIDTHCONTEXTANGLE)(GLeglContext ctx, GLfloat width);
typedef void (GL_APIENTRYP PFNGLLINKPROGRAMCONTEXTANGLE)(GLeglContext ctx, GLuint program);
typedef void (GL_APIENTRYP PFNGLPIXELSTOREICONTEXTANGLE)(GLeglContext ctx, GLenum pname, GLint param);
typedef void (GL_APIENTRYP PFNGLPOLYGONOFFSETCONTEXTANGLE)(GLeglContext ctx, GLfloat factor, GLfloat units);
typedef void (GL_APIENTRYP PFNGLREADPIXELSCONTEXTANGLE)(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
typedef void (GL_APIENTRYP PFNGLRELEASESHADERCOMPILERCONTEXTANGLE)(GLeglContext ctx);
typedef void (GL_APIENTRYP PFNGLRENDERBUFFERSTORAGECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (GL_APIENTRYP PFNGLSAMPLECOVERAGECONTEXTANGLE)(GLeglContext ctx, GLfloat value, GLboolean invert);
typedef void (GL_APIENTRYP PFNGLSCISSORCONTEXTANGLE)(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GL_APIENTRYP PFNGLSHADERBINARYCONTEXTANGLE)(GLeglContext ctx, GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length);
typedef void (GL_APIENTRYP PFNGLSHADERSOURCECONTEXTANGLE)(GLeglContext ctx, GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void (GL_APIENTRYP PFNGLSTENCILFUNCCONTEXTANGLE)(GLeglContext ctx, GLenum func, GLint ref, GLuint mask);
typedef void (GL_APIENTRYP PFNGLSTENCILFUNCSEPARATECONTEXTANGLE)(GLeglContext ctx, GLenum face, GLenum func, GLint ref, GLuint mask);
typedef void (GL_APIENTRYP PFNGLSTENCILMASKCONTEXTANGLE)(GLeglContext ctx, GLuint mask);
typedef void (GL_APIENTRYP PFNGLSTENCILMASKSEPARATECONTEXTANGLE)(GLeglContext ctx, GLenum face, GLuint mask);
typedef void (GL_APIENTRYP PFNGLSTENCILOPCONTEXTANGLE)(GLeglContext ctx, GLenum fail, GLenum zfail, GLenum zpass);
typedef void (GL_APIENTRYP PFNGLSTENCILOPSEPARATECONTEXTANGLE)(GLeglContext ctx, GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
typedef void (GL_APIENTRYP PFNGLTEXIMAGE2DCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void (GL_APIENTRYP PFNGLTEXPARAMETERFCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLfloat param);
typedef void (GL_APIENTRYP PFNGLTEXPARAMETERFVCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, const GLfloat *params);
typedef void (GL_APIENTRYP PFNGLTEXPARAMETERICONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLint param);
typedef void (GL_APIENTRYP PFNGLTEXPARAMETERIVCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, const GLint *params);
typedef void (GL_APIENTRYP PFNGLTEXSUBIMAGE2DCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
typedef void (GL_APIENTRYP PFNGLUNIFORM1FCONTEXTANGLE)(GLeglContext ctx, GLint location, GLfloat v0);
typedef void (GL_APIENTRYP PFNGLUNIFORM1FVCONTEXTANGLE)(GLeglContext ctx, GLint location, GLsizei count, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLUNIFORM1ICONTEXTANGLE)(GLeglContext ctx, GLint location, GLint v0);
typedef void (GL_APIENTRYP PFNGLUNIFORM1IVCONTEXTANGLE)(GLeglContext ctx, GLint location, GLsizei count, const GLint *value);
typedef void (GL_APIENTRYP PFNGLUNIFORM2FCONTEXTANGLE)(GLeglContext ctx, GLint location, GLfloat v0, GLfloat v1);
typedef void (GL_APIENTRYP PFNGLUNIFORM2FVCONTEXTANGLE)(GLeglContext ctx, GLint location, GLsizei count, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLUNIFORM2ICONTEXTANGLE)(GLeglContext ctx, GLint location, GLint v0, GLint v1);
typedef void (GL_APIENTRYP PFNGLUNIFORM2IVCONTEXTANGLE)(GLeglContext ctx, GLint location, GLsizei count, const GLint *value);
typedef void (GL_APIENTRYP PFNGLUNIFORM3FCONTEXTANGLE)(GLeglContext ctx, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (GL_APIENTRYP PFNGLUNIFORM3FVCONTEXTANGLE)(GLeglContext ctx, GLint location, GLsizei count, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLUNIFORM3ICONTEXTANGLE)(GLeglContext ctx, GLint location, GLint v0, GLint v1, GLint v2);
typedef void (GL_APIENTRYP PFNGLUNIFORM3IVCONTEXTANGLE)(GLeglContext ctx, GLint location, GLsizei count, const GLint *value);
typedef void (GL_APIENTRYP PFNGLUNIFORM4FCONTEXTANGLE)(GLeglContext ctx, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (GL_APIENTRYP PFNGLUNIFORM4FVCONTEXTANGLE)(GLeglContext ctx, GLint location, GLsizei count, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLUNIFORM4ICONTEXTANGLE)(GLeglContext ctx, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void (GL_APIENTRYP PFNGLUNIFORM4IVCONTEXTANGLE)(GLeglContext ctx, GLint location, GLsizei count, const GLint *value);
typedef void (GL_APIENTRYP PFNGLUNIFORMMATRIX2FVCONTEXTANGLE)(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLUNIFORMMATRIX3FVCONTEXTANGLE)(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLUNIFORMMATRIX4FVCONTEXTANGLE)(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRYP PFNGLUSEPROGRAMCONTEXTANGLE)(GLeglContext ctx, GLuint program);
typedef void (GL_APIENTRYP PFNGLVALIDATEPROGRAMCONTEXTANGLE)(GLeglContext ctx, GLuint program);
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIB1FCONTEXTANGLE)(GLeglContext ctx, GLuint index, GLfloat x);
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIB1FVCONTEXTANGLE)(GLeglContext ctx, GLuint index, const GLfloat *v);
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIB2FCONTEXTANGLE)(GLeglContext ctx, GLuint index, GLfloat x, GLfloat y);
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIB2FVCONTEXTANGLE)(GLeglContext ctx, GLuint index, const GLfloat *v);
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIB3FCONTEXTANGLE)(GLeglContext ctx, GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIB3FVCONTEXTANGLE)(GLeglContext ctx, GLuint index, const GLfloat *v);
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIB4FCONTEXTANGLE)(GLeglContext ctx, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIB4FVCONTEXTANGLE)(GLeglContext ctx, GLuint index, const GLfloat *v);
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIBPOINTERCONTEXTANGLE)(GLeglContext ctx, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void (GL_APIENTRYP PFNGLVIEWPORTCONTEXTANGLE)(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height);
// GL_ANGLE_framebuffer_blit
typedef void (GL_APIENTRYP PFNGLBLITFRAMEBUFFERANGLECONTEXTANGLE)(GLeglContext ctx, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
// GL_ANGLE_framebuffer_multisample
typedef void (GL_APIENTRYP PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
// GL_ANGLE_instanced_arays
typedef void (GL_APIENTRYP PFNGLDRAWARRAYSINSTANCEDANGLECONTEXTANGLE)(GLeglContext ctx, GLenum mode, GLint first, GLsizei count, GLsizei primcount);
typedef void (GL_APIENTRYP PFNGLDRAWELEMENTSINSTANCEDANGLECONTEXTANGLE)(GLeglContext ctx, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIBDIVISORANGLECONTEXTANGLE)(GLeglContext ctx, GLuint index, GLuint divisor);
typedef void (GL_APIENTRYP PFNGLFRAMEBUFFERTEXTUREMULTIVIEWLAYEREDANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews);
typedef void (GL_APIENTRYP PFNGLFRAMEBUFFERTEXTUREMULTIVIEWSIDEBYSIDEANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum attachment, GLuint texture, GLint level, GLsizei numViews, const GLint * viewportOffsets);
// GL_ANGLE_request_extension
typedef void (GL_APIENTRYP PFNGLREQUESTEXTENSIONANGLECONTEXTANGLE)(GLeglContext ctx, const GLchar * name);
// GL_ANGLE_robust_client_memory
typedef void (GL_APIENTRYP PFNGLGETBOOLEANVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum pname, GLsizei bufSize, GLsizei * length, GLboolean * params);
typedef void (GL_APIENTRYP PFNGLGETBUFFERPARAMETERIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void (GL_APIENTRYP PFNGLGETFLOATVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum pname, GLsizei bufSize, GLsizei * length, GLfloat * params);
typedef void (GL_APIENTRYP PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum attachment, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void (GL_APIENTRYP PFNGLGETINTEGERVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * data);
typedef void (GL_APIENTRYP PFNGLGETPROGRAMIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint program, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void (GL_APIENTRYP PFNGLGETRENDERBUFFERPARAMETERIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void (GL_APIENTRYP PFNGLGETSHADERIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint shader, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void (GL_APIENTRYP PFNGLGETTEXPARAMETERFVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLfloat * params);
typedef void (GL_APIENTRYP PFNGLGETTEXPARAMETERIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void (GL_APIENTRYP PFNGLGETUNIFORMFVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLfloat * params);
typedef void (GL_APIENTRYP PFNGLGETUNIFORMIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void (GL_APIENTRYP PFNGLGETVERTEXATTRIBFVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint index, GLenum pname, GLsizei bufSize, GLsizei * length, GLfloat * params);
typedef void (GL_APIENTRYP PFNGLGETVERTEXATTRIBIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint index, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void (GL_APIENTRYP PFNGLGETVERTEXATTRIBPOINTERVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint index, GLenum pname, GLsizei bufSize, GLsizei * length, void ** pointer);
typedef void (GL_APIENTRYP PFNGLREADPIXELSROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLsizei * length, GLsizei * columns, GLsizei * rows, void * pixels);
typedef void (GL_APIENTRYP PFNGLTEXIMAGE2DROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, GLsizei bufSize, const void * pixels);
typedef void (GL_APIENTRYP PFNGLTEXPARAMETERFVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, const GLfloat * params);
typedef void (GL_APIENTRYP PFNGLTEXPARAMETERIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, const GLint * params);
typedef void (GL_APIENTRYP PFNGLTEXSUBIMAGE2DROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, const void * pixels);
typedef void (GL_APIENTRYP PFNGLTEXIMAGE3DROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, GLsizei bufSize, const void * pixels);
typedef void (GL_APIENTRYP PFNGLTEXSUBIMAGE3DROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, const void * pixels);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXIMAGE2DROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, GLsizei dataSize, const GLvoid * data);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXSUBIMAGE2DROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLint level, GLsizei xoffset, GLsizei yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, GLsizei dataSize, const GLvoid * data);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXIMAGE3DROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, GLsizei dataSize, const GLvoid * data);
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXSUBIMAGE3DROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, GLsizei dataSize, const GLvoid * data);
typedef void (GL_APIENTRYP PFNGLGETQUERYIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTUIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint id, GLenum pname, GLsizei bufSize, GLsizei * length, GLuint * params);
typedef void (GL_APIENTRYP PFNGLGETBUFFERPOINTERVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, void ** params);
typedef void (GL_APIENTRYP PFNGLGETINTEGERI_VROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLuint index, GLsizei bufSize, GLsizei * length, GLint * data);
typedef void (GL_APIENTRYP PFNGLGETINTERNALFORMATIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void (GL_APIENTRYP PFNGLGETVERTEXATTRIBIIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint index, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void (GL_APIENTRYP PFNGLGETVERTEXATTRIBIUIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint index, GLenum pname, GLsizei bufSize, GLsizei * length, GLuint * params);
typedef void (GL_APIENTRYP PFNGLGETUNIFORMUIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLuint * params);
typedef void (GL_APIENTRYP PFNGLGETACTIVEUNIFORMBLOCKIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint program, GLuint uniformBlockIndex, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void (GL_APIENTRYP PFNGLGETINTEGER64VROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum pname, GLsizei bufSize, GLsizei * length, GLint64 * data);
typedef void (GL_APIENTRYP PFNGLGETINTEGER64I_VROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLuint index, GLsizei bufSize, GLsizei * length, GLint64 * data);
typedef void (GL_APIENTRYP PFNGLGETBUFFERPARAMETERI64VROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint64 * params);
typedef void (GL_APIENTRYP PFNGLSAMPLERPARAMETERIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint sampler, GLuint pname, GLsizei bufSize, const GLint * param);
typedef void (GL_APIENTRYP PFNGLSAMPLERPARAMETERFVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, const GLfloat * param);
typedef void (GL_APIENTRYP PFNGLGETSAMPLERPARAMETERIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void (GL_APIENTRYP PFNGLGETSAMPLERPARAMETERFVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei * length, GLfloat * params);
typedef void (GL_APIENTRYP PFNGLGETFRAMEBUFFERPARAMETERIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void (GL_APIENTRYP PFNGLGETPROGRAMINTERFACEIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint program, GLenum programInterface, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void (GL_APIENTRYP PFNGLGETBOOLEANI_VROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLuint index, GLsizei bufSize, GLsizei * length, GLboolean * data);
typedef void (GL_APIENTRYP PFNGLGETMULTISAMPLEFVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum pname, GLuint index, GLsizei bufSize, GLsizei * length, GLfloat * val);
typedef void (GL_APIENTRYP PFNGLGETTEXLEVELPARAMETERIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLint level, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void (GL_APIENTRYP PFNGLGETTEXLEVELPARAMETERFVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLint level, GLenum pname, GLsizei bufSize, GLsizei * length, GLfloat * params);
typedef void (GL_APIENTRYP PFNGLGETPOINTERVROBUSTANGLEROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum pname, GLsizei bufSize, GLsizei * length, void ** params);
typedef void (GL_APIENTRYP PFNGLREADNPIXELSROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLsizei * length, GLsizei * columns, GLsizei * rows, void * data);
typedef void (GL_APIENTRYP PFNGLGETNUNIFORMFVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLfloat * params);
typedef void (GL_APIENTRYP PFNGLGETNUNIFORMIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void (GL_APIENTRYP PFNGLGETNUNIFORMUIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLuint * params);
typedef void (GL_APIENTRYP PFNGLTEXPARAMETERIIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, const GLint * params);
typedef void (GL_APIENTRYP PFNGLTEXPARAMETERIUIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, const GLuint * params);
typedef void (GL_APIENTRYP PFNGLGETTEXPARAMETERIIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void (GL_APIENTRYP PFNGLGETTEXPARAMETERIUIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLuint * params);
typedef void (GL_APIENTRYP PFNGLSAMPLERPARAMETERIIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, const GLint * param);
typedef void (GL_APIENTRYP PFNGLSAMPLERPARAMETERIUIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, const GLuint * param);
typedef void (GL_APIENTRYP PFNGLGETSAMPLERPARAMETERIIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void (GL_APIENTRYP PFNGLGETSAMPLERPARAMETERIUIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei * length, GLuint * params);
typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTIVROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint id, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTI64VROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint id, GLenum pname, GLsizei bufSize, GLsizei * length, GLint64 * params);
typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTUI64VROBUSTANGLECONTEXTANGLE)(GLeglContext ctx, GLuint id, GLenum pname, GLsizei bufSize, GLsizei * length, GLuint64 * params);
// GL_ANGLE_translated_shader_source
typedef void (GL_APIENTRYP PFNGLGETTRANSLATEDSHADERSOURCEANGLECONTEXTANGLE)(GLeglContext ctx, GLuint shader, GLsizei bufsize, GLsizei *length, GLchar *source);
// GL_CHROMIUM_bind_uniform_location
typedef void (GL_APIENTRYP PFNGLBINDUNIFORMLOCATIONCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, const GLchar* name);
// GL_CHROMIUM_copy_compressed_texture
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDCOPYTEXTURECHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLuint sourceId, GLuint destId);
typedef void (GL_APIENTRYP PFNGLCOPYTEXTURECHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLuint sourceId, GLint sourceLevel, GLenum destTarget, GLuint destId, GLint destLevel, GLint internalFormat, GLenum destType, GLboolean unpackFlipY, GLboolean unpackPremultiplyAlpha, GLboolean unpackUnmultiplyAlpha);
typedef void (GL_APIENTRYP PFNGLCOPYSUBTEXTURECHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLuint sourceId, GLint sourceLevel, GLenum destTarget, GLuint destId, GLint destLevel, GLint xoffset, GLint yoffset, GLint x, GLint y, GLint width, GLint height, GLboolean unpackFlipY, GLboolean unpackPremultiplyAlpha, GLboolean unpackUnmultiplyAlpha);
// GL_CHROMIUM_framebuffer_mixed_samples
typedef void (GL_APIENTRYP PFNGLCOVERAGEMODULATIONCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLenum components);
typedef void (GL_APIENTRYP PFNGLMATRIXLOADFCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLenum matrixMode, const GLfloat * matrix);
typedef void (GL_APIENTRYP PFNGLMATRIXLOADIDENTITYCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLenum matrixMode);
// GL_CHROMIUM_path_rendering
typedef void (GL_APIENTRYP PFNGLGENPATHSCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLsizei range);
typedef void (GL_APIENTRYP PFNGLDELETEPATHSCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLuint first, GLsizei range);
typedef void (GL_APIENTRYP PFNGLISPATHCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLuint path);
typedef void (GL_APIENTRYP PFNGLPATHCOMMANDSCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLuint path, GLsizei numCommands, const GLubyte * commands, GLsizei numCoords, GLenum coordType, const void* coords);
typedef void (GL_APIENTRYP PFNGLPATHPARAMETERFCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLuint path, GLenum pname, GLfloat value);
typedef void (GL_APIENTRYP PFNGLPATHPARAMETERICHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLuint path, GLenum pname, GLint value);
typedef void (GL_APIENTRYP PFNGLGETPATHPARAMETERFVCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLuint path, GLenum pname, GLfloat * value);
typedef void (GL_APIENTRYP PFNGLGETPATHPARAMETERIVCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLuint path, GLenum pname, GLint * value);
typedef void (GL_APIENTRYP PFNGLPATHSTENCILFUNCCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLenum func, GLint ref, GLuint mask);
typedef void (GL_APIENTRYP PFNGLSTENCILFILLPATHCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLuint path, GLenum fillMode, GLuint mask);
typedef void (GL_APIENTRYP PFNGLSTENCILSTROKEPATHCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLuint path, GLint reference, GLuint mask);
typedef void (GL_APIENTRYP PFNGLCOVERFILLPATHCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLuint path, GLenum coverMode);
typedef void (GL_APIENTRYP PFNGLCOVERSTROKEPATHCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLuint path, GLenum coverMode);
typedef void (GL_APIENTRYP PFNGLSTENCILTHENCOVERFILLPATHCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLuint path, GLenum fillMode, GLuint mask, GLenum coverMode);
typedef void (GL_APIENTRYP PFNGLSTENCILTHENCOVERSTROKEPATHCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLuint path, GLint reference, GLuint mask, GLenum coverMode);
typedef void (GL_APIENTRYP PFNGLCOVERFILLPATHINSTANCEDCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLsizei numPath, GLenum pathNameType, const void * paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat * transformValues);
typedef void (GL_APIENTRYP PFNGLCOVERSTROKEPATHINSTANCEDCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLsizei numPath, GLenum pathNameType, const void * paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat * transformValues);
typedef void (GL_APIENTRYP PFNGLSTENCILSTROKEPATHINSTANCEDCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLsizei numPath, GLenum pathNameType, const void * paths, GLuint pathBase, GLint reference, GLuint mask, GLenum transformType, const GLfloat * transformValues);
typedef void (GL_APIENTRYP PFNGLSTENCILFILLPATHINSTANCEDCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLsizei numPaths, GLenum pathNameType, const void * paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum transformType, const GLfloat * transformValues);
typedef void (GL_APIENTRYP PFNGLSTENCILTHENCOVERFILLPATHINSTANCEDCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLsizei numPaths, GLenum pathNameType, const void * paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat * transformValues);
typedef void (GL_APIENTRYP PFNGLSTENCILTHENCOVERSTROKEPATHINSTANCEDCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLsizei numPaths, GLenum pathNameType, const void * paths, GLuint pathBase, GLint reference, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat * transformValues);
typedef void (GL_APIENTRYP PFNGLBINDFRAGMENTINPUTLOCATIONCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLuint programs, GLint location, const GLchar * name);
typedef void (GL_APIENTRYP PFNGLPROGRAMPATHFRAGMENTINPUTGENCHROMIUMCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLenum genMode, GLint components, const GLfloat * coeffs);
// GL_EXT_debug_marker
typedef void (GL_APIENTRYP PFNGLINSERTEVENTMARKEREXTCONTEXTANGLE)(GLeglContext ctx, GLsizei length, const GLchar *marker);
typedef void (GL_APIENTRYP PFNGLPOPGROUPMARKEREXTCONTEXTANGLE)(GLeglContext ctx);
typedef void (GL_APIENTRYP PFNGLPUSHGROUPMARKEREXTCONTEXTANGLE)(GLeglContext ctx, GLsizei length, const GLchar *marker);
// GL_EXT_discard_framebuffer
typedef void (GL_APIENTRYP PFNGLDISCARDFRAMEBUFFEREXTCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLsizei numAttachments, const GLenum *attachments);
// GL_EXT_disjoint_timer_query
typedef void (GL_APIENTRYP PFNGLBEGINQUERYEXTCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLuint id);
typedef void (GL_APIENTRYP PFNGLDELETEQUERIESEXTCONTEXTANGLE)(GLeglContext ctx, GLsizei n, const GLuint *ids);
typedef void (GL_APIENTRYP PFNGLENDQUERYEXTCONTEXTANGLE)(GLeglContext ctx, GLenum target);
typedef void (GL_APIENTRYP PFNGLGENQUERIESEXTCONTEXTANGLE)(GLeglContext ctx, GLsizei n, GLuint *ids);
typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTI64VEXTCONTEXTANGLE)(GLeglContext ctx, GLuint id, GLenum pname, GLint64 *params);
typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTIVEXTCONTEXTANGLE)(GLeglContext ctx, GLuint id, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTUI64VEXTCONTEXTANGLE)(GLeglContext ctx, GLuint id, GLenum pname, GLuint64 *params);
typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTUIVEXTCONTEXTANGLE)(GLeglContext ctx, GLuint id, GLenum pname, GLuint *params);
typedef void (GL_APIENTRYP PFNGLGETQUERYIVEXTCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLISQUERYEXTCONTEXTANGLE)(GLeglContext ctx, GLuint id);
typedef void (GL_APIENTRYP PFNGLQUERYCOUNTEREXTCONTEXTANGLE)(GLeglContext ctx, GLuint id, GLenum target);
// GL_EXT_draw_buffers
typedef void (GL_APIENTRYP PFNGLDRAWBUFFERSEXTCONTEXTANGLE)(GLeglContext ctx, GLsizei n, const GLenum *bufs);
// GL_EXT_map_buffer_range
typedef void (GL_APIENTRYP PFNGLFLUSHMAPPEDBUFFERRANGEEXTCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLintptr offset, GLsizeiptr length);
typedef void (GL_APIENTRYP PFNGLMAPBUFFERRANGEEXTCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
// GL_EXT_robustness
typedef void (GL_APIENTRYP PFNGLGETGRAPHICSRESETSTATUSEXTCONTEXTANGLE)(GLeglContext ctx);
typedef void (GL_APIENTRYP PFNGLGETNUNIFORMFVEXTCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETNUNIFORMIVEXTCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLint *params);
typedef void (GL_APIENTRYP PFNGLREADNPIXELSEXTCONTEXTANGLE)(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
// GL_EXT_texture_storage
typedef void (GL_APIENTRYP PFNGLTEXSTORAGE1DEXTCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
typedef void (GL_APIENTRYP PFNGLTEXSTORAGE2DEXTCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (GL_APIENTRYP PFNGLTEXSTORAGE3DEXTCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
// GL_KHR_debug
typedef void (GL_APIENTRYP PFNGLDEBUGMESSAGECALLBACKKHRCONTEXTANGLE)(GLeglContext ctx, GLDEBUGPROCKHR callback, const void *userParam);
typedef void (GL_APIENTRYP PFNGLDEBUGMESSAGECONTROLKHRCONTEXTANGLE)(GLeglContext ctx, GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
typedef void (GL_APIENTRYP PFNGLDEBUGMESSAGEINSERTKHRCONTEXTANGLE)(GLeglContext ctx, GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
typedef void (GL_APIENTRYP PFNGLGETDEBUGMESSAGELOGKHRCONTEXTANGLE)(GLeglContext ctx, GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
typedef void (GL_APIENTRYP PFNGLGETOBJECTLABELKHRCONTEXTANGLE)(GLeglContext ctx, GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label);
typedef void (GL_APIENTRYP PFNGLGETOBJECTPTRLABELKHRCONTEXTANGLE)(GLeglContext ctx, const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label);
typedef void (GL_APIENTRYP PFNGLGETPOINTERVKHRCONTEXTANGLE)(GLeglContext ctx, GLenum pname, void **params);
typedef void (GL_APIENTRYP PFNGLOBJECTLABELKHRCONTEXTANGLE)(GLeglContext ctx, GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
typedef void (GL_APIENTRYP PFNGLOBJECTPTRLABELKHRCONTEXTANGLE)(GLeglContext ctx, const void *ptr, GLsizei length, const GLchar *label);
typedef void (GL_APIENTRYP PFNGLPOPDEBUGGROUPKHRCONTEXTANGLE)(GLeglContext ctx);
typedef void (GL_APIENTRYP PFNGLPUSHDEBUGGROUPKHRCONTEXTANGLE)(GLeglContext ctx, GLenum source, GLuint id, GLsizei length, const GLchar *message);
// GL_NV_fence
typedef void (GL_APIENTRYP PFNGLDELETEFENCESNVCONTEXTANGLE)(GLeglContext ctx, GLsizei n, const GLuint *fences);
typedef void (GL_APIENTRYP PFNGLFINISHFENCENVCONTEXTANGLE)(GLeglContext ctx, GLuint fence);
typedef void (GL_APIENTRYP PFNGLGENFENCESNVCONTEXTANGLE)(GLeglContext ctx, GLsizei n, GLuint *fences);
typedef void (GL_APIENTRYP PFNGLGETFENCEIVNVCONTEXTANGLE)(GLeglContext ctx, GLuint fence, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLISFENCENVCONTEXTANGLE)(GLeglContext ctx, GLuint fence);
typedef void (GL_APIENTRYP PFNGLSETFENCENVCONTEXTANGLE)(GLeglContext ctx, GLuint fence, GLenum condition);
typedef void (GL_APIENTRYP PFNGLTESTFENCENVCONTEXTANGLE)(GLeglContext ctx, GLuint fence);
// GL_OES_EGL_image
typedef void (GL_APIENTRYP PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLeglImageOES image);
typedef void (GL_APIENTRYP PFNGLEGLIMAGETARGETTEXTURE2DOESCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLeglImageOES image);
// GL_OES_get_program_binary
typedef void (GL_APIENTRYP PFNGLGETPROGRAMBINARYOESCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
typedef void (GL_APIENTRYP PFNGLPROGRAMBINARYOESCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLenum binaryFormat, const void *binary, GLint length);
// GL_OES_mapbuffer
typedef void (GL_APIENTRYP PFNGLGETBUFFERPOINTERVOESCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, void **params);
typedef void (GL_APIENTRYP PFNGLMAPBUFFEROESCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum access);
typedef void (GL_APIENTRYP PFNGLUNMAPBUFFEROESCONTEXTANGLE)(GLeglContext ctx, GLenum target);
typedef void (GL_APIENTRYP PFNGLBINDVERTEXARRAYOESCONTEXTANGLE)(GLeglContext ctx, GLuint array);
typedef void (GL_APIENTRYP PFNGLDELETEVERTEXARRAYSOESCONTEXTANGLE)(GLeglContext ctx, GLsizei n, const GLuint *arrays);
typedef void (GL_APIENTRYP PFNGLGENVERTEXARRAYSOESCONTEXTANGLE)(GLeglContext ctx, GLsizei n, GLuint *arrays);
typedef void (GL_APIENTRYP PFNGLISVERTEXARRAYOESCONTEXTANGLE)(GLeglContext ctx, GLuint array);
#ifdef GL_GLEXT_PROTOTYPES
// OpenGL ES 2.0
GL_APICALL void glActiveTextureContextANGLE(GLeglContext ctx, GLenum texture);
GL_APICALL void glAttachShaderContextANGLE(GLeglContext ctx, GLuint program, GLuint shader);
GL_APICALL void glBindAttribLocationContextANGLE(GLeglContext ctx, GLuint program, GLuint index, const GLchar *name);
GL_APICALL void glBindBufferContextANGLE(GLeglContext ctx, GLenum target, GLuint buffer);
GL_APICALL void glBindFramebufferContextANGLE(GLeglContext ctx, GLenum target, GLuint framebuffer);
GL_APICALL void glBindRenderbufferContextANGLE(GLeglContext ctx, GLenum target, GLuint renderbuffer);
GL_APICALL void glBindTextureContextANGLE(GLeglContext ctx, GLenum target, GLuint texture);
GL_APICALL void glBlendColorContextANGLE(GLeglContext ctx, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
GL_APICALL void glBlendEquationContextANGLE(GLeglContext ctx, GLenum mode);
GL_APICALL void glBlendEquationSeparateContextANGLE(GLeglContext ctx, GLenum modeRGB, GLenum modeAlpha);
GL_APICALL void glBlendFuncContextANGLE(GLeglContext ctx, GLenum sfactor, GLenum dfactor);
GL_APICALL void glBlendFuncSeparateContextANGLE(GLeglContext ctx, GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
GL_APICALL void glBufferDataContextANGLE(GLeglContext ctx, GLenum target, GLsizeiptr size, const void *data, GLenum usage);
GL_APICALL void glBufferSubDataContextANGLE(GLeglContext ctx, GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
GL_APICALL GLenum glCheckFramebufferStatusContextANGLE(GLeglContext ctx, GLenum target);
GL_APICALL void glClearContextANGLE(GLeglContext ctx, GLbitfield mask);
GL_APICALL void glClearColorContextANGLE(GLeglContext ctx, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
GL_APICALL void glClearDepthfContextANGLE(GLeglContext ctx, GLfloat d);
GL_APICALL void glClearStencilContextANGLE(GLeglContext ctx, GLint s);
GL_APICALL void glColorMaskContextANGLE(GLeglContext ctx, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
GL_APICALL void glCompileShaderContextANGLE(GLeglContext ctx, GLuint shader);
GL_APICALL void glCompressedTexImage2DContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
GL_APICALL void glCompressedTexSubImage2DContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
GL_APICALL void glCopyTexImage2DContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
GL_APICALL void glCopyTexSubImage2DContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GL_APICALL GLuint glCreateProgramContextANGLE(GLeglContext ctx);
GL_APICALL GLuint glCreateShaderContextANGLE(GLeglContext ctx, GLenum type);
GL_APICALL void glCullFaceContextANGLE(GLeglContext ctx, GLenum mode);
GL_APICALL void glDeleteBuffersContextANGLE(GLeglContext ctx, GLsizei n, const GLuint *buffers);
GL_APICALL void glDeleteFramebuffersContextANGLE(GLeglContext ctx, GLsizei n, const GLuint *framebuffers);
GL_APICALL void glDeleteProgramContextANGLE(GLeglContext ctx, GLuint program);
GL_APICALL void glDeleteRenderbuffersContextANGLE(GLeglContext ctx, GLsizei n, const GLuint *renderbuffers);
GL_APICALL void glDeleteShaderContextANGLE(GLeglContext ctx, GLuint shader);
GL_APICALL void glDeleteTexturesContextANGLE(GLeglContext ctx, GLsizei n, const GLuint *textures);
GL_APICALL void glDepthFuncContextANGLE(GLeglContext ctx, GLenum func);
GL_APICALL void glDepthMaskContextANGLE(GLeglContext ctx, GLboolean flag);
GL_APICALL void glDepthRangefContextANGLE(GLeglContext ctx, GLfloat n, GLfloat f);
GL_APICALL void glDetachShaderContextANGLE(GLeglContext ctx, GLuint program, GLuint shader);
GL_APICALL void glDisableContextANGLE(GLeglContext ctx, GLenum cap);
GL_APICALL void glDisableVertexAttribArrayContextANGLE(GLeglContext ctx, GLuint index);
GL_APICALL void glDrawArraysContextANGLE(GLeglContext ctx, GLenum mode, GLint first, GLsizei count);
GL_APICALL void glDrawElementsContextANGLE(GLeglContext ctx, GLenum mode, GLsizei count, GLenum type, const void *indices);
GL_APICALL void glEnableContextANGLE(GLeglContext ctx, GLenum cap);
GL_APICALL void glEnableVertexAttribArrayContextANGLE(GLeglContext ctx, GLuint index);
GL_APICALL void glFinishContextANGLE(GLeglContext ctx);
GL_APICALL void glFlushContextANGLE(GLeglContext ctx);
GL_APICALL void glFramebufferRenderbufferContextANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
GL_APICALL void glFramebufferTexture2DContextANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
GL_APICALL void glFrontFaceContextANGLE(GLeglContext ctx, GLenum mode);
GL_APICALL void glGenBuffersContextANGLE(GLeglContext ctx, GLsizei n, GLuint *buffers);
GL_APICALL void glGenFramebuffersContextANGLE(GLeglContext ctx, GLsizei n, GLuint *framebuffers);
GL_APICALL void glGenRenderbuffersContextANGLE(GLeglContext ctx, GLsizei n, GLuint *renderbuffers);
GL_APICALL void glGenTexturesContextANGLE(GLeglContext ctx, GLsizei n, GLuint *textures);
GL_APICALL void glGenerateMipmapContextANGLE(GLeglContext ctx, GLenum target);
GL_APICALL void glGetActiveAttribContextANGLE(GLeglContext ctx, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
GL_APICALL void glGetActiveUniformContextANGLE(GLeglContext ctx, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
GL_APICALL void glGetAttachedShadersContextANGLE(GLeglContext ctx, GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
GL_APICALL GLint glGetAttribLocationContextANGLE(GLeglContext ctx, GLuint program, const GLchar *name);
GL_APICALL void glGetBooleanvContextANGLE(GLeglContext ctx, GLenum pname, GLboolean *data);
GL_APICALL void glGetBufferParameterivContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
GL_APICALL GLenum glGetErrorContextANGLE(GLeglContext ctx);
GL_APICALL void glGetFloatvContextANGLE(GLeglContext ctx, GLenum pname, GLfloat *data);
GL_APICALL void glGetFramebufferAttachmentParameterivContextANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLenum pname, GLint *params);
GL_APICALL void glGetIntegervContextANGLE(GLeglContext ctx, GLenum pname, GLint *data);
GL_APICALL void glGetProgramInfoLogContextANGLE(GLeglContext ctx, GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GL_APICALL void glGetProgramivContextANGLE(GLeglContext ctx, GLuint program, GLenum pname, GLint *params);
GL_APICALL void glGetRenderbufferParameterivContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
GL_APICALL void glGetShaderInfoLogContextANGLE(GLeglContext ctx, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GL_APICALL void glGetShaderPrecisionFormatContextANGLE(GLeglContext ctx, GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
GL_APICALL void glGetShaderSourceContextANGLE(GLeglContext ctx, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
GL_APICALL void glGetShaderivContextANGLE(GLeglContext ctx, GLuint shader, GLenum pname, GLint *params);
GL_APICALL const GLubyte * glGetStringContextANGLE(GLeglContext ctx, GLenum name);
GL_APICALL void glGetTexParameterfvContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfloat *params);
GL_APICALL void glGetTexParameterivContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
GL_APICALL GLint glGetUniformLocationContextANGLE(GLeglContext ctx, GLuint program, const GLchar *name);
GL_APICALL void glGetUniformfvContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLfloat *params);
GL_APICALL void glGetUniformivContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLint *params);
GL_APICALL void glGetVertexAttribPointervContextANGLE(GLeglContext ctx, GLuint index, GLenum pname, void **pointer);
GL_APICALL void glGetVertexAttribfvContextANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLfloat *params);
GL_APICALL void glGetVertexAttribivContextANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLint *params);
GL_APICALL void glHintContextANGLE(GLeglContext ctx, GLenum target, GLenum mode);
GL_APICALL GLboolean glIsBufferContextANGLE(GLeglContext ctx, GLuint buffer);
GL_APICALL GLboolean glIsEnabledContextANGLE(GLeglContext ctx, GLenum cap);
GL_APICALL GLboolean glIsFramebufferContextANGLE(GLeglContext ctx, GLuint framebuffer);
GL_APICALL GLboolean glIsProgramContextANGLE(GLeglContext ctx, GLuint program);
GL_APICALL GLboolean glIsRenderbufferContextANGLE(GLeglContext ctx, GLuint renderbuffer);
GL_APICALL GLboolean glIsShaderContextANGLE(GLeglContext ctx, GLuint shader);
GL_APICALL GLboolean glIsTextureContextANGLE(GLeglContext ctx, GLuint texture);
GL_APICALL void glLineWidthContextANGLE(GLeglContext ctx, GLfloat width);
GL_APICALL void glLinkProgramContextANGLE(GLeglContext ctx, GLuint program);
GL_APICALL void glPixelStoreiContextANGLE(GLeglContext ctx, GLenum pname, GLint param);
GL_APICALL void glPolygonOffsetContextANGLE(GLeglContext ctx, GLfloat factor, GLfloat units);
GL_APICALL void glReadPixelsContextANGLE(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
GL_APICALL void glReleaseShaderCompilerContextANGLE(GLeglContext ctx);
GL_APICALL void glRenderbufferStorageContextANGLE(GLeglContext ctx, GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
GL_APICALL void glSampleCoverageContextANGLE(GLeglContext ctx, GLfloat value, GLboolean invert);
GL_APICALL void glScissorContextANGLE(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height);
GL_APICALL void glShaderBinaryContextANGLE(GLeglContext ctx, GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length);
GL_APICALL void glShaderSourceContextANGLE(GLeglContext ctx, GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
GL_APICALL void glStencilFuncContextANGLE(GLeglContext ctx, GLenum func, GLint ref, GLuint mask);
GL_APICALL void glStencilFuncSeparateContextANGLE(GLeglContext ctx, GLenum face, GLenum func, GLint ref, GLuint mask);
GL_APICALL void glStencilMaskContextANGLE(GLeglContext ctx, GLuint mask);
GL_APICALL void glStencilMaskSeparateContextANGLE(GLeglContext ctx, GLenum face, GLuint mask);
GL_APICALL void glStencilOpContextANGLE(GLeglContext ctx, GLenum fail, GLenum zfail, GLenum zpass);
GL_APICALL void glStencilOpSeparateContextANGLE(GLeglContext ctx, GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
GL_APICALL void glTexImage2DContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
GL_APICALL void glTexParameterfContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfloat param);
GL_APICALL void glTexParameterfvContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, const GLfloat *params);
GL_APICALL void glTexParameteriContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint param);
GL_APICALL void glTexParameterivContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, const GLint *params);
GL_APICALL void glTexSubImage2DContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
GL_APICALL void glUniform1fContextANGLE(GLeglContext ctx, GLint location, GLfloat v0);
GL_APICALL void glUniform1fvContextANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void glUniform1iContextANGLE(GLeglContext ctx, GLint location, GLint v0);
GL_APICALL void glUniform1ivContextANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLint *value);
GL_APICALL void glUniform2fContextANGLE(GLeglContext ctx, GLint location, GLfloat v0, GLfloat v1);
GL_APICALL void glUniform2fvContextANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void glUniform2iContextANGLE(GLeglContext ctx, GLint location, GLint v0, GLint v1);
GL_APICALL void glUniform2ivContextANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLint *value);
GL_APICALL void glUniform3fContextANGLE(GLeglContext ctx, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
GL_APICALL void glUniform3fvContextANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void glUniform3iContextANGLE(GLeglContext ctx, GLint location, GLint v0, GLint v1, GLint v2);
GL_APICALL void glUniform3ivContextANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLint *value);
GL_APICALL void glUniform4fContextANGLE(GLeglContext ctx, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
GL_APICALL void glUniform4fvContextANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void glUniform4iContextANGLE(GLeglContext ctx, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
GL_APICALL void glUniform4ivContextANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLint *value);
GL_APICALL void glUniformMatrix2fvContextANGLE(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void glUniformMatrix3fvContextANGLE(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void glUniformMatrix4fvContextANGLE(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void glUseProgramContextANGLE(GLeglContext ctx, GLuint program);
GL_APICALL void glValidateProgramContextANGLE(GLeglContext ctx, GLuint program);
GL_APICALL void glVertexAttrib1fContextANGLE(GLeglContext ctx, GLuint index, GLfloat x);
GL_APICALL void glVertexAttrib1fvContextANGLE(GLeglContext ctx, GLuint index, const GLfloat *v);
GL_APICALL void glVertexAttrib2fContextANGLE(GLeglContext ctx, GLuint index, GLfloat x, GLfloat y);
GL_APICALL void glVertexAttrib2fvContextANGLE(GLeglContext ctx, GLuint index, const GLfloat *v);
GL_APICALL void glVertexAttrib3fContextANGLE(GLeglContext ctx, GLuint index, GLfloat x, GLfloat y, GLfloat z);
GL_APICALL void glVertexAttrib3fvContextANGLE(GLeglContext ctx, GLuint index, const GLfloat *v);
GL_APICALL void glVertexAttrib4fContextANGLE(GLeglContext ctx, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GL_APICALL void glVertexAttrib4fvContextANGLE(GLeglContext ctx, GLuint index, const GLfloat *v);
GL_APICALL void glVertexAttribPointerContextANGLE(GLeglContext ctx, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
GL_APICALL void glViewportContextANGLE(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height);
// GL_ANGLE_framebuffer_blit
GL_APICALL void glBlitFramebufferANGLEContextANGLE(GLeglContext ctx, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
// GL_ANGLE_framebuffer_multisample
GL_APICALL void glRenderbufferStorageMultisampleANGLEContextANGLE(GLeglContext ctx, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
// GL_ANGLE_instanced_arays
GL_APICALL void glDrawArraysInstancedANGLEContextANGLE(GLeglContext ctx, GLenum mode, GLint first, GLsizei count, GLsizei primcount);
GL_APICALL void glDrawElementsInstancedANGLEContextANGLE(GLeglContext ctx, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
GL_APICALL void glVertexAttribDivisorANGLEContextANGLE(GLeglContext ctx, GLuint index, GLuint divisor);
// GL_ANGLE_multiview
GL_APICALL void glFramebufferTextureMultiviewLayeredANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews);
GL_APICALL void glFramebufferTextureMultiviewSideBySideANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLuint texture, GLint level, GLsizei numViews, const GLint * viewportOffsets);
// GL_ANGLE_request_extension
GL_APICALL void glRequestExtensionANGLEContextANGLE(GLeglContext ctx, const GLchar * name);
// GL_ANGLE_robust_client_memory
GL_APICALL void glGetBooleanvRobustANGLEContextANGLE(GLeglContext ctx, GLenum pname, GLsizei bufSize, GLsizei * length, GLboolean * params);
GL_APICALL void glGetBufferParameterivRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void glGetFloatvRobustANGLEContextANGLE(GLeglContext ctx, GLenum pname, GLsizei bufSize, GLsizei * length, GLfloat * params);
GL_APICALL void glGetFramebufferAttachmentParameterivRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void glGetIntegervRobustANGLEContextANGLE(GLeglContext ctx, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * data);
GL_APICALL void glGetProgramivRobustANGLEContextANGLE(GLeglContext ctx, GLuint program, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void glGetRenderbufferParameterivRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void glGetShaderivRobustANGLEContextANGLE(GLeglContext ctx, GLuint shader, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void glGetTexParameterfvRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLfloat * params);
GL_APICALL void glGetTexParameterivRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void glGetUniformfvRobustANGLEContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLfloat * params);
GL_APICALL void glGetUniformivRobustANGLEContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void glGetVertexAttribfvRobustANGLEContextANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLsizei bufSize, GLsizei * length, GLfloat * params);
GL_APICALL void glGetVertexAttribivRobustANGLEContextANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void glGetVertexAttribPointervRobustANGLEContextANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLsizei bufSize, GLsizei * length, void ** pointer);
GL_APICALL void glReadPixelsRobustANGLEContextANGLE(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLsizei * length, GLsizei * columns, GLsizei * rows, void * pixels);
GL_APICALL void glTexImage2DRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, GLsizei bufSize, const void * pixels);
GL_APICALL void glTexParameterfvRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, const GLfloat * params);
GL_APICALL void glTexParameterivRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, const GLint * params);
GL_APICALL void glTexSubImage2DRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, const void * pixels);
GL_APICALL void glTexImage3DRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, GLsizei bufSize, const void * pixels);
GL_APICALL void glTexSubImage3DRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, const void * pixels);
GL_APICALL void glCompressedTexImage2DRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, GLsizei dataSize, const GLvoid * data);
GL_APICALL void glCompressedTexSubImage2DRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLsizei xoffset, GLsizei yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, GLsizei dataSize, const GLvoid * data);
GL_APICALL void glCompressedTexImage3DRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, GLsizei dataSize, const GLvoid * data);
GL_APICALL void glCompressedTexSubImage3DRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, GLsizei dataSize, const GLvoid * data);
GL_APICALL void glGetQueryivRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void glGetQueryObjectuivRobustANGLEContextANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLsizei bufSize, GLsizei * length, GLuint * params);
GL_APICALL void glGetBufferPointervRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, void ** params);
GL_APICALL void glGetIntegeri_vRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLuint index, GLsizei bufSize, GLsizei * length, GLint * data);
GL_APICALL void glGetInternalformativRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void glGetVertexAttribIivRobustANGLEContextANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void glGetVertexAttribIuivRobustANGLEContextANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLsizei bufSize, GLsizei * length, GLuint * params);
GL_APICALL void glGetUniformuivRobustANGLEContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLuint * params);
GL_APICALL void glGetActiveUniformBlockivRobustANGLEContextANGLE(GLeglContext ctx, GLuint program, GLuint uniformBlockIndex, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void glGetInteger64vRobustANGLEContextANGLE(GLeglContext ctx, GLenum pname, GLsizei bufSize, GLsizei * length, GLint64 * data);
GL_APICALL void glGetInteger64i_vRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLuint index, GLsizei bufSize, GLsizei * length, GLint64 * data);
GL_APICALL void glGetBufferParameteri64vRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint64 * params);
GL_APICALL void glSamplerParameterivRobustANGLEContextANGLE(GLeglContext ctx, GLuint sampler, GLuint pname, GLsizei bufSize, const GLint * param);
GL_APICALL void glSamplerParameterfvRobustANGLEContextANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, const GLfloat * param);
GL_APICALL void glGetSamplerParameterivRobustANGLEContextANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void glGetSamplerParameterfvRobustANGLEContextANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei * length, GLfloat * params);
GL_APICALL void glGetFramebufferParameterivRobustANGLEContextANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void glGetProgramInterfaceivRobustANGLEContextANGLE(GLeglContext ctx, GLuint program, GLenum programInterface, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void glGetBooleani_vRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLuint index, GLsizei bufSize, GLsizei * length, GLboolean * data);
GL_APICALL void glGetMultisamplefvRobustANGLEContextANGLE(GLeglContext ctx, GLenum pname, GLuint index, GLsizei bufSize, GLsizei * length, GLfloat * val);
GL_APICALL void glGetTexLevelParameterivRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void glGetTexLevelParameterfvRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum pname, GLsizei bufSize, GLsizei * length, GLfloat * params);
GL_APICALL void glGetPointervRobustANGLERobustANGLEContextANGLE(GLeglContext ctx, GLenum pname, GLsizei bufSize, GLsizei * length, void ** params);
GL_APICALL void glReadnPixelsRobustANGLEContextANGLE(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLsizei * length, GLsizei * columns, GLsizei * rows, void * data);
GL_APICALL void glGetnUniformfvRobustANGLEContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLfloat * params);
GL_APICALL void glGetnUniformivRobustANGLEContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void glGetnUniformuivRobustANGLEContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLuint * params);
GL_APICALL void glTexParameterIivRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, const GLint * params);
GL_APICALL void glTexParameterIuivRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, const GLuint * params);
GL_APICALL void glGetTexParameterIivRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void glGetTexParameterIuivRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLuint * params);
GL_APICALL void glSamplerParameterIivRobustANGLEContextANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, const GLint * param);
GL_APICALL void glSamplerParameterIuivRobustANGLEContextANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, const GLuint * param);
GL_APICALL void glGetSamplerParameterIivRobustANGLEContextANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void glGetSamplerParameterIuivRobustANGLEContextANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei * length, GLuint * params);
GL_APICALL void glGetQueryObjectivRobustANGLEContextANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void glGetQueryObjecti64vRobustANGLEContextANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLsizei bufSize, GLsizei * length, GLint64 * params);
GL_APICALL void glGetQueryObjectui64vRobustANGLEContextANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLsizei bufSize, GLsizei * length, GLuint64 * params);
// GL_ANGLE_translated_shader_source
GL_APICALL void glGetTranslatedShaderSourceANGLEContextANGLE(GLeglContext ctx, GLuint shader, GLsizei bufsize, GLsizei *length, GLchar *source);
// GL_CHROMIUM_bind_uniform_location
GL_APICALL void glBindUniformLocationCHROMIUMContextANGLE(GLeglContext ctx, GLuint program, GLint location, const GLchar* name);
// GL_CHROMIUM_copy_compressed_texture
GL_APICALL void glCompressedCopyTextureCHROMIUMContextANGLE(GLeglContext ctx, GLuint sourceId, GLuint destId);
GL_APICALL void glCopyTextureCHROMIUMContextANGLE(GLeglContext ctx, GLuint sourceId, GLint sourceLevel, GLenum destTarget, GLuint destId, GLint destLevel, GLint internalFormat, GLenum destType, GLboolean unpackFlipY, GLboolean unpackPremultiplyAlpha, GLboolean unpackUnmultiplyAlpha);
GL_APICALL void glCopySubTextureCHROMIUMContextANGLE(GLeglContext ctx, GLuint sourceId, GLint sourceLevel, GLenum destTarget, GLuint destId, GLint destLevel, GLint xoffset, GLint yoffset, GLint x, GLint y, GLint width, GLint height, GLboolean unpackFlipY, GLboolean unpackPremultiplyAlpha, GLboolean unpackUnmultiplyAlpha);
// GL_CHROMIUM_framebuffer_mixed_samples
GL_APICALL void glCoverageModulationCHROMIUMContextANGLE(GLeglContext ctx, GLenum components);
GL_APICALL void glMatrixLoadfCHROMIUMContextANGLE(GLeglContext ctx, GLenum matrixMode, const GLfloat * matrix);
GL_APICALL void glMatrixLoadIdentityCHROMIUMContextANGLE(GLeglContext ctx, GLenum matrixMode);
// GL_CHROMIUM_path_rendering
GL_APICALL GLuint glGenPathsCHROMIUMContextANGLE(GLeglContext ctx, GLsizei range);
GL_APICALL void glDeletePathsCHROMIUMContextANGLE(GLeglContext ctx, GLuint first, GLsizei range);
GL_APICALL GLboolean glIsPathCHROMIUMContextANGLE(GLeglContext ctx, GLuint path);
GL_APICALL void glPathCommandsCHROMIUMContextANGLE(GLeglContext ctx, GLuint path, GLsizei numCommands, const GLubyte * commands, GLsizei numCoords, GLenum coordType, const void* coords);
GL_APICALL void glPathParameterfCHROMIUMContextANGLE(GLeglContext ctx, GLuint path, GLenum pname, GLfloat value);
GL_APICALL void glPathParameteriCHROMIUMContextANGLE(GLeglContext ctx, GLuint path, GLenum pname, GLint value);
GL_APICALL void glGetPathParameterfvCHROMIUMContextANGLE(GLeglContext ctx, GLuint path, GLenum pname, GLfloat * value);
GL_APICALL void glGetPathParameterivCHROMIUMContextANGLE(GLeglContext ctx, GLuint path, GLenum pname, GLint * value);
GL_APICALL void glPathStencilFuncCHROMIUMContextANGLE(GLeglContext ctx, GLenum func, GLint ref, GLuint mask);
GL_APICALL void glStencilFillPathCHROMIUMContextANGLE(GLeglContext ctx, GLuint path, GLenum fillMode, GLuint mask);
GL_APICALL void glStencilStrokePathCHROMIUMContextANGLE(GLeglContext ctx, GLuint path, GLint reference, GLuint mask);
GL_APICALL void glCoverFillPathCHROMIUMContextANGLE(GLeglContext ctx, GLuint path, GLenum coverMode);
GL_APICALL void glCoverStrokePathCHROMIUMContextANGLE(GLeglContext ctx, GLuint path, GLenum coverMode);
GL_APICALL void glStencilThenCoverFillPathCHROMIUMContextANGLE(GLeglContext ctx, GLuint path, GLenum fillMode, GLuint mask, GLenum coverMode);
GL_APICALL void glStencilThenCoverStrokePathCHROMIUMContextANGLE(GLeglContext ctx, GLuint path, GLint reference, GLuint mask, GLenum coverMode);
GL_APICALL void glCoverFillPathInstancedCHROMIUMContextANGLE(GLeglContext ctx, GLsizei numPath, GLenum pathNameType, const void * paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat * transformValues);
GL_APICALL void glCoverStrokePathInstancedCHROMIUMContextANGLE(GLeglContext ctx, GLsizei numPath, GLenum pathNameType, const void * paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat * transformValues);
GL_APICALL void glStencilStrokePathInstancedCHROMIUMContextANGLE(GLeglContext ctx, GLsizei numPath, GLenum pathNameType, const void * paths, GLuint pathBase, GLint reference, GLuint mask, GLenum transformType, const GLfloat * transformValues);
GL_APICALL void glStencilFillPathInstancedCHROMIUMContextANGLE(GLeglContext ctx, GLsizei numPaths, GLenum pathNameType, const void * paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum transformType, const GLfloat * transformValues);
GL_APICALL void glStencilThenCoverFillPathInstancedCHROMIUMContextANGLE(GLeglContext ctx, GLsizei numPaths, GLenum pathNameType, const void * paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat * transformValues);
GL_APICALL void glStencilThenCoverStrokePathInstancedCHROMIUMContextANGLE(GLeglContext ctx, GLsizei numPaths, GLenum pathNameType, const void * paths, GLuint pathBase, GLint reference, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat * transformValues);
GL_APICALL void glBindFragmentInputLocationCHROMIUMContextANGLE(GLeglContext ctx, GLuint programs, GLint location, const GLchar * name);
GL_APICALL void glProgramPathFragmentInputGenCHROMIUMContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLenum genMode, GLint components, const GLfloat * coeffs);
// GL_EXT_debug_marker
GL_APICALL void glInsertEventMarkerEXTContextANGLE(GLeglContext ctx, GLsizei length, const GLchar *marker);
GL_APICALL void glPopGroupMarkerEXTContextANGLE(GLeglContext ctx);
GL_APICALL void glPushGroupMarkerEXTContextANGLE(GLeglContext ctx, GLsizei length, const GLchar *marker);
// GL_EXT_discard_framebuffer
GL_APICALL void glDiscardFramebufferEXTContextANGLE(GLeglContext ctx, GLenum target, GLsizei numAttachments, const GLenum *attachments);
// GL_EXT_disjoint_timer_query
GL_APICALL void glBeginQueryEXTContextANGLE(GLeglContext ctx, GLenum target, GLuint id);
GL_APICALL void glDeleteQueriesEXTContextANGLE(GLeglContext ctx, GLsizei n, const GLuint *ids);
GL_APICALL void glEndQueryEXTContextANGLE(GLeglContext ctx, GLenum target);
GL_APICALL void glGenQueriesEXTContextANGLE(GLeglContext ctx, GLsizei n, GLuint *ids);
GL_APICALL void glGetQueryObjecti64vEXTContextANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLint64 *params);
GL_APICALL void glGetQueryObjectivEXTContextANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLint *params);
GL_APICALL void glGetQueryObjectui64vEXTContextANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLuint64 *params);
GL_APICALL void glGetQueryObjectuivEXTContextANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLuint *params);
GL_APICALL void glGetQueryivEXTContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
GL_APICALL GLboolean glIsQueryEXTContextANGLE(GLeglContext ctx, GLuint id);
GL_APICALL void glQueryCounterEXTContextANGLE(GLeglContext ctx, GLuint id, GLenum target);
// GL_EXT_draw_buffers
GL_APICALL void glDrawBuffersEXTContextANGLE(GLeglContext ctx, GLsizei n, const GLenum *bufs);
// GL_EXT_map_buffer_range
GL_APICALL void glFlushMappedBufferRangeEXTContextANGLE(GLeglContext ctx, GLenum target, GLintptr offset, GLsizeiptr length);
GL_APICALL void * glMapBufferRangeEXTContextANGLE(GLeglContext ctx, GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
// GL_EXT_robustness
GL_APICALL GLenum glGetGraphicsResetStatusEXTContextANGLE(GLeglContext ctx);
GL_APICALL void glGetnUniformfvEXTContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLfloat *params);
GL_APICALL void glGetnUniformivEXTContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLint *params);
GL_APICALL void glReadnPixelsEXTContextANGLE(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
// GL_EXT_texture_storage
GL_APICALL void glTexStorage1DEXTContextANGLE(GLeglContext ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
GL_APICALL void glTexStorage2DEXTContextANGLE(GLeglContext ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
GL_APICALL void glTexStorage3DEXTContextANGLE(GLeglContext ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
// GL_KHR_debug
GL_APICALL void glDebugMessageCallbackKHRContextANGLE(GLeglContext ctx, GLDEBUGPROCKHR callback, const void *userParam);
GL_APICALL void glDebugMessageControlKHRContextANGLE(GLeglContext ctx, GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
GL_APICALL void glDebugMessageInsertKHRContextANGLE(GLeglContext ctx, GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
GL_APICALL GLuint glGetDebugMessageLogKHRContextANGLE(GLeglContext ctx, GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
GL_APICALL void glGetObjectLabelKHRContextANGLE(GLeglContext ctx, GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label);
GL_APICALL void glGetObjectPtrLabelKHRContextANGLE(GLeglContext ctx, const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label);
GL_APICALL void glGetPointervKHRContextANGLE(GLeglContext ctx, GLenum pname, void **params);
GL_APICALL void glObjectLabelKHRContextANGLE(GLeglContext ctx, GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
GL_APICALL void glObjectPtrLabelKHRContextANGLE(GLeglContext ctx, const void *ptr, GLsizei length, const GLchar *label);
GL_APICALL void glPopDebugGroupKHRContextANGLE(GLeglContext ctx);
GL_APICALL void glPushDebugGroupKHRContextANGLE(GLeglContext ctx, GLenum source, GLuint id, GLsizei length, const GLchar *message);
// GL_NV_fence
GL_APICALL void glDeleteFencesNVContextANGLE(GLeglContext ctx, GLsizei n, const GLuint *fences);
GL_APICALL void glFinishFenceNVContextANGLE(GLeglContext ctx, GLuint fence);
GL_APICALL void glGenFencesNVContextANGLE(GLeglContext ctx, GLsizei n, GLuint *fences);
GL_APICALL void glGetFenceivNVContextANGLE(GLeglContext ctx, GLuint fence, GLenum pname, GLint *params);
GL_APICALL GLboolean glIsFenceNVContextANGLE(GLeglContext ctx, GLuint fence);
GL_APICALL void glSetFenceNVContextANGLE(GLeglContext ctx, GLuint fence, GLenum condition);
GL_APICALL GLboolean glTestFenceNVContextANGLE(GLeglContext ctx, GLuint fence);
// GL_OES_EGL_image
GL_APICALL void glEGLImageTargetRenderbufferStorageOESContextANGLE(GLeglContext ctx, GLenum target, GLeglImageOES image);
GL_APICALL void glEGLImageTargetTexture2DOESContextANGLE(GLeglContext ctx, GLenum target, GLeglImageOES image);
// GL_OES_get_program_binary
GL_APICALL void glGetProgramBinaryOESContextANGLE(GLeglContext ctx, GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
GL_APICALL void glProgramBinaryOESContextANGLE(GLeglContext ctx, GLuint program, GLenum binaryFormat, const void *binary, GLint length);
// GL_OES_mapbuffer
GL_APICALL void glGetBufferPointervOESContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, void **params);
GL_APICALL void * glMapBufferOESContextANGLE(GLeglContext ctx, GLenum target, GLenum access);
GL_APICALL GLboolean glUnmapBufferOESContextANGLE(GLeglContext ctx, GLenum target);
// GL_OES_vertex_array_object
GL_APICALL void glBindVertexArrayOESContextANGLE(GLeglContext ctx, GLuint array);
GL_APICALL void glDeleteVertexArraysOESContextANGLE(GLeglContext ctx, GLsizei n, const GLuint *arrays);
GL_APICALL void glGenVertexArraysOESContextANGLE(GLeglContext ctx, GLsizei n, GLuint *arrays);
GL_APICALL GLboolean glIsVertexArrayOESContextANGLE(GLeglContext ctx, GLuint array);
#endif
#endif /* EGL_ANGLE_explicit_context */

// clang-format on

#endif  // INCLUDE_GLES2_GL2EXT_ANGLE_H_
