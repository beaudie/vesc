//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramPipeline.cpp: Implements the gl::ProgramPipeline class.
// Implements GL program pipeline objects and related functionality.
// [OpenGL ES 3.1] section 7.4 page 105.

#include "libANGLE/ProgramPipeline.h"

#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/GLImplFactory.h"
#include "libANGLE/renderer/ProgramPipelineImpl.h"

namespace gl
{

ProgramPipelineState::ProgramPipelineState() : mLabel(), mActiveShaderProgram(nullptr), valid(false)
{
    for (const ShaderType shaderType : gl::AllShaderTypes())
    {
        mPrograms[shaderType] = nullptr;
    }
}

ProgramPipelineState::~ProgramPipelineState() {}

const std::string &ProgramPipelineState::getLabel() const
{
    return mLabel;
}

void ProgramPipelineState::activeShaderProgram(Program *shaderProgram)
{
    mActiveShaderProgram = shaderProgram;
}

void ProgramPipelineState::useProgramStage(const ShaderType shaderType, Program *shaderProgram)
{
    // If program refers to a program object with a valid shader attached for an indicated shader
    // stage, glUseProgramStages installs the executable code for that stage in the indicated
    // program pipeline object pipeline.
    if ((shaderProgram->id().value != 0) && shaderProgram->hasLinkedShaderStage(shaderType))
    {
        mPrograms[shaderType] = shaderProgram;
    }
    // If program is zero, or refers to a program object with no valid shader executable for a given
    // stage, it is as if the pipeline object has no programmable stage configured for the indicated
    // shader stages.
    else
    {
        mPrograms[shaderType] = nullptr;
    }
}

void ProgramPipelineState::useProgramStages(GLbitfield stages, Program *shaderProgram)
{
    if (stages == GL_ALL_SHADER_BITS)
    {
        for (const ShaderType shaderType : gl::AllShaderTypes())
        {
            useProgramStage(shaderType, shaderProgram);
        }
    }
    else
    {
        if (stages & GL_VERTEX_SHADER_BIT)
        {
            useProgramStage(ShaderType::Vertex, shaderProgram);
        }

        if (stages & GL_FRAGMENT_SHADER_BIT)
        {
            useProgramStage(ShaderType::Fragment, shaderProgram);
        }

        if (stages & GL_COMPUTE_SHADER_BIT)
        {
            useProgramStage(ShaderType::Compute, shaderProgram);
        }
    }
}

ProgramPipeline::ProgramPipeline(rx::GLImplFactory *factory, ProgramPipelineID handle)
    : RefCountObject(handle), mProgramPipeline(factory->createProgramPipeline(mState))
{
    ASSERT(mProgramPipeline);
}

ProgramPipeline::~ProgramPipeline()
{
    mProgramPipeline.release();
}

void ProgramPipeline::onDestroy(const Context *context)
{
    getImplementation()->destroy(context);
}

void ProgramPipeline::setLabel(const Context *context, const std::string &label)
{
    mState.mLabel = label;
}

const std::string &ProgramPipeline::getLabel() const
{
    return mState.mLabel;
}

rx::ProgramPipelineImpl *ProgramPipeline::getImplementation() const
{
    return mProgramPipeline.get();
}

void ProgramPipeline::activeShaderProgram(Program *shaderProgram)
{
    mState.activeShaderProgram(shaderProgram);
}

void ProgramPipeline::useProgramStages(GLbitfield stages, Program *shaderProgram)
{
    mState.useProgramStages(stages, shaderProgram);
}

int ProgramPipeline::getInfoLogLength() const
{
    return static_cast<int>(mInfoLog.getLength());
}

void ProgramPipeline::getInfoLog(GLsizei bufSize, GLsizei *length, char *infoLog) const
{
    return mInfoLog.getLog(bufSize, length, infoLog);
}

const gl::ActiveTextureMask &ProgramPipeline::getActiveSamplersMask() const
{
    return getShaderProgram(gl::ShaderType::Fragment)->getActiveSamplersMask();
}

const gl::ActiveTextureArray<gl::TextureType> &ProgramPipeline::getActiveSamplerTypes() const
{
    return getShaderProgram(gl::ShaderType::Fragment)->getActiveSamplerTypes();
}

bool ProgramPipeline::validateSamplers(InfoLog *infoLog, const Caps &caps)
{
    for (const ShaderType shaderType : gl::AllShaderTypes())
    {
        Program *shaderProgram = mState.mPrograms[shaderType];
        if (shaderProgram && !shaderProgram->validateSamplers(infoLog, caps))
        {
            return false;
        }
    }

    return true;
}

angle::Result ProgramPipeline::syncState(const Context *context)
{
    if (mDirtyBits.any())
    {
        for (const ShaderType shaderType : gl::AllShaderTypes())
        {
            Program *shaderProgram = mState.mPrograms[shaderType];
            ANGLE_TRY(shaderProgram->syncState(context));
        }

        mDirtyBits.reset();
    }

    return angle::Result::Continue;
}

}  // namespace gl
