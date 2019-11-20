//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramPipelineVk.cpp:
//    Implements the class methods for ProgramPipelineVk.
//

#include "libANGLE/renderer/vulkan/ProgramPipelineVk.h"

#include "libANGLE/Context.h"

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
    mProgramHelper.release(contextVk);
}

void ProgramPipelineVk::updateShaderStages(const gl::State &glState)
{
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const gl::Program *program = glState.getProgramPipeline()->getShaderProgram(shaderType);
        if (program)
        {
            mPrograms[shaderType] = vk::GetImpl(program);
        }
        else
        {
            mPrograms[shaderType] = nullptr;
        }
    }
}

angle::Result ProgramPipelineVk::getGraphicsPipeline(
    ContextVk *contextVk,
    gl::PrimitiveMode mode,
    const vk::GraphicsPipelineDesc &desc,
    const gl::AttributesMask &activeAttribLocations,
    const vk::GraphicsPipelineDesc **descPtrOut,
    vk::PipelineHelper **pipelineOut)
{
    bool enableLineRasterEmulation = contextVk->useLineRaster(mode);

    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        ProgramVk *program = mPrograms[shaderType];
        if (program)
        {
            ANGLE_TRY(program->initGraphicsShaders(contextVk, mode, enableLineRasterEmulation,
                                                   &mProgramHelper));
        }
    }

    ASSERT(mProgramHelper.isGraphicsProgram());
    RendererVk *renderer             = contextVk->getRenderer();
    vk::PipelineCache *pipelineCache = nullptr;
    ANGLE_TRY(renderer->getPipelineCache(&pipelineCache));
    ProgramVk *vertProgram = mPrograms[gl::ShaderType::Vertex];
    return mProgramHelper.getGraphicsPipeline(
        contextVk, &contextVk->getRenderPassCache(), *pipelineCache,
        contextVk->getCurrentQueueSerial(), vertProgram->getPipelineLayout(), desc,
        activeAttribLocations, vertProgram->getState().getAttributesTypeMask(), descPtrOut,
        pipelineOut);
}

bool ProgramPipelineVk::dirtyUniforms()
{
    bool dirtyUniforms = false;

    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const ProgramVk *program = getShaderProgram(shaderType);
        if (program)
        {
            dirtyUniforms = dirtyUniforms || program->dirtyUniforms();
        }
    }

    return dirtyUniforms;
}

bool ProgramPipelineVk::hasAtomicCounterBuffers()
{
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const ProgramVk *program = getShaderProgram(shaderType);
        if (program && program->hasAtomicCounterBuffers())
        {
            return true;
        }
    }

    return false;
}

bool ProgramPipelineVk::hasImages()
{
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const ProgramVk *program = getShaderProgram(shaderType);
        if (program && program->hasImages())
        {
            return true;
        }
    }

    return false;
}

bool ProgramPipelineVk::hasStorageBuffers()
{
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const ProgramVk *program = getShaderProgram(shaderType);
        if (program && program->hasStorageBuffers())
        {
            return true;
        }
    }

    return false;
}

bool ProgramPipelineVk::hasTransformFeedbackOutput()
{
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const ProgramVk *program = getShaderProgram(shaderType);
        if (program && program->hasTransformFeedbackOutput())
        {
            return true;
        }
    }

    return false;
}

bool ProgramPipelineVk::hasUniformBuffers()
{
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const ProgramVk *program = getShaderProgram(shaderType);
        if (program && program->hasUniformBuffers())
        {
            return true;
        }
    }

    return false;
}

angle::Result ProgramPipelineVk::updateShaderResourcesDescriptorSet(
    ContextVk *contextVk,
    vk::CommandGraphResource *recorder)
{
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        ProgramVk *program = mPrograms[shaderType];
        if (program)
        {
            ANGLE_TRY(program->updateShaderResourcesDescriptorSet(contextVk, recorder));
        }
    }

    return angle::Result::Continue;
}

angle::Result ProgramPipelineVk::updateTransformFeedbackDescriptorSet(
    ContextVk *contextVk,
    vk::FramebufferHelper *framebuffer)
{
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        ProgramVk *program = mPrograms[shaderType];
        if (program)
        {
            ANGLE_TRY(program->updateTransformFeedbackDescriptorSet(contextVk, framebuffer));
        }
    }

    return angle::Result::Continue;
}

angle::Result ProgramPipelineVk::updateUniforms(ContextVk *contextVk)
{
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        ProgramVk *program = mPrograms[shaderType];
        if (program)
        {
            ANGLE_TRY(program->updateUniforms(contextVk));
        }
    }

    return angle::Result::Continue;
}

angle::Result ProgramPipelineVk::updateDescriptorSets(ContextVk *contextVk,
                                                      vk::CommandBuffer *commandBuffer)
{
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        ProgramVk *program = mPrograms[shaderType];
        if (program)
        {
            ANGLE_TRY(program->updateDescriptorSets(contextVk, commandBuffer));
        }
    }

    return angle::Result::Continue;
}

void ProgramPipelineVk::invalidateCurrentShaderResources(ContextVk *contextVk)
{
    for (const gl::ShaderType shaderType : gl::kAllGraphicsShaderTypes)
    {
        ProgramVk *programVk = mPrograms[shaderType];
        if (programVk)
        {
            contextVk->invalidateCurrentShaderResources(programVk);
        }
    }
}

unsigned int ProgramPipelineVk::getMaxActiveAttribLocation() const
{
    const ProgramVk *program = getShaderProgram(gl::ShaderType::Vertex);
    return program->getState().getMaxActiveAttribLocation();
}

const gl::AttributesMask &ProgramPipelineVk::getActiveAttribLocationsMask() const
{
    const ProgramVk *program = getShaderProgram(gl::ShaderType::Vertex);
    return program->getState().getActiveAttribLocationsMask();
}

const gl::AttributesMask &ProgramPipelineVk::getNonBuiltinAttribLocationsMask() const
{
    const ProgramVk *program = getShaderProgram(gl::ShaderType::Vertex);
    return program->getState().getNonBuiltinAttribLocationsMask();
}

bool ProgramPipelineVk::hasTextures()
{
    return getShaderProgram(gl::ShaderType::Fragment)->hasTextures();
}

angle::Result ProgramPipelineVk::updateTexturesDescriptorSet(ContextVk *contextVk)
{
    return mPrograms[gl::ShaderType::Fragment]->updateTexturesDescriptorSet(contextVk);
}

GLboolean ProgramPipelineVk::validate(const gl::Caps &caps, gl::InfoLog *infoLog)
{
    for (const gl::ShaderType shaderType : gl::kAllGraphicsShaderTypes)
    {
        ProgramVk *programVk = mPrograms[shaderType];
        if (programVk)
        {
            bool result = programVk->validate(caps, infoLog);
            if (!result)
            {
                return GL_FALSE;
            }
        }
    }

    return GL_TRUE;
}

}  // namespace rx
