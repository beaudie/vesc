//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BufferImpl.h: Defines the abstract rx::BufferImpl class.

#ifndef LIBANGLE_RENDERER_BUFFERIMPL_H_
#define LIBANGLE_RENDERER_BUFFERIMPL_H_

#include <stdint.h>

#include "common/angleutils.h"
#include "common/mathutil.h"
#include "libANGLE/Error.h"

namespace gl
{
class Buffer;
}

namespace rx
{

class BufferImpl
{
  public:
    virtual ~BufferImpl() { }

    virtual gl::Error setData(size_t size, const uint8_t *data, GLenum usage) = 0;
    virtual gl::Error setSubData(size_t offset, size_t size, const uint8_t *data) = 0;
    virtual gl::Error copySubData(const gl::Buffer *source, size_t sourceOffset, size_t destOffset, size_t size) = 0;
    virtual gl::Error map(GLbitfield access, GLvoid **mapPtr) = 0;
    virtual gl::Error mapRange(size_t offset, size_t length, GLbitfield access, GLvoid **mapPtr) = 0;
    virtual gl::Error unmap(GLboolean *result) = 0;

    virtual gl::Error getIndexRange(GLenum type, size_t offset, size_t count, RangeUI *outRange) const = 0;
};

}

#endif // LIBANGLE_RENDERER_BUFFERIMPL_H_
