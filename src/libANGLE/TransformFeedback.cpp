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
      mActive(false),
      mPrimitiveMode(GL_NONE),
      mPaused(false)
{
    ASSERT(impl != NULL);
}

TransformFeedback::~TransformFeedback()
{
    SafeDelete(mTransformFeedback);
}

void TransformFeedback::begin(GLenum primitiveMode)
{
    mActive = true;
    mPrimitiveMode = primitiveMode;
    mPaused = false;
    mTransformFeedback->begin(primitiveMode);
}

void TransformFeedback::end()
{
    mActive = false;
    mPrimitiveMode = GL_NONE;
    mPaused = false;
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
    mPaused = true;
    mTransformFeedback->pause();
}

void TransformFeedback::resume()
{
    mPaused = false;
    mTransformFeedback->resume();
}

bool TransformFeedback::isPaused() const
{
    return mPaused;
}

}
