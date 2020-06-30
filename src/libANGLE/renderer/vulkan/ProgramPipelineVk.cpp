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
{
    mExecutable.setProgramPipeline(this);
}

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

// TODO: http://anglebug.com/3570: Move/Copy all of the necessary information into
// the ProgramExecutable, so this function can be removed.
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
    GlslangProgramInterfaceInfo glslangProgramInterfaceInfo;
    GlslangWrapperVk::ResetGlslangProgramInterfaceInfo(&glslangProgramInterfaceInfo);

    mExecutable.clearVariableInfoMap();

    // Now that the program pipeline has all of the programs attached, the various descriptor
    // set/binding locations need to be re-assigned to their correct values.
    for (const gl::ShaderType shaderType : glPipeline->getExecutable().getLinkedShaderStages())
    {
        gl::Program *glProgram =
            const_cast<gl::Program *>(glPipeline->getShaderProgram(shaderType));
        if (glProgram)
        {
            // The program interface info must survive across shaders, except
            // for some program-specific values.
            ProgramVk *programVk = vk::GetImpl(glProgram);
            GlslangProgramInterfaceInfo &programProgramInterfaceInfo =
                programVk->getGlslangProgramInterfaceInfo();
            glslangProgramInterfaceInfo.locationsUsedForXfbExtension =
                programProgramInterfaceInfo.locationsUsedForXfbExtension;

            GlslangAssignLocations(options, glProgram->getState().getExecutable(), shaderType,
                                   &glslangProgramInterfaceInfo,
                                   &mExecutable.getShaderInterfaceVariableInfoMap());
        }
    }

    return mExecutable.createPipelineLayout(glContext);
}

size_t ProgramPipelineVk::calcUniformUpdateRequiredSpace(
    ContextVk *contextVk,
    const gl::ProgramExecutable &glExecutable,
    const gl::State &glState,
    gl::ShaderMap<VkDeviceSize> &uniformOffsets) const
{
    size_t requiredSpace = 0;
    for (const gl::ShaderType shaderType : glExecutable.getLinkedShaderStages())
    {
        ProgramVk *programVk = getShaderProgram(glState, shaderType);
        if (programVk && programVk->isShaderUniformDirty(shaderType))
        {
            uniformOffsets[shaderType] = requiredSpace;
            requiredSpace += programVk->getDefaultUniformAlignedSize(contextVk, shaderType);
        }
    }
    return requiredSpace;
}

angle::Result ProgramPipelineVk::updateUniforms(ContextVk *contextVk)
{
    const gl::State &glState                  = contextVk->getState();
    const gl::ProgramExecutable &glExecutable = *glState.getProgramExecutable();
    vk::DynamicBuffer &defaultUniformStorage  = contextVk->getDefaultUniformStorage();
    uint8_t *bufferData                       = nullptr;
    VkDeviceSize bufferOffset                 = 0;
    uint32_t offsetIndex                      = 0;
    gl::ShaderMap<VkDeviceSize> offsets;
    size_t requiredSpace;

    // We usually only update uniform data for shader stages that are actually dirty. But when the
    // buffer for uniform data have switched, because all shader stages are using the same buffer,
    // we then must update uniform data for all shader stages to keep all shader stages' unform data
    // in the same buffer.
    bool retry;
    do
    {
        retry         = false;
        requiredSpace = calcUniformUpdateRequiredSpace(contextVk, glExecutable, glState, offsets);
        if (requiredSpace)
        {
            // Allocate space from dynamicBuffer
            defaultUniformStorage.releaseInFlightBuffers(contextVk);
            bool bufferModified = false;
            ANGLE_TRY(defaultUniformStorage.allocate(contextVk, requiredSpace, &bufferData, nullptr,
                                                     &bufferOffset, &bufferModified));
            if (bufferModified)
            {
                // We only need to retry if we actually end up adding new dirty bits
                retry = setAllDefaultUniformsDirty(contextVk);
            }
        }
    } while (retry);

    for (const gl::ShaderType shaderType : glExecutable.getLinkedShaderStages())
    {
        ProgramVk *programVk = getShaderProgram(glState, shaderType);
        if (programVk && programVk->isShaderUniformDirty(shaderType))
        {
            const angle::MemoryBuffer &uniformData =
                programVk->getDefaultUniformBlocks()[shaderType].uniformData;
            memcpy(&bufferData[offsets[shaderType]], uniformData.data(), uniformData.size());
            mExecutable.mDynamicBufferOffsets[offsetIndex] =
                static_cast<uint32_t>(bufferOffset + offsets[shaderType]);
        }
        ++offsetIndex;
    }
    if (requiredSpace)
    {
        ANGLE_TRY(defaultUniformStorage.flush(contextVk));
    }

    if (!mExecutable.isDefaultUniformDescriptorSetValid(defaultUniformStorage.getCurrentBuffer()))
    {
        // We need to reinitialize the descriptor sets if we newly allocated buffers since we can't
        // modify the descriptor sets once initialized.
        ANGLE_TRY(mExecutable.allocateDescriptorSet(contextVk, kUniformsAndXfbDescriptorSetIndex));

        mExecutable.mDescriptorBuffersCache.clear();
        for (const gl::ShaderType shaderType : glExecutable.getLinkedShaderStages())
        {
            ProgramVk *programVk = getShaderProgram(glState, shaderType);
            if (programVk)
            {
                mExecutable.updateDefaultUniformsDescriptorSet(
                    shaderType, programVk->getDefaultUniformBlocks(),
                    defaultUniformStorage.getCurrentBuffer(), contextVk);
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

bool ProgramPipelineVk::setAllDefaultUniformsDirty(ContextVk *contextVk)
{
    const gl::State &glState                  = contextVk->getState();
    const gl::ProgramExecutable &glExecutable = *glState.getProgramExecutable();

    bool newDirtyBitAdded = false;
    for (const gl::ShaderType shaderType : glExecutable.getLinkedShaderStages())
    {
        ProgramVk *program = getShaderProgram(glState, shaderType);
        if (program)
        {
            newDirtyBitAdded |= program->setShaderUniformDirty(shaderType);
        }
    }
    return newDirtyBitAdded;
}

}  // namespace rx
