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

    angle::Result link(const gl::Context *context) override;

    uint32_t getLinkedShaderStageCount(const gl::State &glState) const;

    angle::Result transformShaderSpirV(const gl::Context *glContext);
    angle::Result createPipelineLayout(const gl::Context *glContext);
    angle::Result getGraphicsPipeline(const gl::Context *glContext,
                                      gl::PrimitiveMode mode,
                                      const vk::GraphicsPipelineDesc &desc,
                                      const gl::AttributesMask &activeAttribLocations,
                                      const vk::GraphicsPipelineDesc **descPtrOut,
                                      vk::PipelineHelper **pipelineOut);

    angle::Result getComputePipeline(ContextVk *contextVk, vk::PipelineAndSerial **pipelineOut)
    {
        ProgramVk *program = getShaderProgram(contextVk->getState(), gl::ShaderType::Compute);
        ASSERT(program);
        vk::ShaderProgramHelper *shaderProgram;
        ANGLE_TRY(program->initComputeProgram(contextVk, mDefaultProgramInfo, &shaderProgram));
        ASSERT(!shaderProgram->isGraphicsProgram());
        return shaderProgram->getComputePipeline(contextVk, mPipelineLayout.get(), pipelineOut);
    }

    ProgramVk *getShaderProgram(const gl::State &glState, gl::ShaderType shaderType) const
    {
        gl::ProgramPipeline *pipeline = glState.getProgramPipeline();
        const gl::Program *program    = pipeline->getShaderProgram(shaderType);
        if (program)
        {
            return vk::GetImpl(program);
        }
        return nullptr;
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
    bool dirtyUniforms(const gl::State &glState);
    bool hasAtomicCounterBuffers(const gl::State &glState);
    bool hasImages(const gl::State &glState);
    bool hasStorageBuffers(const gl::State &glState);
    bool hasTransformFeedbackOutput(const gl::State &glState);
    bool hasUniformBuffers(const gl::State &glState);
    bool hasDefaultUniforms(const gl::State &glState);
    size_t getTransformFeedbackBufferCount(const gl::State &glState);
    const gl::ActiveTextureMask &getActiveSamplersMask(const gl::State &glState) const;

    // Vertex Shader
    unsigned int getMaxActiveAttribLocation(const gl::State &glState) const;
    const gl::AttributesMask &getActiveAttribLocationsMask(const gl::State &glState) const;
    const gl::AttributesMask &getNonBuiltinAttribLocationsMask(const gl::State &glState) const;

    // Fragment Shader
    bool hasTextures(const gl::State &glState);
    angle::Result updateTexturesDescriptorSet(ContextVk *contextVk);

    GLboolean validate(const gl::Caps &caps, gl::InfoLog *infoLog);

    // Used in testing only.
    vk::DynamicDescriptorPool *getDynamicDescriptorPool(uint32_t poolIndex)
    {
        return &mDynamicDescriptorPools[poolIndex];
    }

  private:
    void initializeSourceOptions(const gl::Context *glContext);
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

    ProgramInfo mDefaultProgramInfo;
    ProgramInfo mLineRasterProgramInfo;

    GlslangSourceOptions mGlslangSourceOptions;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_PROGRAMPIPELINEVK_H_
