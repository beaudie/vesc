//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BufferGL.cpp: Implements the BufferGL class.

#include "libANGLE/renderer/gl/BufferGL.h"

namespace rx
{

BufferGL::BufferGL()
{
}

BufferGL::~BufferGL()
{
}

gl::Error BufferGL::setData(const void* data, size_t size, GLenum usage)
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error BufferGL::setSubData(const void* data, size_t size, size_t offset)
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error BufferGL::copySubData(BufferImpl* source, GLintptr sourceOffset, GLintptr destOffset, GLsizeiptr size)
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error BufferGL::map(size_t offset, size_t length, GLbitfield access, GLvoid **mapPtr)
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error BufferGL::unmap()
{
    return gl::Error(GL_NO_ERROR);
}

void BufferGL::markTransformFeedbackUsage()
{
}

gl::Error BufferGL::getData(const uint8_t **outData)
{
    return gl::Error(GL_NO_ERROR);
}

}
