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

namespace rx
{

RenderbufferGL::RenderbufferGL(const FunctionsGL *functions, StateManagerGL *stateManager)
    : RenderbufferImpl(),
      mFunctions(functions),
      mStateManager(stateManager),
      mRenderbufferID(0)
{
    mFunctions->genRenderbuffers(1, &mRenderbufferID);
}

RenderbufferGL::~RenderbufferGL()
{
    if (mRenderbufferID != 0)
    {
        mFunctions->deleteRenderbuffers(1, &mRenderbufferID);
        mRenderbufferID = 0;
    }
}

gl::Error RenderbufferGL::setStorage(GLsizei width, GLsizei height, GLenum internalformat, GLsizei samples)
{
    mStateManager->bindRenderbuffer(mRenderbufferID);
    if (samples > 0)
    {
        mFunctions->renderbufferStorageMultisample(GL_RENDERBUFFER, width, height, internalformat, samples);
    }
    else
    {
        mFunctions->renderbufferStorage(GL_RENDERBUFFER, width, height, internalformat);
    }

    return gl::Error(GL_NO_ERROR);
}

GLuint RenderbufferGL::getRenderbufferID() const
{
    return mRenderbufferID;
}

}
