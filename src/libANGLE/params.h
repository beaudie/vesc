//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// params:
//   Parameter wrapper structs for OpenGL ES. These helpers cache re-used values
//   in entry point routines.

#ifndef LIBANGLE_PARAMS_H_
#define LIBANGLE_PARAMS_H_

#include "angle_gl.h"
#include "common/Optional.h"
#include "common/PackedEnums.h"
#include "common/angleutils.h"
#include "common/mathutil.h"
#include "libANGLE/Error.h"
#include "libANGLE/entry_points_enum_autogen.h"

namespace gl
{
class Context;

template <EntryPoint EP>
struct EntryPointParam;

template <EntryPoint EP>
using EntryPointParamType = typename EntryPointParam<EP>::Type;

class ParamTypeInfo
{
  public:
    constexpr ParamTypeInfo(const char *selfClass, const ParamTypeInfo *parentType)
        : mSelfClass(selfClass), mParentTypeInfo(parentType)
    {
    }

    constexpr bool hasDynamicType(const ParamTypeInfo &typeInfo) const
    {
        return mSelfClass == typeInfo.mSelfClass ||
               (mParentTypeInfo && mParentTypeInfo->hasDynamicType(typeInfo));
    }

    constexpr bool isValid() const { return mSelfClass != nullptr; }

