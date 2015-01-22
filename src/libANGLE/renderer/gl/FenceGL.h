//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FenceGL.h: Defines the FenceSyncGL and FenceNVGL classes.

#ifndef LIBANGLE_RENDERER_GL_FENCEGL_H_
#define LIBANGLE_RENDERER_GL_FENCEGL_H_

#include "libANGLE/renderer/FenceImpl.h"

namespace rx
{

class FenceNVGL : public FenceNVImpl
{
  public:
    FenceNVGL();
    virtual ~FenceNVGL();

    gl::Error set() override;
    gl::Error test(bool flushCommandBuffer, GLboolean *outFinished) override;
    gl::Error finishFence(GLboolean *outFinished) override;

  private:
    DISALLOW_COPY_AND_ASSIGN(FenceNVGL);
};

class FenceSyncGL : public FenceSyncImpl
{
  public:
    FenceSyncGL();
    virtual ~FenceSyncGL();

    gl::Error set() override;
    gl::Error clientWait(GLbitfield flags, GLuint64 timeout, GLenum *outResult) override;
    gl::Error serverWait(GLbitfield flags, GLuint64 timeout) override;
    gl::Error getStatus(GLint *outResult) override;

  private:
    DISALLOW_COPY_AND_ASSIGN(FenceSyncGL);
};

}

#endif // LIBANGLE_RENDERER_GL_FENCEGL_H_
