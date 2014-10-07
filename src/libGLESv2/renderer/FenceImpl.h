//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FenceImpl.h: Defines the rx::FenceImpl class.

#ifndef LIBGLESV2_RENDERER_FENCEIMPL_H_
#define LIBGLESV2_RENDERER_FENCEIMPL_H_

#include "libGLESv2/Error.h"

#include "common/angleutils.h"

#include "angle_gl.h"

namespace rx
{

class FenceImpl
{
  public:
    FenceImpl() { };
    virtual ~FenceImpl() { };

    virtual gl::Error set() = 0;
    virtual gl::Error test(bool flushCommandBuffer, GLboolean *outFinished) = 0;
    // Helps implement FenceNV::finishFence.
    virtual gl::Error finishFence(GLboolean* outFinished) = 0;
    // Helps implement FenceSync, in particular clientWait.
    virtual void initFenceSync() = 0;
    virtual gl::Error clientWait(GLbitfield flags, GLuint64 timeout, GLenum* outResult) = 0;

  private:
    DISALLOW_COPY_AND_ASSIGN(FenceImpl);
};

}

#endif // LIBGLESV2_RENDERER_FENCEIMPL_H_
