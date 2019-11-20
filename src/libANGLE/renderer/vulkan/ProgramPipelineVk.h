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

#include "common/utilities.h"
#include "libANGLE/renderer/glslang_wrapper_utils.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/ProgramVk.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{

class ProgramPipelineVk : public ProgramPipelineImpl
{
  public:
    ProgramPipelineVk(const gl::ProgramPipelineState &state);
    ~ProgramPipelineVk() override;

    void destroy(const gl::Context *context) override;
    void reset(ContextVk *contextVk);

    void updateShaderStages(const gl::State &glState);

    const gl::ShaderBitSet &getLinkedShaderStages() const { return mLinkedShaderStages; }
    bool hasLinkedShaderStage(gl::ShaderType shaderType) const
    {
        return mLinkedShaderStages[shaderType];
    }
    size_t getLinkedShaderStageCount() const { return mLinkedShaderStages.count(); }

    angle::Result createPipelineLayout(const gl::Context *glContext);
    angle::Result getGraphicsPipeline(const gl::Context *glContext,
                                      gl::PrimitiveMode mode,
                                      const vk::GraphicsPipelineDesc &desc,
                                      const gl::AttributesMask &activeAttribLocations,
                                      const vk::GraphicsPipelineDesc **descPtrOut,
                                      vk::PipelineHelper **pipelineOut);

    angle::Result getComputePipeline(ContextVk *contextVk, vk::PipelineAndSerial **pipelineOut)
    {
        ProgramVk *program = mPrograms[gl::ShaderType::Compute];
        ASSERT(program);
        ANGLE_TRY(program->initComputeShader(contextVk, &mProgramHelper));
        ASSERT(!mProgramHelper.isGraphicsProgram());
        return mProgramHelper.getComputePipeline(contextVk, mPipelineLayout.get(), pipelineOut);
    }

    const ProgramVk *getShaderProgram(gl::ShaderType shaderType) const
    {
        return mPrograms[shaderType];
    }

    // Descriptors
    angle::Result allocateDescriptorSet(ContextVk *contextVk, uint32_t descriptorSetIndex);
    angle::Result allocateDescriptorSetAndGetInfo(ContextVk *contextVk,
                                                  uint32_t descriptorSetIndex,
                                                  bool *newPoolAllocatedOut);
    void updateDefaultUniformsDescriptorSet(ContextVk *contextVk);
    angle::Result updateShaderResourcesDescriptorSet(ContextVk *contextVk,
                                                     vk::CommandGraphResource *recorder);
    angle::Result updateTransformFeedbackDescriptorSet(ContextVk *contextVk,
                                                       vk::FramebufferHelper *framebuffer);
    angle::Result updateUniforms(ContextVk *contextVk);
    angle::Result updateDescriptorSets(ContextVk *contextVk, vk::CommandBuffer *commandBuffer);
    void invalidateCurrentShaderResources(ContextVk *contextVk);

    const vk::PipelineLayout &getPipelineLayout() const { return mPipelineLayout.get(); }

    // All Shaders
    bool dirtyUniforms();
    bool hasAtomicCounterBuffers();
    bool hasImages();
    bool hasStorageBuffers();
    bool hasTransformFeedbackOutput();
    bool hasUniformBuffers();

    // Vertex Shader
    unsigned int getMaxActiveAttribLocation() const;
    const gl::AttributesMask &getActiveAttribLocationsMask() const;
    const gl::AttributesMask &getNonBuiltinAttribLocationsMask() const;

    // Fragment Shader
    bool hasTextures();
    angle::Result updateTexturesDescriptorSet(ContextVk *contextVk);

    GLboolean validate(const gl::Caps &caps, gl::InfoLog *infoLog);

    // Used in testing only.
    vk::DynamicDescriptorPool *getDynamicDescriptorPool(uint32_t poolIndex)
    {
        return &mDynamicDescriptorPools[poolIndex];
    }

  private:
    GlslangSourceOptions createSourceOptions(const gl::Context *glContext);
    void updateBindingOffsets(const gl::Context *glContext);
    uint32_t getUniformBlockBindingsOffset() const { return 0; }
    uint32_t getStorageBlockBindingsOffset() const { return mStorageBlockBindingsOffset; }
    uint32_t getAtomicCounterBufferBindingsOffset() const
    {
        return mAtomicCounterBufferBindingsOffset;
    }
    uint32_t getImageBindingsOffset() const { return mImageBindingsOffset; }

    // In their descriptor set, uniform buffers are placed first, then storage buffers, then atomic
    // counter buffers and then images.  These cached values contain the offsets where storage
    // buffer, atomic counter buffer and image bindings start.
    uint32_t mStorageBlockBindingsOffset;
    uint32_t mAtomicCounterBufferBindingsOffset;
    uint32_t mImageBindingsOffset;

    // This is a special "empty" placeholder buffer for when a shader has no uniforms or doesn't
    // use all slots in the atomic counter buffer array.
    //
    // It is necessary because we want to keep a compatible pipeline layout in all cases,
    // and Vulkan does not tolerate having null handles in a descriptor set.
    vk::BufferHelper mEmptyBuffer;

    // Descriptor sets for uniform blocks and textures for this program.
    std::vector<VkDescriptorSet> mDescriptorSets;
    vk::DescriptorSetLayoutArray<VkDescriptorSet> mEmptyDescriptorSets;
    std::vector<vk::BufferHelper *> mDescriptorBuffersCache;

    std::unordered_map<vk::TextureDescriptorDesc, VkDescriptorSet> mTextureDescriptorsCache;

    // We keep a reference to the pipeline and descriptor set layouts. This ensures they don't get
    // deleted while this program is in use.
    vk::BindingPointer<vk::PipelineLayout> mPipelineLayout;
    vk::DescriptorSetLayoutPointerArray mDescriptorSetLayouts;

    // Keep bindings to the descriptor pools. This ensures the pools stay valid while the Program
    // is in use.
    vk::DescriptorSetLayoutArray<vk::RefCountedDescriptorPoolBinding> mDescriptorPoolBindings;

    // Store descriptor pools here. We store the descriptors in the Program to facilitate descriptor
    // cache management. It can also allow fewer descriptors for shaders which use fewer
    // textures/buffers.
    vk::DescriptorSetLayoutArray<vk::DynamicDescriptorPool> mDynamicDescriptorPools;

    gl::ShaderVector<uint32_t> mDynamicBufferOffsets;

    // The shader programs for each stage.
    gl::ShaderMap<ProgramVk *> mPrograms;
    gl::ShaderBitSet mLinkedShaderStages;

    vk::ShaderProgramHelper mProgramHelper;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_PROGRAMPIPELINEVK_H_
