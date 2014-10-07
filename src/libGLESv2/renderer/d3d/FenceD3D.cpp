//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FenceD3D.cpp: Defines the rx::FenceD3D abstract base class.

#include "libGLESv2/renderer/d3d/FenceD3D.h"

#include "libGLESv2/main.h"

namespace rx
{

FenceD3D::FenceD3D()
{
}

gl::Error FenceD3D::finishFence(GLboolean* outFinished)
{
    ASSERT(outFinished);

    while (*outFinished != GL_TRUE)
    {
        gl::Error error = test(true, outFinished);
        if (error.isError())
        {
            return error;
        }

        Sleep(0);
    }

    return gl::Error(GL_NO_ERROR);
}

void FenceD3D::initFenceSync()
{
    LARGE_INTEGER counterFreqency = { 0 };
    BOOL success = QueryPerformanceFrequency(&counterFreqency);
    UNUSED_ASSERTION_VARIABLE(success);
    ASSERT(success);

    mCounterFrequency = counterFreqency.QuadPart;
}

gl::Error FenceD3D::clientWait(GLbitfield flags, GLuint64 timeout, GLenum* outResult)
{
    ASSERT(outResult);

    bool flushCommandBuffer = ((flags & GL_SYNC_FLUSH_COMMANDS_BIT) != 0);

    GLboolean result = GL_FALSE;
    gl::Error error = test(flushCommandBuffer, &result);
    if (error.isError())
    {
        *outResult = GL_WAIT_FAILED;
        return error;
    }

    if (result == GL_TRUE)
    {
        *outResult = GL_ALREADY_SIGNALED;
        return gl::Error(GL_NO_ERROR);
    }

    if (timeout == 0)
    {
        *outResult = GL_TIMEOUT_EXPIRED;
        return gl::Error(GL_NO_ERROR);
    }

    LARGE_INTEGER currentCounter = { 0 };
    BOOL success = QueryPerformanceCounter(&currentCounter);
    UNUSED_ASSERTION_VARIABLE(success);
    ASSERT(success);

    LONGLONG timeoutInSeconds = static_cast<LONGLONG>(timeout) * static_cast<LONGLONG>(1000000ll);
    LONGLONG endCounter = currentCounter.QuadPart + mCounterFrequency * timeoutInSeconds;

    while (currentCounter.QuadPart < endCounter && !result)
    {
        Sleep(0);
        BOOL success = QueryPerformanceCounter(&currentCounter);
        UNUSED_ASSERTION_VARIABLE(success);
        ASSERT(success);

        error = test(flushCommandBuffer, &result);
        if (error.isError())
        {
            *outResult = GL_WAIT_FAILED;
            return error;
        }
    }

    if (currentCounter.QuadPart >= endCounter)
    {
        *outResult = GL_TIMEOUT_EXPIRED;
    }
    else
    {
        *outResult = GL_CONDITION_SATISFIED;
    }

    return gl::Error(GL_NO_ERROR);
}

}
