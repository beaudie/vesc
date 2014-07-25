#include "precompiled.h"
//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Buffer.cpp: Implements the gl::Buffer class, representing storage of vertex and/or
// index data. Implements GL buffer objects and related functionality.
// [OpenGL ES 2.0.24] section 2.9 page 21.

#include "libGLESv2/Buffer.h"
#include "libGLESv2/renderer/BufferImpl.h"
#include "libGLESv2/renderer/Renderer.h"

namespace gl
{

Buffer::Buffer(rx::BufferImpl *impl, GLuint id)
    : RefCountObject(id),
      mBuffer(impl),
      mUsage(GL_DYNAMIC_DRAW),
      mSize(0),
      mAccessFlags(0),
      mMapped(GL_FALSE),
      mMapPointer(NULL),
      mMapOffset(0),
      mMapLength(0)
{
}

Buffer::~Buffer()
{
    delete mBuffer;
}

Error Buffer::bufferData(const void *data, GLsizeiptr size, GLenum usage)
{
    Error error = mBuffer->setData(data, size, usage);
    if (!error.isError())
    {
        mUsage = usage;
        mSize = size;
    }
    return error;
}

Error Buffer::bufferSubData(const void *data, GLsizeiptr size, GLintptr offset)
{
    return mBuffer->setSubData(data, size, offset);
}

Error Buffer::copyBufferSubData(Buffer* source, GLintptr sourceOffset, GLintptr destOffset, GLsizeiptr size)
{
    return mBuffer->copySubData(source->getImplementation(), size, sourceOffset, destOffset);
}

Error Buffer::mapRange(GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    ASSERT(!mMapped);
    ASSERT(offset + length <= mSize);

    Error error = mBuffer->map(offset, length, access, &mMapPointer);
    if (!error.isError())
    {
        mMapped = GL_TRUE;
        mMapOffset = static_cast<GLint64>(offset);
        mMapLength = static_cast<GLint64>(length);
        mAccessFlags = static_cast<GLint>(access);
    }
    else
    {
        mMapPointer = NULL;
    }
    return error;
}

Error Buffer::unmap()
{
    ASSERT(mMapped);

    Error error = mBuffer->unmap();
    if (!error.isError())
    {
        mMapped = GL_FALSE;
        mMapPointer = NULL;
        mMapOffset = 0;
        mMapLength = 0;
        mAccessFlags = 0;
    }
    return error;
}

void Buffer::markTransformFeedbackUsage()
{
    // TODO: Only used by the DX11 backend. Refactor to a more appropriate place.
    mBuffer->markTransformFeedbackUsage();
}

}
