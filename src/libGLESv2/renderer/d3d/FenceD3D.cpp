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

void FenceD3D::finishFence()
{
    while (!test(true))
    {
        Sleep(0);
    }
}

void FenceD3D::initFenceSync()
{
    LARGE_INTEGER counterFreqency = { 0 };
    BOOL success = QueryPerformanceFrequency(&counterFreqency);
    UNUSED_ASSERTION_VARIABLE(success);
    ASSERT(success);

    mCounterFrequency = counterFreqency.QuadPart;
}

GLenum FenceD3D::clientWait(GLbitfield flags, GLuint64 timeout)
{
    bool flushCommandBuffer = ((flags & GL_SYNC_FLUSH_COMMANDS_BIT) != 0);

    if (test(flushCommandBuffer))
    {
        return GL_ALREADY_SIGNALED;
    }

    if (hasError())
    {
        return GL_WAIT_FAILED;
    }

    if (timeout == 0)
    {
        return GL_TIMEOUT_EXPIRED;
    }

    LARGE_INTEGER currentCounter = { 0 };
    BOOL success = QueryPerformanceCounter(&currentCounter);
    UNUSED_ASSERTION_VARIABLE(success);
    ASSERT(success);

    LONGLONG timeoutInSeconds = static_cast<LONGLONG>(timeout) * static_cast<LONGLONG>(1000000ll);
    LONGLONG endCounter = currentCounter.QuadPart + mCounterFrequency * timeoutInSeconds;

    while (currentCounter.QuadPart < endCounter && !test(flushCommandBuffer))
    {
        Sleep(0);
        BOOL success = QueryPerformanceCounter(&currentCounter);
        UNUSED_ASSERTION_VARIABLE(success);
        ASSERT(success);
    }

    if (hasError())
    {
        return GL_WAIT_FAILED;
    }

    if (currentCounter.QuadPart >= endCounter)
    {
        return GL_TIMEOUT_EXPIRED;
    }

    return GL_CONDITION_SATISFIED;
}

}
