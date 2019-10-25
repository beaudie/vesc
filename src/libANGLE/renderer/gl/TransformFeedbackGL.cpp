//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TransformFeedbackGL.cpp: Implements the class methods for TransformFeedbackGL.

#include "libANGLE/renderer/gl/TransformFeedbackGL.h"

#include "common/debug.h"
#include "libANGLE/Context.h"
#include "libANGLE/State.h"
#include "libANGLE/renderer/gl/BufferGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/ProgramGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"
#include "libANGLE/renderer/gl/renderergl_utils.h"

namespace rx
{

TransformFeedbackGL::TransformFeedbackGL(const gl::TransformFeedbackState &state,
                                         GLuint transformFeedbackID)
    : TransformFeedbackImpl(state),
      mTransformFeedbackID(transformFeedbackID),
      mIsActive(false),
      mIsPaused(false),
      mActiveProgram(0)
{}

TransformFeedbackGL::~TransformFeedbackGL()
{
    ASSERT(mTransformFeedbackID == 0);
}

void TransformFeedbackGL::onDestroy(const gl::Context *context)
{
    StateManagerGL *stateManager = GetStateManagerGL(context);
    (void)stateManager->deleteTransformFeedback(context, mTransformFeedbackID);
    mTransformFeedbackID = 0;
}

angle::Result TransformFeedbackGL::begin(const gl::Context *context,
                                         gl::PrimitiveMode primitiveMode)
{
    StateManagerGL *stateManager = GetStateManagerGL(context);
    stateManager->onTransformFeedbackStateChange();
    return angle::Result::Continue;
}

angle::Result TransformFeedbackGL::end(const gl::Context *context)
{
    StateManagerGL *stateManager = GetStateManagerGL(context);
    stateManager->onTransformFeedbackStateChange();

    // Immediately end the transform feedback so that the results are visible.
    return syncActiveState(context, false, gl::PrimitiveMode::InvalidEnum);
}

angle::Result TransformFeedbackGL::pause(const gl::Context *context)
{
    StateManagerGL *stateManager = GetStateManagerGL(context);
    stateManager->onTransformFeedbackStateChange();
    return syncPausedState(context, true);
}

angle::Result TransformFeedbackGL::resume(const gl::Context *context)
{
    StateManagerGL *stateManager = GetStateManagerGL(context);
    stateManager->onTransformFeedbackStateChange();
    return angle::Result::Continue;
}

angle::Result TransformFeedbackGL::bindIndexedBuffer(
    const gl::Context *context,
    size_t index,
    const gl::OffsetBindingPointer<gl::Buffer> &binding)
{
    StateManagerGL *stateManager = GetStateManagerGL(context);
    const FunctionsGL *functions = GetFunctionsGL(context);

    // Directly bind buffer (not through the StateManager methods) because the buffer bindings are
    // tracked per transform feedback object
    ANGLE_TRY(
        stateManager->bindTransformFeedback(context, GL_TRANSFORM_FEEDBACK, mTransformFeedbackID));
    if (binding.get() != nullptr)
    {
        const BufferGL *bufferGL = GetImplAs<BufferGL>(binding.get());
        if (binding.getSize() != 0)
        {
            ANGLE_GL_TRY(context,
                         functions->bindBufferRange(
                             GL_TRANSFORM_FEEDBACK_BUFFER, static_cast<GLuint>(index),
                             bufferGL->getBufferID(), binding.getOffset(), binding.getSize()));
        }
        else
        {
            ANGLE_GL_TRY(context, functions->bindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER,
                                                            static_cast<GLuint>(index),
                                                            bufferGL->getBufferID()));
        }
    }
    else
    {
        ANGLE_GL_TRY(context, functions->bindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER,
                                                        static_cast<GLuint>(index), 0));
    }
    return angle::Result::Continue;
}

GLuint TransformFeedbackGL::getTransformFeedbackID() const
{
    return mTransformFeedbackID;
}

angle::Result TransformFeedbackGL::syncActiveState(const gl::Context *context,
                                                   bool active,
                                                   gl::PrimitiveMode primitiveMode)
{
    if (mIsActive != active)
    {
        StateManagerGL *stateManager = GetStateManagerGL(context);
        const FunctionsGL *functions = GetFunctionsGL(context);

        mIsActive = active;
        mIsPaused = false;

        ANGLE_TRY(stateManager->bindTransformFeedback(context, GL_TRANSFORM_FEEDBACK,
                                                      mTransformFeedbackID));
        if (mIsActive)
        {
            ASSERT(primitiveMode != gl::PrimitiveMode::InvalidEnum);
            mActiveProgram = GetImplAs<ProgramGL>(mState.getBoundProgram())->getProgramID();
            stateManager->useProgram(mActiveProgram);
            ANGLE_GL_TRY(context, functions->beginTransformFeedback(gl::ToGLenum(primitiveMode)));
        }
        else
        {
            // Implementations disagree about what should happen if a different program is bound
            // when calling EndTransformFeedback. We avoid the ambiguity by always re-binding the
            // program associated with this transform feedback.
            GLuint previousProgram = stateManager->getProgramID();
            stateManager->useProgram(mActiveProgram);
            ANGLE_GL_TRY(context, functions->endTransformFeedback());
            // Restore the current program if we changed it.
            stateManager->useProgram(previousProgram);
        }
    }
    return angle::Result::Continue;
}

angle::Result TransformFeedbackGL::syncPausedState(const gl::Context *context, bool paused)
{
    if (mIsActive && mIsPaused != paused)
    {
        StateManagerGL *stateManager = GetStateManagerGL(context);
        const FunctionsGL *functions = GetFunctionsGL(context);

        mIsPaused = paused;

        ANGLE_TRY(stateManager->bindTransformFeedback(context, GL_TRANSFORM_FEEDBACK,
                                                      mTransformFeedbackID));
        if (mIsPaused)
        {
            ANGLE_GL_TRY(context, functions->pauseTransformFeedback());
        }
        else
        {
            ANGLE_GL_TRY(context, functions->resumeTransformFeedback());
        }
    }
    return angle::Result::Continue;
}
}  // namespace rx
