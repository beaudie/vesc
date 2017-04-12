//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// params_gles_2_0:
//   Parameter wrapper structs for OpenGL ES 2.0. These helpers cache re-used values
//   in entry point routines.

#include "libANGLE/params_gles_2_0.h"

#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/VertexArray.h"

namespace gl
{

// static
constexpr ParamTypeInfo ParamsBase::TypeInfo;
constexpr ParamTypeInfo DrawElementsParams::TypeInfo;
constexpr ParamTypeInfo DrawElementsInstancedParams::TypeInfo;
constexpr ParamTypeInfo DrawRangeElementsParams::TypeInfo;

ParamsBase::ParamsBase(Context *context, ...) : mTypeInfo(&TypeInfo), mContext(context)
{}

DrawElementsParams::DrawElementsParams(Context *context,
                                       GLenum mode,
                                       GLsizei count,
                                       GLenum type,
                                       const GLvoid *indices)
    : ParamsBase(context), mMode(mode), mCount(count), mType(type), mIndices(indices)
{
    mTypeInfo = &DrawElementsParams::TypeInfo;
}

const Optional<IndexRange> &DrawElementsParams::getIndexRange() const
{
    if (mIndexRange.valid())
    {
        return mIndexRange;
    }

    const State &state = mContext->getGLState();

    const gl::VertexArray *vao     = state.getVertexArray();
    gl::Buffer *elementArrayBuffer = vao->getElementArrayBuffer().get();

    if (elementArrayBuffer)
    {
        uintptr_t offset = reinterpret_cast<uintptr_t>(mIndices);
        IndexRange indexRange;
        Error error =
            elementArrayBuffer->getIndexRange(mType, static_cast<size_t>(offset), mCount,
                                              state.isPrimitiveRestartEnabled(), &indexRange);
        if (error.isError())
        {
            mContext->handleError(error);
            return mIndexRange;
        }

        mIndexRange = indexRange;
    }
    else
    {
        mIndexRange = ComputeIndexRange(mType, mIndices, mCount, state.isPrimitiveRestartEnabled());
    }

    return mIndexRange;
}

DrawElementsInstancedParams::DrawElementsInstancedParams(Context *context,
                                                         GLenum mode,
                                                         GLsizei count,
                                                         GLenum type,
                                                         const GLvoid *indices,
                                                         GLsizei instanceCount)
    : DrawElementsParams(context, mode, count, type, indices), mInstanceCount(instanceCount)
{
    mTypeInfo = &DrawElementsInstancedParams::TypeInfo;
}

DrawRangeElementsParams::DrawRangeElementsParams(Context *context,
                                                 GLenum mode,
                                                 GLuint start,
                                                 GLuint end,
                                                 GLsizei count,
                                                 GLenum type,
                                                 const GLvoid *indices)
    : DrawElementsParams(context, mode, count, type, indices), mStart(start), mEnd(end)
{
    mTypeInfo = &DrawRangeElementsParams::TypeInfo;
}

}  // namespace gl
