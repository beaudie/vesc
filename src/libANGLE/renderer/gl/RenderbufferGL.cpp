//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderbufferGL.cpp: Implements the class methods for RenderbufferGL.

#include "libANGLE/renderer/gl/RenderbufferGL.h"

#include "common/debug.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"
#include "libANGLE/renderer/gl/renderergl_utils.h"

namespace rx
{

RenderbufferGL::RenderbufferGL(const FunctionsGL *functions, StateManagerGL *stateManager)
    : RenderbufferImpl(),
      mFunctions(functions),
      mStateManager(stateManager),
      mRenderbufferID(0)
{
    GLCall(mFunctions, genRenderbuffers, 1, &mRenderbufferID);
}

RenderbufferGL::~RenderbufferGL()
{
    if (mRenderbufferID != 0)
    {
        GLCall(mFunctions, deleteRenderbuffers, 1, &mRenderbufferID);
        mRenderbufferID = 0;
    }
}

gl::Error RenderbufferGL::setStorage(GLenum internalformat, size_t width, size_t height)
{
    mStateManager->bindRenderbuffer(GL_RENDERBUFFER, mRenderbufferID);
    GLCallNoCheck(mFunctions, renderbufferStorage, GL_RENDERBUFFER, internalformat, width, height);

    // Call probably allocates, check for OOM explicitly
    gl::Error error = nativegl::CheckForGLOutOfMemoryError(mFunctions);
    if (error.isError())
    {
        return error;
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error RenderbufferGL::setStorageMultisample(size_t samples, GLenum internalformat, size_t width, size_t height)
{
    mStateManager->bindRenderbuffer(GL_RENDERBUFFER, mRenderbufferID);
    GLCallNoCheck(mFunctions, renderbufferStorageMultisample, GL_RENDERBUFFER, samples, internalformat, width, height);

    // Call probably allocates, check for OOM explicitly
    gl::Error error = nativegl::CheckForGLOutOfMemoryError(mFunctions);
    if (error.isError())
    {
        return error;
    }

    return gl::Error(GL_NO_ERROR);
}

GLuint RenderbufferGL::getRenderbufferID() const
{
    return mRenderbufferID;
}

}
