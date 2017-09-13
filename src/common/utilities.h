//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// utilities.h: Conversion functions and other utility routines.

#ifndef COMMON_UTILITIES_H_
#define COMMON_UTILITIES_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "angle_gl.h"
#include <string>
#include <math.h>

#include "common/mathutil.h"

namespace gl
{

int VariableComponentCount(GLenum type);
GLenum VariableComponentType(GLenum type);
size_t VariableComponentSize(GLenum type);
size_t VariableInternalSize(GLenum type);
size_t VariableExternalSize(GLenum type);
int VariableRowCount(GLenum type);
int VariableColumnCount(GLenum type);
bool IsSamplerType(GLenum type);
bool IsImageType(GLenum type);
bool IsAtomicCounterType(GLenum type);
bool IsOpaqueType(GLenum type);
GLenum SamplerTypeToTextureType(GLenum samplerType);
bool IsMatrixType(GLenum type);
GLenum TransposeMatrixType(GLenum type);
int VariableRegisterCount(GLenum type);
int MatrixRegisterCount(GLenum type, bool isRowMajorMatrix);
int MatrixComponentCount(GLenum type, bool isRowMajorMatrix);
int VariableSortOrder(GLenum type);
GLenum VariableBoolVectorType(GLenum type);

int AllocateFirstFreeBits(unsigned int *bits, unsigned int allocationSize, unsigned int bitsSize);

static const GLenum FirstCubeMapTextureTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
static const GLenum LastCubeMapTextureTarget = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
bool IsCubeMapTextureTarget(GLenum target);
size_t CubeMapTextureTargetToLayerIndex(GLenum target);
GLenum LayerIndexToCubeMapTextureTarget(size_t index);

// Parse the base resource name and array index.  Returns the base name of the resource.
// outSubscript is set to GL_INVALID_INDEX if the provided name is not an array or the array index
// is invalid.
std::string ParseResourceName(const std::string &name, size_t *outSubscript);

// Find the range of index values in the provided indices pointer.  Primitive restart indices are
// only counted in the range if primitive restart is disabled.
IndexRange ComputeIndexRange(GLenum indexType,
                             const GLvoid *indices,
                             size_t count,
                             bool primitiveRestartEnabled);

// Get the primitive restart index value for the given index type.
GLuint GetPrimitiveRestartIndex(GLenum indexType);

bool IsTriangleMode(GLenum drawMode);
bool IsIntegerFormat(GLenum unsizedFormat);

// [OpenGL ES 3.0.2] Section 2.3.1 page 14
// Data Conversion For State-Setting Commands
// Floating-point values are rounded to the nearest integer, instead of truncated, as done by static_cast.
template <typename outT> outT iround(GLfloat value) { return static_cast<outT>(value > 0.0f ? floor(value + 0.5f) : ceil(value - 0.5f)); }
template <typename outT> outT uiround(GLfloat value) { return static_cast<outT>(value + 0.5f); }

// Helper for converting arbitrary GL types to other GL types used in queries and state setting
// GLES 3.10 section 2.2.1, and 2.2.2
template <typename T>
inline bool isGLStateExternalType()
{
    return (std::is_same<T, GLfloat>::value || std::is_same<T, GLboolean>::value ||
            std::is_same<T, GLint>::value || std::is_same<T, GLuint>::value ||
            std::is_same<T, GLint64>::value);
}

template <typename T>
inline bool isGLStateInternalType()
{
    return (std::is_same<T, float>::value || std::is_same<T, bool>::value ||
            std::is_same<T, int32_t>::value || std::is_same<T, uint32_t>::value ||
            std::is_same<T, int64_t>::value || std::is_same<T, uint64_t>::value);
}

// For state-query commands.
template <typename ReturnType, typename ParamType>
struct GLTypeConvertFromImpl
{
    static ReturnType ConvertFrom(ParamType param)
    {
        DCHECK(isGLStateExternalType<ReturnType>() && isGLStateInternalType<ParamType>());
        return static_cast<ReturnType>(param);
    }
};

template <typename ParamType>
struct GLTypeConvertFromImpl<GLboolean, ParamType>
{
    static GLboolean ConvertFrom(ParamType param)
    {
        DCHECK(isGLStateInternalType<ParamType>());
        return (param == 0 ? GL_FALSE : GL_TRUE);
    }
};

template <typename ReturnType>
struct GLTypeConvertFromImpl<ReturnType, float>
{
    static ReturnType ConvertFrom(float param)
    {
        DCHECK(isGLStateExternalType<ReturnType>() && std::is_integral<ReturnType>::value);
        return clampCast<ReturnType>(std::round(param));
    }
};

template <>
struct GLTypeConvertFromImpl<GLboolean, float>
{
    static GLboolean ConvertFrom(float param) { return (param == 0.0 ? GL_FALSE : GL_TRUE); }
};

template <>
struct GLTypeConvertFromImpl<GLfloat, float>
{
    static GLfloat ConvertFrom(float param) { return param; }
};

template <>
struct GLTypeConvertFromImpl<GLint, uint32_t>
{
    static GLint ConvertFrom(uint32_t param) { return rightClampCast<GLint>(param); }
};

template <>
struct GLTypeConvertFromImpl<GLint, uint64_t>
{
    static GLint ConvertFrom(uint64_t param) { return rightClampCast<GLint>(param); }
};

template <>
struct GLTypeConvertFromImpl<GLint, int64_t>
{
    static GLint ConvertFrom(int64_t param) { return clampCast<GLint>(param); }
};

template <>
struct GLTypeConvertFromImpl<GLuint, int32_t>
{
    static GLuint ConvertFrom(int32_t param) { return leftClampCast<GLuint>(param); }
};

template <>
struct GLTypeConvertFromImpl<GLuint, int64_t>
{
    static GLuint ConvertFrom(int64_t param) { return clampCast<GLuint>(param); }
};

template <>
struct GLTypeConvertFromImpl<GLuint, uint64_t>
{
    static GLuint ConvertFrom(uint64_t param) { return clampCast<GLuint>(param); }
};

template <>
struct GLTypeConvertFromImpl<GLint64, uint64_t>
{
    static GLint64 ConvertFrom(uint64_t param) { return rightClampCast<GLint64>(param); }
};

template <typename ReturnType, typename ParamType>
inline ReturnType ConvertFrom(ParamType param)
{
    return GLTypeConvertFromImpl<ReturnType, ParamType>::ConvertFrom(param);
}

// For state-setting commands.
template <typename ReturnType, typename ParamType>
struct GLTypeConvertToImpl
{
    static ReturnType ConvertTo(ParamType param)
    {
        DCHECK(isGLStateExternalType<ParamType>() && isGLStateInternalType<ReturnType>());
        return static_cast<ReturnType>(param);
    }
};

template <typename ReturnType>
struct GLTypeConvertToImpl<ReturnType, GLboolean>
{
    static ReturnType ConvertTo(GLboolean param)
    {
        DCHECK(isGLStateInternalType<ReturnType>());
        return static_cast<ReturnType>(param == GL_TRUE ? 1 : 0);
    }
};

template <>
struct GLTypeConvertToImpl<float, GLboolean>
{
    static float ConvertTo(float param) { return (param == GL_TRUE ? 1.0f : 0.0f); }
};

template <typename ReturnType>
struct GLTypeConvertToImpl<ReturnType, GLfloat>
{
    static ReturnType ConvertTo(GLfloat param)
    {
        DCHECK(isGLStateInternalType<ReturnType>() && std::is_integral<ReturnType>::value);
        return clampCast<ReturnType>(std::round(param));
    }
};

template <>
struct GLTypeConvertToImpl<bool, GLfloat>
{
    static bool ConvertTo(GLfloat param) { return param != 0.0; }
};

template <>
struct GLTypeConvertToImpl<float, GLfloat>
{
    static float ConvertTo(GLfloat param) { return param; }
};

template <>
struct GLTypeConvertToImpl<uint32_t, GLint>
{
    static uint32_t ConvertTo(GLint param) { return leftClampCast<uint32_t>(param); }
};

template <>
struct GLTypeConvertToImpl<uint64_t, GLint>
{
    static uint64_t ConvertTo(GLint param) { return leftClampCast<uint64_t>(param); }
};

template <>
struct GLTypeConvertToImpl<uint32_t, GLint64>
{
    static uint32_t ConvertTo(GLint64 param) { return clampCast<uint32_t>(param); }
};

template <>
struct GLTypeConvertToImpl<uint64_t, GLint64>
{
    static uint64_t ConvertTo(GLint64 param) { return leftClampCast<uint64_t>(param); }
};

template <>
struct GLTypeConvertToImpl<int32_t, GLint64>
{
    static int32_t ConvertTo(GLint64 param) { return clampCast<int32_t>(param); }
};

template <>
struct GLTypeConvertToImpl<int32_t, GLuint>
{
    static int32_t ConvertTo(GLuint param) { return rightClampCast<int32_t>(param); }
};

template <>
struct GLTypeConvertToImpl<int64_t, GLuint>
{
    static int64_t ConvertTo(GLuint param) { return rightClampCast<int64_t>(param); }
};

template <typename ReturnType, typename ParamType>
inline ReturnType ConvertTo(ParamType param)
{
    return GLTypeConvertToImpl<ReturnType, ParamType>::ConvertTo(param);
}

// GLES 3.10 Section 2.2.2
// When querying bitmask (such as SAMPLE_MASK_VALUE or STENCIL_WRITEMASK) with GetIntegerv, the
// mask value is treated as a signed integer, so that mask values with the high bit set will not be
// clamped when returned as signed integers.
inline GLint ConvertFromMaskValue(GLuint value)
{
    return static_cast<GLint>(value);
}

unsigned int ParseAndStripArrayIndex(std::string *name);

struct UniformTypeInfo final : angle::NonCopyable
{
    constexpr UniformTypeInfo(GLenum type,
                              GLenum componentType,
                              GLenum samplerTextureType,
                              GLenum transposedMatrixType,
                              GLenum boolVectorType,
                              int rowCount,
                              int columnCount,
                              int componentCount,
                              size_t componentSize,
                              size_t internalSize,
                              size_t externalSize,
                              bool isSampler,
                              bool isMatrixType,
                              bool isImageType)
        : type(type),
          componentType(componentType),
          samplerTextureType(samplerTextureType),
          transposedMatrixType(transposedMatrixType),
          boolVectorType(boolVectorType),
          rowCount(rowCount),
          columnCount(columnCount),
          componentCount(componentCount),
          componentSize(componentSize),
          internalSize(internalSize),
          externalSize(externalSize),
          isSampler(isSampler),
          isMatrixType(isMatrixType),
          isImageType(isImageType)
    {
    }

