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
#include "libANGLE/Program.h"

namespace rx
{

ProgramPipelineVk::ProgramPipelineVk(const gl::ProgramPipelineState &state)
    : ProgramPipelineImpl(state),
      mStorageBlockBindingsOffset(0),
      mAtomicCounterBufferBindingsOffset(0),
      mImageBindingsOffset(0),
      mDynamicBufferOffsets{}
{}

ProgramPipelineVk::~ProgramPipelineVk() {}

void ProgramPipelineVk::destroy(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);
    reset(contextVk);
}

void ProgramPipelineVk::reset(ContextVk *contextVk)
{
    RendererVk *renderer = contextVk->getRenderer();

    for (auto &descriptorSetLayout : mDescriptorSetLayouts)
    {
        descriptorSetLayout.reset();
    }
    mPipelineLayout.reset();

    mEmptyBuffer.release(renderer);

    mProgramHelper.release(contextVk);

    mDescriptorSets.clear();
    mEmptyDescriptorSets.fill(VK_NULL_HANDLE);

    for (vk::RefCountedDescriptorPoolBinding &binding : mDescriptorPoolBindings)
    {
        binding.reset();
    }

    for (vk::DynamicDescriptorPool &descriptorPool : mDynamicDescriptorPools)
    {
        descriptorPool.release(contextVk);
    }

    mTextureDescriptorsCache.clear();
    mDescriptorBuffersCache.clear();

    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        mPrograms[shaderType] = nullptr;
    }
}

void ProgramPipelineVk::updateShaderStages(const gl::State &glState)
{
    mLinkedShaderStages.reset();

    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const gl::Program *program = glState.getProgramPipeline()->getShaderProgram(shaderType);
        if (program)
        {
            mPrograms[shaderType] = vk::GetImpl(program);
            mLinkedShaderStages.set(shaderType);
        }
        else
        {
            mPrograms[shaderType] = nullptr;
        }
    }
}

void ProgramPipelineVk::updateBindingOffsets(const gl::Context *glContext)
{
    const gl::ProgramPipeline *glPipeline = glContext->getState().getProgramPipeline();
    uint32_t uniqueUniformBlockCount      = 0;
    uint32_t uniqueStorageBlockCount      = 0;
    uint32_t atomicCounterBindingCount    = 0;

    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const gl::Program *program = glPipeline->getShaderProgram(shaderType);
        if (program)
        {
            uniqueUniformBlockCount += program->getState().getUniqueUniformBlockCount();
            uniqueStorageBlockCount += program->getState().getUniqueStorageBlockCount();
            atomicCounterBindingCount +=
                program->getState().getAtomicCounterBuffers().empty() ? 0 : 1;
        }
    }

    mStorageBlockBindingsOffset = static_cast<uint32_t>(uniqueUniformBlockCount);

    mAtomicCounterBufferBindingsOffset =
        static_cast<uint32_t>(mStorageBlockBindingsOffset + uniqueStorageBlockCount);

    mImageBindingsOffset = mAtomicCounterBufferBindingsOffset + atomicCounterBindingCount;
}

GlslangSourceOptions ProgramPipelineVk::createSourceOptions(const gl::Context *glContext)
{
    const gl::State &glState              = glContext->getState();
    const gl::ProgramPipeline *glPipeline = glState.getProgramPipeline();

    GlslangSourceOptions options;
    options.uniformsAndXfbDescriptorSetIndex = kUniformsAndXfbDescriptorSetIndex;
    options.textureDescriptorSetIndex        = kTextureDescriptorSetIndex;
    options.shaderResourceDescriptorSetIndex = kShaderResourceDescriptorSetIndex;
    options.driverUniformsDescriptorSetIndex = kDriverUniformsDescriptorSetIndex;
    options.xfbBindingIndexStart             = kXfbBindingIndexStart;

    uint32_t uniformBindingIndex = 0;
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const gl::Program *program = glPipeline->getShaderProgram(shaderType);
        options.uniformsAndXfbDescriptorSetIndexMap[shaderType] = uniformBindingIndex;
        if (program)
        {
            uniformBindingIndex++;
        }
    }

    return options;
}

