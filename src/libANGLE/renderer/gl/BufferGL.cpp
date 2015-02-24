//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BufferGL.cpp: Implements the class methods for BufferGL.

#include "libANGLE/renderer/gl/BufferGL.h"

#include "common/debug.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"

namespace rx
{

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

gl::Error BufferGL::setData(const void* data, size_t size, GLenum usage)
{
    mStateManager->setBuffer(GL_ARRAY_BUFFER, mBufferID);
    mFunctions->bufferData(GL_ARRAY_BUFFER, size, data, usage);
    return gl::Error(GL_NO_ERROR);
}

gl::Error BufferGL::setSubData(const void* data, size_t size, size_t offset)
{
    mStateManager->setBuffer(GL_ARRAY_BUFFER, mBufferID);
    mFunctions->bufferSubData(GL_ARRAY_BUFFER, offset, size, data);
    return gl::Error(GL_NO_ERROR);
}

gl::Error BufferGL::copySubData(BufferImpl* source, GLintptr sourceOffset, GLintptr destOffset, GLsizeiptr size)
{
    BufferGL *sourceGL = GetAs<BufferGL>(source);

    mStateManager->setBuffer(GL_ARRAY_BUFFER, mBufferID);
    mStateManager->setBuffer(GL_ELEMENT_ARRAY_BUFFER, sourceGL->getBufferID());

    mFunctions->copyBufferSubData(GL_ELEMENT_ARRAY_BUFFER, GL_ARRAY_BUFFER, sourceOffset, destOffset, size);

    return gl::Error(GL_NO_ERROR);
}

gl::Error BufferGL::map(size_t offset, size_t length, GLbitfield access, GLvoid **mapPtr)
{
    // TODO: look into splitting this into two functions, glMapBuffer is available in 1.5, but
    // glMapBufferRange requires 3.0

    mStateManager->setBuffer(GL_ARRAY_BUFFER, mBufferID);
    *mapPtr = mFunctions->mapBufferRange(GL_ARRAY_BUFFER, offset, length, access);
    return gl::Error(GL_NO_ERROR);
}

gl::Error BufferGL::unmap()
{
    mStateManager->setBuffer(GL_ARRAY_BUFFER, mBufferID);
    mFunctions->unmapBuffer(GL_ARRAY_BUFFER);
    return gl::Error(GL_NO_ERROR);
}

void BufferGL::markTransformFeedbackUsage()
{
    UNIMPLEMENTED();
}

gl::Error BufferGL::getData(const uint8_t **outData)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

GLuint BufferGL::getBufferID() const
{
    return mBufferID;
}

}
