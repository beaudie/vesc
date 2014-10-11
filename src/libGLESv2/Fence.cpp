//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Fence.cpp: Implements the gl::Fence class, which supports the GL_NV_fence extension.

// Important note on accurate timers in Windows:
//
// QueryPerformanceCounter has a few major issues, including being 10x as expensive to call
// as timeGetTime on laptops and "jumping" during certain hardware events.
//
// See the comments at the top of the Chromium source file "chromium/src/base/time/time_win.cc"
//   https://code.google.com/p/chromium/codesearch#chromium/src/base/time/time_win.cc
//
// We still opt to use QPC. In the present and moving forward, most newer systems will not suffer
// from buggy implementations.

#include "libGLESv2/Fence.h"
#include "libGLESv2/renderer/FenceImpl.h"
#include "libGLESv2/renderer/Renderer.h"
#include "libGLESv2/main.h"

#include "angle_gl.h"

namespace gl
{

FenceNV::FenceNV(rx::Renderer *renderer)
    : mFence(renderer->createFenceNV()),
      mIsSet(false),
      mStatus(GL_FALSE),
      mCondition(GL_NONE)
{
}

FenceNV::~FenceNV()
{
    SafeDelete(mFence);
}

GLboolean FenceNV::isFence() const
{
    // GL_NV_fence spec:
    // A name returned by GenFencesNV, but not yet set via SetFenceNV, is not the name of an existing fence.
    return (mIsSet ? GL_TRUE : GL_FALSE);
}

Error FenceNV::setFence(GLenum condition)
{
    Error error = mFence->set();
    if (error.isError())
    {
        return error;
    }

    mCondition = condition;
    mStatus = GL_FALSE;
    mIsSet = true;

    return Error(GL_NO_ERROR);
}

Error FenceNV::testFence(GLboolean *outResult)
{
    // Flush the command buffer by default
    Error error = mFence->test(true, &mStatus);
    if (error.isError())
    {
        return error;
    }

    *outResult = mStatus;
    return Error(GL_NO_ERROR);
}

Error FenceNV::finishFence()
{
    ASSERT(mIsSet);

    return mFence->finishFence(&mStatus);
}

FenceSync::FenceSync(rx::Renderer *renderer, GLuint id)
    : RefCountObject(id),
      mFence(renderer->createFenceSync()),
      mCondition(GL_NONE)
{
}

FenceSync::~FenceSync()
{
    SafeDelete(mFence);
}

Error FenceSync::set(GLenum condition)
{
    Error error = mFence->set();
    if (error.isError())
    {
        return error;
    }

    mCondition = condition;
    return Error(GL_NO_ERROR);
}

Error FenceSync::clientWait(GLbitfield flags, GLuint64 timeout, GLenum *outResult)
{
    ASSERT(mCondition != GL_NONE);
    return mFence->clientWait(flags, timeout, outResult);
}

Error FenceSync::serverWait(GLbitfield flags, GLuint64 timeout)
{
    return mFence->serverWait(flags, timeout);
}

Error FenceSync::getStatus(GLint *outResult) const
{
    return mFence->getStatus(outResult);
}

}