  private:
    const char *mSelfClass;
    const ParamTypeInfo *mParentTypeInfo;
};

#define ANGLE_PARAM_TYPE_INFO(NAME, BASENAME) \
    static constexpr ParamTypeInfo TypeInfo = {#NAME, &BASENAME::TypeInfo}

class ParamsBase : angle::NonCopyable
{
  public:
    ParamsBase(Context *context, ...){};

    template <EntryPoint EP, typename... ArgsT>
    static void Factory(EntryPointParamType<EP> *objBuffer, ArgsT... args);

    static constexpr ParamTypeInfo TypeInfo = {nullptr, nullptr};
};

// static
template <EntryPoint EP, typename... ArgsT>
ANGLE_INLINE void ParamsBase::Factory(EntryPointParamType<EP> *objBuffer, ArgsT... args)
{
    new (objBuffer) EntryPointParamType<EP>(args...);
}

// Helper class that encompasses draw call parameters. It uses the HasIndexRange
// helper class to only pull index range info lazily to prevent unnecessary readback.
// It is also used when syncing state for the VertexArray implementation, since the
// vertex and index buffer updates depend on draw call parameters.
class DrawCallParams final : angle::NonCopyable
{
  public:
    // Called by DrawArrays.
    DrawCallParams(GLenum mode, GLint firstVertex, GLsizei vertexCount)
    {
        memset(this, 0, sizeof(DrawCallParams));
        mMode        = FromGLenum<PrimitiveMode>(mode);
        mFirstVertex = firstVertex;
        mVertexCount = vertexCount;
    }

    // Called by DrawArraysInstanced.
    DrawCallParams(GLenum mode, GLint firstVertex, GLsizei vertexCount, GLsizei instances)
    {
        memset(this, 0, sizeof(DrawCallParams));
        mMode        = FromGLenum<PrimitiveMode>(mode);
        mFirstVertex = firstVertex;
        mVertexCount = vertexCount;
        mInstances   = instances;
    }

    // Called by DrawElements and DrawRangElements.
    DrawCallParams(GLenum mode, GLint indexCount, GLenum type, const void *indices)
    {
        memset(this, 0, sizeof(DrawCallParams));
        mMode       = FromGLenum<PrimitiveMode>(mode);
        mIndexCount = indexCount;
        mType       = type;
        mIndices    = indices;
    }

    // Called by DrawElementsInstanced.
    DrawCallParams(GLenum mode,
                   GLint indexCount,
                   GLenum type,
                   const void *indices,
                   GLsizei instances)
    {
        memset(this, 0, sizeof(DrawCallParams));
        mMode       = FromGLenum<PrimitiveMode>(mode);
        mIndexCount = indexCount;
        mType       = type;
        mIndices    = indices;
        mInstances  = instances;
    }

    // Called internally by DrawElementsIndirect on readback.
    DrawCallParams(GLenum mode,
                   GLint indexCount,
                   GLenum type,
                   const void *indices,
                   GLint baseVertex,
                   GLsizei instances)
    {
        memset(this, 0, sizeof(DrawCallParams));
        mMode       = FromGLenum<PrimitiveMode>(mode);
        mIndexCount = indexCount;
        mType       = type;
        mIndices    = indices;
        mBaseVertex = baseVertex;
        mInstances  = instances;
    }

    // Called by DrawArraysIndirect.
    DrawCallParams(GLenum mode, const void *indirect)
    {
        memset(this, 0, sizeof(DrawCallParams));
        mMode     = FromGLenum<PrimitiveMode>(mode);
        mIndirect = indirect;
    }

    // Called by DrawElementsIndirect.
    DrawCallParams(GLenum mode, GLenum type, const void *indirect)
    {
        memset(this, 0, sizeof(DrawCallParams));
        mMode     = FromGLenum<PrimitiveMode>(mode);
        mType     = type;
        mIndirect = indirect;
    }

    PrimitiveMode mode() const { return mMode; }

    // This value is the sum of 'baseVertex' and the first indexed vertex for DrawElements calls.
    GLint firstVertex() const
    {
        // In some cases we can know the first vertex will be fixed at zero, if we're on the "fast
        // path". In these cases the index range is not resolved. If the first vertex is not zero,
        // however, then it must be because the index range is resolved. This only applies to the
        // D3D11 back-end currently.
        ASSERT(mFirstVertex == 0 || (!isDrawElements() || mIndexRange.valid()));
        return mFirstVertex;
    }

    size_t vertexCount() const
    {
        ASSERT(!isDrawElements() || mIndexRange.valid());
        return mVertexCount;
    }

    GLsizei indexCount() const
    {
        ASSERT(isDrawElements());
        return mIndexCount;
    }

    GLint baseVertex() const { return mBaseVertex; }

    GLenum type() const
    {
        ASSERT(isDrawElements());
        return mType;
    }

    const void *indices() const { return mIndices; }

    GLsizei instances() const { return mInstances; }

    const void *indirect() const { return mIndirect; }

    Error ensureIndexRangeResolved(const Context *context) const;
    bool isDrawElements() const { return (mType != GL_NONE); }

    bool isDrawIndirect() const;

    // ensureIndexRangeResolved must be called first.
    const IndexRange &getIndexRange() const;

    template <typename T>
    T getClampedVertexCount() const;

  private:
    PrimitiveMode mMode;
    mutable Optional<IndexRange> mIndexRange;
    mutable GLint mFirstVertex;
    mutable size_t mVertexCount;
    GLint mIndexCount;
    GLint mBaseVertex;
    GLenum mType;
    const void *mIndices;
    GLsizei mInstances;
    const void *mIndirect;
};

template <typename T>
T DrawCallParams::getClampedVertexCount() const
{
    constexpr size_t kMax = static_cast<size_t>(std::numeric_limits<T>::max());
    return static_cast<T>(mVertexCount > kMax ? kMax : mVertexCount);
}

// Entry point funcs essentially re-map different entry point parameter arrays into
// the format the parameter type class expects. For example, for HasIndexRange, for the
// various indexed draw calls, they drop parameters that aren't useful and re-arrange
// the rest.
#define ANGLE_ENTRY_POINT_FUNC(NAME, CLASS, ...)    \
    \
template<> struct EntryPointParam<EntryPoint::NAME> \
    {                                               \
        using Type = CLASS;                         \
    };                                              \
    \
template<> inline void CLASS::Factory<EntryPoint::NAME>(__VA_ARGS__)

#undef ANGLE_ENTRY_POINT_FUNC

template <EntryPoint EP>
struct EntryPointParam
{
    using Type = ParamsBase;
};

// A template struct for determining the default value to return for each entry point.
template <EntryPoint EP, typename ReturnType>
struct DefaultReturnValue;

// Default return values for each basic return type.
template <EntryPoint EP>
struct DefaultReturnValue<EP, GLint>
{
    static constexpr GLint kValue = -1;
};

// This doubles as the GLenum return value.
template <EntryPoint EP>
struct DefaultReturnValue<EP, GLuint>
{
    static constexpr GLuint kValue = 0;
};

template <EntryPoint EP>
struct DefaultReturnValue<EP, GLboolean>
{
    static constexpr GLboolean kValue = GL_FALSE;
};

// Catch-all rules for pointer types.
template <EntryPoint EP, typename PointerType>
struct DefaultReturnValue<EP, const PointerType *>
{
    static constexpr const PointerType *kValue = nullptr;
};

template <EntryPoint EP, typename PointerType>
struct DefaultReturnValue<EP, PointerType *>
{
    static constexpr PointerType *kValue = nullptr;
};

// Overloaded to return invalid index
template <>
struct DefaultReturnValue<EntryPoint::GetUniformBlockIndex, GLuint>
{
    static constexpr GLuint kValue = GL_INVALID_INDEX;
};

// Specialized enum error value.
template <>
struct DefaultReturnValue<EntryPoint::ClientWaitSync, GLenum>
{
    static constexpr GLenum kValue = GL_WAIT_FAILED;
};

// glTestFenceNV should still return TRUE for an invalid fence.
template <>
struct DefaultReturnValue<EntryPoint::TestFenceNV, GLboolean>
{
    static constexpr GLboolean kValue = GL_TRUE;
};

template <EntryPoint EP, typename ReturnType>
constexpr ANGLE_INLINE ReturnType GetDefaultReturnValue()
{
    return DefaultReturnValue<EP, ReturnType>::kValue;
}

}  // namespace gl

#endif  // LIBANGLE_PARAMS_H_