// TODO(jie.a.chen@intel.com): Parallelize linking.
// http://crbug.com/849576
angle::Result ProgramPipelineVk::createPipelineLayout(const gl::Context *glContext)
{
    const gl::State &glState                 = glContext->getState();
    const gl::ProgramPipeline *glPipeline    = glState.getProgramPipeline();
    ContextVk *contextVk                     = vk::GetImpl(glContext);
    RendererVk *renderer                     = contextVk->getRenderer();
    gl::TransformFeedback *transformFeedback = glState.getCurrentTransformFeedback();
    uint32_t uniformBlockCount               = 0;

    updateBindingOffsets(glContext);

    // Store a reference to the pipeline and descriptor set layouts. This will create them if they
    // don't already exist in the cache.

    // Default uniforms and transform feedback:
    vk::DescriptorSetLayoutDesc uniformsAndXfbSetDesc;
    uint32_t uniformBindingIndex = 0;

    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const gl::Program *program = glPipeline->getShaderProgram(shaderType);
        if (program)
        {
            uniformBlockCount +=
                static_cast<uint32_t>(program->getState().getUniformBlocks().size());

            uniformsAndXfbSetDesc.update(uniformBindingIndex++,
                                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1,
                                         gl_vk::kShaderStageMap[shaderType]);
        }
    }
    const gl::Program *vertProgram = glPipeline->getShaderProgram(gl::ShaderType::Vertex);
    if (vertProgram && transformFeedback &&
        !vertProgram->getState().getLinkedTransformFeedbackVaryings().empty())
    {
        TransformFeedbackVk *transformFeedbackVk = vk::GetImpl(transformFeedback);
        transformFeedbackVk->updateDescriptorSetLayout(contextVk, vertProgram->getState(),
                                                       &uniformsAndXfbSetDesc);
    }

    ANGLE_TRY(renderer->getDescriptorSetLayout(
        contextVk, uniformsAndXfbSetDesc,
        &mDescriptorSetLayouts[kUniformsAndXfbDescriptorSetIndex]));

#if 0  // TIMTIM
    // Uniform and storage buffers, atomic counter buffers and images:
    vk::DescriptorSetLayoutDesc resourcesSetDesc;

    AddInterfaceBlockDescriptorSetDesc(mState.getUniformBlocks(), getUniformBlockBindingsOffset(),
                                       VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &resourcesSetDesc);
    AddInterfaceBlockDescriptorSetDesc(mState.getShaderStorageBlocks(),
                                       getStorageBlockBindingsOffset(),
                                       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &resourcesSetDesc);
    AddAtomicCounterBufferDescriptorSetDesc(mState.getAtomicCounterBuffers(),
                                            getAtomicCounterBufferBindingsOffset(),
                                            &resourcesSetDesc);
    AddImageDescriptorSetDesc(mState, getImageBindingsOffset(), &resourcesSetDesc);

    ANGLE_TRY(renderer->getDescriptorSetLayout(
        contextVk, resourcesSetDesc, &mDescriptorSetLayouts[kShaderResourceDescriptorSetIndex]));

    // Textures:
    vk::DescriptorSetLayoutDesc texturesSetDesc;

    AddTextureDescriptorSetDesc(mState, contextVk->useOldRewriteStructSamplers(), &texturesSetDesc);

    ANGLE_TRY(renderer->getDescriptorSetLayout(contextVk, texturesSetDesc,
                                               &mDescriptorSetLayouts[kTextureDescriptorSetIndex]));
