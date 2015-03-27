//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/TransformFeedback.h"
#include "libANGLE/renderer/TransformFeedbackImpl.h"

namespace gl
{

TransformFeedback::TransformFeedback(rx::TransformFeedbackImpl* impl, GLuint id)
    : RefCountObject(id),
      mTransformFeedback(impl),
      mActive(GL_FALSE),
      mPrimitiveMode(GL_NONE),
      mPaused(GL_FALSE)
{
    ASSERT(impl != NULL);
}

TransformFeedback::~TransformFeedback()
{
    SafeDelete(mTransformFeedback);
}

void TransformFeedback::begin(GLenum primitiveMode)
{
    mActive = GL_TRUE;
    mPrimitiveMode = primitiveMode;
    mPaused = GL_FALSE;
    mTransformFeedback->begin(primitiveMode);
}

void TransformFeedback::end()
{
    mActive = GL_FALSE;
    mPrimitiveMode = GL_NONE;
    mPaused = GL_FALSE;
    mTransformFeedback->end();
}

bool TransformFeedback::isActive() const
{
    return mActive;
}

GLenum TransformFeedback::getPrimitiveMode() const
{
    return mPrimitiveMode;
}

void TransformFeedback::pause()
{
    mPaused = GL_TRUE;
    mTransformFeedback->pause();
}

void TransformFeedback::resume()
{
    mPaused = GL_FALSE;
    mTransformFeedback->resume();
}

bool TransformFeedback::isPaused() const
{
    return mPaused;
}

}
