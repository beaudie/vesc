//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// entry_point_utils:
//   These helpers are used in GL/GLES entry point routines.

#ifndef LIBANGLE_ENTRY_POINT_UTILS_H_
#define LIBANGLE_ENTRY_POINT_UTILS_H_

#include "angle_gl.h"
#include "common/Optional.h"
#include "common/PackedEnums.h"
#include "common/angleutils.h"
#include "common/entry_points_enum_autogen.h"
#include "common/mathutil.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/ErrorStrings.h"

namespace gl
{
// A template struct for determining the default value to return for each entry point.
template <angle::EntryPoint EP, typename ReturnType>
struct DefaultReturnValue;

// Default return values for each basic return type.
template <angle::EntryPoint EP>
struct DefaultReturnValue<EP, GLint>
{
    static constexpr GLint kValue = -1;
};

// This doubles as the GLenum return value.
template <angle::EntryPoint EP>
struct DefaultReturnValue<EP, GLuint>
{
    static constexpr GLuint kValue = 0;
};

template <angle::EntryPoint EP>
struct DefaultReturnValue<EP, GLboolean>
{
    static constexpr GLboolean kValue = GL_FALSE;
};

template <angle::EntryPoint EP>
struct DefaultReturnValue<EP, ShaderProgramID>
{
    static constexpr ShaderProgramID kValue = {0};
};

// Catch-all rules for pointer types.
template <angle::EntryPoint EP, typename PointerType>
struct DefaultReturnValue<EP, const PointerType *>
{
    static constexpr const PointerType *kValue = nullptr;
};

template <angle::EntryPoint EP, typename PointerType>
struct DefaultReturnValue<EP, PointerType *>
{
    static constexpr PointerType *kValue = nullptr;
};

// Overloaded to return invalid index
template <>
struct DefaultReturnValue<angle::EntryPoint::GLGetUniformBlockIndex, GLuint>
{
    static constexpr GLuint kValue = GL_INVALID_INDEX;
};

// Specialized enum error value.
template <>
struct DefaultReturnValue<angle::EntryPoint::GLClientWaitSync, GLenum>
{
    static constexpr GLenum kValue = GL_WAIT_FAILED;
};

// glTestFenceNV should still return TRUE for an invalid fence.
template <>
struct DefaultReturnValue<angle::EntryPoint::GLTestFenceNV, GLboolean>
{
    static constexpr GLboolean kValue = GL_TRUE;
};

template <angle::EntryPoint EP, typename ReturnType>
constexpr ANGLE_INLINE ReturnType GetDefaultReturnValue()
{
    return DefaultReturnValue<EP, ReturnType>::kValue;
}

#if ANGLE_CAPTURE_ENABLED
#    define ANGLE_CAPTURE_GL(Func, ...) CaptureCallToFrameCapture(Capture##Func, __VA_ARGS__)
#    define ANGLE_CAPTURE_EGL(Func, ...) CaptureCallToCaptureEGL(Capture##Func, __VA_ARGS__)
#else
#    define ANGLE_CAPTURE_GL(...)
#    define ANGLE_CAPTURE_EGL(...)
#endif  // ANGLE_CAPTURE_ENABLED

#define EGL_EVENT(EP, FMT, ...) EVENT(nullptr, EGL##EP, FMT, ##__VA_ARGS__)

inline int CID(const Context *context)
{
    return context == nullptr ? 0 : static_cast<int>(context->id().value);
}

constexpr bool IsAllowedWhenPixelLocalStorageActive(angle::EntryPoint entryPoint)
{
    using EntryPoint = angle::EntryPoint;
    switch (entryPoint)
    {
        case EntryPoint::GLActiveTexture:
        case EntryPoint::GLBindBuffer:
        case EntryPoint::GLBindBufferBase:
        case EntryPoint::GLBindBufferRange:
        case EntryPoint::GLBindSampler:
        case EntryPoint::GLBindTexture:
        case EntryPoint::GLBindVertexArray:
        case EntryPoint::GLBlendEquationSeparatei:
        case EntryPoint::GLBlendEquationSeparateiEXT:
        case EntryPoint::GLBlendEquationSeparateiOES:
        case EntryPoint::GLBlendEquationi:
        case EntryPoint::GLBlendEquationiEXT:
        case EntryPoint::GLBlendEquationiOES:
        case EntryPoint::GLBlendFuncSeparatei:
        case EntryPoint::GLBlendFuncSeparateiEXT:
        case EntryPoint::GLBlendFuncSeparateiOES:
        case EntryPoint::GLBlendFunci:
        case EntryPoint::GLBlendFunciEXT:
        case EntryPoint::GLBlendFunciOES:
        case EntryPoint::GLBufferData:
        case EntryPoint::GLBufferSubData:
        case EntryPoint::GLCheckFramebufferStatus:
        case EntryPoint::GLClearBufferfi:
        case EntryPoint::GLClearBufferfv:
        case EntryPoint::GLClearBufferiv:
        case EntryPoint::GLClearBufferuiv:
        case EntryPoint::GLColorMaski:
        case EntryPoint::GLColorMaskiEXT:
        case EntryPoint::GLColorMaskiOES:
        case EntryPoint::GLCullFace:
        case EntryPoint::GLDepthFunc:
        case EntryPoint::GLDepthMask:
        case EntryPoint::GLDepthRangef:
        case EntryPoint::GLDisable:
        case EntryPoint::GLDisableVertexAttribArray:
        case EntryPoint::GLDisablei:
        case EntryPoint::GLDisableiEXT:
        case EntryPoint::GLDisableiOES:
        case EntryPoint::GLDispatchComputeIndirect:
        case EntryPoint::GLDrawArrays:
        case EntryPoint::GLDrawArraysIndirect:
        case EntryPoint::GLDrawArraysInstanced:
        case EntryPoint::GLDrawArraysInstancedANGLE:
        case EntryPoint::GLDrawArraysInstancedBaseInstanceANGLE:
        case EntryPoint::GLDrawArraysInstancedBaseInstanceEXT:
        case EntryPoint::GLDrawArraysInstancedEXT:
        case EntryPoint::GLDrawElements:
        case EntryPoint::GLDrawElementsBaseVertex:
        case EntryPoint::GLDrawElementsBaseVertexEXT:
        case EntryPoint::GLDrawElementsBaseVertexOES:
        case EntryPoint::GLDrawElementsIndirect:
        case EntryPoint::GLDrawElementsInstanced:
        case EntryPoint::GLDrawElementsInstancedANGLE:
        case EntryPoint::GLDrawElementsInstancedBaseInstanceEXT:
        case EntryPoint::GLDrawElementsInstancedBaseVertex:
        case EntryPoint::GLDrawElementsInstancedBaseVertexBaseInstanceANGLE:
        case EntryPoint::GLDrawElementsInstancedBaseVertexBaseInstanceEXT:
        case EntryPoint::GLDrawElementsInstancedBaseVertexEXT:
        case EntryPoint::GLDrawElementsInstancedBaseVertexOES:
        case EntryPoint::GLDrawElementsInstancedEXT:
        case EntryPoint::GLDrawRangeElements:
        case EntryPoint::GLDrawRangeElementsBaseVertex:
        case EntryPoint::GLDrawRangeElementsBaseVertexEXT:
        case EntryPoint::GLDrawRangeElementsBaseVertexOES:
        case EntryPoint::GLEnable:
        case EntryPoint::GLEnableClientState:
        case EntryPoint::GLEnableVertexAttribArray:
        case EntryPoint::GLEnablei:
        case EntryPoint::GLEnableiEXT:
        case EntryPoint::GLEnableiOES:
        case EntryPoint::GLEndPixelLocalStorageANGLE:
        case EntryPoint::GLFrontFace:
        case EntryPoint::GLGetActiveAttrib:
        case EntryPoint::GLGetActiveUniform:
        case EntryPoint::GLGetActiveUniformBlockName:
        case EntryPoint::GLGetActiveUniformBlockiv:
        case EntryPoint::GLGetActiveUniformBlockivRobustANGLE:
        case EntryPoint::GLGetActiveUniformsiv:
        case EntryPoint::GLGetAttachedShaders:
        case EntryPoint::GLGetAttribLocation:
        case EntryPoint::GLGetBooleani_v:
        case EntryPoint::GLGetBooleani_vRobustANGLE:
        case EntryPoint::GLGetBooleanv:
        case EntryPoint::GLGetBooleanvRobustANGLE:
        case EntryPoint::GLGetBufferParameteri64v:
        case EntryPoint::GLGetBufferParameteri64vRobustANGLE:
        case EntryPoint::GLGetBufferParameteriv:
        case EntryPoint::GLGetBufferParameterivRobustANGLE:
        case EntryPoint::GLGetBufferPointerv:
        case EntryPoint::GLGetBufferPointervOES:
        case EntryPoint::GLGetBufferPointervRobustANGLE:
        case EntryPoint::GLGetClipPlanef:
        case EntryPoint::GLGetClipPlanex:
        case EntryPoint::GLGetCompressedTexImageANGLE:
        case EntryPoint::GLGetDebugMessageLog:
        case EntryPoint::GLGetDebugMessageLogKHR:
        case EntryPoint::GLGetError:
        case EntryPoint::GLGetFenceivNV:
        case EntryPoint::GLGetFixedv:
        case EntryPoint::GLGetFloatv:
        case EntryPoint::GLGetFloatvRobustANGLE:
        case EntryPoint::GLGetFragDataIndexEXT:
        case EntryPoint::GLGetFragDataLocation:
        case EntryPoint::GLGetFramebufferAttachmentParameteriv:
        case EntryPoint::GLGetFramebufferAttachmentParameterivOES:
        case EntryPoint::GLGetFramebufferAttachmentParameterivRobustANGLE:
        case EntryPoint::GLGetFramebufferParameteriv:
        case EntryPoint::GLGetFramebufferParameterivMESA:
        case EntryPoint::GLGetFramebufferParameterivRobustANGLE:
        case EntryPoint::GLGetGraphicsResetStatus:
        case EntryPoint::GLGetGraphicsResetStatusEXT:
        case EntryPoint::GLGetInteger64i_v:
        case EntryPoint::GLGetInteger64i_vRobustANGLE:
        case EntryPoint::GLGetInteger64v:
        case EntryPoint::GLGetInteger64vEXT:
        case EntryPoint::GLGetInteger64vRobustANGLE:
        case EntryPoint::GLGetIntegeri_v:
        case EntryPoint::GLGetIntegeri_vRobustANGLE:
        case EntryPoint::GLGetIntegerv:
        case EntryPoint::GLGetIntegervRobustANGLE:
        case EntryPoint::GLGetInternalformativ:
        case EntryPoint::GLGetInternalformativRobustANGLE:
        case EntryPoint::GLGetLightfv:
        case EntryPoint::GLGetLightxv:
        case EntryPoint::GLGetMaterialfv:
        case EntryPoint::GLGetMaterialxv:
        case EntryPoint::GLGetMemoryObjectParameterivEXT:
        case EntryPoint::GLGetMultisamplefv:
        case EntryPoint::GLGetMultisamplefvANGLE:
        case EntryPoint::GLGetMultisamplefvRobustANGLE:
        case EntryPoint::GLGetObjectLabel:
        case EntryPoint::GLGetObjectLabelEXT:
        case EntryPoint::GLGetObjectLabelKHR:
        case EntryPoint::GLGetObjectPtrLabel:
        case EntryPoint::GLGetObjectPtrLabelKHR:
        case EntryPoint::GLGetPerfMonitorCounterDataAMD:
        case EntryPoint::GLGetPerfMonitorCounterInfoAMD:
        case EntryPoint::GLGetPerfMonitorCounterStringAMD:
        case EntryPoint::GLGetPerfMonitorCountersAMD:
        case EntryPoint::GLGetPerfMonitorGroupStringAMD:
        case EntryPoint::GLGetPerfMonitorGroupsAMD:
        case EntryPoint::GLGetPointerv:
        case EntryPoint::GLGetPointervKHR:
        case EntryPoint::GLGetPointervRobustANGLERobustANGLE:
        case EntryPoint::GLGetProgramBinary:
        case EntryPoint::GLGetProgramBinaryOES:
        case EntryPoint::GLGetProgramInfoLog:
        case EntryPoint::GLGetProgramInterfaceiv:
        case EntryPoint::GLGetProgramInterfaceivRobustANGLE:
        case EntryPoint::GLGetProgramPipelineInfoLog:
        case EntryPoint::GLGetProgramPipelineInfoLogEXT:
        case EntryPoint::GLGetProgramPipelineiv:
        case EntryPoint::GLGetProgramPipelineivEXT:
        case EntryPoint::GLGetProgramResourceIndex:
        case EntryPoint::GLGetProgramResourceLocation:
        case EntryPoint::GLGetProgramResourceLocationIndexEXT:
        case EntryPoint::GLGetProgramResourceName:
        case EntryPoint::GLGetProgramResourceiv:
        case EntryPoint::GLGetProgramiv:
        case EntryPoint::GLGetProgramivRobustANGLE:
        case EntryPoint::GLGetQueryObjecti64vEXT:
        case EntryPoint::GLGetQueryObjecti64vRobustANGLE:
        case EntryPoint::GLGetQueryObjectivEXT:
        case EntryPoint::GLGetQueryObjectivRobustANGLE:
        case EntryPoint::GLGetQueryObjectui64vEXT:
        case EntryPoint::GLGetQueryObjectui64vRobustANGLE:
        case EntryPoint::GLGetQueryObjectuiv:
        case EntryPoint::GLGetQueryObjectuivEXT:
        case EntryPoint::GLGetQueryObjectuivRobustANGLE:
        case EntryPoint::GLGetQueryiv:
        case EntryPoint::GLGetQueryivEXT:
        case EntryPoint::GLGetQueryivRobustANGLE:
        case EntryPoint::GLGetRenderbufferImageANGLE:
        case EntryPoint::GLGetRenderbufferParameteriv:
        case EntryPoint::GLGetRenderbufferParameterivOES:
        case EntryPoint::GLGetRenderbufferParameterivRobustANGLE:
        case EntryPoint::GLGetSamplerParameterIiv:
        case EntryPoint::GLGetSamplerParameterIivEXT:
        case EntryPoint::GLGetSamplerParameterIivOES:
        case EntryPoint::GLGetSamplerParameterIivRobustANGLE:
        case EntryPoint::GLGetSamplerParameterIuiv:
        case EntryPoint::GLGetSamplerParameterIuivEXT:
        case EntryPoint::GLGetSamplerParameterIuivOES:
        case EntryPoint::GLGetSamplerParameterIuivRobustANGLE:
        case EntryPoint::GLGetSamplerParameterfv:
        case EntryPoint::GLGetSamplerParameterfvRobustANGLE:
        case EntryPoint::GLGetSamplerParameteriv:
        case EntryPoint::GLGetSamplerParameterivRobustANGLE:
        case EntryPoint::GLGetSemaphoreParameterui64vEXT:
        case EntryPoint::GLGetShaderInfoLog:
        case EntryPoint::GLGetShaderPrecisionFormat:
        case EntryPoint::GLGetShaderSource:
        case EntryPoint::GLGetShaderiv:
        case EntryPoint::GLGetShaderivRobustANGLE:
        case EntryPoint::GLGetString:
        case EntryPoint::GLGetStringi:
        case EntryPoint::GLGetSynciv:
        case EntryPoint::GLGetTexEnvfv:
        case EntryPoint::GLGetTexEnviv:
        case EntryPoint::GLGetTexEnvxv:
        case EntryPoint::GLGetTexGenfvOES:
        case EntryPoint::GLGetTexGenivOES:
        case EntryPoint::GLGetTexGenxvOES:
        case EntryPoint::GLGetTexImageANGLE:
        case EntryPoint::GLGetTexLevelParameterfv:
        case EntryPoint::GLGetTexLevelParameterfvANGLE:
        case EntryPoint::GLGetTexLevelParameterfvRobustANGLE:
        case EntryPoint::GLGetTexLevelParameteriv:
        case EntryPoint::GLGetTexLevelParameterivANGLE:
        case EntryPoint::GLGetTexLevelParameterivRobustANGLE:
        case EntryPoint::GLGetTexParameterIiv:
        case EntryPoint::GLGetTexParameterIivEXT:
        case EntryPoint::GLGetTexParameterIivOES:
        case EntryPoint::GLGetTexParameterIivRobustANGLE:
        case EntryPoint::GLGetTexParameterIuiv:
        case EntryPoint::GLGetTexParameterIuivEXT:
        case EntryPoint::GLGetTexParameterIuivOES:
        case EntryPoint::GLGetTexParameterIuivRobustANGLE:
        case EntryPoint::GLGetTexParameterfv:
        case EntryPoint::GLGetTexParameterfvRobustANGLE:
        case EntryPoint::GLGetTexParameteriv:
        case EntryPoint::GLGetTexParameterivRobustANGLE:
        case EntryPoint::GLGetTexParameterxv:
        case EntryPoint::GLGetTransformFeedbackVarying:
        case EntryPoint::GLGetTranslatedShaderSourceANGLE:
        case EntryPoint::GLGetUniformBlockIndex:
        case EntryPoint::GLGetUniformIndices:
        case EntryPoint::GLGetUniformLocation:
        case EntryPoint::GLGetUniformfv:
        case EntryPoint::GLGetUniformfvRobustANGLE:
        case EntryPoint::GLGetUniformiv:
        case EntryPoint::GLGetUniformivRobustANGLE:
        case EntryPoint::GLGetUniformuiv:
        case EntryPoint::GLGetUniformuivRobustANGLE:
        case EntryPoint::GLGetUnsignedBytei_vEXT:
        case EntryPoint::GLGetUnsignedBytevEXT:
        case EntryPoint::GLGetVertexAttribIiv:
        case EntryPoint::GLGetVertexAttribIivRobustANGLE:
        case EntryPoint::GLGetVertexAttribIuiv:
        case EntryPoint::GLGetVertexAttribIuivRobustANGLE:
        case EntryPoint::GLGetVertexAttribPointerv:
        case EntryPoint::GLGetVertexAttribPointervRobustANGLE:
        case EntryPoint::GLGetVertexAttribfv:
        case EntryPoint::GLGetVertexAttribfvRobustANGLE:
        case EntryPoint::GLGetVertexAttribiv:
        case EntryPoint::GLGetVertexAttribivRobustANGLE:
        case EntryPoint::GLGetnUniformfv:
        case EntryPoint::GLGetnUniformfvEXT:
        case EntryPoint::GLGetnUniformfvRobustANGLE:
        case EntryPoint::GLGetnUniformiv:
        case EntryPoint::GLGetnUniformivEXT:
        case EntryPoint::GLGetnUniformivRobustANGLE:
        case EntryPoint::GLGetnUniformuiv:
        case EntryPoint::GLGetnUniformuivRobustANGLE:
        case EntryPoint::GLPixelLocalStorageBarrierANGLE:
        case EntryPoint::GLSamplerParameterIiv:
        case EntryPoint::GLSamplerParameterIivEXT:
        case EntryPoint::GLSamplerParameterIivOES:
        case EntryPoint::GLSamplerParameterIivRobustANGLE:
        case EntryPoint::GLSamplerParameterIuiv:
        case EntryPoint::GLSamplerParameterIuivEXT:
        case EntryPoint::GLSamplerParameterIuivOES:
        case EntryPoint::GLSamplerParameterIuivRobustANGLE:
        case EntryPoint::GLSamplerParameterf:
        case EntryPoint::GLSamplerParameterfv:
        case EntryPoint::GLSamplerParameterfvRobustANGLE:
        case EntryPoint::GLSamplerParameteri:
        case EntryPoint::GLSamplerParameteriv:
        case EntryPoint::GLSamplerParameterivRobustANGLE:
        case EntryPoint::GLScissor:
        case EntryPoint::GLStencilFunc:
        case EntryPoint::GLStencilFuncSeparate:
        case EntryPoint::GLStencilMask:
        case EntryPoint::GLStencilMaskSeparate:
        case EntryPoint::GLStencilOp:
        case EntryPoint::GLStencilOpSeparate:
        case EntryPoint::GLTexParameterIiv:
        case EntryPoint::GLTexParameterIivEXT:
        case EntryPoint::GLTexParameterIivOES:
        case EntryPoint::GLTexParameterIivRobustANGLE:
        case EntryPoint::GLTexParameterIuiv:
        case EntryPoint::GLTexParameterIuivEXT:
        case EntryPoint::GLTexParameterIuivOES:
        case EntryPoint::GLTexParameterIuivRobustANGLE:
        case EntryPoint::GLTexParameterf:
        case EntryPoint::GLTexParameterfv:
        case EntryPoint::GLTexParameterfvRobustANGLE:
        case EntryPoint::GLTexParameteri:
        case EntryPoint::GLTexParameteriv:
        case EntryPoint::GLTexParameterivRobustANGLE:
        case EntryPoint::GLTexParameterx:
        case EntryPoint::GLTexParameterxv:
        case EntryPoint::GLUniform1f:
        case EntryPoint::GLUniform1fv:
        case EntryPoint::GLUniform1i:
        case EntryPoint::GLUniform1iv:
        case EntryPoint::GLUniform1ui:
        case EntryPoint::GLUniform1uiv:
        case EntryPoint::GLUniform2f:
        case EntryPoint::GLUniform2fv:
        case EntryPoint::GLUniform2i:
        case EntryPoint::GLUniform2iv:
        case EntryPoint::GLUniform2ui:
        case EntryPoint::GLUniform2uiv:
        case EntryPoint::GLUniform3f:
        case EntryPoint::GLUniform3fv:
        case EntryPoint::GLUniform3i:
        case EntryPoint::GLUniform3iv:
        case EntryPoint::GLUniform3ui:
        case EntryPoint::GLUniform3uiv:
        case EntryPoint::GLUniform4f:
        case EntryPoint::GLUniform4fv:
        case EntryPoint::GLUniform4i:
        case EntryPoint::GLUniform4iv:
        case EntryPoint::GLUniform4ui:
        case EntryPoint::GLUniform4uiv:
        case EntryPoint::GLUniformBlockBinding:
        case EntryPoint::GLUniformMatrix2fv:
        case EntryPoint::GLUniformMatrix2x3fv:
        case EntryPoint::GLUniformMatrix2x4fv:
        case EntryPoint::GLUniformMatrix3fv:
        case EntryPoint::GLUniformMatrix3x2fv:
        case EntryPoint::GLUniformMatrix3x4fv:
        case EntryPoint::GLUniformMatrix4fv:
        case EntryPoint::GLUniformMatrix4x2fv:
        case EntryPoint::GLUniformMatrix4x3fv:
        case EntryPoint::GLUnmapBuffer:
        case EntryPoint::GLUseProgram:
        case EntryPoint::GLValidateProgram:
        case EntryPoint::GLVertexAttrib1f:
        case EntryPoint::GLVertexAttrib1fv:
        case EntryPoint::GLVertexAttrib2f:
        case EntryPoint::GLVertexAttrib2fv:
        case EntryPoint::GLVertexAttrib3f:
        case EntryPoint::GLVertexAttrib3fv:
        case EntryPoint::GLVertexAttrib4f:
        case EntryPoint::GLVertexAttrib4fv:
        case EntryPoint::GLVertexAttribBinding:
        case EntryPoint::GLVertexAttribDivisor:
        case EntryPoint::GLVertexAttribDivisorANGLE:
        case EntryPoint::GLVertexAttribDivisorEXT:
        case EntryPoint::GLVertexAttribFormat:
        case EntryPoint::GLVertexAttribI4i:
        case EntryPoint::GLVertexAttribI4iv:
        case EntryPoint::GLVertexAttribI4ui:
        case EntryPoint::GLVertexAttribI4uiv:
        case EntryPoint::GLVertexAttribIFormat:
        case EntryPoint::GLVertexAttribIPointer:
        case EntryPoint::GLVertexAttribPointer:
        case EntryPoint::GLViewport:
            return true;
        default:
            return false;
    }
}

template <angle::EntryPoint EP>
constexpr typename std::enable_if<IsAllowedWhenPixelLocalStorageActive(EP), bool>::type
ValidateEntryPoint(const Context *context)
{
    return true;
}

template <angle::EntryPoint EP>
typename std::enable_if<!IsAllowedWhenPixelLocalStorageActive(EP), bool>::type ValidateEntryPoint(
    const Context *context)
{
    if (context->getState().getPixelLocalStorageActive())
    {
        context->validationError(EP, GL_INVALID_OPERATION, err::kPLSActive);
        return false;
    }
    return true;
}
}  // namespace gl

namespace egl
{
inline int CID(EGLDisplay display, EGLContext context)
{
    auto *displayPtr = reinterpret_cast<const egl::Display *>(display);
    if (!Display::isValidDisplay(displayPtr))
    {
        return -1;
    }
    auto *contextPtr = reinterpret_cast<const gl::Context *>(context);
    if (!displayPtr->isValidContext(contextPtr))
    {
        return -1;
    }
    return gl::CID(contextPtr);
}
}  // namespace egl

#endif  // LIBANGLE_ENTRY_POINT_UTILS_H_
