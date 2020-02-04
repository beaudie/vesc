//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramHelperVk.h: Defines the interface that ProgramVk and ProgramPipelineVk
// must implement to help users of those objects interact with them.

#ifndef LIBANGLE_PROGRAMHELPERVK_H_
#define LIBANGLE_PROGRAMHELPERVK_H_

#include "common/PackedEnums.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/glslang_wrapper_utils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{

class ShaderInfo final : angle::NonCopyable
{
  public:
    ShaderInfo();
    ~ShaderInfo();

    angle::Result initShaders(ContextVk *contextVk,
                              const gl::ShaderMap<std::string> &shaderSources,
                              const ShaderInterfaceVariableInfoMap &variableInfoMap,
                              gl::ShaderMap<SpirvBlob> *spirvBlobsOut);
    void release(ContextVk *contextVk);

    ANGLE_INLINE bool valid() const { return mIsInitialized; }

    gl::ShaderMap<SpirvBlob> &getSpirvBlobs() { return mSpirvBlobs; }
    const gl::ShaderMap<SpirvBlob> &getSpirvBlobs() const { return mSpirvBlobs; }

  private:
    gl::ShaderMap<SpirvBlob> mSpirvBlobs;
    bool mIsInitialized = false;
};

class ProgramInfo final : angle::NonCopyable
{
  public:
    ProgramInfo();
    ~ProgramInfo();

    angle::Result initProgram(ContextVk *contextVk,
                              const ShaderInfo &shaderInfo,
                              bool enableLineRasterEmulation);
    void release(ContextVk *contextVk);

    ANGLE_INLINE bool valid() const { return mProgramHelper.valid(); }

    vk::ShaderProgramHelper *getShaderProgram() { return &mProgramHelper; }

  private:
    vk::ShaderProgramHelper mProgramHelper;
    gl::ShaderMap<vk::RefCounted<vk::ShaderAndSerial>> mShaders;
};

class ProgramHelperVk
{
  public:
    ProgramHelperVk();
    virtual ~ProgramHelperVk() = 0;

    // Also initializes the pipeline layout, descriptor set layouts, and used descriptor ranges.
    virtual angle::Result updateUniforms(ContextVk *contextVk)              = 0;
    virtual angle::Result updateTexturesDescriptorSet(ContextVk *contextVk) = 0;
    virtual angle::Result updateShaderResourcesDescriptorSet(
        ContextVk *contextVk,
        vk::CommandGraphResource *recorder) = 0;
    virtual angle::Result updateTransformFeedbackDescriptorSet(
        ContextVk *contextVk,
        vk::FramebufferHelper *framebuffer)                                      = 0;
    virtual angle::Result updateDescriptorSets(ContextVk *contextVk,
                                               vk::CommandBuffer *commandBuffer) = 0;

    virtual angle::Result getGraphicsPipeline(ContextVk *contextVk,
                                              gl::PrimitiveMode mode,
                                              const vk::GraphicsPipelineDesc &desc,
                                              const gl::AttributesMask &activeAttribLocations,
                                              const vk::GraphicsPipelineDesc **descPtrOut,
                                              vk::PipelineHelper **pipelineOut)   = 0;
    virtual angle::Result getComputePipeline(ContextVk *contextVk,
                                             vk::PipelineAndSerial **pipelineOut) = 0;

    virtual angle::Result allocateDescriptorSet(ContextVk *contextVk,
                                                uint32_t descriptorSetIndex)                 = 0;
    virtual angle::Result allocateDescriptorSetAndGetInfo(ContextVk *contextVk,
                                                          uint32_t descriptorSetIndex,
                                                          bool *newPoolAllocatedOut)         = 0;
    virtual void updateDefaultUniformsDescriptorSet(ContextVk *contextVk)                    = 0;
    virtual void updateTransformFeedbackDescriptorSetImpl(ContextVk *contextVk)              = 0;
    virtual void updateBuffersDescriptorSet(ContextVk *contextVk,
                                            vk::CommandGraphResource *recorder,
                                            const std::vector<gl::InterfaceBlock> &blocks,
                                            VkDescriptorType descriptorType)                 = 0;
    virtual void updateAtomicCounterBuffersDescriptorSet(ContextVk *contextVk,
                                                         vk::CommandGraphResource *recorder) = 0;
    virtual angle::Result updateImagesDescriptorSet(ContextVk *contextVk,
                                                    vk::CommandGraphResource *recorder)      = 0;

    virtual const vk::PipelineLayout &getPipelineLayout() const = 0;

    virtual bool hasDefaultUniforms() const         = 0;
    virtual bool hasTextures() const                = 0;
    virtual bool hasUniformBuffers() const          = 0;
    virtual bool hasStorageBuffers() const          = 0;
    virtual bool hasAtomicCounterBuffers() const    = 0;
    virtual bool hasImages() const                  = 0;
    virtual bool hasTransformFeedbackOutput() const = 0;
    virtual bool dirtyUniforms() const              = 0;

  protected:
    virtual void updateBindingOffsets() = 0;
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

    ProgramInfo mDefaultProgramInfo;
    ProgramInfo mLineRasterProgramInfo;

    gl::ShaderVector<uint32_t> mDynamicBufferOffsets;

    // Store descriptor pools here. We store the descriptors in the Program to facilitate descriptor
    // cache management. It can also allow fewer descriptors for shaders which use fewer
    // textures/buffers.
    vk::DescriptorSetLayoutArray<vk::DynamicDescriptorPool> mDynamicDescriptorPools;
};

}  // namespace rx

#endif  // LIBANGLE_PROGRAMHELPERVK_H_
