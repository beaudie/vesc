//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// validationES31.cpp: Validation functions for OpenGL ES 3.1 entry point parameters

#include "libANGLE/validationES31_autogen.h"

#include "libANGLE/Context.h"
#include "libANGLE/ErrorStrings.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/validationES.h"
#include "libANGLE/validationES2_autogen.h"
#include "libANGLE/validationES3_autogen.h"

#include "common/utilities.h"

using namespace angle;

namespace gl
{
using namespace err;

namespace
{
#ifdef THESE_ARE_IMPLEMENTED_ELSEWHERE
bool ValidateBlendBarrier(Context *context);

// FIXME/TBD: The following is implemented in "validationGL4.cpp".  Is that good enough?
//
// bool ValidateBlendEquationSeparatei(Context *context, GLuint buf, GLenum modeRGB, GLenum
// modeAlpha);

// FIXME/TBD: The following is implemented in "validationGL4.cpp".  Is that good enough?
//
// bool ValidateBlendEquationi(Context *context, GLuint buf, GLenum mode);

// FIXME/TBD: The following is implemented in "validationGL4.cpp".  Is that good enough?
//
// bool ValidateBlendFuncSeparatei(Context *context,
//                                GLuint buf,
//                                GLenum srcRGB,
//                                GLenum dstRGB,
//                                GLenum srcAlpha,
//                                GLenum dstAlpha);

// FIXME/TBD: The following is implemented in "validationGL4.cpp".  Is that good enough?
//
// bool ValidateBlendFunci(Context *context, GLuint buf, GLenum src, GLenum dst);

// FIXME/TBD: The following is implemented in "validationGL3.cpp".  Is that good enough?
//
// bool ValidateColorMaski(Context *context,
//                        GLuint index,
//                        GLboolean r,
//                        GLboolean g,
//                        GLboolean b,
//                        GLboolean a);

// FIXME/TBD: The following is implemented in "validationGL43.cpp".  Is that good enough?
//
// bool ValidateCopyImageSubData(Context *context,
//                              GLuint srcName,
//                              GLenum srcTarget,
//                              GLint srcLevel,
//                              GLint srcX,
//                              GLint srcY,
//                              GLint srcZ,
//                              GLuint dstName,
//                              GLenum dstTarget,
//                              GLint dstLevel,
//                              GLint dstX,
//                              GLint dstY,
//                              GLint dstZ,
//                              GLsizei srcWidth,
//                              GLsizei srcHeight,
//                              GLsizei srcDepth);

bool ValidateDebugMessageCallback(Context *context, GLDEBUGPROC callback, const void *userParam);
bool ValidateDebugMessageControl(Context *context,
                                 GLenum source,
                                 GLenum type,
                                 GLenum severity,
                                 GLsizei count,
                                 const GLuint *ids,
                                 GLboolean enabled);
bool ValidateDebugMessageInsert(Context *context,
                                GLenum source,
                                GLenum type,
                                GLuint id,
                                GLenum severity,
                                GLsizei length,
                                const GLchar *buf);

// FIXME/TBD: The following is implemented in "validationGL3.cpp".  Is that good enough?
//
// bool ValidateDisablei(Context *context, GLenum target, GLuint index);

// FIXME/TBD: The following is implemented in "validationGL32.cpp".  Is that good enough?
//
// bool ValidateDrawElementsBaseVertex(Context *context,
//                                    GLenum mode,
//                                    GLsizei count,
//                                    GLenum type,
//                                    const void *indices,
//                                    GLint basevertex);

// FIXME/TBD: The following is implemented in "validationGL32.cpp".  Is that good enough?
//
// bool ValidateDrawElementsInstancedBaseVertex(Context *context,
//                                             GLenum mode,
//                                             GLsizei count,
//                                             GLenum type,
//                                             const void *indices,
//                                             GLsizei instancecount,
//                                             GLint basevertex);

// FIXME/TBD: The following is implemented in "validationGL32.cpp".  Is that good enough?
//
// bool ValidateDrawRangeElementsBaseVertex(Context *context,
//                                         GLenum mode,
//                                         GLuint start,
//                                         GLuint end,
//                                         GLsizei count,
//                                         GLenum type,
//                                         const void *indices,
//                                         GLint basevertex);

// FIXME/TBD: The following is implemented in "validationGL3.cpp".  Is that good enough?
//
// bool ValidateEnablei(Context *context, GLenum target, GLuint index);

// FIXME/TBD: The following is implemented in "validationGL32.cpp".  It looks
// like it's a yes-man, pass-through.  Is that good enough?
//
// bool ValidateFramebufferTexture(Context *context,
//                                GLenum target,
//                                GLenum attachment,
//                                TextureID texturePacked,
//                                GLint level);

// FIXME/TBD: The following is implemented in "validationGL43.cpp".  It looks
// like it's a yes-man, pass-through.  Is that good enough?
//
// bool ValidateGetDebugMessageLog(Context *context,
//                                GLuint count,
//                                GLsizei bufSize,
//                                GLenum *sources,
//                                GLenum *types,
//                                GLuint *ids,
//                                GLenum *severities,
//                                GLsizei *lengths,
//                                GLchar *messageLog);

// FIXME/TBD: The following is implemented in "validationGL45.cpp".  It looks
// like it's a yes-man, pass-through.  Is that good enough?
//
// bool ValidateGetGraphicsResetStatus(Context *context);

#    if 0
// FIXME/TBD: The following is implemented in "validationGL43.cpp".  It looks
// like it's a yes-man, pass-through.  Is that good enough?
bool ValidateGetObjectLabel(Context *context,
                            GLenum identifier,
                            GLuint name,
                            GLsizei bufSize,
                            GLsizei *length,
                            GLchar *label);
bool ValidateGetObjectPtrLabel(Context *context,
                               const void *ptr,
                               GLsizei bufSize,
                               GLsizei *length,
                               GLchar *label);
#    endif

bool ValidateGetPointerv(Context *context, GLenum pname, void **params)
{
#    if 0
    if (context->getClientVersion() < ES_3_2)
    {
        context->validationError(GL_INVALID_OPERATION, kES32Required);
        return false;
    }
#    endif
    switch (pname)
    {
#    if 0  // TODO/FIXME - DEAL WITH THESE GLES 1.X VALUES
        case GL_VERTEX_ARRAY_POINTER:
        case GL_NORMAL_ARRAY_POINTER:
        case GL_COLOR_ARRAY_POINTER:
        case GL_TEXTURE_COORD_ARRAY_POINTER:
        case GL_POINT_SIZE_ARRAY_POINTER_OES:
#    endif
        case GL_DEBUG_CALLBACK_FUNCTION:
        case GL_DEBUG_CALLBACK_USER_PARAM:
            return true;
        default:
            context->validationError(GL_INVALID_ENUM, kInvalidPointerQuery);
            return false;
    }
}

bool ValidateGetSamplerParameterIiv(Context *context,
                                    SamplerID samplerPacked,
                                    GLenum pname,
                                    GLint *params);
bool ValidateGetSamplerParameterIuiv(Context *context,
                                     SamplerID samplerPacked,
                                     GLenum pname,
                                     GLuint *params);
bool ValidateGetTexParameterIiv(Context *context,
                                TextureType targetPacked,
                                GLenum pname,
                                GLint *params);
bool ValidateGetTexParameterIuiv(Context *context,
                                 TextureType targetPacked,
                                 GLenum pname,
                                 GLuint *params);

// FIXME/TBD: The following is implemented in "validationGL45.cpp".  Is that good enough?
//
// bool ValidateGetnUniformfv(Context *context,
//                           ShaderProgramID programPacked,
//                           GLint location,
//                           GLsizei bufSize,
//                           GLfloat *params);

// FIXME/TBD: The following is implemented in "validationGL45.cpp".  Is that good enough?
//
// bool ValidateGetnUniformiv(Context *context,
//                           ShaderProgramID programPacked,
//                           GLint location,
//                           GLsizei bufSize,
//                           GLint *params);

// FIXME/TBD: The following is implemented in "validationGL45.cpp".  Is that good enough?
//
// bool ValidateGetnUniformuiv(Context *context,
//                            ShaderProgramID programPacked,
//                            GLint location,
//                            GLsizei bufSize,
//                            GLuint *params);

// FIXME/TBD: The following is implemented in "validationGL3.cpp".  Is that good enough?
//
// bool ValidateIsEnabledi(Context *context, GLenum target, GLuint index);

// FIXME/TBD: The following is implemented in "validationGL4.cpp".  Is that good enough?
//
// bool ValidateMinSampleShading(Context *context, GLfloat value);

bool ValidateObjectLabel(Context *context,
                         GLenum identifier,
                         GLuint name,
                         GLsizei length,
                         const GLchar *label);
bool ValidateObjectPtrLabel(Context *context, const void *ptr, GLsizei length, const GLchar *label);

// FIXME/TBD: The following is implemented in "validationGL4.cpp".  Is that good enough?
//
// bool ValidatePatchParameteri(Context *context, GLenum pname, GLint value);

bool ValidatePopDebugGroup(Context *context);
bool ValidatePrimitiveBoundingBox(Context *context,
                                  GLfloat minX,
                                  GLfloat minY,
                                  GLfloat minZ,
                                  GLfloat minW,
                                  GLfloat maxX,
                                  GLfloat maxY,
                                  GLfloat maxZ,
                                  GLfloat maxW);
bool ValidatePushDebugGroup(Context *context,
                            GLenum source,
                            GLuint id,
                            GLsizei length,
                            const GLchar *message);

// FIXME/TBD: The following is implemented in "validationGL45.cpp".  It looks
// like it's a yes-man, pass-through.  Is that good enough?
//
// bool ValidateReadnPixels(Context *context,
//                         GLint x,
//                         GLint y,
//                         GLsizei width,
//                         GLsizei height,
//                         GLenum format,
//                         GLenum type,
//                         GLsizei bufSize,
//                         void *data);

bool ValidateSamplerParameterIiv(Context *context,
                                 SamplerID samplerPacked,
                                 GLenum pname,
                                 const GLint *param);
bool ValidateSamplerParameterIuiv(Context *context,
                                  SamplerID samplerPacked,
                                  GLenum pname,
                                  const GLuint *param);

// FIXME/TBD: The following is implemented in "validationGL31.cpp".  Is that good enough?
//
// bool ValidateTexBuffer(Context *context,
//                       GLenum target,
//                       GLenum internalformat,
//                       BufferID bufferPacked);

// FIXME/TBD: The following is implemented in "validationGL43.cpp".  Is that good enough?
//
// bool ValidateTexBufferRange(Context *context,
//                            GLenum target,
//                            GLenum internalformat,
//                            BufferID bufferPacked,
//                            GLintptr offset,
//                            GLsizeiptr size);

bool ValidateTexParameterIiv(Context *context,
                             TextureType targetPacked,
                             GLenum pname,
                             const GLint *params);
bool ValidateTexParameterIuiv(Context *context,
                              TextureType targetPacked,
                              GLenum pname,
                              const GLuint *params);
bool ValidateTexStorage3DMultisample(Context *context,
                                     TextureType targetPacked,
                                     GLsizei samples,
                                     GLenum internalformat,
                                     GLsizei width,
                                     GLsizei height,
                                     GLsizei depth,
                                     GLboolean fixedsamplelocations);

#endif  // THESE_ARE_IMPLEMENTED_ELSEWHERE

}  // anonymous namespace

}  // namespace gl