#endif

    // Driver uniforms:
    VkShaderStageFlags driverUniformsStages =
        mState.isCompute() ? VK_SHADER_STAGE_COMPUTE_BIT : VK_SHADER_STAGE_ALL_GRAPHICS;
    vk::DescriptorSetLayoutDesc driverUniformsSetDesc =
        contextVk->getDriverUniformsDescriptorSetDesc(driverUniformsStages);
    ANGLE_TRY(renderer->getDescriptorSetLayout(
        contextVk, driverUniformsSetDesc,
        &mDescriptorSetLayouts[kDriverUniformsDescriptorSetIndex]));

    // Create pipeline layout with these 4 descriptor sets.
    vk::PipelineLayoutDesc pipelineLayoutDesc;
    pipelineLayoutDesc.updateDescriptorSetLayout(kUniformsAndXfbDescriptorSetIndex,
                                                 uniformsAndXfbSetDesc);
#if 0  // TIMTIM
    pipelineLayoutDesc.updateDescriptorSetLayout(kShaderResourceDescriptorSetIndex,
                                                 resourcesSetDesc);
    pipelineLayoutDesc.updateDescriptorSetLayout(kTextureDescriptorSetIndex, texturesSetDesc);
#endif
    pipelineLayoutDesc.updateDescriptorSetLayout(kDriverUniformsDescriptorSetIndex,
                                                 driverUniformsSetDesc);

    ANGLE_TRY(renderer->getPipelineLayout(contextVk, pipelineLayoutDesc, mDescriptorSetLayouts,
                                          &mPipelineLayout));

    // Initialize descriptor pools.
    std::array<VkDescriptorPoolSize, 2> uniformAndXfbSetSize = {
        {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
          static_cast<uint32_t>(glPipeline->getLinkedShaderStageCount())},
         {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_BUFFERS}}};

#if 0  // TIMTIM
    uint32_t storageBlockCount = static_cast<uint32_t>(mState.getShaderStorageBlocks().size());
    uint32_t atomicCounterBufferCount =
        static_cast<uint32_t>(mState.getAtomicCounterBuffers().size());
    uint32_t imageCount   = static_cast<uint32_t>(mState.getImageBindings().size());
    uint32_t textureCount = static_cast<uint32_t>(mState.getSamplerBindings().size());
#endif

    if (renderer->getFeatures().bindEmptyForUnusedDescriptorSets.enabled)
    {
        // For this workaround, we have to create an empty descriptor set for each descriptor set
        // index, so make sure their pools are initialized.
        uniformBlockCount = std::max(uniformBlockCount, 1u);
        //        textureCount      = std::max(textureCount, 1u);
    }

    constexpr size_t kResourceTypesInResourcesSet = 3;
    angle::FixedVector<VkDescriptorPoolSize, kResourceTypesInResourcesSet> resourceSetSize;
    if (uniformBlockCount > 0)
    {
        resourceSetSize.emplace_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniformBlockCount);
    }
#if 0  // TIMTIM
    if (storageBlockCount > 0 || atomicCounterBufferCount > 0)
    {
        // Note that we always use an array of IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFERS storage
        // buffers for emulating atomic counters, so if there are any atomic counter buffers, we
        // need to allocate IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFERS descriptors.
        const uint32_t atomicCounterStorageBufferCount =
            atomicCounterBufferCount > 0 ? gl::IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFERS : 0;
        const uint32_t storageBufferDescCount = storageBlockCount + atomicCounterStorageBufferCount;
        resourceSetSize.emplace_back(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, storageBufferDescCount);
    }
    if (imageCount > 0)
    {
        resourceSetSize.emplace_back(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, imageCount);
    }

    VkDescriptorPoolSize textureSetSize = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, textureCount};
#endif

    ANGLE_TRY(mDynamicDescriptorPools[kUniformsAndXfbDescriptorSetIndex].init(
        contextVk, uniformAndXfbSetSize.data(), uniformAndXfbSetSize.size()));
    if (resourceSetSize.size() > 0)
    {
        ANGLE_TRY(mDynamicDescriptorPools[kShaderResourceDescriptorSetIndex].init(
            contextVk, resourceSetSize.data(), static_cast<uint32_t>(resourceSetSize.size())));
    }
