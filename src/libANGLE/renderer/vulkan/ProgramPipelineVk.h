//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramPipelineVk.h:
//    Defines the class interface for ProgramPipelineVk, implementing ProgramPipelineImpl.
//

#ifndef LIBANGLE_RENDERER_VULKAN_PROGRAMPIPELINEVK_H_
#define LIBANGLE_RENDERER_VULKAN_PROGRAMPIPELINEVK_H_

#include "libANGLE/renderer/ProgramPipelineImpl.h"
#include "libANGLE/renderer/vulkan/ProgramHelperVk.h"

namespace rx
{

class ProgramPipelineVk : public ProgramPipelineImpl, public ProgramHelperVk
{
  public:
    ProgramPipelineVk(const gl::ProgramPipelineState &state);
    ~ProgramPipelineVk() override;

    // ProgramHelperVk Interface
    void updateBindingOffsets() override;
    // Also initializes the pipeline layout, descriptor set layouts, and used descriptor ranges.
    angle::Result updateUniforms(ContextVk *contextVk) override;
    angle::Result updateTexturesDescriptorSet(ContextVk *contextVk) override;
    angle::Result updateShaderResourcesDescriptorSet(ContextVk *contextVk,
                                                     vk::CommandGraphResource *recorder) override;
    angle::Result updateTransformFeedbackDescriptorSet(ContextVk *contextVk,
                                                       vk::FramebufferHelper *framebuffer) override;
    angle::Result updateDescriptorSets(ContextVk *contextVk,
                                       vk::CommandBuffer *commandBuffer) override;

    angle::Result getGraphicsPipeline(ContextVk *contextVk,
                                      gl::PrimitiveMode mode,
                                      const vk::GraphicsPipelineDesc &desc,
                                      const gl::AttributesMask &activeAttribLocations,
                                      const vk::GraphicsPipelineDesc **descPtrOut,
                                      vk::PipelineHelper **pipelineOut) override;
    angle::Result getComputePipeline(ContextVk *contextVk,
                                     vk::PipelineAndSerial **pipelineOut) override;

    angle::Result allocateDescriptorSet(ContextVk *contextVk, uint32_t descriptorSetIndex) override;
    angle::Result allocateDescriptorSetAndGetInfo(ContextVk *contextVk,
                                                  uint32_t descriptorSetIndex,
                                                  bool *newPoolAllocatedOut) override;
    void updateDefaultUniformsDescriptorSet(ContextVk *contextVk) override;
    void updateTransformFeedbackDescriptorSetImpl(ContextVk *contextVk) override;
    void updateBuffersDescriptorSet(ContextVk *contextVk,
                                    vk::CommandGraphResource *recorder,
                                    const std::vector<gl::InterfaceBlock> &blocks,
                                    VkDescriptorType descriptorType) override;
    void updateAtomicCounterBuffersDescriptorSet(ContextVk *contextVk,
                                                 vk::CommandGraphResource *recorder) override;
    angle::Result updateImagesDescriptorSet(ContextVk *contextVk,
                                            vk::CommandGraphResource *recorder) override;

    bool hasDefaultUniforms() const override;
    bool hasTextures() const override;
    bool hasUniformBuffers() const override;
    bool hasStorageBuffers() const override;
    bool hasAtomicCounterBuffers() const override;
    bool hasImages() const override;
    bool hasTransformFeedbackOutput() const override;
    bool dirtyUniforms() const override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_PROGRAMPIPELINEVK_H_
