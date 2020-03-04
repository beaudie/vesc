//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramPipelineVk.cpp:
//    Implements the class methods for ProgramPipelineVk.
//

#include "libANGLE/renderer/vulkan/ProgramPipelineVk.h"

#include "libANGLE/renderer/glslang_wrapper_utils.h"
#include "libANGLE/renderer/vulkan/GlslangWrapperVk.h"

namespace rx
{

ProgramPipelineVk::ProgramPipelineVk(const gl::ProgramPipelineState &state)
    : ProgramPipelineImpl(state)
{}

ProgramPipelineVk::~ProgramPipelineVk() {}

void ProgramPipelineVk::destroy(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);
    reset(contextVk);
}

void ProgramPipelineVk::reset(ContextVk *contextVk)
{
    mExecutable.reset(contextVk);
}

void ProgramPipelineVk::fillProgramStateMap(
    const ContextVk *contextVk,
    gl::ShaderMap<const gl::ProgramState *> *programStatesOut)
{
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        (*programStatesOut)[shaderType] = nullptr;

        ProgramVk *programVk = getShaderProgram(contextVk->getState(), shaderType);
        if (programVk)
        {
            (*programStatesOut)[shaderType] = &programVk->getState();
        }
    }
}

angle::Result ProgramPipelineVk::link(const gl::Context *glContext)
{
    ContextVk *contextVk                  = vk::GetImpl(glContext);
    const gl::State &glState              = glContext->getState();
    const gl::ProgramPipeline *glPipeline = glState.getProgramPipeline();
    GlslangSourceOptions options =
        GlslangWrapperVk::CreateSourceOptions(contextVk->getRenderer()->getFeatures());

    mExecutable.clearVariableInfoMap();

    for (const gl::ShaderType shaderType : glPipeline->getExecutable().getLinkedShaderStages())
    {
        gl::Program *glProgram =
            const_cast<gl::Program *>(glPipeline->getShaderProgram(shaderType));
        if (glProgram)
        {
            // Re-assign locations.
            AssignLocations(options, glProgram->getState(), glProgram->getResources(), shaderType,
                            &mExecutable.getShaderInterfaceVariableInfoMap());
        }
    }
    return angle::Result::Continue;
}

angle::Result ProgramPipelineVk::transformShaderSpirV(const gl::Context *glContext)
{
    ContextVk *contextVk                    = vk::GetImpl(glContext);
    const gl::ProgramExecutable *executable = contextVk->getState().getProgramExecutable();
    ASSERT(executable);

    for (const gl::ShaderType shaderType : executable->getLinkedShaderStages())
    {
        ProgramVk *programVk = getShaderProgram(contextVk->getState(), shaderType);
        if (programVk)
        {
            ShaderInterfaceVariableInfoMap &variableInfoMap =
                mExecutable.mVariableInfoMap[shaderType];
            std::vector<uint32_t> transformedSpirvBlob;

            ANGLE_TRY(GlslangWrapperVk::TransformSpirV(
                contextVk, shaderType, variableInfoMap,
                programVk->getShaderInfo().getSpirvBlobs()[shaderType], &transformedSpirvBlob));

            // Save the newly transformed SPIR-V
            programVk->getShaderInfo().getSpirvBlobs()[shaderType] = transformedSpirvBlob;
        }
    }
    return angle::Result::Continue;
}

angle::Result ProgramPipelineVk::initGraphicsProgram(ContextVk *contextVk,
                                                     bool enableLineRasterEmulation,
                                                     ProgramInfo &programInfo)
{
    const gl::State &glState                = contextVk->getState();
    const gl::ProgramExecutable *executable = glState.getProgramExecutable();
    ASSERT(executable);

    for (const gl::ShaderType shaderType : executable->getLinkedShaderStages())
    {
        ProgramVk *programVk = getShaderProgram(glState, shaderType);
        if (programVk)
        {
            ANGLE_TRY(programVk->initGraphicsShaderProgram(contextVk, shaderType,
                                                           enableLineRasterEmulation, programInfo));
        }
    }

    return angle::Result::Continue;
}

angle::Result ProgramPipelineVk::initComputeProgram(ContextVk *contextVk, ProgramInfo &programInfo)
{
    const gl::State &glState                = contextVk->getState();
    const gl::ProgramExecutable *executable = glState.getProgramExecutable();
    ASSERT(executable);

    ProgramVk *programVk = getShaderProgram(glState, gl::ShaderType::Compute);
    ASSERT(programVk);
    return programVk->initComputeProgram(contextVk, programInfo);
}

angle::Result ProgramPipelineVk::updateUniforms(ContextVk *contextVk)
{
    uint32_t offsetIndex                      = 0;
    bool anyNewBufferAllocated                = false;
    const gl::ProgramExecutable *glExecutable = contextVk->getState().getProgramExecutable();

    for (const gl::ShaderType shaderType : glExecutable->getLinkedShaderStages())
    {
        ProgramVk *programVk = getShaderProgram(contextVk->getState(), shaderType);
        if (programVk && programVk->dirtyUniforms())
        {
            ANGLE_TRY(programVk->updateShaderUniforms(
                contextVk, shaderType, &mExecutable.mDynamicBufferOffsets[offsetIndex],
                &anyNewBufferAllocated));
        }
        ++offsetIndex;
    }

    if (anyNewBufferAllocated)
    {
        // We need to reinitialize the descriptor sets if we newly allocated buffers since we can't
        // modify the descriptor sets once initialized.
        ANGLE_TRY(mExecutable.allocateDescriptorSet(contextVk, kUniformsAndXfbDescriptorSetIndex));

        mExecutable.mDescriptorBuffersCache.clear();
        for (const gl::ShaderType shaderType : glExecutable->getLinkedShaderStages())
        {
            ProgramVk *programVk = getShaderProgram(contextVk->getState(), shaderType);
            if (programVk)
            {
                mExecutable.updateDefaultUniformsDescriptorSet(
                    shaderType, programVk->getDefaultUniformBlocks(), contextVk);
                mExecutable.updateTransformFeedbackDescriptorSetImpl(programVk->getState(),
                                                                     contextVk);
            }
        }
    }

    return angle::Result::Continue;
}

bool ProgramPipelineVk::dirtyUniforms(const gl::State &glState)
{
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const ProgramVk *program = getShaderProgram(glState, shaderType);
        if (program && program->dirtyUniforms())
        {
            return true;
        }
    }

    return false;
}

}  // namespace rx
