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

#ifndef EGL_ANGLE_explicit_context
#define EGL_ANGLE_explicit_context
// The GLclampx type is defined in gl.h, and not in later GL headers. In order to keep declarations for this extension contained in this file, I define it here.
typedef khronos_int32_t GLclampx;
typedef void *GLeglContext;
typedef void GL_APIENTRY PFNGLACTIVESHADERPROGRAMCONTEXTANGLE(GLeglContext ctx, GLuint pipeline, GLuint program);
typedef void GL_APIENTRY PFNGLACTIVETEXTURECONTEXTANGLE(GLeglContext ctx, GLenum texture);
typedef void GL_APIENTRY PFNGLALPHAFUNCCONTEXTANGLE(GLeglContext ctx, GLenum func, GLfloat ref);
typedef void GL_APIENTRY PFNGLALPHAFUNCXCONTEXTANGLE(GLeglContext ctx, GLenum func, GLfixed ref);
typedef void GL_APIENTRY PFNGLATTACHSHADERCONTEXTANGLE(GLeglContext ctx, GLuint program, GLuint shader);
typedef void GL_APIENTRY PFNGLBEGINQUERYCONTEXTANGLE(GLeglContext ctx, GLenum target, GLuint id);
typedef void GL_APIENTRY PFNGLBEGINQUERYEXTCONTEXTANGLE(GLeglContext ctx, GLenum target, GLuint id);
typedef void GL_APIENTRY PFNGLBEGINTRANSFORMFEEDBACKCONTEXTANGLE(GLeglContext ctx, GLenum primitiveMode);
typedef void GL_APIENTRY PFNGLBINDATTRIBLOCATIONCONTEXTANGLE(GLeglContext ctx, GLuint program, GLuint index, const GLchar *name);
typedef void GL_APIENTRY PFNGLBINDBUFFERCONTEXTANGLE(GLeglContext ctx, GLenum target, GLuint buffer);
typedef void GL_APIENTRY PFNGLBINDBUFFERBASECONTEXTANGLE(GLeglContext ctx, GLenum target, GLuint index, GLuint buffer);
typedef void GL_APIENTRY PFNGLBINDBUFFERRANGECONTEXTANGLE(GLeglContext ctx, GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef void GL_APIENTRY PFNGLBINDFRAMEBUFFERCONTEXTANGLE(GLeglContext ctx, GLenum target, GLuint framebuffer);
typedef void GL_APIENTRY PFNGLBINDFRAMEBUFFEROESCONTEXTANGLE(GLeglContext ctx, GLenum target, GLuint framebuffer);
typedef void GL_APIENTRY PFNGLBINDIMAGETEXTURECONTEXTANGLE(GLeglContext ctx, GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
typedef void GL_APIENTRY PFNGLBINDPROGRAMPIPELINECONTEXTANGLE(GLeglContext ctx, GLuint pipeline);
typedef void GL_APIENTRY PFNGLBINDRENDERBUFFERCONTEXTANGLE(GLeglContext ctx, GLenum target, GLuint renderbuffer);
typedef void GL_APIENTRY PFNGLBINDRENDERBUFFEROESCONTEXTANGLE(GLeglContext ctx, GLenum target, GLuint renderbuffer);
typedef void GL_APIENTRY PFNGLBINDSAMPLERCONTEXTANGLE(GLeglContext ctx, GLuint unit, GLuint sampler);
typedef void GL_APIENTRY PFNGLBINDTEXTURECONTEXTANGLE(GLeglContext ctx, GLenum target, GLuint texture);
typedef void GL_APIENTRY PFNGLBINDTRANSFORMFEEDBACKCONTEXTANGLE(GLeglContext ctx, GLenum target, GLuint id);
typedef void GL_APIENTRY PFNGLBINDVERTEXARRAYCONTEXTANGLE(GLeglContext ctx, GLuint array);
typedef void GL_APIENTRY PFNGLBINDVERTEXARRAYOESCONTEXTANGLE(GLeglContext ctx, GLuint array);
typedef void GL_APIENTRY PFNGLBINDVERTEXBUFFERCONTEXTANGLE(GLeglContext ctx, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
typedef void GL_APIENTRY PFNGLBLENDCOLORCONTEXTANGLE(GLeglContext ctx, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void GL_APIENTRY PFNGLBLENDEQUATIONCONTEXTANGLE(GLeglContext ctx, GLenum mode);
typedef void GL_APIENTRY PFNGLBLENDEQUATIONSEPARATECONTEXTANGLE(GLeglContext ctx, GLenum modeRGB, GLenum modeAlpha);
typedef void GL_APIENTRY PFNGLBLENDFUNCCONTEXTANGLE(GLeglContext ctx, GLenum sfactor, GLenum dfactor);
typedef void GL_APIENTRY PFNGLBLENDFUNCSEPARATECONTEXTANGLE(GLeglContext ctx, GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef void GL_APIENTRY PFNGLBLITFRAMEBUFFERCONTEXTANGLE(GLeglContext ctx, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef void GL_APIENTRY PFNGLBLITFRAMEBUFFERANGLECONTEXTANGLE(GLeglContext ctx, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef void GL_APIENTRY PFNGLBUFFERDATACONTEXTANGLE(GLeglContext ctx, GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void GL_APIENTRY PFNGLBUFFERSUBDATACONTEXTANGLE(GLeglContext ctx, GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
typedef GLenum GL_APIENTRY PFNGLCHECKFRAMEBUFFERSTATUSCONTEXTANGLE(GLeglContext ctx, GLenum target);
typedef GLenum GL_APIENTRY PFNGLCHECKFRAMEBUFFERSTATUSOESCONTEXTANGLE(GLeglContext ctx, GLenum target);
typedef void GL_APIENTRY PFNGLCLEARCONTEXTANGLE(GLeglContext ctx, GLbitfield mask);
typedef void GL_APIENTRY PFNGLCLEARBUFFERFICONTEXTANGLE(GLeglContext ctx, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
typedef void GL_APIENTRY PFNGLCLEARBUFFERFVCONTEXTANGLE(GLeglContext ctx, GLenum buffer, GLint drawbuffer, const GLfloat *value);
typedef void GL_APIENTRY PFNGLCLEARBUFFERIVCONTEXTANGLE(GLeglContext ctx, GLenum buffer, GLint drawbuffer, const GLint *value);
typedef void GL_APIENTRY PFNGLCLEARBUFFERUIVCONTEXTANGLE(GLeglContext ctx, GLenum buffer, GLint drawbuffer, const GLuint *value);
typedef void GL_APIENTRY PFNGLCLEARCOLORCONTEXTANGLE(GLeglContext ctx, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void GL_APIENTRY PFNGLCLEARCOLORXCONTEXTANGLE(GLeglContext ctx, GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
typedef void GL_APIENTRY PFNGLCLEARDEPTHFCONTEXTANGLE(GLeglContext ctx, GLfloat d);
typedef void GL_APIENTRY PFNGLCLEARDEPTHXCONTEXTANGLE(GLeglContext ctx, GLfixed depth);
typedef void GL_APIENTRY PFNGLCLEARSTENCILCONTEXTANGLE(GLeglContext ctx, GLint s);
typedef void GL_APIENTRY PFNGLCLIENTACTIVETEXTURECONTEXTANGLE(GLeglContext ctx, GLenum texture);
typedef GLenum GL_APIENTRY PFNGLCLIENTWAITSYNCCONTEXTANGLE(GLeglContext ctx, GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef void GL_APIENTRY PFNGLCLIPPLANEFCONTEXTANGLE(GLeglContext ctx, GLenum p, const GLfloat *eqn);
typedef void GL_APIENTRY PFNGLCLIPPLANEXCONTEXTANGLE(GLeglContext ctx, GLenum plane, const GLfixed *equation);
typedef void GL_APIENTRY PFNGLCOLOR4FCONTEXTANGLE(GLeglContext ctx, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void GL_APIENTRY PFNGLCOLOR4UBCONTEXTANGLE(GLeglContext ctx, GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
typedef void GL_APIENTRY PFNGLCOLOR4XCONTEXTANGLE(GLeglContext ctx, GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
typedef void GL_APIENTRY PFNGLCOLORMASKCONTEXTANGLE(GLeglContext ctx, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
typedef void GL_APIENTRY PFNGLCOLORPOINTERCONTEXTANGLE(GLeglContext ctx, GLint size, GLenum type, GLsizei stride, const void *pointer);
typedef void GL_APIENTRY PFNGLCOMPILESHADERCONTEXTANGLE(GLeglContext ctx, GLuint shader);
typedef void GL_APIENTRY PFNGLCOMPRESSEDTEXIMAGE2DCONTEXTANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
typedef void GL_APIENTRY PFNGLCOMPRESSEDTEXIMAGE3DCONTEXTANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
typedef void GL_APIENTRY PFNGLCOMPRESSEDTEXSUBIMAGE2DCONTEXTANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
typedef void GL_APIENTRY PFNGLCOMPRESSEDTEXSUBIMAGE3DCONTEXTANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
typedef void GL_APIENTRY PFNGLCOPYBUFFERSUBDATACONTEXTANGLE(GLeglContext ctx, GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
typedef void GL_APIENTRY PFNGLCOPYTEXIMAGE2DCONTEXTANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void GL_APIENTRY PFNGLCOPYTEXSUBIMAGE2DCONTEXTANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void GL_APIENTRY PFNGLCOPYTEXSUBIMAGE3DCONTEXTANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef GLuint GL_APIENTRY PFNGLCREATEPROGRAMCONTEXTANGLE(GLeglContext ctx);
typedef GLuint GL_APIENTRY PFNGLCREATESHADERCONTEXTANGLE(GLeglContext ctx, GLenum type);
typedef GLuint GL_APIENTRY PFNGLCREATESHADERPROGRAMVCONTEXTANGLE(GLeglContext ctx, GLenum type, GLsizei count, const GLchar *const*strings);
typedef void GL_APIENTRY PFNGLCULLFACECONTEXTANGLE(GLeglContext ctx, GLenum mode);
typedef void GL_APIENTRY PFNGLCURRENTPALETTEMATRIXOESCONTEXTANGLE(GLeglContext ctx, GLuint matrixpaletteindex);
typedef void GL_APIENTRY PFNGLDEBUGMESSAGECALLBACKKHRCONTEXTANGLE(GLeglContext ctx, GLDEBUGPROCKHR callback, const void *userParam);
typedef void GL_APIENTRY PFNGLDEBUGMESSAGECONTROLKHRCONTEXTANGLE(GLeglContext ctx, GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
typedef void GL_APIENTRY PFNGLDEBUGMESSAGEINSERTKHRCONTEXTANGLE(GLeglContext ctx, GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
typedef void GL_APIENTRY PFNGLDELETEBUFFERSCONTEXTANGLE(GLeglContext ctx, GLsizei n, const GLuint *buffers);
typedef void GL_APIENTRY PFNGLDELETEFENCESNVCONTEXTANGLE(GLeglContext ctx, GLsizei n, const GLuint *fences);
typedef void GL_APIENTRY PFNGLDELETEFRAMEBUFFERSCONTEXTANGLE(GLeglContext ctx, GLsizei n, const GLuint *framebuffers);
typedef void GL_APIENTRY PFNGLDELETEFRAMEBUFFERSOESCONTEXTANGLE(GLeglContext ctx, GLsizei n, const GLuint *framebuffers);
typedef void GL_APIENTRY PFNGLDELETEPROGRAMCONTEXTANGLE(GLeglContext ctx, GLuint program);
typedef void GL_APIENTRY PFNGLDELETEPROGRAMPIPELINESCONTEXTANGLE(GLeglContext ctx, GLsizei n, const GLuint *pipelines);
typedef void GL_APIENTRY PFNGLDELETEQUERIESCONTEXTANGLE(GLeglContext ctx, GLsizei n, const GLuint *ids);
typedef void GL_APIENTRY PFNGLDELETEQUERIESEXTCONTEXTANGLE(GLeglContext ctx, GLsizei n, const GLuint *ids);
typedef void GL_APIENTRY PFNGLDELETERENDERBUFFERSCONTEXTANGLE(GLeglContext ctx, GLsizei n, const GLuint *renderbuffers);
typedef void GL_APIENTRY PFNGLDELETERENDERBUFFERSOESCONTEXTANGLE(GLeglContext ctx, GLsizei n, const GLuint *renderbuffers);
typedef void GL_APIENTRY PFNGLDELETESAMPLERSCONTEXTANGLE(GLeglContext ctx, GLsizei count, const GLuint *samplers);
typedef void GL_APIENTRY PFNGLDELETESHADERCONTEXTANGLE(GLeglContext ctx, GLuint shader);
typedef void GL_APIENTRY PFNGLDELETESYNCCONTEXTANGLE(GLeglContext ctx, GLsync sync);
typedef void GL_APIENTRY PFNGLDELETETEXTURESCONTEXTANGLE(GLeglContext ctx, GLsizei n, const GLuint *textures);
typedef void GL_APIENTRY PFNGLDELETETRANSFORMFEEDBACKSCONTEXTANGLE(GLeglContext ctx, GLsizei n, const GLuint *ids);
typedef void GL_APIENTRY PFNGLDELETEVERTEXARRAYSCONTEXTANGLE(GLeglContext ctx, GLsizei n, const GLuint *arrays);
typedef void GL_APIENTRY PFNGLDELETEVERTEXARRAYSOESCONTEXTANGLE(GLeglContext ctx, GLsizei n, const GLuint *arrays);
typedef void GL_APIENTRY PFNGLDEPTHFUNCCONTEXTANGLE(GLeglContext ctx, GLenum func);
typedef void GL_APIENTRY PFNGLDEPTHMASKCONTEXTANGLE(GLeglContext ctx, GLboolean flag);
typedef void GL_APIENTRY PFNGLDEPTHRANGEFCONTEXTANGLE(GLeglContext ctx, GLfloat n, GLfloat f);
typedef void GL_APIENTRY PFNGLDEPTHRANGEXCONTEXTANGLE(GLeglContext ctx, GLfixed n, GLfixed f);
typedef void GL_APIENTRY PFNGLDETACHSHADERCONTEXTANGLE(GLeglContext ctx, GLuint program, GLuint shader);
typedef void GL_APIENTRY PFNGLDISABLECONTEXTANGLE(GLeglContext ctx, GLenum cap);
typedef void GL_APIENTRY PFNGLDISABLECLIENTSTATECONTEXTANGLE(GLeglContext ctx, GLenum array);
typedef void GL_APIENTRY PFNGLDISABLEVERTEXATTRIBARRAYCONTEXTANGLE(GLeglContext ctx, GLuint index);
typedef void GL_APIENTRY PFNGLDISCARDFRAMEBUFFEREXTCONTEXTANGLE(GLeglContext ctx, GLenum target, GLsizei numAttachments, const GLenum *attachments);
typedef void GL_APIENTRY PFNGLDISPATCHCOMPUTECONTEXTANGLE(GLeglContext ctx, GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
typedef void GL_APIENTRY PFNGLDISPATCHCOMPUTEINDIRECTCONTEXTANGLE(GLeglContext ctx, GLintptr indirect);
typedef void GL_APIENTRY PFNGLDRAWARRAYSCONTEXTANGLE(GLeglContext ctx, GLenum mode, GLint first, GLsizei count);
typedef void GL_APIENTRY PFNGLDRAWARRAYSINDIRECTCONTEXTANGLE(GLeglContext ctx, GLenum mode, const void *indirect);
typedef void GL_APIENTRY PFNGLDRAWARRAYSINSTANCEDCONTEXTANGLE(GLeglContext ctx, GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
typedef void GL_APIENTRY PFNGLDRAWARRAYSINSTANCEDANGLECONTEXTANGLE(GLeglContext ctx, GLenum mode, GLint first, GLsizei count, GLsizei primcount);
typedef void GL_APIENTRY PFNGLDRAWBUFFERSCONTEXTANGLE(GLeglContext ctx, GLsizei n, const GLenum *bufs);
typedef void GL_APIENTRY PFNGLDRAWBUFFERSEXTCONTEXTANGLE(GLeglContext ctx, GLsizei n, const GLenum *bufs);
typedef void GL_APIENTRY PFNGLDRAWELEMENTSCONTEXTANGLE(GLeglContext ctx, GLenum mode, GLsizei count, GLenum type, const void *indices);
typedef void GL_APIENTRY PFNGLDRAWELEMENTSINDIRECTCONTEXTANGLE(GLeglContext ctx, GLenum mode, GLenum type, const void *indirect);
typedef void GL_APIENTRY PFNGLDRAWELEMENTSINSTANCEDCONTEXTANGLE(GLeglContext ctx, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
typedef void GL_APIENTRY PFNGLDRAWELEMENTSINSTANCEDANGLECONTEXTANGLE(GLeglContext ctx, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
typedef void GL_APIENTRY PFNGLDRAWRANGEELEMENTSCONTEXTANGLE(GLeglContext ctx, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
typedef void GL_APIENTRY PFNGLDRAWTEXFOESCONTEXTANGLE(GLeglContext ctx, GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height);
typedef void GL_APIENTRY PFNGLDRAWTEXFVOESCONTEXTANGLE(GLeglContext ctx, const GLfloat *coords);
typedef void GL_APIENTRY PFNGLDRAWTEXIOESCONTEXTANGLE(GLeglContext ctx, GLint x, GLint y, GLint z, GLint width, GLint height);
typedef void GL_APIENTRY PFNGLDRAWTEXIVOESCONTEXTANGLE(GLeglContext ctx, const GLint *coords);
typedef void GL_APIENTRY PFNGLDRAWTEXSOESCONTEXTANGLE(GLeglContext ctx, GLshort x, GLshort y, GLshort z, GLshort width, GLshort height);
typedef void GL_APIENTRY PFNGLDRAWTEXSVOESCONTEXTANGLE(GLeglContext ctx, const GLshort *coords);
typedef void GL_APIENTRY PFNGLDRAWTEXXOESCONTEXTANGLE(GLeglContext ctx, GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height);
typedef void GL_APIENTRY PFNGLDRAWTEXXVOESCONTEXTANGLE(GLeglContext ctx, const GLfixed *coords);
typedef void GL_APIENTRY PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESCONTEXTANGLE(GLeglContext ctx, GLenum target, GLeglImageOES image);
typedef void GL_APIENTRY PFNGLEGLIMAGETARGETTEXTURE2DOESCONTEXTANGLE(GLeglContext ctx, GLenum target, GLeglImageOES image);
typedef void GL_APIENTRY PFNGLENABLECONTEXTANGLE(GLeglContext ctx, GLenum cap);
typedef void GL_APIENTRY PFNGLENABLECLIENTSTATECONTEXTANGLE(GLeglContext ctx, GLenum array);
typedef void GL_APIENTRY PFNGLENABLEVERTEXATTRIBARRAYCONTEXTANGLE(GLeglContext ctx, GLuint index);
typedef void GL_APIENTRY PFNGLENDQUERYCONTEXTANGLE(GLeglContext ctx, GLenum target);
typedef void GL_APIENTRY PFNGLENDQUERYEXTCONTEXTANGLE(GLeglContext ctx, GLenum target);
typedef void GL_APIENTRY PFNGLENDTRANSFORMFEEDBACKCONTEXTANGLE(GLeglContext ctx);
typedef GLsync GL_APIENTRY PFNGLFENCESYNCCONTEXTANGLE(GLeglContext ctx, GLenum condition, GLbitfield flags);
typedef void GL_APIENTRY PFNGLFINISHCONTEXTANGLE(GLeglContext ctx);
typedef void GL_APIENTRY PFNGLFINISHFENCENVCONTEXTANGLE(GLeglContext ctx, GLuint fence);
typedef void GL_APIENTRY PFNGLFLUSHCONTEXTANGLE(GLeglContext ctx);
typedef void GL_APIENTRY PFNGLFLUSHMAPPEDBUFFERRANGECONTEXTANGLE(GLeglContext ctx, GLenum target, GLintptr offset, GLsizeiptr length);
typedef void GL_APIENTRY PFNGLFLUSHMAPPEDBUFFERRANGEEXTCONTEXTANGLE(GLeglContext ctx, GLenum target, GLintptr offset, GLsizeiptr length);
typedef void GL_APIENTRY PFNGLFOGFCONTEXTANGLE(GLeglContext ctx, GLenum pname, GLfloat param);
typedef void GL_APIENTRY PFNGLFOGFVCONTEXTANGLE(GLeglContext ctx, GLenum pname, const GLfloat *params);
typedef void GL_APIENTRY PFNGLFOGXCONTEXTANGLE(GLeglContext ctx, GLenum pname, GLfixed param);
typedef void GL_APIENTRY PFNGLFOGXVCONTEXTANGLE(GLeglContext ctx, GLenum pname, const GLfixed *param);
typedef void GL_APIENTRY PFNGLFRAMEBUFFERPARAMETERICONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint param);
typedef void GL_APIENTRY PFNGLFRAMEBUFFERRENDERBUFFERCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void GL_APIENTRY PFNGLFRAMEBUFFERRENDERBUFFEROESCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void GL_APIENTRY PFNGLFRAMEBUFFERTEXTURE2DCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void GL_APIENTRY PFNGLFRAMEBUFFERTEXTURE2DOESCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void GL_APIENTRY PFNGLFRAMEBUFFERTEXTURELAYERCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
typedef void GL_APIENTRY PFNGLFRONTFACECONTEXTANGLE(GLeglContext ctx, GLenum mode);
typedef void GL_APIENTRY PFNGLFRUSTUMFCONTEXTANGLE(GLeglContext ctx, GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f);
typedef void GL_APIENTRY PFNGLFRUSTUMXCONTEXTANGLE(GLeglContext ctx, GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f);
typedef void GL_APIENTRY PFNGLGENBUFFERSCONTEXTANGLE(GLeglContext ctx, GLsizei n, GLuint *buffers);
typedef void GL_APIENTRY PFNGLGENFENCESNVCONTEXTANGLE(GLeglContext ctx, GLsizei n, GLuint *fences);
typedef void GL_APIENTRY PFNGLGENFRAMEBUFFERSCONTEXTANGLE(GLeglContext ctx, GLsizei n, GLuint *framebuffers);
typedef void GL_APIENTRY PFNGLGENFRAMEBUFFERSOESCONTEXTANGLE(GLeglContext ctx, GLsizei n, GLuint *framebuffers);
typedef void GL_APIENTRY PFNGLGENPROGRAMPIPELINESCONTEXTANGLE(GLeglContext ctx, GLsizei n, GLuint *pipelines);
typedef void GL_APIENTRY PFNGLGENQUERIESCONTEXTANGLE(GLeglContext ctx, GLsizei n, GLuint *ids);
typedef void GL_APIENTRY PFNGLGENQUERIESEXTCONTEXTANGLE(GLeglContext ctx, GLsizei n, GLuint *ids);
typedef void GL_APIENTRY PFNGLGENRENDERBUFFERSCONTEXTANGLE(GLeglContext ctx, GLsizei n, GLuint *renderbuffers);
typedef void GL_APIENTRY PFNGLGENRENDERBUFFERSOESCONTEXTANGLE(GLeglContext ctx, GLsizei n, GLuint *renderbuffers);
typedef void GL_APIENTRY PFNGLGENSAMPLERSCONTEXTANGLE(GLeglContext ctx, GLsizei count, GLuint *samplers);
typedef void GL_APIENTRY PFNGLGENTEXTURESCONTEXTANGLE(GLeglContext ctx, GLsizei n, GLuint *textures);
typedef void GL_APIENTRY PFNGLGENTRANSFORMFEEDBACKSCONTEXTANGLE(GLeglContext ctx, GLsizei n, GLuint *ids);
typedef void GL_APIENTRY PFNGLGENVERTEXARRAYSCONTEXTANGLE(GLeglContext ctx, GLsizei n, GLuint *arrays);
typedef void GL_APIENTRY PFNGLGENVERTEXARRAYSOESCONTEXTANGLE(GLeglContext ctx, GLsizei n, GLuint *arrays);
typedef void GL_APIENTRY PFNGLGENERATEMIPMAPCONTEXTANGLE(GLeglContext ctx, GLenum target);
typedef void GL_APIENTRY PFNGLGENERATEMIPMAPOESCONTEXTANGLE(GLeglContext ctx, GLenum target);
typedef void GL_APIENTRY PFNGLGETACTIVEATTRIBCONTEXTANGLE(GLeglContext ctx, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
typedef void GL_APIENTRY PFNGLGETACTIVEUNIFORMCONTEXTANGLE(GLeglContext ctx, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
typedef void GL_APIENTRY PFNGLGETACTIVEUNIFORMBLOCKNAMECONTEXTANGLE(GLeglContext ctx, GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
typedef void GL_APIENTRY PFNGLGETACTIVEUNIFORMBLOCKIVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
typedef void GL_APIENTRY PFNGLGETACTIVEUNIFORMSIVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
typedef void GL_APIENTRY PFNGLGETATTACHEDSHADERSCONTEXTANGLE(GLeglContext ctx, GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
typedef GLint GL_APIENTRY PFNGLGETATTRIBLOCATIONCONTEXTANGLE(GLeglContext ctx, GLuint program, const GLchar *name);
typedef void GL_APIENTRY PFNGLGETBOOLEANI_VCONTEXTANGLE(GLeglContext ctx, GLenum target, GLuint index, GLboolean *data);
typedef void GL_APIENTRY PFNGLGETBOOLEANVCONTEXTANGLE(GLeglContext ctx, GLenum pname, GLboolean *data);
typedef void GL_APIENTRY PFNGLGETBUFFERPARAMETERI64VCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint64 *params);
typedef void GL_APIENTRY PFNGLGETBUFFERPARAMETERIVCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
typedef void GL_APIENTRY PFNGLGETBUFFERPOINTERVCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, void **params);
typedef void GL_APIENTRY PFNGLGETBUFFERPOINTERVOESCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, void **params);
typedef void GL_APIENTRY PFNGLGETCLIPPLANEFCONTEXTANGLE(GLeglContext ctx, GLenum plane, GLfloat *equation);
typedef void GL_APIENTRY PFNGLGETCLIPPLANEXCONTEXTANGLE(GLeglContext ctx, GLenum plane, GLfixed *equation);
typedef GLuint GL_APIENTRY PFNGLGETDEBUGMESSAGELOGKHRCONTEXTANGLE(GLeglContext ctx, GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
typedef GLenum GL_APIENTRY PFNGLGETERRORCONTEXTANGLE(GLeglContext ctx);
typedef void GL_APIENTRY PFNGLGETFENCEIVNVCONTEXTANGLE(GLeglContext ctx, GLuint fence, GLenum pname, GLint *params);
typedef void GL_APIENTRY PFNGLGETFIXEDVCONTEXTANGLE(GLeglContext ctx, GLenum pname, GLfixed *params);
typedef void GL_APIENTRY PFNGLGETFLOATVCONTEXTANGLE(GLeglContext ctx, GLenum pname, GLfloat *data);
typedef GLint GL_APIENTRY PFNGLGETFRAGDATALOCATIONCONTEXTANGLE(GLeglContext ctx, GLuint program, const GLchar *name);
typedef void GL_APIENTRY PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLenum pname, GLint *params);
typedef void GL_APIENTRY PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVOESCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLenum pname, GLint *params);
typedef void GL_APIENTRY PFNGLGETFRAMEBUFFERPARAMETERIVCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
typedef GLenum GL_APIENTRY PFNGLGETGRAPHICSRESETSTATUSEXTCONTEXTANGLE(GLeglContext ctx);
typedef void GL_APIENTRY PFNGLGETINTEGER64I_VCONTEXTANGLE(GLeglContext ctx, GLenum target, GLuint index, GLint64 *data);
typedef void GL_APIENTRY PFNGLGETINTEGER64VCONTEXTANGLE(GLeglContext ctx, GLenum pname, GLint64 *data);
typedef void GL_APIENTRY PFNGLGETINTEGERI_VCONTEXTANGLE(GLeglContext ctx, GLenum target, GLuint index, GLint *data);
typedef void GL_APIENTRY PFNGLGETINTEGERVCONTEXTANGLE(GLeglContext ctx, GLenum pname, GLint *data);
typedef void GL_APIENTRY PFNGLGETINTERNALFORMATIVCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params);
typedef void GL_APIENTRY PFNGLGETLIGHTFVCONTEXTANGLE(GLeglContext ctx, GLenum light, GLenum pname, GLfloat *params);
typedef void GL_APIENTRY PFNGLGETLIGHTXVCONTEXTANGLE(GLeglContext ctx, GLenum light, GLenum pname, GLfixed *params);
typedef void GL_APIENTRY PFNGLGETMATERIALFVCONTEXTANGLE(GLeglContext ctx, GLenum face, GLenum pname, GLfloat *params);
typedef void GL_APIENTRY PFNGLGETMATERIALXVCONTEXTANGLE(GLeglContext ctx, GLenum face, GLenum pname, GLfixed *params);
typedef void GL_APIENTRY PFNGLGETMULTISAMPLEFVCONTEXTANGLE(GLeglContext ctx, GLenum pname, GLuint index, GLfloat *val);
typedef void GL_APIENTRY PFNGLGETOBJECTLABELKHRCONTEXTANGLE(GLeglContext ctx, GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label);
typedef void GL_APIENTRY PFNGLGETOBJECTPTRLABELKHRCONTEXTANGLE(GLeglContext ctx, const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label);
typedef void GL_APIENTRY PFNGLGETPOINTERVCONTEXTANGLE(GLeglContext ctx, GLenum pname, void **params);
typedef void GL_APIENTRY PFNGLGETPOINTERVKHRCONTEXTANGLE(GLeglContext ctx, GLenum pname, void **params);
typedef void GL_APIENTRY PFNGLGETPROGRAMBINARYCONTEXTANGLE(GLeglContext ctx, GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
typedef void GL_APIENTRY PFNGLGETPROGRAMBINARYOESCONTEXTANGLE(GLeglContext ctx, GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
typedef void GL_APIENTRY PFNGLGETPROGRAMINFOLOGCONTEXTANGLE(GLeglContext ctx, GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void GL_APIENTRY PFNGLGETPROGRAMINTERFACEIVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLenum programInterface, GLenum pname, GLint *params);
typedef void GL_APIENTRY PFNGLGETPROGRAMPIPELINEINFOLOGCONTEXTANGLE(GLeglContext ctx, GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void GL_APIENTRY PFNGLGETPROGRAMPIPELINEIVCONTEXTANGLE(GLeglContext ctx, GLuint pipeline, GLenum pname, GLint *params);
typedef GLuint GL_APIENTRY PFNGLGETPROGRAMRESOURCEINDEXCONTEXTANGLE(GLeglContext ctx, GLuint program, GLenum programInterface, const GLchar *name);
typedef GLint GL_APIENTRY PFNGLGETPROGRAMRESOURCELOCATIONCONTEXTANGLE(GLeglContext ctx, GLuint program, GLenum programInterface, const GLchar *name);
typedef void GL_APIENTRY PFNGLGETPROGRAMRESOURCENAMECONTEXTANGLE(GLeglContext ctx, GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
typedef void GL_APIENTRY PFNGLGETPROGRAMRESOURCEIVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params);
typedef void GL_APIENTRY PFNGLGETPROGRAMIVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLenum pname, GLint *params);
typedef void GL_APIENTRY PFNGLGETQUERYOBJECTI64VEXTCONTEXTANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLint64 *params);
typedef void GL_APIENTRY PFNGLGETQUERYOBJECTIVEXTCONTEXTANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLint *params);
typedef void GL_APIENTRY PFNGLGETQUERYOBJECTUI64VEXTCONTEXTANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLuint64 *params);
typedef void GL_APIENTRY PFNGLGETQUERYOBJECTUIVCONTEXTANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLuint *params);
typedef void GL_APIENTRY PFNGLGETQUERYOBJECTUIVEXTCONTEXTANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLuint *params);
typedef void GL_APIENTRY PFNGLGETQUERYIVCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
typedef void GL_APIENTRY PFNGLGETQUERYIVEXTCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
typedef void GL_APIENTRY PFNGLGETRENDERBUFFERPARAMETERIVCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
typedef void GL_APIENTRY PFNGLGETRENDERBUFFERPARAMETERIVOESCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
typedef void GL_APIENTRY PFNGLGETSAMPLERPARAMETERFVCONTEXTANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLfloat *params);
typedef void GL_APIENTRY PFNGLGETSAMPLERPARAMETERIVCONTEXTANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLint *params);
typedef void GL_APIENTRY PFNGLGETSHADERINFOLOGCONTEXTANGLE(GLeglContext ctx, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void GL_APIENTRY PFNGLGETSHADERPRECISIONFORMATCONTEXTANGLE(GLeglContext ctx, GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
typedef void GL_APIENTRY PFNGLGETSHADERSOURCECONTEXTANGLE(GLeglContext ctx, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
typedef void GL_APIENTRY PFNGLGETSHADERIVCONTEXTANGLE(GLeglContext ctx, GLuint shader, GLenum pname, GLint *params);
typedef const GLubyte *GL_APIENTRY PFNGLGETSTRINGCONTEXTANGLE(GLeglContext ctx, GLenum name);
typedef const GLubyte *GL_APIENTRY PFNGLGETSTRINGICONTEXTANGLE(GLeglContext ctx, GLenum name, GLuint index);
typedef void GL_APIENTRY PFNGLGETSYNCIVCONTEXTANGLE(GLeglContext ctx, GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);
typedef void GL_APIENTRY PFNGLGETTEXENVFVCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfloat *params);
typedef void GL_APIENTRY PFNGLGETTEXENVIVCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
typedef void GL_APIENTRY PFNGLGETTEXENVXVCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfixed *params);
typedef void GL_APIENTRY PFNGLGETTEXGENFVOESCONTEXTANGLE(GLeglContext ctx, GLenum coord, GLenum pname, GLfloat *params);
typedef void GL_APIENTRY PFNGLGETTEXGENIVOESCONTEXTANGLE(GLeglContext ctx, GLenum coord, GLenum pname, GLint *params);
typedef void GL_APIENTRY PFNGLGETTEXGENXVOESCONTEXTANGLE(GLeglContext ctx, GLenum coord, GLenum pname, GLfixed *params);
typedef void GL_APIENTRY PFNGLGETTEXLEVELPARAMETERFVCONTEXTANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum pname, GLfloat *params);
typedef void GL_APIENTRY PFNGLGETTEXLEVELPARAMETERIVCONTEXTANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum pname, GLint *params);
typedef void GL_APIENTRY PFNGLGETTEXPARAMETERFVCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfloat *params);
typedef void GL_APIENTRY PFNGLGETTEXPARAMETERIVCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
typedef void GL_APIENTRY PFNGLGETTEXPARAMETERXVCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfixed *params);
typedef void GL_APIENTRY PFNGLGETTRANSFORMFEEDBACKVARYINGCONTEXTANGLE(GLeglContext ctx, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
typedef void GL_APIENTRY PFNGLGETTRANSLATEDSHADERSOURCEANGLECONTEXTANGLE(GLeglContext ctx, GLuint shader, GLsizei bufsize, GLsizei *length, GLchar *source);
typedef GLuint GL_APIENTRY PFNGLGETUNIFORMBLOCKINDEXCONTEXTANGLE(GLeglContext ctx, GLuint program, const GLchar *uniformBlockName);
typedef void GL_APIENTRY PFNGLGETUNIFORMINDICESCONTEXTANGLE(GLeglContext ctx, GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices);
typedef GLint GL_APIENTRY PFNGLGETUNIFORMLOCATIONCONTEXTANGLE(GLeglContext ctx, GLuint program, const GLchar *name);
typedef void GL_APIENTRY PFNGLGETUNIFORMFVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLfloat *params);
typedef void GL_APIENTRY PFNGLGETUNIFORMIVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLint *params);
typedef void GL_APIENTRY PFNGLGETUNIFORMUIVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLuint *params);
typedef void GL_APIENTRY PFNGLGETVERTEXATTRIBIIVCONTEXTANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLint *params);
typedef void GL_APIENTRY PFNGLGETVERTEXATTRIBIUIVCONTEXTANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLuint *params);
typedef void GL_APIENTRY PFNGLGETVERTEXATTRIBPOINTERVCONTEXTANGLE(GLeglContext ctx, GLuint index, GLenum pname, void **pointer);
typedef void GL_APIENTRY PFNGLGETVERTEXATTRIBFVCONTEXTANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLfloat *params);
typedef void GL_APIENTRY PFNGLGETVERTEXATTRIBIVCONTEXTANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLint *params);
typedef void GL_APIENTRY PFNGLGETNUNIFORMFVEXTCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLfloat *params);
typedef void GL_APIENTRY PFNGLGETNUNIFORMIVEXTCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLint *params);
typedef void GL_APIENTRY PFNGLHINTCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum mode);
typedef void GL_APIENTRY PFNGLINSERTEVENTMARKEREXTCONTEXTANGLE(GLeglContext ctx, GLsizei length, const GLchar *marker);
typedef void GL_APIENTRY PFNGLINVALIDATEFRAMEBUFFERCONTEXTANGLE(GLeglContext ctx, GLenum target, GLsizei numAttachments, const GLenum *attachments);
typedef void GL_APIENTRY PFNGLINVALIDATESUBFRAMEBUFFERCONTEXTANGLE(GLeglContext ctx, GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
typedef GLboolean GL_APIENTRY PFNGLISBUFFERCONTEXTANGLE(GLeglContext ctx, GLuint buffer);
typedef GLboolean GL_APIENTRY PFNGLISENABLEDCONTEXTANGLE(GLeglContext ctx, GLenum cap);
typedef GLboolean GL_APIENTRY PFNGLISFENCENVCONTEXTANGLE(GLeglContext ctx, GLuint fence);
typedef GLboolean GL_APIENTRY PFNGLISFRAMEBUFFERCONTEXTANGLE(GLeglContext ctx, GLuint framebuffer);
typedef GLboolean GL_APIENTRY PFNGLISFRAMEBUFFEROESCONTEXTANGLE(GLeglContext ctx, GLuint framebuffer);
typedef GLboolean GL_APIENTRY PFNGLISPROGRAMCONTEXTANGLE(GLeglContext ctx, GLuint program);
typedef GLboolean GL_APIENTRY PFNGLISPROGRAMPIPELINECONTEXTANGLE(GLeglContext ctx, GLuint pipeline);
typedef GLboolean GL_APIENTRY PFNGLISQUERYCONTEXTANGLE(GLeglContext ctx, GLuint id);
typedef GLboolean GL_APIENTRY PFNGLISQUERYEXTCONTEXTANGLE(GLeglContext ctx, GLuint id);
typedef GLboolean GL_APIENTRY PFNGLISRENDERBUFFERCONTEXTANGLE(GLeglContext ctx, GLuint renderbuffer);
typedef GLboolean GL_APIENTRY PFNGLISRENDERBUFFEROESCONTEXTANGLE(GLeglContext ctx, GLuint renderbuffer);
typedef GLboolean GL_APIENTRY PFNGLISSAMPLERCONTEXTANGLE(GLeglContext ctx, GLuint sampler);
typedef GLboolean GL_APIENTRY PFNGLISSHADERCONTEXTANGLE(GLeglContext ctx, GLuint shader);
typedef GLboolean GL_APIENTRY PFNGLISSYNCCONTEXTANGLE(GLeglContext ctx, GLsync sync);
typedef GLboolean GL_APIENTRY PFNGLISTEXTURECONTEXTANGLE(GLeglContext ctx, GLuint texture);
typedef GLboolean GL_APIENTRY PFNGLISTRANSFORMFEEDBACKCONTEXTANGLE(GLeglContext ctx, GLuint id);
typedef GLboolean GL_APIENTRY PFNGLISVERTEXARRAYCONTEXTANGLE(GLeglContext ctx, GLuint array);
typedef GLboolean GL_APIENTRY PFNGLISVERTEXARRAYOESCONTEXTANGLE(GLeglContext ctx, GLuint array);
typedef void GL_APIENTRY PFNGLLIGHTMODELFCONTEXTANGLE(GLeglContext ctx, GLenum pname, GLfloat param);
typedef void GL_APIENTRY PFNGLLIGHTMODELFVCONTEXTANGLE(GLeglContext ctx, GLenum pname, const GLfloat *params);
typedef void GL_APIENTRY PFNGLLIGHTMODELXCONTEXTANGLE(GLeglContext ctx, GLenum pname, GLfixed param);
typedef void GL_APIENTRY PFNGLLIGHTMODELXVCONTEXTANGLE(GLeglContext ctx, GLenum pname, const GLfixed *param);
typedef void GL_APIENTRY PFNGLLIGHTFCONTEXTANGLE(GLeglContext ctx, GLenum light, GLenum pname, GLfloat param);
typedef void GL_APIENTRY PFNGLLIGHTFVCONTEXTANGLE(GLeglContext ctx, GLenum light, GLenum pname, const GLfloat *params);
typedef void GL_APIENTRY PFNGLLIGHTXCONTEXTANGLE(GLeglContext ctx, GLenum light, GLenum pname, GLfixed param);
typedef void GL_APIENTRY PFNGLLIGHTXVCONTEXTANGLE(GLeglContext ctx, GLenum light, GLenum pname, const GLfixed *params);
typedef void GL_APIENTRY PFNGLLINEWIDTHCONTEXTANGLE(GLeglContext ctx, GLfloat width);
typedef void GL_APIENTRY PFNGLLINEWIDTHXCONTEXTANGLE(GLeglContext ctx, GLfixed width);
typedef void GL_APIENTRY PFNGLLINKPROGRAMCONTEXTANGLE(GLeglContext ctx, GLuint program);
typedef void GL_APIENTRY PFNGLLOADIDENTITYCONTEXTANGLE(GLeglContext ctx);
typedef void GL_APIENTRY PFNGLLOADMATRIXFCONTEXTANGLE(GLeglContext ctx, const GLfloat *m);
typedef void GL_APIENTRY PFNGLLOADMATRIXXCONTEXTANGLE(GLeglContext ctx, const GLfixed *m);
typedef void GL_APIENTRY PFNGLLOADPALETTEFROMMODELVIEWMATRIXOESCONTEXTANGLE(GLeglContext ctx);
typedef void GL_APIENTRY PFNGLLOGICOPCONTEXTANGLE(GLeglContext ctx, GLenum opcode);
typedef void *GL_APIENTRY PFNGLMAPBUFFEROESCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum access);
typedef void *GL_APIENTRY PFNGLMAPBUFFERRANGECONTEXTANGLE(GLeglContext ctx, GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef void *GL_APIENTRY PFNGLMAPBUFFERRANGEEXTCONTEXTANGLE(GLeglContext ctx, GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef void GL_APIENTRY PFNGLMATERIALFCONTEXTANGLE(GLeglContext ctx, GLenum face, GLenum pname, GLfloat param);
typedef void GL_APIENTRY PFNGLMATERIALFVCONTEXTANGLE(GLeglContext ctx, GLenum face, GLenum pname, const GLfloat *params);
typedef void GL_APIENTRY PFNGLMATERIALXCONTEXTANGLE(GLeglContext ctx, GLenum face, GLenum pname, GLfixed param);
typedef void GL_APIENTRY PFNGLMATERIALXVCONTEXTANGLE(GLeglContext ctx, GLenum face, GLenum pname, const GLfixed *param);
typedef void GL_APIENTRY PFNGLMATRIXINDEXPOINTEROESCONTEXTANGLE(GLeglContext ctx, GLint size, GLenum type, GLsizei stride, const void *pointer);
typedef void GL_APIENTRY PFNGLMATRIXMODECONTEXTANGLE(GLeglContext ctx, GLenum mode);
typedef void GL_APIENTRY PFNGLMEMORYBARRIERCONTEXTANGLE(GLeglContext ctx, GLbitfield barriers);
typedef void GL_APIENTRY PFNGLMEMORYBARRIERBYREGIONCONTEXTANGLE(GLeglContext ctx, GLbitfield barriers);
typedef void GL_APIENTRY PFNGLMULTMATRIXFCONTEXTANGLE(GLeglContext ctx, const GLfloat *m);
typedef void GL_APIENTRY PFNGLMULTMATRIXXCONTEXTANGLE(GLeglContext ctx, const GLfixed *m);
typedef void GL_APIENTRY PFNGLMULTITEXCOORD4FCONTEXTANGLE(GLeglContext ctx, GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void GL_APIENTRY PFNGLMULTITEXCOORD4XCONTEXTANGLE(GLeglContext ctx, GLenum texture, GLfixed s, GLfixed t, GLfixed r, GLfixed q);
typedef void GL_APIENTRY PFNGLNORMAL3FCONTEXTANGLE(GLeglContext ctx, GLfloat nx, GLfloat ny, GLfloat nz);
typedef void GL_APIENTRY PFNGLNORMAL3XCONTEXTANGLE(GLeglContext ctx, GLfixed nx, GLfixed ny, GLfixed nz);
typedef void GL_APIENTRY PFNGLNORMALPOINTERCONTEXTANGLE(GLeglContext ctx, GLenum type, GLsizei stride, const void *pointer);
typedef void GL_APIENTRY PFNGLOBJECTLABELKHRCONTEXTANGLE(GLeglContext ctx, GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
typedef void GL_APIENTRY PFNGLOBJECTPTRLABELKHRCONTEXTANGLE(GLeglContext ctx, const void *ptr, GLsizei length, const GLchar *label);
typedef void GL_APIENTRY PFNGLORTHOFCONTEXTANGLE(GLeglContext ctx, GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f);
typedef void GL_APIENTRY PFNGLORTHOXCONTEXTANGLE(GLeglContext ctx, GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f);
typedef void GL_APIENTRY PFNGLPAUSETRANSFORMFEEDBACKCONTEXTANGLE(GLeglContext ctx);
typedef void GL_APIENTRY PFNGLPIXELSTOREICONTEXTANGLE(GLeglContext ctx, GLenum pname, GLint param);
typedef void GL_APIENTRY PFNGLPOINTPARAMETERFCONTEXTANGLE(GLeglContext ctx, GLenum pname, GLfloat param);
typedef void GL_APIENTRY PFNGLPOINTPARAMETERFVCONTEXTANGLE(GLeglContext ctx, GLenum pname, const GLfloat *params);
typedef void GL_APIENTRY PFNGLPOINTPARAMETERXCONTEXTANGLE(GLeglContext ctx, GLenum pname, GLfixed param);
typedef void GL_APIENTRY PFNGLPOINTPARAMETERXVCONTEXTANGLE(GLeglContext ctx, GLenum pname, const GLfixed *params);
typedef void GL_APIENTRY PFNGLPOINTSIZECONTEXTANGLE(GLeglContext ctx, GLfloat size);
typedef void GL_APIENTRY PFNGLPOINTSIZEPOINTEROESCONTEXTANGLE(GLeglContext ctx, GLenum type, GLsizei stride, const void *pointer);
typedef void GL_APIENTRY PFNGLPOINTSIZEXCONTEXTANGLE(GLeglContext ctx, GLfixed size);
typedef void GL_APIENTRY PFNGLPOLYGONOFFSETCONTEXTANGLE(GLeglContext ctx, GLfloat factor, GLfloat units);
typedef void GL_APIENTRY PFNGLPOLYGONOFFSETXCONTEXTANGLE(GLeglContext ctx, GLfixed factor, GLfixed units);
typedef void GL_APIENTRY PFNGLPOPDEBUGGROUPKHRCONTEXTANGLE(GLeglContext ctx);
typedef void GL_APIENTRY PFNGLPOPGROUPMARKEREXTCONTEXTANGLE(GLeglContext ctx);
typedef void GL_APIENTRY PFNGLPOPMATRIXCONTEXTANGLE(GLeglContext ctx);
typedef void GL_APIENTRY PFNGLPROGRAMBINARYCONTEXTANGLE(GLeglContext ctx, GLuint program, GLenum binaryFormat, const void *binary, GLsizei length);
typedef void GL_APIENTRY PFNGLPROGRAMBINARYOESCONTEXTANGLE(GLeglContext ctx, GLuint program, GLenum binaryFormat, const void *binary, GLint length);
typedef void GL_APIENTRY PFNGLPROGRAMPARAMETERICONTEXTANGLE(GLeglContext ctx, GLuint program, GLenum pname, GLint value);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM1FCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLfloat v0);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM1FVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM1ICONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLint v0);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM1IVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM1UICONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLuint v0);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM1UIVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM2FCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLfloat v0, GLfloat v1);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM2FVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM2ICONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLint v0, GLint v1);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM2IVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM2UICONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLuint v0, GLuint v1);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM2UIVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM3FCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM3FVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM3ICONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM3IVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM3UICONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM3UIVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM4FCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM4FVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM4ICONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM4IVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM4UICONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORM4UIVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORMMATRIX2FVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORMMATRIX2X3FVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORMMATRIX2X4FVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORMMATRIX3FVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORMMATRIX3X2FVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORMMATRIX3X4FVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORMMATRIX4FVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORMMATRIX4X2FVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GL_APIENTRY PFNGLPROGRAMUNIFORMMATRIX4X3FVCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GL_APIENTRY PFNGLPUSHDEBUGGROUPKHRCONTEXTANGLE(GLeglContext ctx, GLenum source, GLuint id, GLsizei length, const GLchar *message);
typedef void GL_APIENTRY PFNGLPUSHGROUPMARKEREXTCONTEXTANGLE(GLeglContext ctx, GLsizei length, const GLchar *marker);
typedef void GL_APIENTRY PFNGLPUSHMATRIXCONTEXTANGLE(GLeglContext ctx);
typedef void GL_APIENTRY PFNGLQUERYCOUNTEREXTCONTEXTANGLE(GLeglContext ctx, GLuint id, GLenum target);
typedef GLbitfield GL_APIENTRY PFNGLQUERYMATRIXXOESCONTEXTANGLE(GLeglContext ctx, GLfixed *mantissa, GLint *exponent);
typedef void GL_APIENTRY PFNGLREADBUFFERCONTEXTANGLE(GLeglContext ctx, GLenum src);
typedef void GL_APIENTRY PFNGLREADPIXELSCONTEXTANGLE(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
typedef void GL_APIENTRY PFNGLREADNPIXELSEXTCONTEXTANGLE(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
typedef void GL_APIENTRY PFNGLRELEASESHADERCOMPILERCONTEXTANGLE(GLeglContext ctx);
typedef void GL_APIENTRY PFNGLRENDERBUFFERSTORAGECONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void GL_APIENTRY PFNGLRENDERBUFFERSTORAGEMULTISAMPLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
typedef void GL_APIENTRY PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
typedef void GL_APIENTRY PFNGLRENDERBUFFERSTORAGEOESCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void GL_APIENTRY PFNGLRESUMETRANSFORMFEEDBACKCONTEXTANGLE(GLeglContext ctx);
typedef void GL_APIENTRY PFNGLROTATEFCONTEXTANGLE(GLeglContext ctx, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
typedef void GL_APIENTRY PFNGLROTATEXCONTEXTANGLE(GLeglContext ctx, GLfixed angle, GLfixed x, GLfixed y, GLfixed z);
typedef void GL_APIENTRY PFNGLSAMPLECOVERAGECONTEXTANGLE(GLeglContext ctx, GLfloat value, GLboolean invert);
typedef void GL_APIENTRY PFNGLSAMPLECOVERAGEXCONTEXTANGLE(GLeglContext ctx, GLclampx value, GLboolean invert);
typedef void GL_APIENTRY PFNGLSAMPLEMASKICONTEXTANGLE(GLeglContext ctx, GLuint maskNumber, GLbitfield mask);
typedef void GL_APIENTRY PFNGLSAMPLERPARAMETERFCONTEXTANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLfloat param);
typedef void GL_APIENTRY PFNGLSAMPLERPARAMETERFVCONTEXTANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, const GLfloat *param);
typedef void GL_APIENTRY PFNGLSAMPLERPARAMETERICONTEXTANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLint param);
typedef void GL_APIENTRY PFNGLSAMPLERPARAMETERIVCONTEXTANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, const GLint *param);
typedef void GL_APIENTRY PFNGLSCALEFCONTEXTANGLE(GLeglContext ctx, GLfloat x, GLfloat y, GLfloat z);
typedef void GL_APIENTRY PFNGLSCALEXCONTEXTANGLE(GLeglContext ctx, GLfixed x, GLfixed y, GLfixed z);
typedef void GL_APIENTRY PFNGLSCISSORCONTEXTANGLE(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void GL_APIENTRY PFNGLSETFENCENVCONTEXTANGLE(GLeglContext ctx, GLuint fence, GLenum condition);
typedef void GL_APIENTRY PFNGLSHADEMODELCONTEXTANGLE(GLeglContext ctx, GLenum mode);
typedef void GL_APIENTRY PFNGLSHADERBINARYCONTEXTANGLE(GLeglContext ctx, GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length);
typedef void GL_APIENTRY PFNGLSHADERSOURCECONTEXTANGLE(GLeglContext ctx, GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void GL_APIENTRY PFNGLSTENCILFUNCCONTEXTANGLE(GLeglContext ctx, GLenum func, GLint ref, GLuint mask);
typedef void GL_APIENTRY PFNGLSTENCILFUNCSEPARATECONTEXTANGLE(GLeglContext ctx, GLenum face, GLenum func, GLint ref, GLuint mask);
typedef void GL_APIENTRY PFNGLSTENCILMASKCONTEXTANGLE(GLeglContext ctx, GLuint mask);
typedef void GL_APIENTRY PFNGLSTENCILMASKSEPARATECONTEXTANGLE(GLeglContext ctx, GLenum face, GLuint mask);
typedef void GL_APIENTRY PFNGLSTENCILOPCONTEXTANGLE(GLeglContext ctx, GLenum fail, GLenum zfail, GLenum zpass);
typedef void GL_APIENTRY PFNGLSTENCILOPSEPARATECONTEXTANGLE(GLeglContext ctx, GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
typedef GLboolean GL_APIENTRY PFNGLTESTFENCENVCONTEXTANGLE(GLeglContext ctx, GLuint fence);
typedef void GL_APIENTRY PFNGLTEXCOORDPOINTERCONTEXTANGLE(GLeglContext ctx, GLint size, GLenum type, GLsizei stride, const void *pointer);
typedef void GL_APIENTRY PFNGLTEXENVFCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfloat param);
typedef void GL_APIENTRY PFNGLTEXENVFVCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, const GLfloat *params);
typedef void GL_APIENTRY PFNGLTEXENVICONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint param);
typedef void GL_APIENTRY PFNGLTEXENVIVCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, const GLint *params);
typedef void GL_APIENTRY PFNGLTEXENVXCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfixed param);
typedef void GL_APIENTRY PFNGLTEXENVXVCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, const GLfixed *params);
typedef void GL_APIENTRY PFNGLTEXGENFOESCONTEXTANGLE(GLeglContext ctx, GLenum coord, GLenum pname, GLfloat param);
typedef void GL_APIENTRY PFNGLTEXGENFVOESCONTEXTANGLE(GLeglContext ctx, GLenum coord, GLenum pname, const GLfloat *params);
typedef void GL_APIENTRY PFNGLTEXGENIOESCONTEXTANGLE(GLeglContext ctx, GLenum coord, GLenum pname, GLint param);
typedef void GL_APIENTRY PFNGLTEXGENIVOESCONTEXTANGLE(GLeglContext ctx, GLenum coord, GLenum pname, const GLint *params);
typedef void GL_APIENTRY PFNGLTEXGENXOESCONTEXTANGLE(GLeglContext ctx, GLenum coord, GLenum pname, GLfixed param);
typedef void GL_APIENTRY PFNGLTEXGENXVOESCONTEXTANGLE(GLeglContext ctx, GLenum coord, GLenum pname, const GLfixed *params);
typedef void GL_APIENTRY PFNGLTEXIMAGE2DCONTEXTANGLE(GLeglContext ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void GL_APIENTRY PFNGLTEXIMAGE3DCONTEXTANGLE(GLeglContext ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void GL_APIENTRY PFNGLTEXPARAMETERFCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfloat param);
typedef void GL_APIENTRY PFNGLTEXPARAMETERFVCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, const GLfloat *params);
typedef void GL_APIENTRY PFNGLTEXPARAMETERICONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint param);
typedef void GL_APIENTRY PFNGLTEXPARAMETERIVCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, const GLint *params);
typedef void GL_APIENTRY PFNGLTEXPARAMETERXCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfixed param);
typedef void GL_APIENTRY PFNGLTEXPARAMETERXVCONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, const GLfixed *params);
typedef void GL_APIENTRY PFNGLTEXSTORAGE1DEXTCONTEXTANGLE(GLeglContext ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
typedef void GL_APIENTRY PFNGLTEXSTORAGE2DCONTEXTANGLE(GLeglContext ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
typedef void GL_APIENTRY PFNGLTEXSTORAGE2DEXTCONTEXTANGLE(GLeglContext ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
typedef void GL_APIENTRY PFNGLTEXSTORAGE2DMULTISAMPLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void GL_APIENTRY PFNGLTEXSTORAGE3DCONTEXTANGLE(GLeglContext ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
typedef void GL_APIENTRY PFNGLTEXSTORAGE3DEXTCONTEXTANGLE(GLeglContext ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
typedef void GL_APIENTRY PFNGLTEXSUBIMAGE2DCONTEXTANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
typedef void GL_APIENTRY PFNGLTEXSUBIMAGE3DCONTEXTANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
typedef void GL_APIENTRY PFNGLTRANSFORMFEEDBACKVARYINGSCONTEXTANGLE(GLeglContext ctx, GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode);
typedef void GL_APIENTRY PFNGLTRANSLATEFCONTEXTANGLE(GLeglContext ctx, GLfloat x, GLfloat y, GLfloat z);
typedef void GL_APIENTRY PFNGLTRANSLATEXCONTEXTANGLE(GLeglContext ctx, GLfixed x, GLfixed y, GLfixed z);
typedef void GL_APIENTRY PFNGLUNIFORM1FCONTEXTANGLE(GLeglContext ctx, GLint location, GLfloat v0);
typedef void GL_APIENTRY PFNGLUNIFORM1FVCONTEXTANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLfloat *value);
typedef void GL_APIENTRY PFNGLUNIFORM1ICONTEXTANGLE(GLeglContext ctx, GLint location, GLint v0);
typedef void GL_APIENTRY PFNGLUNIFORM1IVCONTEXTANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLint *value);
typedef void GL_APIENTRY PFNGLUNIFORM1UICONTEXTANGLE(GLeglContext ctx, GLint location, GLuint v0);
typedef void GL_APIENTRY PFNGLUNIFORM1UIVCONTEXTANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLuint *value);
typedef void GL_APIENTRY PFNGLUNIFORM2FCONTEXTANGLE(GLeglContext ctx, GLint location, GLfloat v0, GLfloat v1);
typedef void GL_APIENTRY PFNGLUNIFORM2FVCONTEXTANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLfloat *value);
typedef void GL_APIENTRY PFNGLUNIFORM2ICONTEXTANGLE(GLeglContext ctx, GLint location, GLint v0, GLint v1);
typedef void GL_APIENTRY PFNGLUNIFORM2IVCONTEXTANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLint *value);
typedef void GL_APIENTRY PFNGLUNIFORM2UICONTEXTANGLE(GLeglContext ctx, GLint location, GLuint v0, GLuint v1);
typedef void GL_APIENTRY PFNGLUNIFORM2UIVCONTEXTANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLuint *value);
typedef void GL_APIENTRY PFNGLUNIFORM3FCONTEXTANGLE(GLeglContext ctx, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void GL_APIENTRY PFNGLUNIFORM3FVCONTEXTANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLfloat *value);
typedef void GL_APIENTRY PFNGLUNIFORM3ICONTEXTANGLE(GLeglContext ctx, GLint location, GLint v0, GLint v1, GLint v2);
typedef void GL_APIENTRY PFNGLUNIFORM3IVCONTEXTANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLint *value);
typedef void GL_APIENTRY PFNGLUNIFORM3UICONTEXTANGLE(GLeglContext ctx, GLint location, GLuint v0, GLuint v1, GLuint v2);
typedef void GL_APIENTRY PFNGLUNIFORM3UIVCONTEXTANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLuint *value);
typedef void GL_APIENTRY PFNGLUNIFORM4FCONTEXTANGLE(GLeglContext ctx, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void GL_APIENTRY PFNGLUNIFORM4FVCONTEXTANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLfloat *value);
typedef void GL_APIENTRY PFNGLUNIFORM4ICONTEXTANGLE(GLeglContext ctx, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void GL_APIENTRY PFNGLUNIFORM4IVCONTEXTANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLint *value);
typedef void GL_APIENTRY PFNGLUNIFORM4UICONTEXTANGLE(GLeglContext ctx, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
typedef void GL_APIENTRY PFNGLUNIFORM4UIVCONTEXTANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLuint *value);
typedef void GL_APIENTRY PFNGLUNIFORMBLOCKBINDINGCONTEXTANGLE(GLeglContext ctx, GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
typedef void GL_APIENTRY PFNGLUNIFORMMATRIX2FVCONTEXTANGLE(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GL_APIENTRY PFNGLUNIFORMMATRIX2X3FVCONTEXTANGLE(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GL_APIENTRY PFNGLUNIFORMMATRIX2X4FVCONTEXTANGLE(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GL_APIENTRY PFNGLUNIFORMMATRIX3FVCONTEXTANGLE(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GL_APIENTRY PFNGLUNIFORMMATRIX3X2FVCONTEXTANGLE(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GL_APIENTRY PFNGLUNIFORMMATRIX3X4FVCONTEXTANGLE(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GL_APIENTRY PFNGLUNIFORMMATRIX4FVCONTEXTANGLE(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GL_APIENTRY PFNGLUNIFORMMATRIX4X2FVCONTEXTANGLE(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GL_APIENTRY PFNGLUNIFORMMATRIX4X3FVCONTEXTANGLE(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef GLboolean GL_APIENTRY PFNGLUNMAPBUFFERCONTEXTANGLE(GLeglContext ctx, GLenum target);
typedef GLboolean GL_APIENTRY PFNGLUNMAPBUFFEROESCONTEXTANGLE(GLeglContext ctx, GLenum target);
typedef void GL_APIENTRY PFNGLUSEPROGRAMCONTEXTANGLE(GLeglContext ctx, GLuint program);
typedef void GL_APIENTRY PFNGLUSEPROGRAMSTAGESCONTEXTANGLE(GLeglContext ctx, GLuint pipeline, GLbitfield stages, GLuint program);
typedef void GL_APIENTRY PFNGLVALIDATEPROGRAMCONTEXTANGLE(GLeglContext ctx, GLuint program);
typedef void GL_APIENTRY PFNGLVALIDATEPROGRAMPIPELINECONTEXTANGLE(GLeglContext ctx, GLuint pipeline);
typedef void GL_APIENTRY PFNGLVERTEXATTRIB1FCONTEXTANGLE(GLeglContext ctx, GLuint index, GLfloat x);
typedef void GL_APIENTRY PFNGLVERTEXATTRIB1FVCONTEXTANGLE(GLeglContext ctx, GLuint index, const GLfloat *v);
typedef void GL_APIENTRY PFNGLVERTEXATTRIB2FCONTEXTANGLE(GLeglContext ctx, GLuint index, GLfloat x, GLfloat y);
typedef void GL_APIENTRY PFNGLVERTEXATTRIB2FVCONTEXTANGLE(GLeglContext ctx, GLuint index, const GLfloat *v);
typedef void GL_APIENTRY PFNGLVERTEXATTRIB3FCONTEXTANGLE(GLeglContext ctx, GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void GL_APIENTRY PFNGLVERTEXATTRIB3FVCONTEXTANGLE(GLeglContext ctx, GLuint index, const GLfloat *v);
typedef void GL_APIENTRY PFNGLVERTEXATTRIB4FCONTEXTANGLE(GLeglContext ctx, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void GL_APIENTRY PFNGLVERTEXATTRIB4FVCONTEXTANGLE(GLeglContext ctx, GLuint index, const GLfloat *v);
typedef void GL_APIENTRY PFNGLVERTEXATTRIBBINDINGCONTEXTANGLE(GLeglContext ctx, GLuint attribindex, GLuint bindingindex);
typedef void GL_APIENTRY PFNGLVERTEXATTRIBDIVISORCONTEXTANGLE(GLeglContext ctx, GLuint index, GLuint divisor);
typedef void GL_APIENTRY PFNGLVERTEXATTRIBDIVISORANGLECONTEXTANGLE(GLeglContext ctx, GLuint index, GLuint divisor);
typedef void GL_APIENTRY PFNGLVERTEXATTRIBFORMATCONTEXTANGLE(GLeglContext ctx, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
typedef void GL_APIENTRY PFNGLVERTEXATTRIBI4ICONTEXTANGLE(GLeglContext ctx, GLuint index, GLint x, GLint y, GLint z, GLint w);
typedef void GL_APIENTRY PFNGLVERTEXATTRIBI4IVCONTEXTANGLE(GLeglContext ctx, GLuint index, const GLint *v);
typedef void GL_APIENTRY PFNGLVERTEXATTRIBI4UICONTEXTANGLE(GLeglContext ctx, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
typedef void GL_APIENTRY PFNGLVERTEXATTRIBI4UIVCONTEXTANGLE(GLeglContext ctx, GLuint index, const GLuint *v);
typedef void GL_APIENTRY PFNGLVERTEXATTRIBIFORMATCONTEXTANGLE(GLeglContext ctx, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
typedef void GL_APIENTRY PFNGLVERTEXATTRIBIPOINTERCONTEXTANGLE(GLeglContext ctx, GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
typedef void GL_APIENTRY PFNGLVERTEXATTRIBPOINTERCONTEXTANGLE(GLeglContext ctx, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void GL_APIENTRY PFNGLVERTEXBINDINGDIVISORCONTEXTANGLE(GLeglContext ctx, GLuint bindingindex, GLuint divisor);
typedef void GL_APIENTRY PFNGLVERTEXPOINTERCONTEXTANGLE(GLeglContext ctx, GLint size, GLenum type, GLsizei stride, const void *pointer);
typedef void GL_APIENTRY PFNGLVIEWPORTCONTEXTANGLE(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void GL_APIENTRY PFNGLWAITSYNCCONTEXTANGLE(GLeglContext ctx, GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef void GL_APIENTRY PFNGLWEIGHTPOINTEROESCONTEXTANGLE(GLeglContext ctx, GLint size, GLenum type, GLsizei stride, const void *pointer);
typedef void GL_APIENTRY PFNGLBINDUNIFORMLOCATIONCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, const GLchar* name);
typedef void GL_APIENTRY PFNGLCOVERAGEMODULATIONCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLenum components);
typedef void GL_APIENTRY PFNGLMATRIXLOADFCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLenum matrixMode, const GLfloat * matrix);
typedef void GL_APIENTRY PFNGLMATRIXLOADIDENTITYCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLenum matrixMode);
typedef GLuint GL_APIENTRY PFNGLGENPATHSCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLsizei range);
typedef void GL_APIENTRY PFNGLDELETEPATHSCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLuint first, GLsizei range);
typedef GLboolean GL_APIENTRY PFNGLISPATHCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLuint path);
typedef void GL_APIENTRY PFNGLPATHCOMMANDSCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLuint path, GLsizei numCommands, const GLubyte * commands, GLsizei numCoords, GLenum coordType, const void* coords);
typedef void GL_APIENTRY PFNGLPATHPARAMETERFCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLuint path, GLenum pname, GLfloat value);
typedef void GL_APIENTRY PFNGLPATHPARAMETERICHROMIUMCONTEXTANGLE(GLeglContext ctx, GLuint path, GLenum pname, GLint value);
typedef void GL_APIENTRY PFNGLGETPATHPARAMETERFVCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLuint path, GLenum pname, GLfloat * value);
typedef void GL_APIENTRY PFNGLGETPATHPARAMETERIVCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLuint path, GLenum pname, GLint * value);
typedef void GL_APIENTRY PFNGLPATHSTENCILFUNCCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLenum func, GLint ref, GLuint mask);
typedef void GL_APIENTRY PFNGLSTENCILFILLPATHCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLuint path, GLenum fillMode, GLuint mask);
typedef void GL_APIENTRY PFNGLSTENCILSTROKEPATHCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLuint path, GLint reference, GLuint mask);
typedef void GL_APIENTRY PFNGLCOVERFILLPATHCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLuint path, GLenum coverMode);
typedef void GL_APIENTRY PFNGLCOVERSTROKEPATHCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLuint path, GLenum coverMode);
typedef void GL_APIENTRY PFNGLSTENCILTHENCOVERFILLPATHCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLuint path, GLenum fillMode, GLuint mask, GLenum coverMode);
typedef void GL_APIENTRY PFNGLSTENCILTHENCOVERSTROKEPATHCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLuint path, GLint reference, GLuint mask, GLenum coverMode);
typedef void GL_APIENTRY PFNGLCOVERFILLPATHINSTANCEDCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLsizei numPath, GLenum pathNameType, const void * paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat * transformValues);
typedef void GL_APIENTRY PFNGLCOVERSTROKEPATHINSTANCEDCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLsizei numPath, GLenum pathNameType, const void * paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat * transformValues);
typedef void GL_APIENTRY PFNGLSTENCILSTROKEPATHINSTANCEDCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLsizei numPath, GLenum pathNameType, const void * paths, GLuint pathBase, GLint reference, GLuint mask, GLenum transformType, const GLfloat * transformValues);
typedef void GL_APIENTRY PFNGLSTENCILFILLPATHINSTANCEDCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLsizei numPaths, GLenum pathNameType, const void * paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum transformType, const GLfloat * transformValues);
typedef void GL_APIENTRY PFNGLSTENCILTHENCOVERFILLPATHINSTANCEDCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLsizei numPaths, GLenum pathNameType, const void * paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat * transformValues);
typedef void GL_APIENTRY PFNGLSTENCILTHENCOVERSTROKEPATHINSTANCEDCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLsizei numPaths, GLenum pathNameType, const void * paths, GLuint pathBase, GLint reference, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat * transformValues);
typedef void GL_APIENTRY PFNGLBINDFRAGMENTINPUTLOCATIONCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLuint programs, GLint location, const GLchar * name);
typedef void GL_APIENTRY PFNGLPROGRAMPATHFRAGMENTINPUTGENCHROMIUMCONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLenum genMode, GLint components, const GLfloat * coeffs);
typedef void GL_APIENTRY PFNGLCOPYTEXTURECHROMIUMCONTEXTANGLE(GLeglContext ctx, GLuint sourceId, GLint sourceLevel, GLenum destTarget, GLuint destId, GLint destLevel, GLint internalFormat, GLenum destType, GLboolean unpackFlipY, GLboolean unpackPremultiplyAlpha, GLboolean unpackUnmultiplyAlpha);
typedef void GL_APIENTRY PFNGLCOPYSUBTEXTURECHROMIUMCONTEXTANGLE(GLeglContext ctx, GLuint sourceId, GLint sourceLevel, GLenum destTarget, GLuint destId, GLint destLevel, GLint xoffset, GLint yoffset, GLint x, GLint y, GLint width, GLint height, GLboolean unpackFlipY, GLboolean unpackPremultiplyAlpha, GLboolean unpackUnmultiplyAlpha);
typedef void GL_APIENTRY PFNGLCOMPRESSEDCOPYTEXTURECHROMIUMCONTEXTANGLE(GLeglContext ctx, GLuint sourceId, GLuint destId);
typedef void GL_APIENTRY PFNGLREQUESTEXTENSIONANGLECONTEXTANGLE(GLeglContext ctx, const GLchar * name);
typedef void GL_APIENTRY PFNGLGETBOOLEANVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum pname, GLsizei bufSize, GLsizei * length, GLboolean * params);
typedef void GL_APIENTRY PFNGLGETBUFFERPARAMETERIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void GL_APIENTRY PFNGLGETFLOATVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum pname, GLsizei bufSize, GLsizei * length, GLfloat * params);
typedef void GL_APIENTRY PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void GL_APIENTRY PFNGLGETINTEGERVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * data);
typedef void GL_APIENTRY PFNGLGETPROGRAMIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint program, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void GL_APIENTRY PFNGLGETRENDERBUFFERPARAMETERIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void GL_APIENTRY PFNGLGETSHADERIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint shader, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void GL_APIENTRY PFNGLGETTEXPARAMETERFVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLfloat * params);
typedef void GL_APIENTRY PFNGLGETTEXPARAMETERIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void GL_APIENTRY PFNGLGETUNIFORMFVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLfloat * params);
typedef void GL_APIENTRY PFNGLGETUNIFORMIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void GL_APIENTRY PFNGLGETVERTEXATTRIBFVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLsizei bufSize, GLsizei * length, GLfloat * params);
typedef void GL_APIENTRY PFNGLGETVERTEXATTRIBIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void GL_APIENTRY PFNGLGETVERTEXATTRIBPOINTERVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLsizei bufSize, GLsizei * length, void ** pointer);
typedef void GL_APIENTRY PFNGLREADPIXELSROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLsizei * length, GLsizei * columns, GLsizei * rows, void * pixels);
typedef void GL_APIENTRY PFNGLTEXIMAGE2DROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, GLsizei bufSize, const void * pixels);
typedef void GL_APIENTRY PFNGLTEXPARAMETERFVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, const GLfloat * params);
typedef void GL_APIENTRY PFNGLTEXPARAMETERIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, const GLint * params);
typedef void GL_APIENTRY PFNGLTEXSUBIMAGE2DROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, const void * pixels);
typedef void GL_APIENTRY PFNGLTEXIMAGE3DROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, GLsizei bufSize, const void * pixels);
typedef void GL_APIENTRY PFNGLTEXSUBIMAGE3DROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, const void * pixels);
typedef void GL_APIENTRY PFNGLCOMPRESSEDTEXIMAGE2DROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, GLsizei dataSize, const GLvoid * data);
typedef void GL_APIENTRY PFNGLCOMPRESSEDTEXSUBIMAGE2DROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLint level, GLsizei xoffset, GLsizei yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, GLsizei dataSize, const GLvoid * data);
typedef void GL_APIENTRY PFNGLCOMPRESSEDTEXIMAGE3DROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, GLsizei dataSize, const GLvoid * data);
typedef void GL_APIENTRY PFNGLCOMPRESSEDTEXSUBIMAGE3DROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, GLsizei dataSize, const GLvoid * data);
typedef void GL_APIENTRY PFNGLGETQUERYIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void GL_APIENTRY PFNGLGETQUERYOBJECTUIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLsizei bufSize, GLsizei * length, GLuint * params);
typedef void GL_APIENTRY PFNGLGETBUFFERPOINTERVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, void ** params);
typedef void GL_APIENTRY PFNGLGETINTEGERI_VROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLuint index, GLsizei bufSize, GLsizei * length, GLint * data);
typedef void GL_APIENTRY PFNGLGETINTERNALFORMATIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void GL_APIENTRY PFNGLGETVERTEXATTRIBIIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void GL_APIENTRY PFNGLGETVERTEXATTRIBIUIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLsizei bufSize, GLsizei * length, GLuint * params);
typedef void GL_APIENTRY PFNGLGETUNIFORMUIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLuint * params);
typedef void GL_APIENTRY PFNGLGETACTIVEUNIFORMBLOCKIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint program, GLuint uniformBlockIndex, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void GL_APIENTRY PFNGLGETINTEGER64VROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum pname, GLsizei bufSize, GLsizei * length, GLint64 * data);
typedef void GL_APIENTRY PFNGLGETINTEGER64I_VROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLuint index, GLsizei bufSize, GLsizei * length, GLint64 * data);
typedef void GL_APIENTRY PFNGLGETBUFFERPARAMETERI64VROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint64 * params);
typedef void GL_APIENTRY PFNGLSAMPLERPARAMETERIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint sampler, GLuint pname, GLsizei bufSize, const GLint * param);
typedef void GL_APIENTRY PFNGLSAMPLERPARAMETERFVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, const GLfloat * param);
typedef void GL_APIENTRY PFNGLGETSAMPLERPARAMETERIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void GL_APIENTRY PFNGLGETSAMPLERPARAMETERFVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei * length, GLfloat * params);
typedef void GL_APIENTRY PFNGLGETFRAMEBUFFERPARAMETERIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void GL_APIENTRY PFNGLGETPROGRAMINTERFACEIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint program, GLenum programInterface, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void GL_APIENTRY PFNGLGETBOOLEANI_VROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLuint index, GLsizei bufSize, GLsizei * length, GLboolean * data);
typedef void GL_APIENTRY PFNGLGETMULTISAMPLEFVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum pname, GLuint index, GLsizei bufSize, GLsizei * length, GLfloat * val);
typedef void GL_APIENTRY PFNGLGETTEXLEVELPARAMETERIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void GL_APIENTRY PFNGLGETTEXLEVELPARAMETERFVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum pname, GLsizei bufSize, GLsizei * length, GLfloat * params);
typedef void GL_APIENTRY PFNGLGETPOINTERVROBUSTANGLEROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum pname, GLsizei bufSize, GLsizei * length, void ** params);
typedef void GL_APIENTRY PFNGLREADNPIXELSROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLsizei * length, GLsizei * columns, GLsizei * rows, void * data);
typedef void GL_APIENTRY PFNGLGETNUNIFORMFVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLfloat * params);
typedef void GL_APIENTRY PFNGLGETNUNIFORMIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void GL_APIENTRY PFNGLGETNUNIFORMUIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLuint * params);
typedef void GL_APIENTRY PFNGLTEXPARAMETERIIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, const GLint * params);
typedef void GL_APIENTRY PFNGLTEXPARAMETERIUIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, const GLuint * params);
typedef void GL_APIENTRY PFNGLGETTEXPARAMETERIIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void GL_APIENTRY PFNGLGETTEXPARAMETERIUIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLuint * params);
typedef void GL_APIENTRY PFNGLSAMPLERPARAMETERIIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, const GLint * param);
typedef void GL_APIENTRY PFNGLSAMPLERPARAMETERIUIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, const GLuint * param);
typedef void GL_APIENTRY PFNGLGETSAMPLERPARAMETERIIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void GL_APIENTRY PFNGLGETSAMPLERPARAMETERIUIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei * length, GLuint * params);
typedef void GL_APIENTRY PFNGLGETQUERYOBJECTIVROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
typedef void GL_APIENTRY PFNGLGETQUERYOBJECTI64VROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLsizei bufSize, GLsizei * length, GLint64 * params);
typedef void GL_APIENTRY PFNGLGETQUERYOBJECTUI64VROBUSTANGLECONTEXTANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLsizei bufSize, GLsizei * length, GLuint64 * params);
typedef void GL_APIENTRY PFNGLFRAMEBUFFERTEXTUREMULTIVIEWLAYEREDANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews);
typedef void GL_APIENTRY PFNGLFRAMEBUFFERTEXTUREMULTIVIEWSIDEBYSIDEANGLECONTEXTANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLuint texture, GLint level, GLsizei numViews, const GLint * viewportOffsets);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glActiveShaderProgramContextANGLE(GLeglContext ctx, GLuint pipeline, GLuint program);
GL_APICALL void GL_APIENTRY glActiveTextureContextANGLE(GLeglContext ctx, GLenum texture);
GL_APICALL void GL_APIENTRY glAlphaFuncContextANGLE(GLeglContext ctx, GLenum func, GLfloat ref);
GL_APICALL void GL_APIENTRY glAlphaFuncxContextANGLE(GLeglContext ctx, GLenum func, GLfixed ref);
GL_APICALL void GL_APIENTRY glAttachShaderContextANGLE(GLeglContext ctx, GLuint program, GLuint shader);
GL_APICALL void GL_APIENTRY glBeginQueryContextANGLE(GLeglContext ctx, GLenum target, GLuint id);
GL_APICALL void GL_APIENTRY glBeginQueryEXTContextANGLE(GLeglContext ctx, GLenum target, GLuint id);
GL_APICALL void GL_APIENTRY glBeginTransformFeedbackContextANGLE(GLeglContext ctx, GLenum primitiveMode);
GL_APICALL void GL_APIENTRY glBindAttribLocationContextANGLE(GLeglContext ctx, GLuint program, GLuint index, const GLchar *name);
GL_APICALL void GL_APIENTRY glBindBufferContextANGLE(GLeglContext ctx, GLenum target, GLuint buffer);
GL_APICALL void GL_APIENTRY glBindBufferBaseContextANGLE(GLeglContext ctx, GLenum target, GLuint index, GLuint buffer);
GL_APICALL void GL_APIENTRY glBindBufferRangeContextANGLE(GLeglContext ctx, GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
GL_APICALL void GL_APIENTRY glBindFramebufferContextANGLE(GLeglContext ctx, GLenum target, GLuint framebuffer);
GL_APICALL void GL_APIENTRY glBindFramebufferOESContextANGLE(GLeglContext ctx, GLenum target, GLuint framebuffer);
GL_APICALL void GL_APIENTRY glBindImageTextureContextANGLE(GLeglContext ctx, GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
GL_APICALL void GL_APIENTRY glBindProgramPipelineContextANGLE(GLeglContext ctx, GLuint pipeline);
GL_APICALL void GL_APIENTRY glBindRenderbufferContextANGLE(GLeglContext ctx, GLenum target, GLuint renderbuffer);
GL_APICALL void GL_APIENTRY glBindRenderbufferOESContextANGLE(GLeglContext ctx, GLenum target, GLuint renderbuffer);
GL_APICALL void GL_APIENTRY glBindSamplerContextANGLE(GLeglContext ctx, GLuint unit, GLuint sampler);
GL_APICALL void GL_APIENTRY glBindTextureContextANGLE(GLeglContext ctx, GLenum target, GLuint texture);
GL_APICALL void GL_APIENTRY glBindTransformFeedbackContextANGLE(GLeglContext ctx, GLenum target, GLuint id);
GL_APICALL void GL_APIENTRY glBindVertexArrayContextANGLE(GLeglContext ctx, GLuint array);
GL_APICALL void GL_APIENTRY glBindVertexArrayOESContextANGLE(GLeglContext ctx, GLuint array);
GL_APICALL void GL_APIENTRY glBindVertexBufferContextANGLE(GLeglContext ctx, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
GL_APICALL void GL_APIENTRY glBlendColorContextANGLE(GLeglContext ctx, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
GL_APICALL void GL_APIENTRY glBlendEquationContextANGLE(GLeglContext ctx, GLenum mode);
GL_APICALL void GL_APIENTRY glBlendEquationSeparateContextANGLE(GLeglContext ctx, GLenum modeRGB, GLenum modeAlpha);
GL_APICALL void GL_APIENTRY glBlendFuncContextANGLE(GLeglContext ctx, GLenum sfactor, GLenum dfactor);
GL_APICALL void GL_APIENTRY glBlendFuncSeparateContextANGLE(GLeglContext ctx, GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
GL_APICALL void GL_APIENTRY glBlitFramebufferContextANGLE(GLeglContext ctx, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
GL_APICALL void GL_APIENTRY glBlitFramebufferANGLEContextANGLE(GLeglContext ctx, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
GL_APICALL void GL_APIENTRY glBufferDataContextANGLE(GLeglContext ctx, GLenum target, GLsizeiptr size, const void *data, GLenum usage);
GL_APICALL void GL_APIENTRY glBufferSubDataContextANGLE(GLeglContext ctx, GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
GL_APICALL GLenum GL_APIENTRY glCheckFramebufferStatusContextANGLE(GLeglContext ctx, GLenum target);
GL_APICALL GLenum GL_APIENTRY glCheckFramebufferStatusOESContextANGLE(GLeglContext ctx, GLenum target);
GL_APICALL void GL_APIENTRY glClearContextANGLE(GLeglContext ctx, GLbitfield mask);
GL_APICALL void GL_APIENTRY glClearBufferfiContextANGLE(GLeglContext ctx, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
GL_APICALL void GL_APIENTRY glClearBufferfvContextANGLE(GLeglContext ctx, GLenum buffer, GLint drawbuffer, const GLfloat *value);
GL_APICALL void GL_APIENTRY glClearBufferivContextANGLE(GLeglContext ctx, GLenum buffer, GLint drawbuffer, const GLint *value);
GL_APICALL void GL_APIENTRY glClearBufferuivContextANGLE(GLeglContext ctx, GLenum buffer, GLint drawbuffer, const GLuint *value);
GL_APICALL void GL_APIENTRY glClearColorContextANGLE(GLeglContext ctx, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
GL_APICALL void GL_APIENTRY glClearColorxContextANGLE(GLeglContext ctx, GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
GL_APICALL void GL_APIENTRY glClearDepthfContextANGLE(GLeglContext ctx, GLfloat d);
GL_APICALL void GL_APIENTRY glClearDepthxContextANGLE(GLeglContext ctx, GLfixed depth);
GL_APICALL void GL_APIENTRY glClearStencilContextANGLE(GLeglContext ctx, GLint s);
GL_APICALL void GL_APIENTRY glClientActiveTextureContextANGLE(GLeglContext ctx, GLenum texture);
GL_APICALL GLenum GL_APIENTRY glClientWaitSyncContextANGLE(GLeglContext ctx, GLsync sync, GLbitfield flags, GLuint64 timeout);
GL_APICALL void GL_APIENTRY glClipPlanefContextANGLE(GLeglContext ctx, GLenum p, const GLfloat *eqn);
GL_APICALL void GL_APIENTRY glClipPlanexContextANGLE(GLeglContext ctx, GLenum plane, const GLfixed *equation);
GL_APICALL void GL_APIENTRY glColor4fContextANGLE(GLeglContext ctx, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
GL_APICALL void GL_APIENTRY glColor4ubContextANGLE(GLeglContext ctx, GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
GL_APICALL void GL_APIENTRY glColor4xContextANGLE(GLeglContext ctx, GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
GL_APICALL void GL_APIENTRY glColorMaskContextANGLE(GLeglContext ctx, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
GL_APICALL void GL_APIENTRY glColorPointerContextANGLE(GLeglContext ctx, GLint size, GLenum type, GLsizei stride, const void *pointer);
GL_APICALL void GL_APIENTRY glCompileShaderContextANGLE(GLeglContext ctx, GLuint shader);
GL_APICALL void GL_APIENTRY glCompressedTexImage2DContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
GL_APICALL void GL_APIENTRY glCompressedTexImage3DContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
GL_APICALL void GL_APIENTRY glCompressedTexSubImage2DContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
GL_APICALL void GL_APIENTRY glCompressedTexSubImage3DContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
GL_APICALL void GL_APIENTRY glCopyBufferSubDataContextANGLE(GLeglContext ctx, GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
GL_APICALL void GL_APIENTRY glCopyTexImage2DContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
GL_APICALL void GL_APIENTRY glCopyTexSubImage2DContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GL_APICALL void GL_APIENTRY glCopyTexSubImage3DContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GL_APICALL GLuint GL_APIENTRY glCreateProgramContextANGLE(GLeglContext ctx);
GL_APICALL GLuint GL_APIENTRY glCreateShaderContextANGLE(GLeglContext ctx, GLenum type);
GL_APICALL GLuint GL_APIENTRY glCreateShaderProgramvContextANGLE(GLeglContext ctx, GLenum type, GLsizei count, const GLchar *const*strings);
GL_APICALL void GL_APIENTRY glCullFaceContextANGLE(GLeglContext ctx, GLenum mode);
GL_APICALL void GL_APIENTRY glCurrentPaletteMatrixOESContextANGLE(GLeglContext ctx, GLuint matrixpaletteindex);
GL_APICALL void GL_APIENTRY glDebugMessageCallbackKHRContextANGLE(GLeglContext ctx, GLDEBUGPROCKHR callback, const void *userParam);
GL_APICALL void GL_APIENTRY glDebugMessageControlKHRContextANGLE(GLeglContext ctx, GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
GL_APICALL void GL_APIENTRY glDebugMessageInsertKHRContextANGLE(GLeglContext ctx, GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
GL_APICALL void GL_APIENTRY glDeleteBuffersContextANGLE(GLeglContext ctx, GLsizei n, const GLuint *buffers);
GL_APICALL void GL_APIENTRY glDeleteFencesNVContextANGLE(GLeglContext ctx, GLsizei n, const GLuint *fences);
GL_APICALL void GL_APIENTRY glDeleteFramebuffersContextANGLE(GLeglContext ctx, GLsizei n, const GLuint *framebuffers);
GL_APICALL void GL_APIENTRY glDeleteFramebuffersOESContextANGLE(GLeglContext ctx, GLsizei n, const GLuint *framebuffers);
GL_APICALL void GL_APIENTRY glDeleteProgramContextANGLE(GLeglContext ctx, GLuint program);
GL_APICALL void GL_APIENTRY glDeleteProgramPipelinesContextANGLE(GLeglContext ctx, GLsizei n, const GLuint *pipelines);
GL_APICALL void GL_APIENTRY glDeleteQueriesContextANGLE(GLeglContext ctx, GLsizei n, const GLuint *ids);
GL_APICALL void GL_APIENTRY glDeleteQueriesEXTContextANGLE(GLeglContext ctx, GLsizei n, const GLuint *ids);
GL_APICALL void GL_APIENTRY glDeleteRenderbuffersContextANGLE(GLeglContext ctx, GLsizei n, const GLuint *renderbuffers);
GL_APICALL void GL_APIENTRY glDeleteRenderbuffersOESContextANGLE(GLeglContext ctx, GLsizei n, const GLuint *renderbuffers);
GL_APICALL void GL_APIENTRY glDeleteSamplersContextANGLE(GLeglContext ctx, GLsizei count, const GLuint *samplers);
GL_APICALL void GL_APIENTRY glDeleteShaderContextANGLE(GLeglContext ctx, GLuint shader);
GL_APICALL void GL_APIENTRY glDeleteSyncContextANGLE(GLeglContext ctx, GLsync sync);
GL_APICALL void GL_APIENTRY glDeleteTexturesContextANGLE(GLeglContext ctx, GLsizei n, const GLuint *textures);
GL_APICALL void GL_APIENTRY glDeleteTransformFeedbacksContextANGLE(GLeglContext ctx, GLsizei n, const GLuint *ids);
GL_APICALL void GL_APIENTRY glDeleteVertexArraysContextANGLE(GLeglContext ctx, GLsizei n, const GLuint *arrays);
GL_APICALL void GL_APIENTRY glDeleteVertexArraysOESContextANGLE(GLeglContext ctx, GLsizei n, const GLuint *arrays);
GL_APICALL void GL_APIENTRY glDepthFuncContextANGLE(GLeglContext ctx, GLenum func);
GL_APICALL void GL_APIENTRY glDepthMaskContextANGLE(GLeglContext ctx, GLboolean flag);
GL_APICALL void GL_APIENTRY glDepthRangefContextANGLE(GLeglContext ctx, GLfloat n, GLfloat f);
GL_APICALL void GL_APIENTRY glDepthRangexContextANGLE(GLeglContext ctx, GLfixed n, GLfixed f);
GL_APICALL void GL_APIENTRY glDetachShaderContextANGLE(GLeglContext ctx, GLuint program, GLuint shader);
GL_APICALL void GL_APIENTRY glDisableContextANGLE(GLeglContext ctx, GLenum cap);
GL_APICALL void GL_APIENTRY glDisableClientStateContextANGLE(GLeglContext ctx, GLenum array);
GL_APICALL void GL_APIENTRY glDisableVertexAttribArrayContextANGLE(GLeglContext ctx, GLuint index);
GL_APICALL void GL_APIENTRY glDiscardFramebufferEXTContextANGLE(GLeglContext ctx, GLenum target, GLsizei numAttachments, const GLenum *attachments);
GL_APICALL void GL_APIENTRY glDispatchComputeContextANGLE(GLeglContext ctx, GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
GL_APICALL void GL_APIENTRY glDispatchComputeIndirectContextANGLE(GLeglContext ctx, GLintptr indirect);
GL_APICALL void GL_APIENTRY glDrawArraysContextANGLE(GLeglContext ctx, GLenum mode, GLint first, GLsizei count);
GL_APICALL void GL_APIENTRY glDrawArraysIndirectContextANGLE(GLeglContext ctx, GLenum mode, const void *indirect);
GL_APICALL void GL_APIENTRY glDrawArraysInstancedContextANGLE(GLeglContext ctx, GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
GL_APICALL void GL_APIENTRY glDrawArraysInstancedANGLEContextANGLE(GLeglContext ctx, GLenum mode, GLint first, GLsizei count, GLsizei primcount);
GL_APICALL void GL_APIENTRY glDrawBuffersContextANGLE(GLeglContext ctx, GLsizei n, const GLenum *bufs);
GL_APICALL void GL_APIENTRY glDrawBuffersEXTContextANGLE(GLeglContext ctx, GLsizei n, const GLenum *bufs);
GL_APICALL void GL_APIENTRY glDrawElementsContextANGLE(GLeglContext ctx, GLenum mode, GLsizei count, GLenum type, const void *indices);
GL_APICALL void GL_APIENTRY glDrawElementsIndirectContextANGLE(GLeglContext ctx, GLenum mode, GLenum type, const void *indirect);
GL_APICALL void GL_APIENTRY glDrawElementsInstancedContextANGLE(GLeglContext ctx, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
GL_APICALL void GL_APIENTRY glDrawElementsInstancedANGLEContextANGLE(GLeglContext ctx, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
GL_APICALL void GL_APIENTRY glDrawRangeElementsContextANGLE(GLeglContext ctx, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
GL_APICALL void GL_APIENTRY glDrawTexfOESContextANGLE(GLeglContext ctx, GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height);
GL_APICALL void GL_APIENTRY glDrawTexfvOESContextANGLE(GLeglContext ctx, const GLfloat *coords);
GL_APICALL void GL_APIENTRY glDrawTexiOESContextANGLE(GLeglContext ctx, GLint x, GLint y, GLint z, GLint width, GLint height);
GL_APICALL void GL_APIENTRY glDrawTexivOESContextANGLE(GLeglContext ctx, const GLint *coords);
GL_APICALL void GL_APIENTRY glDrawTexsOESContextANGLE(GLeglContext ctx, GLshort x, GLshort y, GLshort z, GLshort width, GLshort height);
GL_APICALL void GL_APIENTRY glDrawTexsvOESContextANGLE(GLeglContext ctx, const GLshort *coords);
GL_APICALL void GL_APIENTRY glDrawTexxOESContextANGLE(GLeglContext ctx, GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height);
GL_APICALL void GL_APIENTRY glDrawTexxvOESContextANGLE(GLeglContext ctx, const GLfixed *coords);
GL_APICALL void GL_APIENTRY glEGLImageTargetRenderbufferStorageOESContextANGLE(GLeglContext ctx, GLenum target, GLeglImageOES image);
GL_APICALL void GL_APIENTRY glEGLImageTargetTexture2DOESContextANGLE(GLeglContext ctx, GLenum target, GLeglImageOES image);
GL_APICALL void GL_APIENTRY glEnableContextANGLE(GLeglContext ctx, GLenum cap);
GL_APICALL void GL_APIENTRY glEnableClientStateContextANGLE(GLeglContext ctx, GLenum array);
GL_APICALL void GL_APIENTRY glEnableVertexAttribArrayContextANGLE(GLeglContext ctx, GLuint index);
GL_APICALL void GL_APIENTRY glEndQueryContextANGLE(GLeglContext ctx, GLenum target);
GL_APICALL void GL_APIENTRY glEndQueryEXTContextANGLE(GLeglContext ctx, GLenum target);
GL_APICALL void GL_APIENTRY glEndTransformFeedbackContextANGLE(GLeglContext ctx);
GL_APICALL GLsync GL_APIENTRY glFenceSyncContextANGLE(GLeglContext ctx, GLenum condition, GLbitfield flags);
GL_APICALL void GL_APIENTRY glFinishContextANGLE(GLeglContext ctx);
GL_APICALL void GL_APIENTRY glFinishFenceNVContextANGLE(GLeglContext ctx, GLuint fence);
GL_APICALL void GL_APIENTRY glFlushContextANGLE(GLeglContext ctx);
GL_APICALL void GL_APIENTRY glFlushMappedBufferRangeContextANGLE(GLeglContext ctx, GLenum target, GLintptr offset, GLsizeiptr length);
GL_APICALL void GL_APIENTRY glFlushMappedBufferRangeEXTContextANGLE(GLeglContext ctx, GLenum target, GLintptr offset, GLsizeiptr length);
GL_APICALL void GL_APIENTRY glFogfContextANGLE(GLeglContext ctx, GLenum pname, GLfloat param);
GL_APICALL void GL_APIENTRY glFogfvContextANGLE(GLeglContext ctx, GLenum pname, const GLfloat *params);
GL_APICALL void GL_APIENTRY glFogxContextANGLE(GLeglContext ctx, GLenum pname, GLfixed param);
GL_APICALL void GL_APIENTRY glFogxvContextANGLE(GLeglContext ctx, GLenum pname, const GLfixed *param);
GL_APICALL void GL_APIENTRY glFramebufferParameteriContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint param);
GL_APICALL void GL_APIENTRY glFramebufferRenderbufferContextANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
GL_APICALL void GL_APIENTRY glFramebufferRenderbufferOESContextANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
GL_APICALL void GL_APIENTRY glFramebufferTexture2DContextANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
GL_APICALL void GL_APIENTRY glFramebufferTexture2DOESContextANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
GL_APICALL void GL_APIENTRY glFramebufferTextureLayerContextANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
GL_APICALL void GL_APIENTRY glFrontFaceContextANGLE(GLeglContext ctx, GLenum mode);
GL_APICALL void GL_APIENTRY glFrustumfContextANGLE(GLeglContext ctx, GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f);
GL_APICALL void GL_APIENTRY glFrustumxContextANGLE(GLeglContext ctx, GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f);
GL_APICALL void GL_APIENTRY glGenBuffersContextANGLE(GLeglContext ctx, GLsizei n, GLuint *buffers);
GL_APICALL void GL_APIENTRY glGenFencesNVContextANGLE(GLeglContext ctx, GLsizei n, GLuint *fences);
GL_APICALL void GL_APIENTRY glGenFramebuffersContextANGLE(GLeglContext ctx, GLsizei n, GLuint *framebuffers);
GL_APICALL void GL_APIENTRY glGenFramebuffersOESContextANGLE(GLeglContext ctx, GLsizei n, GLuint *framebuffers);
GL_APICALL void GL_APIENTRY glGenProgramPipelinesContextANGLE(GLeglContext ctx, GLsizei n, GLuint *pipelines);
GL_APICALL void GL_APIENTRY glGenQueriesContextANGLE(GLeglContext ctx, GLsizei n, GLuint *ids);
GL_APICALL void GL_APIENTRY glGenQueriesEXTContextANGLE(GLeglContext ctx, GLsizei n, GLuint *ids);
GL_APICALL void GL_APIENTRY glGenRenderbuffersContextANGLE(GLeglContext ctx, GLsizei n, GLuint *renderbuffers);
GL_APICALL void GL_APIENTRY glGenRenderbuffersOESContextANGLE(GLeglContext ctx, GLsizei n, GLuint *renderbuffers);
GL_APICALL void GL_APIENTRY glGenSamplersContextANGLE(GLeglContext ctx, GLsizei count, GLuint *samplers);
GL_APICALL void GL_APIENTRY glGenTexturesContextANGLE(GLeglContext ctx, GLsizei n, GLuint *textures);
GL_APICALL void GL_APIENTRY glGenTransformFeedbacksContextANGLE(GLeglContext ctx, GLsizei n, GLuint *ids);
GL_APICALL void GL_APIENTRY glGenVertexArraysContextANGLE(GLeglContext ctx, GLsizei n, GLuint *arrays);
GL_APICALL void GL_APIENTRY glGenVertexArraysOESContextANGLE(GLeglContext ctx, GLsizei n, GLuint *arrays);
GL_APICALL void GL_APIENTRY glGenerateMipmapContextANGLE(GLeglContext ctx, GLenum target);
GL_APICALL void GL_APIENTRY glGenerateMipmapOESContextANGLE(GLeglContext ctx, GLenum target);
GL_APICALL void GL_APIENTRY glGetActiveAttribContextANGLE(GLeglContext ctx, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
GL_APICALL void GL_APIENTRY glGetActiveUniformContextANGLE(GLeglContext ctx, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
GL_APICALL void GL_APIENTRY glGetActiveUniformBlockNameContextANGLE(GLeglContext ctx, GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
GL_APICALL void GL_APIENTRY glGetActiveUniformBlockivContextANGLE(GLeglContext ctx, GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetActiveUniformsivContextANGLE(GLeglContext ctx, GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetAttachedShadersContextANGLE(GLeglContext ctx, GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
GL_APICALL GLint GL_APIENTRY glGetAttribLocationContextANGLE(GLeglContext ctx, GLuint program, const GLchar *name);
GL_APICALL void GL_APIENTRY glGetBooleani_vContextANGLE(GLeglContext ctx, GLenum target, GLuint index, GLboolean *data);
GL_APICALL void GL_APIENTRY glGetBooleanvContextANGLE(GLeglContext ctx, GLenum pname, GLboolean *data);
GL_APICALL void GL_APIENTRY glGetBufferParameteri64vContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint64 *params);
GL_APICALL void GL_APIENTRY glGetBufferParameterivContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetBufferPointervContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, void **params);
GL_APICALL void GL_APIENTRY glGetBufferPointervOESContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, void **params);
GL_APICALL void GL_APIENTRY glGetClipPlanefContextANGLE(GLeglContext ctx, GLenum plane, GLfloat *equation);
GL_APICALL void GL_APIENTRY glGetClipPlanexContextANGLE(GLeglContext ctx, GLenum plane, GLfixed *equation);
GL_APICALL GLuint GL_APIENTRY glGetDebugMessageLogKHRContextANGLE(GLeglContext ctx, GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
GL_APICALL GLenum GL_APIENTRY glGetErrorContextANGLE(GLeglContext ctx);
GL_APICALL void GL_APIENTRY glGetFenceivNVContextANGLE(GLeglContext ctx, GLuint fence, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetFixedvContextANGLE(GLeglContext ctx, GLenum pname, GLfixed *params);
GL_APICALL void GL_APIENTRY glGetFloatvContextANGLE(GLeglContext ctx, GLenum pname, GLfloat *data);
GL_APICALL GLint GL_APIENTRY glGetFragDataLocationContextANGLE(GLeglContext ctx, GLuint program, const GLchar *name);
GL_APICALL void GL_APIENTRY glGetFramebufferAttachmentParameterivContextANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetFramebufferAttachmentParameterivOESContextANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetFramebufferParameterivContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
GL_APICALL GLenum GL_APIENTRY glGetGraphicsResetStatusEXTContextANGLE(GLeglContext ctx);
GL_APICALL void GL_APIENTRY glGetInteger64i_vContextANGLE(GLeglContext ctx, GLenum target, GLuint index, GLint64 *data);
GL_APICALL void GL_APIENTRY glGetInteger64vContextANGLE(GLeglContext ctx, GLenum pname, GLint64 *data);
GL_APICALL void GL_APIENTRY glGetIntegeri_vContextANGLE(GLeglContext ctx, GLenum target, GLuint index, GLint *data);
GL_APICALL void GL_APIENTRY glGetIntegervContextANGLE(GLeglContext ctx, GLenum pname, GLint *data);
GL_APICALL void GL_APIENTRY glGetInternalformativContextANGLE(GLeglContext ctx, GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params);
GL_APICALL void GL_APIENTRY glGetLightfvContextANGLE(GLeglContext ctx, GLenum light, GLenum pname, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetLightxvContextANGLE(GLeglContext ctx, GLenum light, GLenum pname, GLfixed *params);
GL_APICALL void GL_APIENTRY glGetMaterialfvContextANGLE(GLeglContext ctx, GLenum face, GLenum pname, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetMaterialxvContextANGLE(GLeglContext ctx, GLenum face, GLenum pname, GLfixed *params);
GL_APICALL void GL_APIENTRY glGetMultisamplefvContextANGLE(GLeglContext ctx, GLenum pname, GLuint index, GLfloat *val);
GL_APICALL void GL_APIENTRY glGetObjectLabelKHRContextANGLE(GLeglContext ctx, GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label);
GL_APICALL void GL_APIENTRY glGetObjectPtrLabelKHRContextANGLE(GLeglContext ctx, const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label);
GL_APICALL void GL_APIENTRY glGetPointervContextANGLE(GLeglContext ctx, GLenum pname, void **params);
GL_APICALL void GL_APIENTRY glGetPointervKHRContextANGLE(GLeglContext ctx, GLenum pname, void **params);
GL_APICALL void GL_APIENTRY glGetProgramBinaryContextANGLE(GLeglContext ctx, GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
GL_APICALL void GL_APIENTRY glGetProgramBinaryOESContextANGLE(GLeglContext ctx, GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
GL_APICALL void GL_APIENTRY glGetProgramInfoLogContextANGLE(GLeglContext ctx, GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GL_APICALL void GL_APIENTRY glGetProgramInterfaceivContextANGLE(GLeglContext ctx, GLuint program, GLenum programInterface, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetProgramPipelineInfoLogContextANGLE(GLeglContext ctx, GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GL_APICALL void GL_APIENTRY glGetProgramPipelineivContextANGLE(GLeglContext ctx, GLuint pipeline, GLenum pname, GLint *params);
GL_APICALL GLuint GL_APIENTRY glGetProgramResourceIndexContextANGLE(GLeglContext ctx, GLuint program, GLenum programInterface, const GLchar *name);
GL_APICALL GLint GL_APIENTRY glGetProgramResourceLocationContextANGLE(GLeglContext ctx, GLuint program, GLenum programInterface, const GLchar *name);
GL_APICALL void GL_APIENTRY glGetProgramResourceNameContextANGLE(GLeglContext ctx, GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
GL_APICALL void GL_APIENTRY glGetProgramResourceivContextANGLE(GLeglContext ctx, GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL void GL_APIENTRY glGetProgramivContextANGLE(GLeglContext ctx, GLuint program, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetQueryObjecti64vEXTContextANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLint64 *params);
GL_APICALL void GL_APIENTRY glGetQueryObjectivEXTContextANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetQueryObjectui64vEXTContextANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLuint64 *params);
GL_APICALL void GL_APIENTRY glGetQueryObjectuivContextANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLuint *params);
GL_APICALL void GL_APIENTRY glGetQueryObjectuivEXTContextANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLuint *params);
GL_APICALL void GL_APIENTRY glGetQueryivContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetQueryivEXTContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetRenderbufferParameterivContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetRenderbufferParameterivOESContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetSamplerParameterfvContextANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetSamplerParameterivContextANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetShaderInfoLogContextANGLE(GLeglContext ctx, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GL_APICALL void GL_APIENTRY glGetShaderPrecisionFormatContextANGLE(GLeglContext ctx, GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
GL_APICALL void GL_APIENTRY glGetShaderSourceContextANGLE(GLeglContext ctx, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
GL_APICALL void GL_APIENTRY glGetShaderivContextANGLE(GLeglContext ctx, GLuint shader, GLenum pname, GLint *params);
GL_APICALL const GLubyte *GL_APIENTRY glGetStringContextANGLE(GLeglContext ctx, GLenum name);
GL_APICALL const GLubyte *GL_APIENTRY glGetStringiContextANGLE(GLeglContext ctx, GLenum name, GLuint index);
GL_APICALL void GL_APIENTRY glGetSyncivContextANGLE(GLeglContext ctx, GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);
GL_APICALL void GL_APIENTRY glGetTexEnvfvContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetTexEnvivContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetTexEnvxvContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfixed *params);
GL_APICALL void GL_APIENTRY glGetTexGenfvOESContextANGLE(GLeglContext ctx, GLenum coord, GLenum pname, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetTexGenivOESContextANGLE(GLeglContext ctx, GLenum coord, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetTexGenxvOESContextANGLE(GLeglContext ctx, GLenum coord, GLenum pname, GLfixed *params);
GL_APICALL void GL_APIENTRY glGetTexLevelParameterfvContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum pname, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetTexLevelParameterivContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetTexParameterfvContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetTexParameterivContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetTexParameterxvContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfixed *params);
GL_APICALL void GL_APIENTRY glGetTransformFeedbackVaryingContextANGLE(GLeglContext ctx, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
GL_APICALL void GL_APIENTRY glGetTranslatedShaderSourceANGLEContextANGLE(GLeglContext ctx, GLuint shader, GLsizei bufsize, GLsizei *length, GLchar *source);
GL_APICALL GLuint GL_APIENTRY glGetUniformBlockIndexContextANGLE(GLeglContext ctx, GLuint program, const GLchar *uniformBlockName);
GL_APICALL void GL_APIENTRY glGetUniformIndicesContextANGLE(GLeglContext ctx, GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices);
GL_APICALL GLint GL_APIENTRY glGetUniformLocationContextANGLE(GLeglContext ctx, GLuint program, const GLchar *name);
GL_APICALL void GL_APIENTRY glGetUniformfvContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetUniformivContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLint *params);
GL_APICALL void GL_APIENTRY glGetUniformuivContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLuint *params);
GL_APICALL void GL_APIENTRY glGetVertexAttribIivContextANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetVertexAttribIuivContextANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLuint *params);
GL_APICALL void GL_APIENTRY glGetVertexAttribPointervContextANGLE(GLeglContext ctx, GLuint index, GLenum pname, void **pointer);
GL_APICALL void GL_APIENTRY glGetVertexAttribfvContextANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetVertexAttribivContextANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetnUniformfvEXTContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetnUniformivEXTContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLint *params);
GL_APICALL void GL_APIENTRY glHintContextANGLE(GLeglContext ctx, GLenum target, GLenum mode);
GL_APICALL void GL_APIENTRY glInsertEventMarkerEXTContextANGLE(GLeglContext ctx, GLsizei length, const GLchar *marker);
GL_APICALL void GL_APIENTRY glInvalidateFramebufferContextANGLE(GLeglContext ctx, GLenum target, GLsizei numAttachments, const GLenum *attachments);
GL_APICALL void GL_APIENTRY glInvalidateSubFramebufferContextANGLE(GLeglContext ctx, GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
GL_APICALL GLboolean GL_APIENTRY glIsBufferContextANGLE(GLeglContext ctx, GLuint buffer);
GL_APICALL GLboolean GL_APIENTRY glIsEnabledContextANGLE(GLeglContext ctx, GLenum cap);
GL_APICALL GLboolean GL_APIENTRY glIsFenceNVContextANGLE(GLeglContext ctx, GLuint fence);
GL_APICALL GLboolean GL_APIENTRY glIsFramebufferContextANGLE(GLeglContext ctx, GLuint framebuffer);
GL_APICALL GLboolean GL_APIENTRY glIsFramebufferOESContextANGLE(GLeglContext ctx, GLuint framebuffer);
GL_APICALL GLboolean GL_APIENTRY glIsProgramContextANGLE(GLeglContext ctx, GLuint program);
GL_APICALL GLboolean GL_APIENTRY glIsProgramPipelineContextANGLE(GLeglContext ctx, GLuint pipeline);
GL_APICALL GLboolean GL_APIENTRY glIsQueryContextANGLE(GLeglContext ctx, GLuint id);
GL_APICALL GLboolean GL_APIENTRY glIsQueryEXTContextANGLE(GLeglContext ctx, GLuint id);
GL_APICALL GLboolean GL_APIENTRY glIsRenderbufferContextANGLE(GLeglContext ctx, GLuint renderbuffer);
GL_APICALL GLboolean GL_APIENTRY glIsRenderbufferOESContextANGLE(GLeglContext ctx, GLuint renderbuffer);
GL_APICALL GLboolean GL_APIENTRY glIsSamplerContextANGLE(GLeglContext ctx, GLuint sampler);
GL_APICALL GLboolean GL_APIENTRY glIsShaderContextANGLE(GLeglContext ctx, GLuint shader);
GL_APICALL GLboolean GL_APIENTRY glIsSyncContextANGLE(GLeglContext ctx, GLsync sync);
GL_APICALL GLboolean GL_APIENTRY glIsTextureContextANGLE(GLeglContext ctx, GLuint texture);
GL_APICALL GLboolean GL_APIENTRY glIsTransformFeedbackContextANGLE(GLeglContext ctx, GLuint id);
GL_APICALL GLboolean GL_APIENTRY glIsVertexArrayContextANGLE(GLeglContext ctx, GLuint array);
GL_APICALL GLboolean GL_APIENTRY glIsVertexArrayOESContextANGLE(GLeglContext ctx, GLuint array);
GL_APICALL void GL_APIENTRY glLightModelfContextANGLE(GLeglContext ctx, GLenum pname, GLfloat param);
GL_APICALL void GL_APIENTRY glLightModelfvContextANGLE(GLeglContext ctx, GLenum pname, const GLfloat *params);
GL_APICALL void GL_APIENTRY glLightModelxContextANGLE(GLeglContext ctx, GLenum pname, GLfixed param);
GL_APICALL void GL_APIENTRY glLightModelxvContextANGLE(GLeglContext ctx, GLenum pname, const GLfixed *param);
GL_APICALL void GL_APIENTRY glLightfContextANGLE(GLeglContext ctx, GLenum light, GLenum pname, GLfloat param);
GL_APICALL void GL_APIENTRY glLightfvContextANGLE(GLeglContext ctx, GLenum light, GLenum pname, const GLfloat *params);
GL_APICALL void GL_APIENTRY glLightxContextANGLE(GLeglContext ctx, GLenum light, GLenum pname, GLfixed param);
GL_APICALL void GL_APIENTRY glLightxvContextANGLE(GLeglContext ctx, GLenum light, GLenum pname, const GLfixed *params);
GL_APICALL void GL_APIENTRY glLineWidthContextANGLE(GLeglContext ctx, GLfloat width);
GL_APICALL void GL_APIENTRY glLineWidthxContextANGLE(GLeglContext ctx, GLfixed width);
GL_APICALL void GL_APIENTRY glLinkProgramContextANGLE(GLeglContext ctx, GLuint program);
GL_APICALL void GL_APIENTRY glLoadIdentityContextANGLE(GLeglContext ctx);
GL_APICALL void GL_APIENTRY glLoadMatrixfContextANGLE(GLeglContext ctx, const GLfloat *m);
GL_APICALL void GL_APIENTRY glLoadMatrixxContextANGLE(GLeglContext ctx, const GLfixed *m);
GL_APICALL void GL_APIENTRY glLoadPaletteFromModelViewMatrixOESContextANGLE(GLeglContext ctx);
GL_APICALL void GL_APIENTRY glLogicOpContextANGLE(GLeglContext ctx, GLenum opcode);
GL_APICALL void *GL_APIENTRY glMapBufferOESContextANGLE(GLeglContext ctx, GLenum target, GLenum access);
GL_APICALL void *GL_APIENTRY glMapBufferRangeContextANGLE(GLeglContext ctx, GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
GL_APICALL void *GL_APIENTRY glMapBufferRangeEXTContextANGLE(GLeglContext ctx, GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
GL_APICALL void GL_APIENTRY glMaterialfContextANGLE(GLeglContext ctx, GLenum face, GLenum pname, GLfloat param);
GL_APICALL void GL_APIENTRY glMaterialfvContextANGLE(GLeglContext ctx, GLenum face, GLenum pname, const GLfloat *params);
GL_APICALL void GL_APIENTRY glMaterialxContextANGLE(GLeglContext ctx, GLenum face, GLenum pname, GLfixed param);
GL_APICALL void GL_APIENTRY glMaterialxvContextANGLE(GLeglContext ctx, GLenum face, GLenum pname, const GLfixed *param);
GL_APICALL void GL_APIENTRY glMatrixIndexPointerOESContextANGLE(GLeglContext ctx, GLint size, GLenum type, GLsizei stride, const void *pointer);
GL_APICALL void GL_APIENTRY glMatrixModeContextANGLE(GLeglContext ctx, GLenum mode);
GL_APICALL void GL_APIENTRY glMemoryBarrierContextANGLE(GLeglContext ctx, GLbitfield barriers);
GL_APICALL void GL_APIENTRY glMemoryBarrierByRegionContextANGLE(GLeglContext ctx, GLbitfield barriers);
GL_APICALL void GL_APIENTRY glMultMatrixfContextANGLE(GLeglContext ctx, const GLfloat *m);
GL_APICALL void GL_APIENTRY glMultMatrixxContextANGLE(GLeglContext ctx, const GLfixed *m);
GL_APICALL void GL_APIENTRY glMultiTexCoord4fContextANGLE(GLeglContext ctx, GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
GL_APICALL void GL_APIENTRY glMultiTexCoord4xContextANGLE(GLeglContext ctx, GLenum texture, GLfixed s, GLfixed t, GLfixed r, GLfixed q);
GL_APICALL void GL_APIENTRY glNormal3fContextANGLE(GLeglContext ctx, GLfloat nx, GLfloat ny, GLfloat nz);
GL_APICALL void GL_APIENTRY glNormal3xContextANGLE(GLeglContext ctx, GLfixed nx, GLfixed ny, GLfixed nz);
GL_APICALL void GL_APIENTRY glNormalPointerContextANGLE(GLeglContext ctx, GLenum type, GLsizei stride, const void *pointer);
GL_APICALL void GL_APIENTRY glObjectLabelKHRContextANGLE(GLeglContext ctx, GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
GL_APICALL void GL_APIENTRY glObjectPtrLabelKHRContextANGLE(GLeglContext ctx, const void *ptr, GLsizei length, const GLchar *label);
GL_APICALL void GL_APIENTRY glOrthofContextANGLE(GLeglContext ctx, GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f);
GL_APICALL void GL_APIENTRY glOrthoxContextANGLE(GLeglContext ctx, GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f);
GL_APICALL void GL_APIENTRY glPauseTransformFeedbackContextANGLE(GLeglContext ctx);
GL_APICALL void GL_APIENTRY glPixelStoreiContextANGLE(GLeglContext ctx, GLenum pname, GLint param);
GL_APICALL void GL_APIENTRY glPointParameterfContextANGLE(GLeglContext ctx, GLenum pname, GLfloat param);
GL_APICALL void GL_APIENTRY glPointParameterfvContextANGLE(GLeglContext ctx, GLenum pname, const GLfloat *params);
GL_APICALL void GL_APIENTRY glPointParameterxContextANGLE(GLeglContext ctx, GLenum pname, GLfixed param);
GL_APICALL void GL_APIENTRY glPointParameterxvContextANGLE(GLeglContext ctx, GLenum pname, const GLfixed *params);
GL_APICALL void GL_APIENTRY glPointSizeContextANGLE(GLeglContext ctx, GLfloat size);
GL_APICALL void GL_APIENTRY glPointSizePointerOESContextANGLE(GLeglContext ctx, GLenum type, GLsizei stride, const void *pointer);
GL_APICALL void GL_APIENTRY glPointSizexContextANGLE(GLeglContext ctx, GLfixed size);
GL_APICALL void GL_APIENTRY glPolygonOffsetContextANGLE(GLeglContext ctx, GLfloat factor, GLfloat units);
GL_APICALL void GL_APIENTRY glPolygonOffsetxContextANGLE(GLeglContext ctx, GLfixed factor, GLfixed units);
GL_APICALL void GL_APIENTRY glPopDebugGroupKHRContextANGLE(GLeglContext ctx);
GL_APICALL void GL_APIENTRY glPopGroupMarkerEXTContextANGLE(GLeglContext ctx);
GL_APICALL void GL_APIENTRY glPopMatrixContextANGLE(GLeglContext ctx);
GL_APICALL void GL_APIENTRY glProgramBinaryContextANGLE(GLeglContext ctx, GLuint program, GLenum binaryFormat, const void *binary, GLsizei length);
GL_APICALL void GL_APIENTRY glProgramBinaryOESContextANGLE(GLeglContext ctx, GLuint program, GLenum binaryFormat, const void *binary, GLint length);
GL_APICALL void GL_APIENTRY glProgramParameteriContextANGLE(GLeglContext ctx, GLuint program, GLenum pname, GLint value);
GL_APICALL void GL_APIENTRY glProgramUniform1fContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLfloat v0);
GL_APICALL void GL_APIENTRY glProgramUniform1fvContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniform1iContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLint v0);
GL_APICALL void GL_APIENTRY glProgramUniform1ivContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLint *value);
GL_APICALL void GL_APIENTRY glProgramUniform1uiContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLuint v0);
GL_APICALL void GL_APIENTRY glProgramUniform1uivContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLuint *value);
GL_APICALL void GL_APIENTRY glProgramUniform2fContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLfloat v0, GLfloat v1);
GL_APICALL void GL_APIENTRY glProgramUniform2fvContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniform2iContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLint v0, GLint v1);
GL_APICALL void GL_APIENTRY glProgramUniform2ivContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLint *value);
GL_APICALL void GL_APIENTRY glProgramUniform2uiContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLuint v0, GLuint v1);
GL_APICALL void GL_APIENTRY glProgramUniform2uivContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLuint *value);
GL_APICALL void GL_APIENTRY glProgramUniform3fContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
GL_APICALL void GL_APIENTRY glProgramUniform3fvContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniform3iContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
GL_APICALL void GL_APIENTRY glProgramUniform3ivContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLint *value);
GL_APICALL void GL_APIENTRY glProgramUniform3uiContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
GL_APICALL void GL_APIENTRY glProgramUniform3uivContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLuint *value);
GL_APICALL void GL_APIENTRY glProgramUniform4fContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
GL_APICALL void GL_APIENTRY glProgramUniform4fvContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniform4iContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
GL_APICALL void GL_APIENTRY glProgramUniform4ivContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLint *value);
GL_APICALL void GL_APIENTRY glProgramUniform4uiContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
GL_APICALL void GL_APIENTRY glProgramUniform4uivContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLuint *value);
GL_APICALL void GL_APIENTRY glProgramUniformMatrix2fvContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniformMatrix2x3fvContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniformMatrix2x4fvContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniformMatrix3fvContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniformMatrix3x2fvContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniformMatrix3x4fvContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniformMatrix4fvContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniformMatrix4x2fvContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glProgramUniformMatrix4x3fvContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glPushDebugGroupKHRContextANGLE(GLeglContext ctx, GLenum source, GLuint id, GLsizei length, const GLchar *message);
GL_APICALL void GL_APIENTRY glPushGroupMarkerEXTContextANGLE(GLeglContext ctx, GLsizei length, const GLchar *marker);
GL_APICALL void GL_APIENTRY glPushMatrixContextANGLE(GLeglContext ctx);
GL_APICALL void GL_APIENTRY glQueryCounterEXTContextANGLE(GLeglContext ctx, GLuint id, GLenum target);
GL_APICALL GLbitfield GL_APIENTRY glQueryMatrixxOESContextANGLE(GLeglContext ctx, GLfixed *mantissa, GLint *exponent);
GL_APICALL void GL_APIENTRY glReadBufferContextANGLE(GLeglContext ctx, GLenum src);
GL_APICALL void GL_APIENTRY glReadPixelsContextANGLE(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
GL_APICALL void GL_APIENTRY glReadnPixelsEXTContextANGLE(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
GL_APICALL void GL_APIENTRY glReleaseShaderCompilerContextANGLE(GLeglContext ctx);
GL_APICALL void GL_APIENTRY glRenderbufferStorageContextANGLE(GLeglContext ctx, GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
GL_APICALL void GL_APIENTRY glRenderbufferStorageMultisampleContextANGLE(GLeglContext ctx, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
GL_APICALL void GL_APIENTRY glRenderbufferStorageMultisampleANGLEContextANGLE(GLeglContext ctx, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
GL_APICALL void GL_APIENTRY glRenderbufferStorageOESContextANGLE(GLeglContext ctx, GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
GL_APICALL void GL_APIENTRY glResumeTransformFeedbackContextANGLE(GLeglContext ctx);
GL_APICALL void GL_APIENTRY glRotatefContextANGLE(GLeglContext ctx, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
GL_APICALL void GL_APIENTRY glRotatexContextANGLE(GLeglContext ctx, GLfixed angle, GLfixed x, GLfixed y, GLfixed z);
GL_APICALL void GL_APIENTRY glSampleCoverageContextANGLE(GLeglContext ctx, GLfloat value, GLboolean invert);
GL_APICALL void GL_APIENTRY glSampleCoveragexContextANGLE(GLeglContext ctx, GLclampx value, GLboolean invert);
GL_APICALL void GL_APIENTRY glSampleMaskiContextANGLE(GLeglContext ctx, GLuint maskNumber, GLbitfield mask);
GL_APICALL void GL_APIENTRY glSamplerParameterfContextANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLfloat param);
GL_APICALL void GL_APIENTRY glSamplerParameterfvContextANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, const GLfloat *param);
GL_APICALL void GL_APIENTRY glSamplerParameteriContextANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLint param);
GL_APICALL void GL_APIENTRY glSamplerParameterivContextANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, const GLint *param);
GL_APICALL void GL_APIENTRY glScalefContextANGLE(GLeglContext ctx, GLfloat x, GLfloat y, GLfloat z);
GL_APICALL void GL_APIENTRY glScalexContextANGLE(GLeglContext ctx, GLfixed x, GLfixed y, GLfixed z);
GL_APICALL void GL_APIENTRY glScissorContextANGLE(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height);
GL_APICALL void GL_APIENTRY glSetFenceNVContextANGLE(GLeglContext ctx, GLuint fence, GLenum condition);
GL_APICALL void GL_APIENTRY glShadeModelContextANGLE(GLeglContext ctx, GLenum mode);
GL_APICALL void GL_APIENTRY glShaderBinaryContextANGLE(GLeglContext ctx, GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length);
GL_APICALL void GL_APIENTRY glShaderSourceContextANGLE(GLeglContext ctx, GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
GL_APICALL void GL_APIENTRY glStencilFuncContextANGLE(GLeglContext ctx, GLenum func, GLint ref, GLuint mask);
GL_APICALL void GL_APIENTRY glStencilFuncSeparateContextANGLE(GLeglContext ctx, GLenum face, GLenum func, GLint ref, GLuint mask);
GL_APICALL void GL_APIENTRY glStencilMaskContextANGLE(GLeglContext ctx, GLuint mask);
GL_APICALL void GL_APIENTRY glStencilMaskSeparateContextANGLE(GLeglContext ctx, GLenum face, GLuint mask);
GL_APICALL void GL_APIENTRY glStencilOpContextANGLE(GLeglContext ctx, GLenum fail, GLenum zfail, GLenum zpass);
GL_APICALL void GL_APIENTRY glStencilOpSeparateContextANGLE(GLeglContext ctx, GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
GL_APICALL GLboolean GL_APIENTRY glTestFenceNVContextANGLE(GLeglContext ctx, GLuint fence);
GL_APICALL void GL_APIENTRY glTexCoordPointerContextANGLE(GLeglContext ctx, GLint size, GLenum type, GLsizei stride, const void *pointer);
GL_APICALL void GL_APIENTRY glTexEnvfContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfloat param);
GL_APICALL void GL_APIENTRY glTexEnvfvContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, const GLfloat *params);
GL_APICALL void GL_APIENTRY glTexEnviContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint param);
GL_APICALL void GL_APIENTRY glTexEnvivContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, const GLint *params);
GL_APICALL void GL_APIENTRY glTexEnvxContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfixed param);
GL_APICALL void GL_APIENTRY glTexEnvxvContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, const GLfixed *params);
GL_APICALL void GL_APIENTRY glTexGenfOESContextANGLE(GLeglContext ctx, GLenum coord, GLenum pname, GLfloat param);
GL_APICALL void GL_APIENTRY glTexGenfvOESContextANGLE(GLeglContext ctx, GLenum coord, GLenum pname, const GLfloat *params);
GL_APICALL void GL_APIENTRY glTexGeniOESContextANGLE(GLeglContext ctx, GLenum coord, GLenum pname, GLint param);
GL_APICALL void GL_APIENTRY glTexGenivOESContextANGLE(GLeglContext ctx, GLenum coord, GLenum pname, const GLint *params);
GL_APICALL void GL_APIENTRY glTexGenxOESContextANGLE(GLeglContext ctx, GLenum coord, GLenum pname, GLfixed param);
GL_APICALL void GL_APIENTRY glTexGenxvOESContextANGLE(GLeglContext ctx, GLenum coord, GLenum pname, const GLfixed *params);
GL_APICALL void GL_APIENTRY glTexImage2DContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
GL_APICALL void GL_APIENTRY glTexImage3DContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
GL_APICALL void GL_APIENTRY glTexParameterfContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfloat param);
GL_APICALL void GL_APIENTRY glTexParameterfvContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, const GLfloat *params);
GL_APICALL void GL_APIENTRY glTexParameteriContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint param);
GL_APICALL void GL_APIENTRY glTexParameterivContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, const GLint *params);
GL_APICALL void GL_APIENTRY glTexParameterxContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfixed param);
GL_APICALL void GL_APIENTRY glTexParameterxvContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, const GLfixed *params);
GL_APICALL void GL_APIENTRY glTexStorage1DEXTContextANGLE(GLeglContext ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
GL_APICALL void GL_APIENTRY glTexStorage2DContextANGLE(GLeglContext ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
GL_APICALL void GL_APIENTRY glTexStorage2DEXTContextANGLE(GLeglContext ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
GL_APICALL void GL_APIENTRY glTexStorage2DMultisampleContextANGLE(GLeglContext ctx, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
GL_APICALL void GL_APIENTRY glTexStorage3DContextANGLE(GLeglContext ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
GL_APICALL void GL_APIENTRY glTexStorage3DEXTContextANGLE(GLeglContext ctx, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
GL_APICALL void GL_APIENTRY glTexSubImage2DContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
GL_APICALL void GL_APIENTRY glTexSubImage3DContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
GL_APICALL void GL_APIENTRY glTransformFeedbackVaryingsContextANGLE(GLeglContext ctx, GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode);
GL_APICALL void GL_APIENTRY glTranslatefContextANGLE(GLeglContext ctx, GLfloat x, GLfloat y, GLfloat z);
GL_APICALL void GL_APIENTRY glTranslatexContextANGLE(GLeglContext ctx, GLfixed x, GLfixed y, GLfixed z);
GL_APICALL void GL_APIENTRY glUniform1fContextANGLE(GLeglContext ctx, GLint location, GLfloat v0);
GL_APICALL void GL_APIENTRY glUniform1fvContextANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void GL_APIENTRY glUniform1iContextANGLE(GLeglContext ctx, GLint location, GLint v0);
GL_APICALL void GL_APIENTRY glUniform1ivContextANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLint *value);
GL_APICALL void GL_APIENTRY glUniform1uiContextANGLE(GLeglContext ctx, GLint location, GLuint v0);
GL_APICALL void GL_APIENTRY glUniform1uivContextANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLuint *value);
GL_APICALL void GL_APIENTRY glUniform2fContextANGLE(GLeglContext ctx, GLint location, GLfloat v0, GLfloat v1);
GL_APICALL void GL_APIENTRY glUniform2fvContextANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void GL_APIENTRY glUniform2iContextANGLE(GLeglContext ctx, GLint location, GLint v0, GLint v1);
GL_APICALL void GL_APIENTRY glUniform2ivContextANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLint *value);
GL_APICALL void GL_APIENTRY glUniform2uiContextANGLE(GLeglContext ctx, GLint location, GLuint v0, GLuint v1);
GL_APICALL void GL_APIENTRY glUniform2uivContextANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLuint *value);
GL_APICALL void GL_APIENTRY glUniform3fContextANGLE(GLeglContext ctx, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
GL_APICALL void GL_APIENTRY glUniform3fvContextANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void GL_APIENTRY glUniform3iContextANGLE(GLeglContext ctx, GLint location, GLint v0, GLint v1, GLint v2);
GL_APICALL void GL_APIENTRY glUniform3ivContextANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLint *value);
GL_APICALL void GL_APIENTRY glUniform3uiContextANGLE(GLeglContext ctx, GLint location, GLuint v0, GLuint v1, GLuint v2);
GL_APICALL void GL_APIENTRY glUniform3uivContextANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLuint *value);
GL_APICALL void GL_APIENTRY glUniform4fContextANGLE(GLeglContext ctx, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
GL_APICALL void GL_APIENTRY glUniform4fvContextANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void GL_APIENTRY glUniform4iContextANGLE(GLeglContext ctx, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
GL_APICALL void GL_APIENTRY glUniform4ivContextANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLint *value);
GL_APICALL void GL_APIENTRY glUniform4uiContextANGLE(GLeglContext ctx, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
GL_APICALL void GL_APIENTRY glUniform4uivContextANGLE(GLeglContext ctx, GLint location, GLsizei count, const GLuint *value);
GL_APICALL void GL_APIENTRY glUniformBlockBindingContextANGLE(GLeglContext ctx, GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
GL_APICALL void GL_APIENTRY glUniformMatrix2fvContextANGLE(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glUniformMatrix2x3fvContextANGLE(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glUniformMatrix2x4fvContextANGLE(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glUniformMatrix3fvContextANGLE(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glUniformMatrix3x2fvContextANGLE(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glUniformMatrix3x4fvContextANGLE(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glUniformMatrix4fvContextANGLE(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glUniformMatrix4x2fvContextANGLE(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY glUniformMatrix4x3fvContextANGLE(GLeglContext ctx, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL GLboolean GL_APIENTRY glUnmapBufferContextANGLE(GLeglContext ctx, GLenum target);
GL_APICALL GLboolean GL_APIENTRY glUnmapBufferOESContextANGLE(GLeglContext ctx, GLenum target);
GL_APICALL void GL_APIENTRY glUseProgramContextANGLE(GLeglContext ctx, GLuint program);
GL_APICALL void GL_APIENTRY glUseProgramStagesContextANGLE(GLeglContext ctx, GLuint pipeline, GLbitfield stages, GLuint program);
GL_APICALL void GL_APIENTRY glValidateProgramContextANGLE(GLeglContext ctx, GLuint program);
GL_APICALL void GL_APIENTRY glValidateProgramPipelineContextANGLE(GLeglContext ctx, GLuint pipeline);
GL_APICALL void GL_APIENTRY glVertexAttrib1fContextANGLE(GLeglContext ctx, GLuint index, GLfloat x);
GL_APICALL void GL_APIENTRY glVertexAttrib1fvContextANGLE(GLeglContext ctx, GLuint index, const GLfloat *v);
GL_APICALL void GL_APIENTRY glVertexAttrib2fContextANGLE(GLeglContext ctx, GLuint index, GLfloat x, GLfloat y);
GL_APICALL void GL_APIENTRY glVertexAttrib2fvContextANGLE(GLeglContext ctx, GLuint index, const GLfloat *v);
GL_APICALL void GL_APIENTRY glVertexAttrib3fContextANGLE(GLeglContext ctx, GLuint index, GLfloat x, GLfloat y, GLfloat z);
GL_APICALL void GL_APIENTRY glVertexAttrib3fvContextANGLE(GLeglContext ctx, GLuint index, const GLfloat *v);
GL_APICALL void GL_APIENTRY glVertexAttrib4fContextANGLE(GLeglContext ctx, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GL_APICALL void GL_APIENTRY glVertexAttrib4fvContextANGLE(GLeglContext ctx, GLuint index, const GLfloat *v);
GL_APICALL void GL_APIENTRY glVertexAttribBindingContextANGLE(GLeglContext ctx, GLuint attribindex, GLuint bindingindex);
GL_APICALL void GL_APIENTRY glVertexAttribDivisorContextANGLE(GLeglContext ctx, GLuint index, GLuint divisor);
GL_APICALL void GL_APIENTRY glVertexAttribDivisorANGLEContextANGLE(GLeglContext ctx, GLuint index, GLuint divisor);
GL_APICALL void GL_APIENTRY glVertexAttribFormatContextANGLE(GLeglContext ctx, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
GL_APICALL void GL_APIENTRY glVertexAttribI4iContextANGLE(GLeglContext ctx, GLuint index, GLint x, GLint y, GLint z, GLint w);
GL_APICALL void GL_APIENTRY glVertexAttribI4ivContextANGLE(GLeglContext ctx, GLuint index, const GLint *v);
GL_APICALL void GL_APIENTRY glVertexAttribI4uiContextANGLE(GLeglContext ctx, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
GL_APICALL void GL_APIENTRY glVertexAttribI4uivContextANGLE(GLeglContext ctx, GLuint index, const GLuint *v);
GL_APICALL void GL_APIENTRY glVertexAttribIFormatContextANGLE(GLeglContext ctx, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
GL_APICALL void GL_APIENTRY glVertexAttribIPointerContextANGLE(GLeglContext ctx, GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
GL_APICALL void GL_APIENTRY glVertexAttribPointerContextANGLE(GLeglContext ctx, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
GL_APICALL void GL_APIENTRY glVertexBindingDivisorContextANGLE(GLeglContext ctx, GLuint bindingindex, GLuint divisor);
GL_APICALL void GL_APIENTRY glVertexPointerContextANGLE(GLeglContext ctx, GLint size, GLenum type, GLsizei stride, const void *pointer);
GL_APICALL void GL_APIENTRY glViewportContextANGLE(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height);
GL_APICALL void GL_APIENTRY glWaitSyncContextANGLE(GLeglContext ctx, GLsync sync, GLbitfield flags, GLuint64 timeout);
GL_APICALL void GL_APIENTRY glWeightPointerOESContextANGLE(GLeglContext ctx, GLint size, GLenum type, GLsizei stride, const void *pointer);
GL_APICALL void GL_APIENTRY glBindUniformLocationCHROMIUMContextANGLE(GLeglContext ctx, GLuint program, GLint location, const GLchar* name);
GL_APICALL void GL_APIENTRY glCoverageModulationCHROMIUMContextANGLE(GLeglContext ctx, GLenum components);
GL_APICALL void GL_APIENTRY glMatrixLoadfCHROMIUMContextANGLE(GLeglContext ctx, GLenum matrixMode, const GLfloat * matrix);
GL_APICALL void GL_APIENTRY glMatrixLoadIdentityCHROMIUMContextANGLE(GLeglContext ctx, GLenum matrixMode);
GL_APICALL GLuint GL_APIENTRY glGenPathsCHROMIUMContextANGLE(GLeglContext ctx, GLsizei range);
GL_APICALL void GL_APIENTRY glDeletePathsCHROMIUMContextANGLE(GLeglContext ctx, GLuint first, GLsizei range);
GL_APICALL GLboolean GL_APIENTRY glIsPathCHROMIUMContextANGLE(GLeglContext ctx, GLuint path);
GL_APICALL void GL_APIENTRY glPathCommandsCHROMIUMContextANGLE(GLeglContext ctx, GLuint path, GLsizei numCommands, const GLubyte * commands, GLsizei numCoords, GLenum coordType, const void* coords);
GL_APICALL void GL_APIENTRY glPathParameterfCHROMIUMContextANGLE(GLeglContext ctx, GLuint path, GLenum pname, GLfloat value);
GL_APICALL void GL_APIENTRY glPathParameteriCHROMIUMContextANGLE(GLeglContext ctx, GLuint path, GLenum pname, GLint value);
GL_APICALL void GL_APIENTRY glGetPathParameterfvCHROMIUMContextANGLE(GLeglContext ctx, GLuint path, GLenum pname, GLfloat * value);
GL_APICALL void GL_APIENTRY glGetPathParameterivCHROMIUMContextANGLE(GLeglContext ctx, GLuint path, GLenum pname, GLint * value);
GL_APICALL void GL_APIENTRY glPathStencilFuncCHROMIUMContextANGLE(GLeglContext ctx, GLenum func, GLint ref, GLuint mask);
GL_APICALL void GL_APIENTRY glStencilFillPathCHROMIUMContextANGLE(GLeglContext ctx, GLuint path, GLenum fillMode, GLuint mask);
GL_APICALL void GL_APIENTRY glStencilStrokePathCHROMIUMContextANGLE(GLeglContext ctx, GLuint path, GLint reference, GLuint mask);
GL_APICALL void GL_APIENTRY glCoverFillPathCHROMIUMContextANGLE(GLeglContext ctx, GLuint path, GLenum coverMode);
GL_APICALL void GL_APIENTRY glCoverStrokePathCHROMIUMContextANGLE(GLeglContext ctx, GLuint path, GLenum coverMode);
GL_APICALL void GL_APIENTRY glStencilThenCoverFillPathCHROMIUMContextANGLE(GLeglContext ctx, GLuint path, GLenum fillMode, GLuint mask, GLenum coverMode);
GL_APICALL void GL_APIENTRY glStencilThenCoverStrokePathCHROMIUMContextANGLE(GLeglContext ctx, GLuint path, GLint reference, GLuint mask, GLenum coverMode);
GL_APICALL void GL_APIENTRY glCoverFillPathInstancedCHROMIUMContextANGLE(GLeglContext ctx, GLsizei numPath, GLenum pathNameType, const void * paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat * transformValues);
GL_APICALL void GL_APIENTRY glCoverStrokePathInstancedCHROMIUMContextANGLE(GLeglContext ctx, GLsizei numPath, GLenum pathNameType, const void * paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat * transformValues);
GL_APICALL void GL_APIENTRY glStencilStrokePathInstancedCHROMIUMContextANGLE(GLeglContext ctx, GLsizei numPath, GLenum pathNameType, const void * paths, GLuint pathBase, GLint reference, GLuint mask, GLenum transformType, const GLfloat * transformValues);
GL_APICALL void GL_APIENTRY glStencilFillPathInstancedCHROMIUMContextANGLE(GLeglContext ctx, GLsizei numPaths, GLenum pathNameType, const void * paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum transformType, const GLfloat * transformValues);
GL_APICALL void GL_APIENTRY glStencilThenCoverFillPathInstancedCHROMIUMContextANGLE(GLeglContext ctx, GLsizei numPaths, GLenum pathNameType, const void * paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat * transformValues);
GL_APICALL void GL_APIENTRY glStencilThenCoverStrokePathInstancedCHROMIUMContextANGLE(GLeglContext ctx, GLsizei numPaths, GLenum pathNameType, const void * paths, GLuint pathBase, GLint reference, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat * transformValues);
GL_APICALL void GL_APIENTRY glBindFragmentInputLocationCHROMIUMContextANGLE(GLeglContext ctx, GLuint programs, GLint location, const GLchar * name);
GL_APICALL void GL_APIENTRY glProgramPathFragmentInputGenCHROMIUMContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLenum genMode, GLint components, const GLfloat * coeffs);
GL_APICALL void GL_APIENTRY glCopyTextureCHROMIUMContextANGLE(GLeglContext ctx, GLuint sourceId, GLint sourceLevel, GLenum destTarget, GLuint destId, GLint destLevel, GLint internalFormat, GLenum destType, GLboolean unpackFlipY, GLboolean unpackPremultiplyAlpha, GLboolean unpackUnmultiplyAlpha);
GL_APICALL void GL_APIENTRY glCopySubTextureCHROMIUMContextANGLE(GLeglContext ctx, GLuint sourceId, GLint sourceLevel, GLenum destTarget, GLuint destId, GLint destLevel, GLint xoffset, GLint yoffset, GLint x, GLint y, GLint width, GLint height, GLboolean unpackFlipY, GLboolean unpackPremultiplyAlpha, GLboolean unpackUnmultiplyAlpha);
GL_APICALL void GL_APIENTRY glCompressedCopyTextureCHROMIUMContextANGLE(GLeglContext ctx, GLuint sourceId, GLuint destId);
GL_APICALL void GL_APIENTRY glRequestExtensionANGLEContextANGLE(GLeglContext ctx, const GLchar * name);
GL_APICALL void GL_APIENTRY glGetBooleanvRobustANGLEContextANGLE(GLeglContext ctx, GLenum pname, GLsizei bufSize, GLsizei * length, GLboolean * params);
GL_APICALL void GL_APIENTRY glGetBufferParameterivRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void GL_APIENTRY glGetFloatvRobustANGLEContextANGLE(GLeglContext ctx, GLenum pname, GLsizei bufSize, GLsizei * length, GLfloat * params);
GL_APICALL void GL_APIENTRY glGetFramebufferAttachmentParameterivRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void GL_APIENTRY glGetIntegervRobustANGLEContextANGLE(GLeglContext ctx, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * data);
GL_APICALL void GL_APIENTRY glGetProgramivRobustANGLEContextANGLE(GLeglContext ctx, GLuint program, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void GL_APIENTRY glGetRenderbufferParameterivRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void GL_APIENTRY glGetShaderivRobustANGLEContextANGLE(GLeglContext ctx, GLuint shader, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void GL_APIENTRY glGetTexParameterfvRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLfloat * params);
GL_APICALL void GL_APIENTRY glGetTexParameterivRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void GL_APIENTRY glGetUniformfvRobustANGLEContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLfloat * params);
GL_APICALL void GL_APIENTRY glGetUniformivRobustANGLEContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void GL_APIENTRY glGetVertexAttribfvRobustANGLEContextANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLsizei bufSize, GLsizei * length, GLfloat * params);
GL_APICALL void GL_APIENTRY glGetVertexAttribivRobustANGLEContextANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void GL_APIENTRY glGetVertexAttribPointervRobustANGLEContextANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLsizei bufSize, GLsizei * length, void ** pointer);
GL_APICALL void GL_APIENTRY glReadPixelsRobustANGLEContextANGLE(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLsizei * length, GLsizei * columns, GLsizei * rows, void * pixels);
GL_APICALL void GL_APIENTRY glTexImage2DRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, GLsizei bufSize, const void * pixels);
GL_APICALL void GL_APIENTRY glTexParameterfvRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, const GLfloat * params);
GL_APICALL void GL_APIENTRY glTexParameterivRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, const GLint * params);
GL_APICALL void GL_APIENTRY glTexSubImage2DRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, const void * pixels);
GL_APICALL void GL_APIENTRY glTexImage3DRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, GLsizei bufSize, const void * pixels);
GL_APICALL void GL_APIENTRY glTexSubImage3DRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, const void * pixels);
GL_APICALL void GL_APIENTRY glCompressedTexImage2DRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, GLsizei dataSize, const GLvoid * data);
GL_APICALL void GL_APIENTRY glCompressedTexSubImage2DRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLsizei xoffset, GLsizei yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, GLsizei dataSize, const GLvoid * data);
GL_APICALL void GL_APIENTRY glCompressedTexImage3DRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, GLsizei dataSize, const GLvoid * data);
GL_APICALL void GL_APIENTRY glCompressedTexSubImage3DRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, GLsizei dataSize, const GLvoid * data);
GL_APICALL void GL_APIENTRY glGetQueryivRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void GL_APIENTRY glGetQueryObjectuivRobustANGLEContextANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLsizei bufSize, GLsizei * length, GLuint * params);
GL_APICALL void GL_APIENTRY glGetBufferPointervRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, void ** params);
GL_APICALL void GL_APIENTRY glGetIntegeri_vRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLuint index, GLsizei bufSize, GLsizei * length, GLint * data);
GL_APICALL void GL_APIENTRY glGetInternalformativRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void GL_APIENTRY glGetVertexAttribIivRobustANGLEContextANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void GL_APIENTRY glGetVertexAttribIuivRobustANGLEContextANGLE(GLeglContext ctx, GLuint index, GLenum pname, GLsizei bufSize, GLsizei * length, GLuint * params);
GL_APICALL void GL_APIENTRY glGetUniformuivRobustANGLEContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLuint * params);
GL_APICALL void GL_APIENTRY glGetActiveUniformBlockivRobustANGLEContextANGLE(GLeglContext ctx, GLuint program, GLuint uniformBlockIndex, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void GL_APIENTRY glGetInteger64vRobustANGLEContextANGLE(GLeglContext ctx, GLenum pname, GLsizei bufSize, GLsizei * length, GLint64 * data);
GL_APICALL void GL_APIENTRY glGetInteger64i_vRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLuint index, GLsizei bufSize, GLsizei * length, GLint64 * data);
GL_APICALL void GL_APIENTRY glGetBufferParameteri64vRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint64 * params);
GL_APICALL void GL_APIENTRY glSamplerParameterivRobustANGLEContextANGLE(GLeglContext ctx, GLuint sampler, GLuint pname, GLsizei bufSize, const GLint * param);
GL_APICALL void GL_APIENTRY glSamplerParameterfvRobustANGLEContextANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, const GLfloat * param);
GL_APICALL void GL_APIENTRY glGetSamplerParameterivRobustANGLEContextANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void GL_APIENTRY glGetSamplerParameterfvRobustANGLEContextANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei * length, GLfloat * params);
GL_APICALL void GL_APIENTRY glGetFramebufferParameterivRobustANGLEContextANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void GL_APIENTRY glGetProgramInterfaceivRobustANGLEContextANGLE(GLeglContext ctx, GLuint program, GLenum programInterface, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void GL_APIENTRY glGetBooleani_vRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLuint index, GLsizei bufSize, GLsizei * length, GLboolean * data);
GL_APICALL void GL_APIENTRY glGetMultisamplefvRobustANGLEContextANGLE(GLeglContext ctx, GLenum pname, GLuint index, GLsizei bufSize, GLsizei * length, GLfloat * val);
GL_APICALL void GL_APIENTRY glGetTexLevelParameterivRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void GL_APIENTRY glGetTexLevelParameterfvRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum pname, GLsizei bufSize, GLsizei * length, GLfloat * params);
GL_APICALL void GL_APIENTRY glGetPointervRobustANGLERobustANGLEContextANGLE(GLeglContext ctx, GLenum pname, GLsizei bufSize, GLsizei * length, void ** params);
GL_APICALL void GL_APIENTRY glReadnPixelsRobustANGLEContextANGLE(GLeglContext ctx, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLsizei * length, GLsizei * columns, GLsizei * rows, void * data);
GL_APICALL void GL_APIENTRY glGetnUniformfvRobustANGLEContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLfloat * params);
GL_APICALL void GL_APIENTRY glGetnUniformivRobustANGLEContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void GL_APIENTRY glGetnUniformuivRobustANGLEContextANGLE(GLeglContext ctx, GLuint program, GLint location, GLsizei bufSize, GLsizei * length, GLuint * params);
GL_APICALL void GL_APIENTRY glTexParameterIivRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, const GLint * params);
GL_APICALL void GL_APIENTRY glTexParameterIuivRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, const GLuint * params);
GL_APICALL void GL_APIENTRY glGetTexParameterIivRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void GL_APIENTRY glGetTexParameterIuivRobustANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLsizei bufSize, GLsizei * length, GLuint * params);
GL_APICALL void GL_APIENTRY glSamplerParameterIivRobustANGLEContextANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, const GLint * param);
GL_APICALL void GL_APIENTRY glSamplerParameterIuivRobustANGLEContextANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, const GLuint * param);
GL_APICALL void GL_APIENTRY glGetSamplerParameterIivRobustANGLEContextANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void GL_APIENTRY glGetSamplerParameterIuivRobustANGLEContextANGLE(GLeglContext ctx, GLuint sampler, GLenum pname, GLsizei bufSize, GLsizei * length, GLuint * params);
GL_APICALL void GL_APIENTRY glGetQueryObjectivRobustANGLEContextANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * params);
GL_APICALL void GL_APIENTRY glGetQueryObjecti64vRobustANGLEContextANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLsizei bufSize, GLsizei * length, GLint64 * params);
GL_APICALL void GL_APIENTRY glGetQueryObjectui64vRobustANGLEContextANGLE(GLeglContext ctx, GLuint id, GLenum pname, GLsizei bufSize, GLsizei * length, GLuint64 * params);
GL_APICALL void GL_APIENTRY glFramebufferTextureMultiviewLayeredANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews);
GL_APICALL void GL_APIENTRY glFramebufferTextureMultiviewSideBySideANGLEContextANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLuint texture, GLint level, GLsizei numViews, const GLint * viewportOffsets);
#endif
#endif /* EGL_ANGLE_explicit_context */

// clang-format on

#endif  // INCLUDE_GLES2_GL2EXT_ANGLE_H_
