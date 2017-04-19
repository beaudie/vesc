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
#include "common/angleutils.h"
#include "common/mathutil.h"
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
    constexpr ParamTypeInfo(EntryPoint selfType, const ParamTypeInfo *parentType)
        : mSelfType(selfType), mParentTypeInfo(parentType)
    {
    }

    constexpr bool hasDynamicType(EntryPoint dynamicType) const
    {
        return mSelfType == dynamicType ||
               (mParentTypeInfo && mParentTypeInfo->hasDynamicType(dynamicType));
    }

  private:
    EntryPoint mSelfType;
    const ParamTypeInfo *mParentTypeInfo;
};

class ParamsBase : angle::NonCopyable
{
  public:
    ParamsBase(Context *context, ...);

    static constexpr ParamTypeInfo TypeInfo = {EntryPoint::Invalid, nullptr};

    bool hasDynamicType(EntryPoint dynamicType) const
    {
        return mTypeInfo->hasDynamicType(dynamicType);
    }

  protected:
    const ParamTypeInfo *mTypeInfo;
    Context *mContext;
};

class DrawElementsParams : public ParamsBase
{
  public:
    DrawElementsParams(Context *context,
                       GLenum mode,
                       GLsizei count,
                       GLenum type,
                       const GLvoid *indices);

    const Optional<IndexRange> &getIndexRange() const;

    static constexpr ParamTypeInfo TypeInfo = {EntryPoint::DrawElements, nullptr};

  protected:
    GLenum mMode;
    GLsizei mCount;
    GLenum mType;
    const GLvoid *mIndices;

    // Cached stuff.
    mutable Optional<IndexRange> mIndexRange;
};

class DrawElementsInstancedParams : public DrawElementsParams
{
  public:
    DrawElementsInstancedParams(Context *context,
                                GLenum mode,
                                GLsizei count,
                                GLenum type,
                                const GLvoid *indices,
                                GLsizei instanceCount);

    static constexpr ParamTypeInfo TypeInfo = {EntryPoint::DrawElementsInstanced,
                                               &DrawElementsParams::TypeInfo};

  protected:
    GLsizei mInstanceCount;
};

class DrawRangeElementsParams : public DrawElementsParams
{
  public:
    DrawRangeElementsParams(Context *context,
                            GLenum mode,
                            GLuint start,
                            GLuint end,
                            GLsizei count,
                            GLenum type,
                            const GLvoid *indices);

    static constexpr ParamTypeInfo TypeInfo = {EntryPoint::DrawRangeElements,
                                               &DrawElementsParams::TypeInfo};

  protected:
    GLuint mStart;
    GLuint mEnd;
};

template <>
struct EntryPointParam<EntryPoint::DrawElements>
{
    using Type = DrawElementsParams;
};

template <>
struct EntryPointParam<EntryPoint::DrawElementsInstanced>
{
    using Type = DrawElementsInstancedParams;
};

template <>
struct EntryPointParam<EntryPoint::DrawElementsInstancedANGLE>
{
    using Type = DrawElementsInstancedParams;
};

template <>
struct EntryPointParam<EntryPoint::DrawRangeElements>
{
    using Type = DrawRangeElementsParams;
};

template <EntryPoint EP>
struct EntryPointParam
{
    using Type = ParamsBase;
};

}  // namespace gl

#endif  // LIBANGLE_PARAMS_H_
