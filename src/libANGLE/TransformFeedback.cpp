//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/TransformFeedback.h"

#include "libANGLE/Buffer.h"
#include "libANGLE/Caps.h"
#include "libANGLE/Context.h"
#include "libANGLE/ContextState.h"
#include "libANGLE/Program.h"
#include "libANGLE/renderer/GLImplFactory.h"
#include "libANGLE/renderer/TransformFeedbackImpl.h"

namespace gl
{

TransformFeedbackState::TransformFeedbackState(size_t maxIndexedBuffers)
    : mLabel(),
      mActive(false),
      mPrimitiveMode(GL_NONE),
      mPaused(false),
      mVerticesDrawn(0),
      mProgram(nullptr),
      mIndexedBuffers(maxIndexedBuffers)
{
}

TransformFeedbackState::~TransformFeedbackState()
{
}

const OffsetBindingPointer<Buffer> &TransformFeedbackState::getIndexedBuffer(size_t idx) const
{
    return mIndexedBuffers[idx];
}

const std::vector<OffsetBindingPointer<Buffer>> &TransformFeedbackState::getIndexedBuffers() const
{
    return mIndexedBuffers;
}

TransformFeedback::TransformFeedback(rx::GLImplFactory *implFactory, GLuint id, const Caps &caps)
    : RefCountObject(id),
      mState(caps.maxTransformFeedbackSeparateAttributes),
      mImplementation(implFactory->createTransformFeedback(mState))
{
    ASSERT(mImplementation != nullptr);
}

Error TransformFeedback::onDestroy(const Context *context)
{
    ASSERT(!context || !context->isCurrentTransformFeedback(this));
    if (mState.mProgram)
    {
        mState.mProgram->release(context);
        mState.mProgram = nullptr;
    }

    ASSERT(!mState.mProgram);
    for (size_t i = 0; i < mState.mIndexedBuffers.size(); i++)
    {
        mState.mIndexedBuffers[i].set(context, nullptr);
    }

    return NoError();
}

TransformFeedback::~TransformFeedback()
{
    SafeDelete(mImplementation);
}

void TransformFeedback::setLabel(const std::string &label)
{
    mState.mLabel = label;
}

const std::string &TransformFeedback::getLabel() const
{
    return mState.mLabel;
}

void TransformFeedback::begin(const Context *context, GLenum primitiveMode, Program *program)
{
    mState.mActive        = true;
    mState.mPrimitiveMode = primitiveMode;
    mState.mPaused        = false;
    mState.mVerticesDrawn = 0;
    mImplementation->begin(primitiveMode);
    bindProgram(context, program);
}

void TransformFeedback::end(const Context *context)
{
    mState.mActive        = false;
    mState.mPrimitiveMode = GL_NONE;
    mState.mPaused        = false;
    mState.mVerticesDrawn = 0;
    mImplementation->end();
    if (mState.mProgram)
    {
        mState.mProgram->release(context);
        mState.mProgram = nullptr;
    }
}

void TransformFeedback::pause()
{
    mState.mPaused = true;
    mImplementation->pause();
}

void TransformFeedback::resume()
{
    mState.mPaused = false;
    mImplementation->resume();
}

bool TransformFeedback::isActive() const
{
    return mState.mActive;
}

bool TransformFeedback::isPaused() const
{
    return mState.mPaused;
}

GLenum TransformFeedback::getPrimitiveMode() const
{
    return mState.mPrimitiveMode;
}

bool TransformFeedback::checkBufferSpaceForDraw(GLsizei count, GLsizei primcount) const
{
    angle::CheckedNumeric<GLsizeiptr> vertices = getVerticesNeededForDraw(count, primcount);
    auto vertexSizes = mState.mProgram->getTransformFeedbackPerVertexSizes();
    if (vertexSizes.size() > mState.mIndexedBuffers.size())
    {
        return false;
    }
    for (size_t index = 0; index < vertexSizes.size(); index++)
    {
        angle::CheckedNumeric<GLsizeiptr> size = vertices * vertexSizes[index];
        if (!size.IsValid() || !mState.mIndexedBuffers[index].get() ||
            size.ValueOrDie() > GetAvailableSize(mState.mIndexedBuffers[index]))
            return false;
    }
    return true;
}

angle::CheckedNumeric<GLsizeiptr> TransformFeedback::getVerticesNeededForDraw(
    GLsizei count,
    GLsizei primcount) const
{
    if (count < 0 || primcount < 0)
    {
        return false;
    }
    // Transform feedback only outputs complete primitives, so we need to round  down to the nearest
    // complete primitive before multiplying by the number of instances.
    angle::CheckedNumeric<GLsizeiptr> checkedVertices  = mState.mVerticesDrawn;
    angle::CheckedNumeric<GLsizeiptr> checkedCount     = count;
    angle::CheckedNumeric<GLsizeiptr> checkedPrimcount = primcount;
    switch (mState.mPrimitiveMode)
    {
        case GL_TRIANGLES:
            checkedVertices += checkedPrimcount * (checkedCount - checkedCount % 3);
            break;
        case GL_LINES:
            checkedVertices += checkedPrimcount * (checkedCount - checkedCount % 2);
            break;
        case GL_POINTS:
            checkedVertices += checkedPrimcount * checkedCount;
            break;
        default:
            NOTREACHED();
    }
    return checkedVertices;
}

void TransformFeedback::onVerticesDrawn(GLsizei count, GLsizei primcount)
{
    ASSERT(mState.mActive && !mState.mPaused);
    // All draws should be validated with checkBufferSpaceForDraw so ValueOrDie should never fail.
    mState.mVerticesDrawn = getVerticesNeededForDraw(count, primcount).ValueOrDie();
}

void TransformFeedback::bindProgram(const Context *context, Program *program)
{
    if (mState.mProgram != program)
    {
        if (mState.mProgram != nullptr)
        {
            mState.mProgram->release(context);
        }
        mState.mProgram = program;
        if (mState.mProgram != nullptr)
        {
            mState.mProgram->addRef();
        }
    }
}

bool TransformFeedback::hasBoundProgram(GLuint program) const
{
    return mState.mProgram != nullptr && mState.mProgram->id() == program;
}

void TransformFeedback::detachBuffer(const Context *context, GLuint bufferName)
{
    bool isBound = context->isCurrentTransformFeedback(this);
    for (size_t index = 0; index < mState.mIndexedBuffers.size(); index++)
    {
        if (mState.mIndexedBuffers[index].id() == bufferName)
        {
            if (isBound)
            {
                mState.mIndexedBuffers[index]->onBindingChanged(false,
                                                                BufferBinding::TransformFeedback);
            }
            mState.mIndexedBuffers[index].set(context, nullptr);
            mImplementation->bindIndexedBuffer(index, mState.mIndexedBuffers[index]);
        }
    }
}

void TransformFeedback::bindIndexedBuffer(const Context *context,
                                          size_t index,
                                          Buffer *buffer,
                                          size_t offset,
                                          size_t size)
{
    ASSERT(index < mState.mIndexedBuffers.size());
    bool isBound = context && context->isCurrentTransformFeedback(this);
    if (isBound && mState.mIndexedBuffers[index].get())
    {
        mState.mIndexedBuffers[index]->onBindingChanged(false, BufferBinding::TransformFeedback);
    }
    mState.mIndexedBuffers[index].set(context, buffer, offset, size);
    if (isBound && buffer)
    {
        buffer->onBindingChanged(true, BufferBinding::TransformFeedback);
    }

    mImplementation->bindIndexedBuffer(index, mState.mIndexedBuffers[index]);
}

const OffsetBindingPointer<Buffer> &TransformFeedback::getIndexedBuffer(size_t index) const
{
    ASSERT(index < mState.mIndexedBuffers.size());
    return mState.mIndexedBuffers[index];
}

size_t TransformFeedback::getIndexedBufferCount() const
{
    return mState.mIndexedBuffers.size();
}

bool TransformFeedback::buffersBoundForOtherUse() const
{
    for (auto &buffer : mState.mIndexedBuffers)
    {
        if (buffer.get() && buffer->isBoundForTransformFeedbackAndOtherUse())
        {
            return true;
        }
    }
    return false;
}

rx::TransformFeedbackImpl *TransformFeedback::getImplementation()
{
    return mImplementation;
}

const rx::TransformFeedbackImpl *TransformFeedback::getImplementation() const
{
    return mImplementation;
}

void TransformFeedback::onBindingChanged(bool bound)
{
    for (auto &buffer : mState.mIndexedBuffers)
    {
        if (buffer.get())
        {
            buffer->onBindingChanged(bound, BufferBinding::TransformFeedback);
        }
    }
}
}
