//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// frame_capture_utils.h:
//   ANGLE frame capture utils interface.
//
#ifndef FRAME_CAPTURE_UTILS_H_
#define FRAME_CAPTURE_UTILS_H_

#include <vector>

#include "common/Color.h"
#include "libANGLE/BinaryStream.h"
#include "libANGLE/Error.h"

namespace gl
{
template <class ObjectType>
class BindingPointer;
class Context;
template <class ObjectType>
class OffsetBindingPointer;
}  // namespace gl

typedef unsigned int GLenum;

namespace angle
{
class MemoryBuffer;
class ScratchBuffer;

Result SerializeContext(gl::BinaryOutputStream *bos, const gl::Context *context);

template <typename T>
void SerializeColor(gl::BinaryOutputStream *bos, const Color<T> &color)
{
    bos->writeInt(color.red);
    bos->writeInt(color.green);
    bos->writeInt(color.blue);
    bos->writeInt(color.alpha);
}

template <class ObjectType>
void SerializeOffsetBindingPointerVector(
    gl::BinaryOutputStream *bos,
    const std::vector<gl::OffsetBindingPointer<ObjectType>> &offsetBindingPointerVector)
{
    for (size_t i = 0; i < offsetBindingPointerVector.size(); i++)
    {
        bos->writeInt(offsetBindingPointerVector[i].id().value);
        bos->writeInt(offsetBindingPointerVector[i].getOffset());
        bos->writeInt(offsetBindingPointerVector[i].getSize());
    }
}

template <class ObjectType>
void SerializeBindingPointerVector(
    gl::BinaryOutputStream *bos,
    const std::vector<gl::BindingPointer<ObjectType>> &bindingPointerVector)
{
    for (size_t i = 0; i < bindingPointerVector.size(); i++)
    {
        bos->writeInt(bindingPointerVector[i].id().value);
    }
}

}  // namespace angle
#endif  // FRAME_CAPTURE_UTILS_H_