#if 0  // TIMTIM
    if (textureCount > 0)
    {
        ANGLE_TRY(mDynamicDescriptorPools[kTextureDescriptorSetIndex].init(contextVk,
                                                                           &textureSetSize, 1));
    }
#endif

    mDynamicBufferOffsets.resize(glPipeline->getLinkedShaderStageCount());

    // Initialize an "empty" buffer for use with default uniform blocks where there are no uniforms,
    // or atomic counter buffer array indices that are unused.
    constexpr VkBufferUsageFlags kEmptyBufferUsage =
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VkBufferCreateInfo emptyBufferInfo    = {};
    emptyBufferInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    emptyBufferInfo.flags                 = 0;
    emptyBufferInfo.size                  = 4;
    emptyBufferInfo.usage                 = kEmptyBufferUsage;
    emptyBufferInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    emptyBufferInfo.queueFamilyIndexCount = 0;
    emptyBufferInfo.pQueueFamilyIndices   = nullptr;

    constexpr VkMemoryPropertyFlags kMemoryType = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    return mEmptyBuffer.init(contextVk, emptyBufferInfo, kMemoryType);
}

angle::Result ProgramPipelineVk::getGraphicsPipeline(
    const gl::Context *glContext,
    gl::PrimitiveMode mode,
    const vk::GraphicsPipelineDesc &desc,
    const gl::AttributesMask &activeAttribLocations,
    const vk::GraphicsPipelineDesc **descPtrOut,
    vk::PipelineHelper **pipelineOut)
{
    ContextVk *contextVk                  = vk::GetImpl(glContext);
    bool enableLineRasterEmulation        = contextVk->useLineRaster(mode);
    const gl::State &glState              = glContext->getState();
    const gl::ProgramPipeline *glPipeline = glState.getProgramPipeline();
    const GlslangSourceOptions options    = createSourceOptions(glContext);

    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        gl::Program *glProgram =
            const_cast<gl::Program *>(glPipeline->getShaderProgram(shaderType));
        if (glProgram)
        {
            ProgramVk *programVk = mPrograms[shaderType];
            programVk->setGlslangSourceOptions(options);
            glProgram->resolveLink(glContext);
            ANGLE_TRY(glProgram->link(glContext));
            ANGLE_TRY(programVk->initGraphicsShaders(contextVk, mode, enableLineRasterEmulation,
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
        contextVk->getCurrentQueueSerial(), getPipelineLayout(), desc, activeAttribLocations,
        vertProgram->getState().getAttributesTypeMask(), descPtrOut, pipelineOut);
}

angle::Result ProgramPipelineVk::allocateDescriptorSet(ContextVk *contextVk,
                                                       uint32_t descriptorSetIndex)
{
    bool ignoreNewPoolAllocated;
    return allocateDescriptorSetAndGetInfo(contextVk, descriptorSetIndex, &ignoreNewPoolAllocated);
}

angle::Result ProgramPipelineVk::allocateDescriptorSetAndGetInfo(ContextVk *contextVk,
                                                                 uint32_t descriptorSetIndex,
                                                                 bool *newPoolAllocatedOut)
{
    vk::DynamicDescriptorPool &dynamicDescriptorPool = mDynamicDescriptorPools[descriptorSetIndex];

    uint32_t potentialNewCount = descriptorSetIndex + 1;
    if (potentialNewCount > mDescriptorSets.size())
    {
        mDescriptorSets.resize(potentialNewCount, VK_NULL_HANDLE);
    }

    const vk::DescriptorSetLayout &descriptorSetLayout =
        mDescriptorSetLayouts[descriptorSetIndex].get();
    ANGLE_TRY(dynamicDescriptorPool.allocateSetsAndGetInfo(
        contextVk, descriptorSetLayout.ptr(), 1, &mDescriptorPoolBindings[descriptorSetIndex],
        &mDescriptorSets[descriptorSetIndex], newPoolAllocatedOut));
    mEmptyDescriptorSets[descriptorSetIndex] = VK_NULL_HANDLE;

    return angle::Result::Continue;
}

void ProgramPipelineVk::updateDefaultUniformsDescriptorSet(ContextVk *contextVk)
{
    uint32_t shaderStageCount = static_cast<uint32_t>(getLinkedShaderStageCount());
    ASSERT(shaderStageCount <= kReservedDefaultUniformBindingCount);

    gl::ShaderVector<VkDescriptorBufferInfo> descriptorBufferInfo(shaderStageCount);
    gl::ShaderVector<VkWriteDescriptorSet> writeDescriptorInfo(shaderStageCount);

    uint32_t bindingIndex = 0;

    mDescriptorBuffersCache.clear();

    // Write default uniforms for each shader type.
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        ProgramVk *program = mPrograms[shaderType];
        if (!program)
        {
            continue;
        }

        VkDescriptorBufferInfo &bufferInfo = descriptorBufferInfo[bindingIndex];
        VkWriteDescriptorSet &writeInfo    = writeDescriptorInfo[bindingIndex];
        vk::BufferHelper *bufferHelper     = program->getUniformBlockCurrentBuffer(shaderType);

        if (bufferHelper)
        {
            bufferInfo.buffer = bufferHelper->getBuffer().getHandle();
            mDescriptorBuffersCache.emplace_back(bufferHelper);
        }
        else
        {
            mEmptyBuffer.onGraphAccess(contextVk->getCommandGraph());
            bufferInfo.buffer = mEmptyBuffer.getBuffer().getHandle();
            mDescriptorBuffersCache.emplace_back(&mEmptyBuffer);
        }

        bufferInfo.offset = 0;
        bufferInfo.range  = VK_WHOLE_SIZE;

        writeInfo.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeInfo.pNext            = nullptr;
        writeInfo.dstSet           = mDescriptorSets[kUniformsAndXfbDescriptorSetIndex];
        writeInfo.dstBinding       = bindingIndex;
        writeInfo.dstArrayElement  = 0;
        writeInfo.descriptorCount  = 1;
        writeInfo.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        writeInfo.pImageInfo       = nullptr;
        writeInfo.pBufferInfo      = &bufferInfo;
        writeInfo.pTexelBufferView = nullptr;

        ++bindingIndex;
    }

    VkDevice device = contextVk->getDevice();

    vkUpdateDescriptorSets(device, shaderStageCount, writeDescriptorInfo.data(), 0, nullptr);
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

angle::Result ProgramPipelineVk::updateDescriptorSets(ContextVk *contextVk,
                                                      vk::CommandBuffer *commandBuffer)
{
    // Can probably use better dirty bits here.

    if (mDescriptorSets.empty())
        return angle::Result::Continue;

    // Find the maximum non-null descriptor set.  This is used in conjunction with a driver
    // workaround to bind empty descriptor sets only for gaps in between 0 and max and avoid
    // binding unnecessary empty descriptor sets for the sets beyond max.
    size_t descriptorSetRange = 0;
    for (size_t descriptorSetIndex = 0; descriptorSetIndex < mDescriptorSets.size();
         ++descriptorSetIndex)
    {
        if (mDescriptorSets[descriptorSetIndex] != VK_NULL_HANDLE)
        {
            descriptorSetRange = descriptorSetIndex + 1;
        }
    }

    const VkPipelineBindPoint pipelineBindPoint =
        mState.isCompute() ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS;

    for (uint32_t descriptorSetIndex = 0; descriptorSetIndex < descriptorSetRange;
         ++descriptorSetIndex)
    {
        VkDescriptorSet descSet = mDescriptorSets[descriptorSetIndex];
        if (descSet == VK_NULL_HANDLE)
        {
            if (!contextVk->getRenderer()->getFeatures().bindEmptyForUnusedDescriptorSets.enabled)
            {
                continue;
            }

            // Workaround a driver bug where missing (though unused) descriptor sets indices cause
            // later sets to misbehave.
            if (mEmptyDescriptorSets[descriptorSetIndex] == VK_NULL_HANDLE)
            {
                const vk::DescriptorSetLayout &descriptorSetLayout =
                    mDescriptorSetLayouts[descriptorSetIndex].get();

                ANGLE_TRY(mDynamicDescriptorPools[descriptorSetIndex].allocateSets(
                    contextVk, descriptorSetLayout.ptr(), 1,
                    &mDescriptorPoolBindings[descriptorSetIndex],
                    &mEmptyDescriptorSets[descriptorSetIndex]));
            }
            descSet = mEmptyDescriptorSets[descriptorSetIndex];
        }

        // Default uniforms are encompassed in a block per shader stage, and they are assigned
        // through dynamic uniform buffers (requiring dynamic offsets).  No other descriptor
        // requires a dynamic offset.
        const uint32_t uniformBlockOffsetCount =
            descriptorSetIndex == kUniformsAndXfbDescriptorSetIndex
                ? static_cast<uint32_t>(mDynamicBufferOffsets.size())
                : 0;

        commandBuffer->bindDescriptorSets(mPipelineLayout.get(), pipelineBindPoint,
                                          descriptorSetIndex, 1, &descSet, uniformBlockOffsetCount,
                                          mDynamicBufferOffsets.data());
    }

    for (vk::BufferHelper *buffer : mDescriptorBuffersCache)
    {
        buffer->onGraphAccess(contextVk->getCommandGraph());
    }

    return angle::Result::Continue;
}

angle::Result ProgramPipelineVk::updateShaderResourcesDescriptorSet(
    ContextVk *contextVk,
    vk::CommandGraphResource *recorder)
{
#if 0  // TIMTIM
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        ProgramVk *program = mPrograms[shaderType];
        if (program)
        {
            ANGLE_TRY(program->updateShaderResourcesDescriptorSet(contextVk, recorder));
        }
    }
#endif

    return angle::Result::Continue;
}

angle::Result ProgramPipelineVk::updateTransformFeedbackDescriptorSet(
    ContextVk *contextVk,
    vk::FramebufferHelper *framebuffer)
{
#if 0  // TIMTIM
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        ProgramVk *program = mPrograms[shaderType];
        if (program)
        {
            ANGLE_TRY(program->updateTransformFeedbackDescriptorSet(contextVk, framebuffer));
        }
    }
#endif

    return angle::Result::Continue;
}

