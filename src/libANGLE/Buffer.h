//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Buffer.h: Defines the gl::Buffer class, representing storage of vertex and/or
// index data. Implements GL buffer objects and related functionality.
// [OpenGL ES 2.0.24] section 2.9 page 21.

#ifndef LIBANGLE_BUFFER_H_
#define LIBANGLE_BUFFER_H_

#include <stdint.h>

#include "common/angleutils.h"
#include "common/mathutil.h"
#include "libANGLE/Error.h"
#include "libANGLE/RefCountObject.h"

namespace rx
{
class BufferImpl;
};

namespace gl
{

class Buffer : public RefCountObject
{
  public:
    Buffer(rx::BufferImpl *impl, GLuint id);

    virtual ~Buffer();

    Error setData(size_t size, const uint8_t *data, GLenum usage);
    Error setSubData(size_t offset, size_t size, const uint8_t *data);
    Error copySubData(const Buffer *source, size_t sourceOffset, size_t destOffset, size_t size);
    Error map(GLbitfield access);
    Error mapRange(size_t offset, size_t length, GLbitfield access);
    Error unmap(GLboolean *result);

    Error getIndexRange(GLenum type, size_t offset, size_t count, rx::RangeUI *outRange) const;

    GLenum getUsage() const { return mUsage; }
    GLint getAccessFlags() const {  return mAccessFlags; }
    GLboolean isMapped() const { return mMapped; }
    GLvoid *getMapPointer() const { return mMapPointer; }
    GLint64 getMapOffset() const { return mMapOffset; }
    GLint64 getMapLength() const { return mMapLength; }
    GLint64 getSize() const { return mSize; }

    rx::BufferImpl *getImplementation() const { return mBuffer; }

  private:
    DISALLOW_COPY_AND_ASSIGN(Buffer);

    rx::BufferImpl *mBuffer;

    GLenum mUsage;
    GLint64 mSize;
    GLint mAccessFlags;
    GLboolean mMapped;
    GLvoid *mMapPointer;
    GLint64 mMapOffset;
    GLint64 mMapLength;
};

}

#endif   // LIBANGLE_BUFFER_H_