    GLenum type;
    GLenum componentType;
    GLenum samplerTextureType;
    GLenum transposedMatrixType;
    GLenum boolVectorType;
    int rowCount;
    int columnCount;
    int componentCount;
    size_t componentSize;
    size_t internalSize;
    size_t externalSize;
    bool isSampler;
    bool isMatrixType;
    bool isImageType;
};

const UniformTypeInfo &GetUniformTypeInfo(GLenum uniformType);

}  // namespace gl

namespace egl
{
static const EGLenum FirstCubeMapTextureTarget = EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR;
static const EGLenum LastCubeMapTextureTarget = EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_KHR;
bool IsCubeMapTextureTarget(EGLenum target);
size_t CubeMapTextureTargetToLayerIndex(EGLenum target);
EGLenum LayerIndexToCubeMapTextureTarget(size_t index);
bool IsTextureTarget(EGLenum target);
bool IsRenderbufferTarget(EGLenum target);
}

namespace egl_gl
{
GLenum EGLCubeMapTargetToGLCubeMapTarget(EGLenum eglTarget);
GLenum EGLImageTargetToGLTextureTarget(EGLenum eglTarget);
GLuint EGLClientBufferToGLObjectHandle(EGLClientBuffer buffer);
}

namespace gl_egl
{
EGLenum GLComponentTypeToEGLColorComponentType(GLenum glComponentType);
}  // namespace gl_egl

#if !defined(ANGLE_ENABLE_WINDOWS_STORE)
std::string getTempPath();
void writeFile(const char* path, const void* data, size_t size);
#endif

#if defined (ANGLE_PLATFORM_WINDOWS)
void ScheduleYield();
#endif

#endif  // COMMON_UTILITIES_H_
