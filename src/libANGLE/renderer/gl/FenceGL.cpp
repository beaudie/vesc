//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FenceGL.cpp: Implements the FenceSyncGL and FenceNVGL classes.

#include "libANGLE/renderer/gl/FenceGL.h"

namespace rx
{

FenceNVGL::FenceNVGL()
{
}

FenceNVGL::~FenceNVGL()
{
}

gl::Error FenceNVGL::set()
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error FenceNVGL::test(bool flushCommandBuffer, GLboolean *outFinished)
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error FenceNVGL::finishFence(GLboolean *outFinished)
{
    return gl::Error(GL_NO_ERROR);
}

FenceSyncGL::FenceSyncGL()
{
}

FenceSyncGL::~FenceSyncGL()
{
}

gl::Error FenceSyncGL::set()
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error FenceSyncGL::clientWait(GLbitfield flags, GLuint64 timeout, GLenum *outResult)
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error FenceSyncGL::serverWait(GLbitfield flags, GLuint64 timeout)
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error FenceSyncGL::getStatus(GLint *outResult)
{
    return gl::Error(GL_NO_ERROR);
}

}
