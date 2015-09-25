//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TransformFeedbackGL.cpp: Implements the class methods for TransformFeedbackGL.

#include "libANGLE/renderer/gl/TransformFeedbackGL.h"

#include "common/debug.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"

namespace rx
{

TransformFeedbackGL::TransformFeedbackGL(const FunctionsGL *functions, StateManagerGL *stateManager, size_t maxTransformFeedbackBufferBindings)
    : TransformFeedbackImpl(),
      mFunctions(functions),
      mStateManager(stateManager),
      mTransformFeedbackID(0),
      mCurrentGenericBufferID(0),
      mCurrentIndexedBufferIDs(maxTransformFeedbackBufferBindings, 0)
{
    mFunctions->genTransformFeedbacks(1, &mTransformFeedbackID);
}

TransformFeedbackGL::~TransformFeedbackGL()
{
    mStateManager->deleteTransformFeedback(mTransformFeedbackID);
    mTransformFeedbackID = 0;
}

void TransformFeedbackGL::begin(GLenum primitiveMode)
{
    // Skipped to prevent assertions in tests
    // UNIMPLEMENTED();
}

void TransformFeedbackGL::end()
{
    // Skipped to prevent assertions in tests
    // UNIMPLEMENTED();
}

void TransformFeedbackGL::pause()
{
    // Skipped to prevent assertions in tests
    // UNIMPLEMENTED();
}

void TransformFeedbackGL::resume()
{
    // Skipped to prevent assertions in tests
    // UNIMPLEMENTED();
}

void TransformFeedbackGL::bindGenericBuffer(const BindingPointer<gl::Buffer> &binding)
{
    // Skipped to prevent assertions in tests
    // UNIMPLEMENTED();
}

void TransformFeedbackGL::bindIndexedBuffer(size_t index, const OffsetBindingPointer<gl::Buffer> &binding)
{
    // Skipped to prevent assertions in tests
    // UNIMPLEMENTED();
}

}