angle::Result ProgramPipelineVk::updateUniforms(ContextVk *contextVk)
{
    uint32_t offsetIndex       = 0;
    bool anyNewBufferAllocated = false;

    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        ProgramVk *program = mPrograms[shaderType];
        if (program && program->dirtyUniforms())
        {
            ANGLE_TRY(program->updateUniforms(contextVk, shaderType,
                                              &mDynamicBufferOffsets[offsetIndex],
                                              &anyNewBufferAllocated));
        }
        ++offsetIndex;
    }

    if (anyNewBufferAllocated)
    {
        // We need to reinitialize the descriptor sets if we newly allocated buffers since we can't
        // modify the descriptor sets once initialized.
        ANGLE_TRY(allocateDescriptorSet(contextVk, kUniformsAndXfbDescriptorSetIndex));
        updateDefaultUniformsDescriptorSet(contextVk);
#if 0  // TIMTIM
        updateTransformFeedbackDescriptorSetImpl(contextVk);
#endif
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
#if 0  // TIMTIM
    return mPrograms[gl::ShaderType::Fragment]->updateTexturesDescriptorSet(contextVk);
#else
    return angle::Result::Continue;
#endif
}

GLboolean ProgramPipelineVk::validate(const gl::Caps &caps, gl::InfoLog *infoLog)
{
    for (const gl::ShaderType shaderType : gl::kAllGraphicsShaderTypes)
    {
        ProgramVk *programVk = mPrograms[shaderType];
        if (programVk)
        {
            GLboolean result = programVk->validate(caps, infoLog);
            if (!result)
            {
                return GL_FALSE;
            }
        }
    }

    return GL_TRUE;
}

}  // namespace rx
