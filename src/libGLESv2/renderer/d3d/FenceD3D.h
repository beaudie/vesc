//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FenceD3D.h: Provides a common base class for rx::Fence9 and rx::Fence11.

#ifndef LIBGLESV2_RENDERER_FENCED3D_H_
#define LIBGLESV2_RENDERER_FENCED3D_H_

#include "libGLESv2/renderer/FenceImpl.h"

namespace rx
{
class FenceD3D : public FenceImpl
{
  public:
    FenceD3D();

    // Helps implement FenceNV::finishFence.
    virtual gl::Error finishFence(GLboolean* outFinished);
    // Helps implement FenceSync, in particular clientWait.
    virtual void initFenceSync();
    virtual gl::Error clientWait(GLbitfield flags, GLuint64 timeout, GLenum* outResult);

  private:
    DISALLOW_COPY_AND_ASSIGN(FenceD3D);

    // Used only for FenceSync objects.
    LONGLONG mCounterFrequency;
};

}

#endif // LIBGLESV2_RENDERER_FENCED3D_H_
