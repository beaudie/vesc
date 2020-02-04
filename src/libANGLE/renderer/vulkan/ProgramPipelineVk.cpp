//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramPipelineVk.cpp:
//    Implements the class methods for ProgramPipelineVk.
//

#include "libANGLE/renderer/vulkan/ProgramPipelineVk.h"

namespace rx
{

ProgramPipelineVk::ProgramPipelineVk(const gl::ProgramPipelineState &state)
    : ProgramPipelineImpl(state)
{}

ProgramPipelineVk::~ProgramPipelineVk() {}

void ProgramPipelineVk::updateBindingOffsets()
{
    UNIMPLEMENTED();
}

angle::Result ProgramPipelineVk::updateUniforms(ContextVk *contextVk)
{
    UNIMPLEMENTED();
    return angle::Result::Stop;
}

angle::Result ProgramPipelineVk::updateTexturesDescriptorSet(ContextVk *contextVk)
{
    UNIMPLEMENTED();
    return angle::Result::Stop;
}

angle::Result ProgramPipelineVk::updateShaderResourcesDescriptorSet(
    ContextVk *contextVk,
    vk::CommandGraphResource *recorder)
{
    UNIMPLEMENTED();
    return angle::Result::Stop;
}

angle::Result ProgramPipelineVk::updateTransformFeedbackDescriptorSet(
    ContextVk *contextVk,
    vk::FramebufferHelper *framebuffer)
{
    UNIMPLEMENTED();
    return angle::Result::Stop;
}

angle::Result ProgramPipelineVk::updateDescriptorSets(ContextVk *contextVk,
                                                      vk::CommandBuffer *commandBuffer)
{
    UNIMPLEMENTED();
    return angle::Result::Stop;
}

angle::Result ProgramPipelineVk::getGraphicsPipeline(
    ContextVk *contextVk,
    gl::PrimitiveMode mode,
    const vk::GraphicsPipelineDesc &desc,
    const gl::AttributesMask &activeAttribLocations,
    const vk::GraphicsPipelineDesc **descPtrOut,
    vk::PipelineHelper **pipelineOut)
{
    UNIMPLEMENTED();
    return angle::Result::Stop;
}

angle::Result ProgramPipelineVk::getComputePipeline(ContextVk *contextVk,
                                                    vk::PipelineAndSerial **pipelineOut)
{
    UNIMPLEMENTED();
    return angle::Result::Stop;
}

angle::Result ProgramPipelineVk::allocateDescriptorSet(ContextVk *contextVk,
                                                       uint32_t descriptorSetIndex)
{
    UNIMPLEMENTED();
    return angle::Result::Stop;
}

angle::Result ProgramPipelineVk::allocateDescriptorSetAndGetInfo(ContextVk *contextVk,
                                                                 uint32_t descriptorSetIndex,
                                                                 bool *newPoolAllocatedOut)
{
    UNIMPLEMENTED();
    return angle::Result::Stop;
}

void ProgramPipelineVk::updateDefaultUniformsDescriptorSet(ContextVk *contextVk)
{
    UNIMPLEMENTED();
}

void ProgramPipelineVk::updateTransformFeedbackDescriptorSetImpl(ContextVk *contextVk)
{
    UNIMPLEMENTED();
}

void ProgramPipelineVk::updateBuffersDescriptorSet(ContextVk *contextVk,
                                                   vk::CommandGraphResource *recorder,
                                                   const std::vector<gl::InterfaceBlock> &blocks,
                                                   VkDescriptorType descriptorType)
{
    UNIMPLEMENTED();
}

void ProgramPipelineVk::updateAtomicCounterBuffersDescriptorSet(ContextVk *contextVk,
                                                                vk::CommandGraphResource *recorder)
{
    UNIMPLEMENTED();
}

angle::Result ProgramPipelineVk::updateImagesDescriptorSet(ContextVk *contextVk,
                                                           vk::CommandGraphResource *recorder)
{
    UNIMPLEMENTED();
    return angle::Result::Stop;
}

bool ProgramPipelineVk::hasDefaultUniforms() const
{
    UNIMPLEMENTED();
    return false;
}

bool ProgramPipelineVk::hasTextures() const
{
    UNIMPLEMENTED();
    return false;
}

bool ProgramPipelineVk::hasUniformBuffers() const
{
    UNIMPLEMENTED();
    return false;
}

bool ProgramPipelineVk::hasStorageBuffers() const
{
    UNIMPLEMENTED();
    return false;
}

bool ProgramPipelineVk::hasAtomicCounterBuffers() const
{
    UNIMPLEMENTED();
    return false;
}

bool ProgramPipelineVk::hasImages() const
{
    UNIMPLEMENTED();
    return false;
}

bool ProgramPipelineVk::hasTransformFeedbackOutput() const
{
    UNIMPLEMENTED();
    return false;
}

bool ProgramPipelineVk::dirtyUniforms() const
{
    UNIMPLEMENTED();
    return false;
}

}  // namespace rx
