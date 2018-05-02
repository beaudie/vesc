#ifdef __cplusplus
extern "C" {
#endif

//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// gl31ext_angle.h: ANGLE modifications to the gl31.h header file.

#ifndef INCLUDE_GLES31_GL31EXT_ANGLE_H_
#define INCLUDE_GLES31_GL31EXT_ANGLE_H_

// clang-format off

#ifdef EGL_ANGLE_explicit_context
typedef void *GLeglContext;
// OpenGL ES 3.1
typedef void (GL_APIENTRY PFNACTIVESHADERPROGRAMCONTEXTANGLE)(GLeglContext ctx, GLuint pipeline, GLuint program);
typedef void (GL_APIENTRY PFNBINDIMAGETEXTURECONTEXTANGLE)(GLeglContext ctx, GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
typedef void (GL_APIENTRY PFNBINDPROGRAMPIPELINECONTEXTANGLE)(GLeglContext ctx, GLuint pipeline);
typedef void (GL_APIENTRY PFNBINDVERTEXBUFFERCONTEXTANGLE)(GLeglContext ctx, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
typedef void (GL_APIENTRY PFNCREATESHADERPROGRAMVCONTEXTANGLE)(GLeglContext ctx, GLenum type, GLsizei count, const GLchar *const*strings);
typedef void (GL_APIENTRY PFNDELETEPROGRAMPIPELINESCONTEXTANGLE)(GLeglContext ctx, GLsizei n, const GLuint *pipelines);
typedef void (GL_APIENTRY PFNDISPATCHCOMPUTECONTEXTANGLE)(GLeglContext ctx, GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
typedef void (GL_APIENTRY PFNDISPATCHCOMPUTEINDIRECTCONTEXTANGLE)(GLeglContext ctx, GLintptr indirect);
typedef void (GL_APIENTRY PFNDRAWARRAYSINDIRECTCONTEXTANGLE)(GLeglContext ctx, GLenum mode, const void *indirect);
typedef void (GL_APIENTRY PFNDRAWELEMENTSINDIRECTCONTEXTANGLE)(GLeglContext ctx, GLenum mode, GLenum type, const void *indirect);
typedef void (GL_APIENTRY PFNFRAMEBUFFERPARAMETERICONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLint param);
typedef void (GL_APIENTRY PFNGENPROGRAMPIPELINESCONTEXTANGLE)(GLeglContext ctx, GLsizei n, GLuint *pipelines);
typedef void (GL_APIENTRY PFNGETBOOLEANI_VCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLuint index, GLboolean *data);
typedef void (GL_APIENTRY PFNGETFRAMEBUFFERPARAMETERIVCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
typedef void (GL_APIENTRY PFNGETMULTISAMPLEFVCONTEXTANGLE)(GLeglContext ctx, GLenum pname, GLuint index, GLfloat *val);
typedef void (GL_APIENTRY PFNGETPROGRAMINTERFACEIVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLenum programInterface, GLenum pname, GLint *params);
typedef void (GL_APIENTRY PFNGETPROGRAMPIPELINEINFOLOGCONTEXTANGLE)(GLeglContext ctx, GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (GL_APIENTRY PFNGETPROGRAMPIPELINEIVCONTEXTANGLE)(GLeglContext ctx, GLuint pipeline, GLenum pname, GLint *params);
typedef void (GL_APIENTRY PFNGETPROGRAMRESOURCEINDEXCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLenum programInterface, const GLchar *name);
typedef void (GL_APIENTRY PFNGETPROGRAMRESOURCELOCATIONCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLenum programInterface, const GLchar *name);
typedef void (GL_APIENTRY PFNGETPROGRAMRESOURCENAMECONTEXTANGLE)(GLeglContext ctx, GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
typedef void (GL_APIENTRY PFNGETPROGRAMRESOURCEIVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params);
typedef void (GL_APIENTRY PFNGETTEXLEVELPARAMETERFVCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLint level, GLenum pname, GLfloat *params);
typedef void (GL_APIENTRY PFNGETTEXLEVELPARAMETERIVCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLint level, GLenum pname, GLint *params);
typedef void (GL_APIENTRY PFNISPROGRAMPIPELINECONTEXTANGLE)(GLeglContext ctx, GLuint pipeline);
typedef void (GL_APIENTRY PFNMEMORYBARRIERCONTEXTANGLE)(GLeglContext ctx, GLbitfield barriers);
typedef void (GL_APIENTRY PFNMEMORYBARRIERBYREGIONCONTEXTANGLE)(GLeglContext ctx, GLbitfield barriers);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM1FCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLfloat v0);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM1FVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM1ICONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLint v0);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM1IVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM1UICONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLuint v0);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM1UIVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM2FCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLfloat v0, GLfloat v1);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM2FVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM2ICONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLint v0, GLint v1);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM2IVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM2UICONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLuint v0, GLuint v1);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM2UIVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM3FCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM3FVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM3ICONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM3IVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM3UICONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM3UIVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM4FCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM4FVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM4ICONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM4IVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM4UICONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORM4UIVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORMMATRIX2FVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORMMATRIX2X3FVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORMMATRIX2X4FVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORMMATRIX3FVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORMMATRIX3X2FVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORMMATRIX3X4FVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORMMATRIX4FVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORMMATRIX4X2FVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRY PFNPROGRAMUNIFORMMATRIX4X3FVCONTEXTANGLE)(GLeglContext ctx, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRY PFNSAMPLEMASKICONTEXTANGLE)(GLeglContext ctx, GLuint maskNumber, GLbitfield mask);
typedef void (GL_APIENTRY PFNTEXSTORAGE2DMULTISAMPLECONTEXTANGLE)(GLeglContext ctx, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void (GL_APIENTRY PFNUSEPROGRAMSTAGESCONTEXTANGLE)(GLeglContext ctx, GLuint pipeline, GLbitfield stages, GLuint program);
typedef void (GL_APIENTRY PFNVALIDATEPROGRAMPIPELINECONTEXTANGLE)(GLeglContext ctx, GLuint pipeline);
typedef void (GL_APIENTRY PFNVERTEXATTRIBBINDINGCONTEXTANGLE)(GLeglContext ctx, GLuint attribindex, GLuint bindingindex);
typedef void (GL_APIENTRY PFNVERTEXATTRIBFORMATCONTEXTANGLE)(GLeglContext ctx, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
typedef void (GL_APIENTRY PFNVERTEXATTRIBIFORMATCONTEXTANGLE)(GLeglContext ctx, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
typedef void (GL_APIENTRY PFNVERTEXBINDINGDIVISORCONTEXTANGLE)(GLeglContext ctx, GLuint bindingindex, GLuint divisor);
#ifdef GL_GLEXT_PROTOTYPES
// OpenGL ES 3.1
GL_APICALL void GL_APIENTRY glActiveShaderProgramContextANGLE(GLeglContext ctx, GLuint pipeline, GLuint program);
GL_APICALL void GL_APIENTRY glBindImageTextureContextANGLE(GLeglContext ctx, GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
GL_APICALL void GL_APIENTRY glBindProgramPipelineContextANGLE(GLeglContext ctx, GLuint pipeline);
GL_APICALL void GL_APIENTRY glBindVertexBufferContextANGLE(GLeglContext ctx, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
GL_APICALL GLuint GL_APIENTRY glCreateShaderProgramvContextANGLE(GLeglContext ctx, GLenum type, GLsizei count, const GLchar *const*strings);
GL_APICALL void GL_APIENTRY glDeleteProgramPipelinesContextANGLE(GLeglContext ctx, GLsizei n, const GLuint *pipelines);
GL_APICALL void GL_APIENTRY glDispatchComputeContextANGLE(GLeglContext ctx, GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
GL_APICALL void GL_APIENTRY glDispatchComputeIndirectContextANGLE(GLeglContext ctx, GLintptr indirect);
GL_APICALL void GL_APIENTRY glDrawArraysIndirectContextANGLE(GLeglContext ctx, GLenum mode, const void *indirect);
GL_APICALL void GL_APIENTRY glDrawElementsIndirectContextANGLE(GLeglContext ctx, GLenum mode, GLenum type, const void *indirect);
GL_APICALL void GL_APIENTRY glFramebufferParameteriContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint param);
GL_APICALL void GL_APIENTRY glGenProgramPipelinesContextANGLE(GLeglContext ctx, GLsizei n, GLuint *pipelines);
GL_APICALL void GL_APIENTRY glGetBooleani_vContextANGLE(GLeglContext ctx, GLenum target, GLuint index, GLboolean *data);
GL_APICALL void GL_APIENTRY glGetFramebufferParameterivContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetMultisamplefvContextANGLE(GLeglContext ctx, GLenum pname, GLuint index, GLfloat *val);
GL_APICALL void GL_APIENTRY glGetProgramInterfaceivContextANGLE(GLeglContext ctx, GLuint program, GLenum programInterface, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetProgramPipelineInfoLogContextANGLE(GLeglContext ctx, GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GL_APICALL void GL_APIENTRY glGetProgramPipelineivContextANGLE(GLeglContext ctx, GLuint pipeline, GLenum pname, GLint *params);
GL_APICALL GLuint GL_APIENTRY glGetProgramResourceIndexContextANGLE(GLeglContext ctx, GLuint program, GLenum programInterface, const GLchar *name);
GL_APICALL GLint GL_APIENTRY glGetProgramResourceLocationContextANGLE(GLeglContext ctx, GLuint program, GLenum programInterface, const GLchar *name);
GL_APICALL void GL_APIENTRY glGetProgramResourceNameContextANGLE(GLeglContext ctx, GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
GL_APICALL void GL_APIENTRY glGetProgramResourceivContextANGLE(GLeglContext ctx, GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL void GL_APIENTRY glGetTexLevelParameterfvContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum pname, GLfloat *params);
GL_APICALL void GL_APIENTRY glGetTexLevelParameterivContextANGLE(GLeglContext ctx, GLenum target, GLint level, GLenum pname, GLint *params);
GL_APICALL GLboolean GL_APIENTRY glIsProgramPipelineContextANGLE(GLeglContext ctx, GLuint pipeline);
GL_APICALL void GL_APIENTRY glMemoryBarrierContextANGLE(GLeglContext ctx, GLbitfield barriers);
GL_APICALL void GL_APIENTRY glMemoryBarrierByRegionContextANGLE(GLeglContext ctx, GLbitfield barriers);
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
GL_APICALL void GL_APIENTRY glSampleMaskiContextANGLE(GLeglContext ctx, GLuint maskNumber, GLbitfield mask);
GL_APICALL void GL_APIENTRY glTexStorage2DMultisampleContextANGLE(GLeglContext ctx, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
GL_APICALL void GL_APIENTRY glUseProgramStagesContextANGLE(GLeglContext ctx, GLuint pipeline, GLbitfield stages, GLuint program);
GL_APICALL void GL_APIENTRY glValidateProgramPipelineContextANGLE(GLeglContext ctx, GLuint pipeline);
GL_APICALL void GL_APIENTRY glVertexAttribBindingContextANGLE(GLeglContext ctx, GLuint attribindex, GLuint bindingindex);
GL_APICALL void GL_APIENTRY glVertexAttribFormatContextANGLE(GLeglContext ctx, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
GL_APICALL void GL_APIENTRY glVertexAttribIFormatContextANGLE(GLeglContext ctx, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
GL_APICALL void GL_APIENTRY glVertexBindingDivisorContextANGLE(GLeglContext ctx, GLuint bindingindex, GLuint divisor);
#endif
#endif /* EGL_ANGLE_explicit_context */

// clang-format on
#ifdef __cplusplus
}
#endif
#endif /* INCLUDE_GLES31_GL31EXT_ANGLE_H */