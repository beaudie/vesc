//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BufferGL.cpp: Implements the class methods for BufferGL.

#include "libANGLE/renderer/gl/BufferGL.h"

#include "common/debug.h"
#include "common/utilities.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"

namespace rx
{

// Use the GL_COPY_READ_BUFFER binding when two buffers need to be bound simultaneously.
// GL_ELEMENT_ARRAY_BUFFER is supported on more versions but can modify the state of the currently
// bound VAO.  Two simultaneous buffer bindings are only needed for glCopyBufferSubData which also
// adds the GL_COPY_READ_BUFFER binding.
static const GLenum SourceBufferOperationTarget = GL_COPY_READ_BUFFER;

// Use the GL_ELEMENT_ARRAY_BUFFER binding for most operations since it's available on all
// supported GL versions and doesn't affect any current state when it changes.
static const GLenum DestBufferOperationTarget = GL_ARRAY_BUFFER;

BufferGL::BufferGL(const FunctionsGL *functions, StateManagerGL *stateManager)
    : BufferImpl(),
      mFunctions(functions),
      mStateManager(stateManager),
      mBufferID(0)
{
    ASSERT(mFunctions);
    ASSERT(mStateManager);

    mFunctions->genBuffers(1, &mBufferID);
}

BufferGL::~BufferGL()
{
    if (mBufferID)
    {
        mFunctions->deleteBuffers(1, &mBufferID);
        mBufferID = 0;
    }
}

gl::Error BufferGL::setData(size_t size, const uint8_t *data, GLenum usage)
{
    mStateManager->bindBuffer(DestBufferOperationTarget, mBufferID);
    mFunctions->bufferData(DestBufferOperationTarget, size, data, usage);

    mIndexRangeCache.clear();

    return gl::Error(GL_NO_ERROR);
}

gl::Error BufferGL::setSubData(size_t offset, size_t size, const uint8_t *data)
{
    mStateManager->bindBuffer(DestBufferOperationTarget, mBufferID);
    mFunctions->bufferSubData(DestBufferOperationTarget, offset, size, data);

    mIndexRangeCache.invalidateRange(offset, size);

    return gl::Error(GL_NO_ERROR);
}

gl::Error BufferGL::copySubData(const gl::Buffer *source, size_t sourceOffset, size_t destOffset, size_t size)
{
    const BufferGL *sourceGL = GetImplAs<BufferGL>(source);

    mStateManager->bindBuffer(DestBufferOperationTarget, mBufferID);
    mStateManager->bindBuffer(SourceBufferOperationTarget, sourceGL->getBufferID());

    mFunctions->copyBufferSubData(SourceBufferOperationTarget, DestBufferOperationTarget, sourceOffset, destOffset, size);

    mIndexRangeCache.invalidateRange(destOffset, size);

    return gl::Error(GL_NO_ERROR);
}

gl::Error BufferGL::map(GLbitfield access, GLvoid **mapPtr)
{
    mStateManager->bindBuffer(DestBufferOperationTarget, mBufferID);
    *mapPtr = mFunctions->mapBuffer(DestBufferOperationTarget, access);

    if ((access & GL_MAP_WRITE_BIT) != 0)
    {
        mIndexRangeCache.clear();
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error BufferGL::mapRange(size_t offset, size_t length, GLbitfield access, GLvoid **mapPtr)
{
    mStateManager->bindBuffer(DestBufferOperationTarget, mBufferID);
    *mapPtr = mFunctions->mapBufferRange(DestBufferOperationTarget, offset, length, access);

    if ((access & GL_MAP_WRITE_BIT) != 0)
    {
        mIndexRangeCache.invalidateRange(offset, length);
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error BufferGL::unmap(GLboolean *result)
{
    ASSERT(*result);

    mStateManager->bindBuffer(DestBufferOperationTarget, mBufferID);
    *result = mFunctions->unmapBuffer(DestBufferOperationTarget);
    return gl::Error(GL_NO_ERROR);
}

gl::Error BufferGL::getIndexRange(GLenum type, size_t offset, size_t count, RangeUI *outRange) const
{
    unsigned int streamOffset;
    if (mIndexRangeCache.findRange(type, offset, count, outRange, &streamOffset))
    {
        return gl::Error(GL_NO_ERROR);
    }

    mStateManager->bindBuffer(DestBufferOperationTarget, mBufferID);
    const uint8_t *bufferData = reinterpret_cast<uint8_t*>(mFunctions->mapBuffer(DestBufferOperationTarget, GL_READ_ONLY));
    gl::ComputeIndexRange(type, bufferData, count, &outRange->start, &outRange->end);
    mFunctions->unmapBuffer(DestBufferOperationTarget);

    mIndexRangeCache.addRange(type, offset, count, *outRange, 0);

    return gl::Error(GL_NO_ERROR);
}

GLuint BufferGL::getBufferID() const
{
    return mBufferID;
}

}
