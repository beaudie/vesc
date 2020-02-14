//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramExecutable.cpp: Collects the interfaces common to both Programs and
// ProgramPipelines in order to execute/draw with either.

#include "libANGLE/ProgramExecutable.h"

#include "libANGLE/Context.h"

namespace gl
{

ProgramExecutable::ProgramExecutable() : mMaxActiveAttribLocation(0) {}

ProgramExecutable::~ProgramExecutable() = default;

int ProgramExecutable::getInfoLogLength() const
{
    return static_cast<int>(mInfoLog.getLength());
}

void ProgramExecutable::getInfoLog(GLsizei bufSize, GLsizei *length, char *infoLog) const
{
    return mInfoLog.getLog(bufSize, length, infoLog);
}

std::string ProgramExecutable::getInfoLogString() const
{
    return mInfoLog.str();
}

bool ProgramExecutable::isAttribLocationActive(size_t attribLocation) const
{
    // TODO(timvp): http://anglebug.com/3570: Enable this assert here somehow.
    //    ASSERT(mLinkResolved);
    ASSERT(attribLocation < mActiveAttribLocationsMask.size());
    return mActiveAttribLocationsMask[attribLocation];
}

AttributesMask ProgramExecutable::getAttributesMask() const
{
    // TODO(timvp): http://anglebug.com/3570: Enable this assert here somehow.
    //    ASSERT(mLinkResolved);
    return mAttributesMask;
}

bool ProgramExecutable::hasDefaultUniforms(const gl::State &glState) const
{
    gl::Program *program = glState.getProgram();

    if (program)
    {
        return !program->getState().getDefaultUniformRange().empty();
    }

    // TODO(timvp): http://anglebug.com/3570: Support program pipelines

    return false;
}

bool ProgramExecutable::hasTextures(const gl::State &glState) const
{
    gl::Program *program = glState.getProgram();

    if (program)
    {
        return !program->getState().getSamplerBindings().empty();
    }

    // TODO(timvp): http://anglebug.com/3570: Support program pipelines

    return false;
}

bool ProgramExecutable::hasUniformBuffers(const gl::State &glState) const
{
    gl::Program *program = glState.getProgram();

    if (program)
    {
        return !program->getState().getUniformBlocks().empty();
    }

    // TODO(timvp): http://anglebug.com/3570: Support program pipelines

    return false;
}

bool ProgramExecutable::hasStorageBuffers(const gl::State &glState) const
{
    gl::Program *program = glState.getProgram();

    if (program)
    {
        return !program->getState().getShaderStorageBlocks().empty();
    }

    // TODO(timvp): http://anglebug.com/3570: Support program pipelines

    return false;
}

bool ProgramExecutable::hasAtomicCounterBuffers(const gl::State &glState) const
{
    gl::Program *program = glState.getProgram();

    if (program)
    {
        return !program->getState().getAtomicCounterBuffers().empty();
    }

    // TODO(timvp): http://anglebug.com/3570: Support program pipelines

    return false;
}

bool ProgramExecutable::hasImages(const gl::State &glState) const
{
    gl::Program *program = glState.getProgram();

    if (program)
    {
        return !program->getState().getImageBindings().empty();
    }

    // TODO(timvp): http://anglebug.com/3570: Support program pipelines

    return false;
}

bool ProgramExecutable::hasTransformFeedbackOutput(const gl::State &glState) const
{
    gl::Program *program = glState.getProgram();

    if (program)
    {
        return !program->getState().getLinkedTransformFeedbackVaryings().empty();
    }

    // TODO(timvp): http://anglebug.com/3570: Support program pipelines

    return false;
}

size_t ProgramExecutable::getUniqueUniformBlockCount(const gl::State &glState) const
{
    gl::Program *program = glState.getProgram();

    if (program)
    {
        return program->getState().getUniqueUniformBlockCount();
    }

    // TODO(timvp): http://anglebug.com/3570: Support program pipelines

    return 0;
}

size_t ProgramExecutable::getUniqueStorageBlockCount(const gl::State &glState) const
{
    gl::Program *program = glState.getProgram();

    if (program)
    {
        return program->getState().getUniqueStorageBlockCount();
    }

    // TODO(timvp): http://anglebug.com/3570: Support program pipelines

    return 0;
}

size_t ProgramExecutable::getAtomicCounterBuffersCount(const gl::State &glState) const
{
    gl::Program *program = glState.getProgram();

    if (program)
    {
        return program->getState().getAtomicCounterBuffersCount();
    }

    // TODO(timvp): http://anglebug.com/3570: Support program pipelines

    return 0;
}

size_t ProgramExecutable::getTransformFeedbackBufferCount(const gl::State &glState) const
{
    gl::Program *program = glState.getProgram();

    if (program)
    {
        return program->getState().getTransformFeedbackBufferCount();
    }

    // TODO(timvp): http://anglebug.com/3570: Support program pipelines

    return 0;
}

}  // namespace gl
