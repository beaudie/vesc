//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// capture_gles32_params.cpp:
//   Pointer parameter capture functions for the OpenGL ES 3.2 entry points.

#include "libANGLE/capture_gles_3_2_autogen.h"

using namespace angle;

namespace gl
{

void CaptureDebugMessageCallback_userParam(Context *context,
                                           GLDEBUGPROC callback,
                                           const void *userParam,
                                           ParamCapture *userParamParam)
{
    UNIMPLEMENTED();
}

void CaptureDebugMessageControl_ids(Context *context,
                                    GLenum source,
                                    GLenum type,
                                    GLenum severity,
                                    GLsizei count,
                                    const GLuint *ids,
                                    GLboolean enabled,
                                    ParamCapture *idsParam)
{
    UNIMPLEMENTED();
}

void CaptureDebugMessageInsert_buf(Context *context,
                                   GLenum source,
                                   GLenum type,
                                   GLuint id,
                                   GLenum severity,
                                   GLsizei length,
                                   const GLchar *buf,
                                   ParamCapture *bufParam)
{
    UNIMPLEMENTED();
}

void CaptureDrawElementsBaseVertex_indices(Context *context,
                                           GLenum mode,
                                           GLsizei count,
                                           GLenum type,
                                           const void *indices,
                                           GLint basevertex,
                                           ParamCapture *indicesParam)
{
    UNIMPLEMENTED();
}

void CaptureDrawElementsInstancedBaseVertex_indices(Context *context,
                                                    GLenum mode,
                                                    GLsizei count,
                                                    GLenum type,
                                                    const void *indices,
                                                    GLsizei instancecount,
                                                    GLint basevertex,
                                                    ParamCapture *indicesParam)
{
    UNIMPLEMENTED();
}

void CaptureDrawRangeElementsBaseVertex_indices(Context *context,
                                                GLenum mode,
                                                GLuint start,
                                                GLuint end,
                                                GLsizei count,
                                                GLenum type,
                                                const void *indices,
                                                GLint basevertex,
                                                ParamCapture *indicesParam)
{
    UNIMPLEMENTED();
}

void CaptureGetDebugMessageLog_sources(Context *context,
                                       GLuint count,
                                       GLsizei bufSize,
                                       GLenum *sources,
                                       GLenum *types,
                                       GLuint *ids,
                                       GLenum *severities,
                                       GLsizei *lengths,
                                       GLchar *messageLog,
                                       ParamCapture *sourcesParam)
{
    UNIMPLEMENTED();
}

void CaptureGetDebugMessageLog_types(Context *context,
                                     GLuint count,
                                     GLsizei bufSize,
                                     GLenum *sources,
                                     GLenum *types,
                                     GLuint *ids,
                                     GLenum *severities,
                                     GLsizei *lengths,
                                     GLchar *messageLog,
                                     ParamCapture *typesParam)
{
    UNIMPLEMENTED();
}

void CaptureGetDebugMessageLog_ids(Context *context,
                                   GLuint count,
                                   GLsizei bufSize,
                                   GLenum *sources,
                                   GLenum *types,
                                   GLuint *ids,
                                   GLenum *severities,
                                   GLsizei *lengths,
                                   GLchar *messageLog,
                                   ParamCapture *idsParam)
{
    UNIMPLEMENTED();
}

void CaptureGetDebugMessageLog_severities(Context *context,
                                          GLuint count,
                                          GLsizei bufSize,
                                          GLenum *sources,
                                          GLenum *types,
                                          GLuint *ids,
                                          GLenum *severities,
                                          GLsizei *lengths,
                                          GLchar *messageLog,
                                          ParamCapture *severitiesParam)
{
    UNIMPLEMENTED();
}

void CaptureGetDebugMessageLog_lengths(Context *context,
                                       GLuint count,
                                       GLsizei bufSize,
                                       GLenum *sources,
                                       GLenum *types,
                                       GLuint *ids,
                                       GLenum *severities,
                                       GLsizei *lengths,
                                       GLchar *messageLog,
                                       ParamCapture *lengthsParam)
{
    UNIMPLEMENTED();
}

void CaptureGetDebugMessageLog_messageLog(Context *context,
                                          GLuint count,
                                          GLsizei bufSize,
                                          GLenum *sources,
                                          GLenum *types,
                                          GLuint *ids,
                                          GLenum *severities,
                                          GLsizei *lengths,
                                          GLchar *messageLog,
                                          ParamCapture *messageLogParam)
{
    UNIMPLEMENTED();
}

void CaptureGetObjectLabel_length(Context *context,
                                  GLenum identifier,
                                  GLuint name,
                                  GLsizei bufSize,
                                  GLsizei *length,
                                  GLchar *label,
                                  ParamCapture *lengthParam)
{
    UNIMPLEMENTED();
}

void CaptureGetObjectLabel_label(Context *context,
                                 GLenum identifier,
                                 GLuint name,
                                 GLsizei bufSize,
                                 GLsizei *length,
                                 GLchar *label,
                                 ParamCapture *labelParam)
{
    UNIMPLEMENTED();
}

void CaptureGetObjectPtrLabel_ptr(Context *context,
                                  const void *ptr,
                                  GLsizei bufSize,
                                  GLsizei *length,
                                  GLchar *label,
                                  ParamCapture *ptrParam)
{
    UNIMPLEMENTED();
}

void CaptureGetObjectPtrLabel_length(Context *context,
                                     const void *ptr,
                                     GLsizei bufSize,
                                     GLsizei *length,
                                     GLchar *label,
                                     ParamCapture *lengthParam)
{
    UNIMPLEMENTED();
}

void CaptureGetObjectPtrLabel_label(Context *context,
                                    const void *ptr,
                                    GLsizei bufSize,
                                    GLsizei *length,
                                    GLchar *label,
                                    ParamCapture *labelParam)
{
    UNIMPLEMENTED();
}

void CaptureGetPointerv_params(const Context *context,
                               bool isCallValid,
                               GLenum pname,
                               void **params,
                               ParamCapture *paramCapture)
{
    UNIMPLEMENTED();
}

void CaptureGetSamplerParameterIiv_params(Context *context,
                                          SamplerID sampler,
                                          GLenum pname,
                                          GLint *params,
                                          ParamCapture *paramsParam)
{
    UNIMPLEMENTED();
}

void CaptureGetSamplerParameterIuiv_params(Context *context,
                                           SamplerID sampler,
                                           GLenum pname,
                                           GLuint *params,
                                           ParamCapture *paramsParam)
{
    UNIMPLEMENTED();
}

void CaptureGetTexParameterIiv_params(Context *context,
                                      TextureType targetPacked,
                                      GLenum pname,
                                      GLint *params,
                                      ParamCapture *paramsParam)
{
    UNIMPLEMENTED();
}

void CaptureGetTexParameterIuiv_params(Context *context,
                                       TextureType targetPacked,
                                       GLenum pname,
                                       GLuint *params,
                                       ParamCapture *paramsParam)
{
    UNIMPLEMENTED();
}

void CaptureGetnUniformfv_params(Context *context,
                                 ShaderProgramID program,
                                 GLint location,
                                 GLsizei bufSize,
                                 GLfloat *params,
                                 ParamCapture *paramsParam)
{
    UNIMPLEMENTED();
}

void CaptureGetnUniformiv_params(Context *context,
                                 ShaderProgramID program,
                                 GLint location,
                                 GLsizei bufSize,
                                 GLint *params,
                                 ParamCapture *paramsParam)
{
    UNIMPLEMENTED();
}

void CaptureGetnUniformuiv_params(Context *context,
                                  ShaderProgramID program,
                                  GLint location,
                                  GLsizei bufSize,
                                  GLuint *params,
                                  ParamCapture *paramsParam)
{
    UNIMPLEMENTED();
}

void CaptureObjectLabel_label(Context *context,
                              GLenum identifier,
                              GLuint name,
                              GLsizei length,
                              const GLchar *label,
                              ParamCapture *labelParam)
{
    UNIMPLEMENTED();
}

void CaptureObjectPtrLabel_label(Context *context,
                                 const void *ptr,
                                 GLsizei length,
                                 const GLchar *label,
                                 ParamCapture *labelParam)
{
    UNIMPLEMENTED();
}

void CaptureReadnPixels_data(Context *context,
                             GLint x,
                             GLint y,
                             GLsizei width,
                             GLsizei height,
                             GLenum format,
                             GLenum type,
                             GLsizei bufSize,
                             void *data,
                             ParamCapture *dataParam)
{
    UNIMPLEMENTED();
}

void CaptureSamplerParameterIiv_param(Context *context,
                                      SamplerID sampler,
                                      GLenum pname,
                                      const GLint *param,
                                      ParamCapture *paramParam)
{
    UNIMPLEMENTED();
}

void CaptureSamplerParameterIuiv_param(Context *context,
                                       SamplerID sampler,
                                       GLenum pname,
                                       const GLuint *param,
                                       ParamCapture *paramParam)
{
    UNIMPLEMENTED();
}

void CaptureTexParameterIiv_params(Context *context,
                                   TextureType targetPacked,
                                   GLenum pname,
                                   const GLint *params,
                                   ParamCapture *paramParam)
{
    UNIMPLEMENTED();
}

void CaptureTexParameterIuiv_params(Context *context,
                                    TextureType targetPacked,
                                    GLenum pname,
                                    const GLuint *params,
                                    ParamCapture *paramParam)
{
    UNIMPLEMENTED();
}

}  // namespace gl
