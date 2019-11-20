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
#include "libANGLE/renderer/glslang_wrapper_utils.h"
#include "libANGLE/renderer/vulkan/GlslangWrapperVk.h"
#include "libANGLE/renderer/vulkan/SamplerVk.h"
#include "libANGLE/renderer/vulkan/TextureVk.h"

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

    mDefaultProgramInfo.release(contextVk);
    mLineRasterProgramInfo.release(contextVk);

    mEmptyBuffer.release(renderer);

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
}

uint32_t ProgramPipelineVk::getLinkedShaderStageCount(const gl::State &glState) const
{
    uint32_t linkedShaderStageCount = 0;

    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        if (getShaderProgram(glState, shaderType))
        {
            linkedShaderStageCount++;
        }
    }

    return linkedShaderStageCount;
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

uint32_t GetInterfaceBlockArraySize(const std::vector<gl::InterfaceBlock> &blocks,
                                    uint32_t bufferIndex)
{
    const gl::InterfaceBlock &block = blocks[bufferIndex];

    if (!block.isArray)
    {
        return 1;
    }

    ASSERT(block.arrayElement == 0);

    // Search consecutively until all array indices of this block are visited.
    uint32_t arraySize;
    for (arraySize = 1; bufferIndex + arraySize < blocks.size(); ++arraySize)
    {
        const gl::InterfaceBlock &nextBlock = blocks[bufferIndex + arraySize];

        if (nextBlock.arrayElement != arraySize)
        {
            break;
        }

        // It's unexpected for an array to start at a non-zero array size, so we can always rely on
        // the sequential `arrayElement`s to belong to the same block.
        ASSERT(nextBlock.name == block.name);
        ASSERT(nextBlock.isArray);
    }

    return arraySize;
}

uint32_t AddInterfaceBlockDescriptorSetDesc(const std::vector<gl::InterfaceBlock> &blocks,
                                            uint32_t bindingStart,
                                            const gl::ShaderType shaderType,
                                            VkDescriptorType descType,
                                            ShaderInterfaceVariableInfoMap *variableInfoMap,
                                            vk::DescriptorSetLayoutDesc *descOut)
{
    uint32_t bindingIndex = 0;
    for (uint32_t bufferIndex = 0; bufferIndex < blocks.size();)
    {
        const uint32_t arraySize        = GetInterfaceBlockArraySize(blocks, bufferIndex);
        gl::InterfaceBlock block        = blocks[bufferIndex];
        VkShaderStageFlags activeStages = gl_vk::GetShaderStageFlags(block.activeShaders());

        // We are only interested in buffers active for the specified shader type.
        activeStages &= gl_vk::kShaderStageMap[shaderType];

        if (activeStages)
        {
            descOut->update(bindingStart + bindingIndex, descType, arraySize, activeStages);
            // Record the descriptor set binding value for this interface block used by this shader
            ShaderInterfaceVariableInfo &info = (*variableInfoMap)[block.name];
            info.binding                      = bindingStart + bindingIndex;
        }

        bufferIndex += arraySize;
        ++bindingIndex;
    }

    return bindingIndex;
}

void AddAtomicCounterBufferDescriptorSetDesc(
    const std::vector<gl::AtomicCounterBuffer> &atomicCounterBuffers,
    uint32_t bindingStart,
    vk::DescriptorSetLayoutDesc *descOut)
{
    if (atomicCounterBuffers.empty())
    {
        return;
    }

    VkShaderStageFlags activeStages = 0;
    for (const gl::AtomicCounterBuffer &buffer : atomicCounterBuffers)
    {
        activeStages |= gl_vk::GetShaderStageFlags(buffer.activeShaders());
    }

    // A single storage buffer array is used for all stages for simplicity.
    descOut->update(bindingStart, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    gl::IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFERS, activeStages);
}

uint32_t AddImageDescriptorSetDesc(const gl::ProgramState &programState,
                                   uint32_t bindingStart,
                                   vk::DescriptorSetLayoutDesc *descOut)
{
    const std::vector<gl::ImageBinding> &imageBindings = programState.getImageBindings();
    const std::vector<gl::LinkedUniform> &uniforms     = programState.getUniforms();
    uint32_t bindingIndex                              = bindingStart;

    for (uint32_t imageIndex = 0; imageIndex < imageBindings.size(); ++imageIndex)
    {
        const gl::ImageBinding &imageBinding = imageBindings[imageIndex];

        uint32_t uniformIndex = programState.getUniformIndexFromImageIndex(imageIndex);
        const gl::LinkedUniform &imageUniform = uniforms[uniformIndex];

        // The front-end always binds array image units sequentially.
        uint32_t arraySize = static_cast<uint32_t>(imageBinding.boundImageUnits.size());
        VkShaderStageFlags activeStages = gl_vk::GetShaderStageFlags(imageUniform.activeShaders());

        bindingIndex += imageIndex;
        descOut->update(bindingIndex, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, arraySize, activeStages);
    }

    return bindingIndex;
}

uint32_t AddTextureDescriptorSetDesc(gl::Program *program,
                                     const gl::ShaderType shaderType,
                                     uint32_t bindingStart,
                                     bool useOldRewriteStructSamplers,
                                     VkShaderStageFlags activeStages,
                                     vk::DescriptorSetLayoutDesc *descOut)
{
    const gl::ProgramState &programState                   = program->getState();
    const std::vector<gl::SamplerBinding> &samplerBindings = programState.getSamplerBindings();
    const std::vector<gl::LinkedUniform> &uniforms         = programState.getUniforms();

    for (uint32_t textureIndex = 0; textureIndex < samplerBindings.size(); ++textureIndex)
    {
        const gl::SamplerBinding &samplerBinding = samplerBindings[textureIndex];

        uint32_t uniformIndex = programState.getUniformIndexFromSamplerIndex(textureIndex);
        const gl::LinkedUniform &samplerUniform = uniforms[uniformIndex];

        if (!samplerUniform.isActive(shaderType))
        {
            continue;
        }

        // The front-end always binds array sampler units sequentially.
        uint32_t arraySize = static_cast<uint32_t>(samplerBinding.boundTextureUnits.size());

        if (!useOldRewriteStructSamplers)
        {
            // 2D arrays are split into multiple 1D arrays when generating
            // LinkedUniforms. Since they are flattened into one array, ignore the
            // nonzero elements and expand the array to the total array size.
            if (gl::SamplerNameContainsNonZeroArrayElement(samplerUniform.name))
            {
                continue;
            }

            for (unsigned int outerArraySize : samplerUniform.outerArraySizes)
            {
                arraySize *= outerArraySize;
            }
        }

        // Record the descriptor set binding value for this texture used by this shader
        ProgramVk *programVk = vk::GetImpl(program);
        ShaderInterfaceVariableInfoMap &variableInfoMap =
            programVk->getShaderInterfaceVariableInfoMap()[shaderType];
        const std::string samplerName = useOldRewriteStructSamplers
                                            ? GetMappedSamplerNameOld(samplerUniform.name)
                                            : GlslangGetMappedSamplerName(samplerUniform.name);
        ShaderInterfaceVariableInfo &shaderInterfaceVariableInfo = variableInfoMap[samplerName];
        shaderInterfaceVariableInfo.binding                      = bindingStart;

        descOut->update(bindingStart++, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, arraySize,
                        activeStages);
    }

    return bindingStart;
}

#if 0  // TIMTIM
void WriteBufferDescriptorSetBinding(const gl::OffsetBindingPointer<gl::Buffer> &bufferBinding,
                                     VkDeviceSize maxSize,
                                     VkDescriptorSet descSet,
                                     VkDescriptorType descType,
                                     uint32_t bindingIndex,
                                     uint32_t arrayElement,
                                     VkDeviceSize requiredOffsetAlignment,
                                     VkDescriptorBufferInfo *bufferInfoOut,
                                     VkWriteDescriptorSet *writeInfoOut)
{
    gl::Buffer *buffer = bufferBinding.get();
    ASSERT(buffer != nullptr);

    // Make sure there's no possible under/overflow with binding size.
    static_assert(sizeof(VkDeviceSize) >= sizeof(bufferBinding.getSize()),
                  "VkDeviceSize too small");
    ASSERT(bufferBinding.getSize() >= 0);

    BufferVk *bufferVk             = vk::GetImpl(buffer);
    VkDeviceSize offset            = bufferBinding.getOffset();
    VkDeviceSize size              = bufferBinding.getSize();
    vk::BufferHelper &bufferHelper = bufferVk->getBuffer();

    // If size is 0, we can't always use VK_WHOLE_SIZE (or bufferHelper.getSize()), as the
    // backing buffer may be larger than max*BufferRange.  In that case, we use the minimum of
    // the backing buffer size (what's left after offset) and the buffer size as defined by the
    // shader.  That latter is only valid for UBOs, as SSBOs may have variable length arrays.
    size = size > 0 ? size : (bufferHelper.getSize() - offset);
    if (maxSize > 0)
    {
        size = std::min(size, maxSize);
    }

    // If requiredOffsetAlignment is 0, the buffer offset is guaranteed to have the necessary
    // alignment through other means (the backend specifying the alignment through a GLES limit that
    // the frontend then enforces).  If it's not 0, we need to bind the buffer at an offset that's
    // aligned.  The difference in offsets is communicated to the shader via driver uniforms.
    if (requiredOffsetAlignment)
    {
        VkDeviceSize alignedOffset = (offset / requiredOffsetAlignment) * requiredOffsetAlignment;
        VkDeviceSize offsetDiff    = offset - alignedOffset;

        offset = alignedOffset;
        size += offsetDiff;
    }

    bufferInfoOut->buffer = bufferHelper.getBuffer().getHandle();
    bufferInfoOut->offset = offset;
    bufferInfoOut->range  = size;

    writeInfoOut->sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfoOut->pNext            = nullptr;
    writeInfoOut->dstSet           = descSet;
    writeInfoOut->dstBinding       = bindingIndex;
    writeInfoOut->dstArrayElement  = arrayElement;
    writeInfoOut->descriptorCount  = 1;
    writeInfoOut->descriptorType   = descType;
    writeInfoOut->pImageInfo       = nullptr;
    writeInfoOut->pBufferInfo      = bufferInfoOut;
    writeInfoOut->pTexelBufferView = nullptr;
    ASSERT(writeInfoOut->pBufferInfo[0].buffer != VK_NULL_HANDLE);
}
#endif

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

    reset(contextVk);
    updateBindingOffsets(glContext);

    // Store a reference to the pipeline and descriptor set layouts. This will create them if they
    // don't already exist in the cache.

    // Default uniforms and transform feedback:
    vk::DescriptorSetLayoutDesc uniformsAndXfbSetDesc;
    uint32_t bindingIndex = 0;

    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const gl::Program *program = glPipeline->getShaderProgram(shaderType);
        if (program)
        {
            uniformBlockCount +=
                static_cast<uint32_t>(program->getState().getUniformBlocks().size());

            uniformsAndXfbSetDesc.update(bindingIndex, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1,
                                         gl_vk::kShaderStageMap[shaderType]);

            // Record the descriptor set binding value for this default uniform block used by this
            // shader
            ProgramVk *programVk = vk::GetImpl(program);
            ShaderInterfaceVariableInfoMap &variableInfoMap =
                programVk->getShaderInterfaceVariableInfoMap()[shaderType];
            const std::string uniformBlockName = kDefaultUniformNames[shaderType];
            ShaderInterfaceVariableInfo &shaderInterfaceVariableInfo =
                variableInfoMap[uniformBlockName];
            shaderInterfaceVariableInfo.binding = bindingIndex;

            bindingIndex++;
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

    // Uniform and storage buffers, atomic counter buffers and images:
    vk::DescriptorSetLayoutDesc resourcesSetDesc;

    bindingIndex = getUniformBlockBindingsOffset();
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const gl::Program *program = glPipeline->getShaderProgram(shaderType);
        if (program)
        {
            ProgramVk *programVk = vk::GetImpl(program);
            ShaderInterfaceVariableInfoMap &variableInfoMap =
                programVk->getShaderInterfaceVariableInfoMap()[shaderType];
            bindingIndex = AddInterfaceBlockDescriptorSetDesc(
                program->getState().getUniformBlocks(), bindingIndex, shaderType,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &variableInfoMap, &resourcesSetDesc);
        }
    }
    bindingIndex = getStorageBlockBindingsOffset();
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const gl::Program *program = glPipeline->getShaderProgram(shaderType);
        if (program)
        {
            ProgramVk *programVk = vk::GetImpl(program);
            ShaderInterfaceVariableInfoMap &variableInfoMap =
                programVk->getShaderInterfaceVariableInfoMap()[shaderType];
            bindingIndex = AddInterfaceBlockDescriptorSetDesc(
                program->getState().getShaderStorageBlocks(), bindingIndex, shaderType,
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &variableInfoMap, &resourcesSetDesc);
        }
    }

    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const gl::Program *program = glPipeline->getShaderProgram(shaderType);
        if (program)
        {
            // TIMTIM: need to get the binding used for the next shader stage?
            //            bindingIndex =
            AddAtomicCounterBufferDescriptorSetDesc(program->getState().getAtomicCounterBuffers(),
                                                    getAtomicCounterBufferBindingsOffset(),
                                                    &resourcesSetDesc);
        }
    }
    bindingIndex = getImageBindingsOffset();
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const gl::Program *program = glPipeline->getShaderProgram(shaderType);
        if (program)
        {
            bindingIndex =
                AddImageDescriptorSetDesc(program->getState(), bindingIndex, &resourcesSetDesc);
        }
    }

    ANGLE_TRY(renderer->getDescriptorSetLayout(
        contextVk, resourcesSetDesc, &mDescriptorSetLayouts[kShaderResourceDescriptorSetIndex]));

    // Textures:
    vk::DescriptorSetLayoutDesc texturesSetDesc;

    bindingIndex = 0;
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        gl::Program *program = glPipeline->getShaderProgram(shaderType);
        if (program)
        {
            bindingIndex = AddTextureDescriptorSetDesc(
                program, shaderType, bindingIndex, contextVk->useOldRewriteStructSamplers(),
                gl_vk::kShaderStageMap[shaderType], &texturesSetDesc);
        }
    }

    ANGLE_TRY(renderer->getDescriptorSetLayout(contextVk, texturesSetDesc,
                                               &mDescriptorSetLayouts[kTextureDescriptorSetIndex]));

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
    pipelineLayoutDesc.updateDescriptorSetLayout(kShaderResourceDescriptorSetIndex,
                                                 resourcesSetDesc);
    pipelineLayoutDesc.updateDescriptorSetLayout(kTextureDescriptorSetIndex, texturesSetDesc);
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
#endif

    uint32_t imageCount   = 0;
    uint32_t textureCount = 0;
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const gl::Program *program = glPipeline->getShaderProgram(shaderType);
        if (program)
        {
            imageCount += static_cast<uint32_t>(program->getState().getImageBindings().size());
            textureCount += static_cast<uint32_t>(program->getState().getSamplerBindings().size());
        }
    }

    if (renderer->getFeatures().bindEmptyForUnusedDescriptorSets.enabled)
    {
        // For this workaround, we have to create an empty descriptor set for each descriptor set
        // index, so make sure their pools are initialized.
        uniformBlockCount = std::max(uniformBlockCount, 1u);
        textureCount      = std::max(textureCount, 1u);
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
#endif
    if (imageCount > 0)
    {
        resourceSetSize.emplace_back(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, imageCount);
    }

    VkDescriptorPoolSize textureSetSize = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, textureCount};

    ANGLE_TRY(mDynamicDescriptorPools[kUniformsAndXfbDescriptorSetIndex].init(
        contextVk, uniformAndXfbSetSize.data(), uniformAndXfbSetSize.size()));
    if (resourceSetSize.size() > 0)
    {
        ANGLE_TRY(mDynamicDescriptorPools[kShaderResourceDescriptorSetIndex].init(
            contextVk, resourceSetSize.data(), static_cast<uint32_t>(resourceSetSize.size())));
    }
    if (textureCount > 0)
    {
        ANGLE_TRY(mDynamicDescriptorPools[kTextureDescriptorSetIndex].init(contextVk,
                                                                           &textureSetSize, 1));
    }

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

angle::Result ProgramPipelineVk::link(const gl::Context *glContext)
{
    ContextVk *contextVk                  = vk::GetImpl(glContext);
    const gl::State &glState              = glContext->getState();
    const gl::ProgramPipeline *glPipeline = glState.getProgramPipeline();
    std::set<gl::Program *> glPrograms;

    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        gl::Program *glProgram =
            const_cast<gl::Program *>(glPipeline->getShaderProgram(shaderType));
        if (glProgram && (glPrograms.find(glProgram) == glPrograms.end()))
        {
            ProgramVk *programVk = getShaderProgram(contextVk->getState(), shaderType);
            programVk->clearVariableInfoMap();

            gl::ShaderMap<ShaderInterfaceVariableInfoMap> &variableInfoMap =
                programVk->getShaderInterfaceVariableInfoMap();

            // Re-assign locations.
            AssignLocations(
                GlslangWrapperVk::CreateSourceOptions(contextVk->getRenderer()->getFeatures()),
                glProgram->getState(), glProgram->getResources(), &variableInfoMap);

            glPrograms.emplace(glProgram);
        }
    }
    return angle::Result::Continue;
}

angle::Result ProgramPipelineVk::transformShaderSpirV(const gl::Context *glContext)
{
    ContextVk *contextVk                  = vk::GetImpl(glContext);
    const gl::State &glState              = glContext->getState();
    const gl::ProgramPipeline *glPipeline = glState.getProgramPipeline();
    std::set<gl::Program *> glPrograms;

    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        gl::Program *glProgram =
            const_cast<gl::Program *>(glPipeline->getShaderProgram(shaderType));
        if (glProgram && (glPrograms.find(glProgram) == glPrograms.end()))
        {
            ProgramVk *programVk = getShaderProgram(contextVk->getState(), shaderType);
            ShaderInterfaceVariableInfoMap &variableInfoMap =
                programVk->getShaderInterfaceVariableInfoMap()[shaderType];
            std::vector<uint32_t> transformedSpirvBlob;

            ANGLE_TRY(GlslangWrapperVk::TransformSpirV(
                contextVk, shaderType, variableInfoMap,
                programVk->getShaderInfo().getSpirvBlobs()[shaderType], &transformedSpirvBlob));

            // Save the newly transformed SPIR-V
            programVk->getShaderInfo().getSpirvBlobs()[shaderType] = transformedSpirvBlob;
            glPrograms.emplace(glProgram);
        }
    }
    return angle::Result::Continue;
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
    const gl::State &glState              = glContext->getState();
    const gl::ProgramPipeline *glPipeline = glState.getProgramPipeline();
    bool enableLineRasterEmulation        = contextVk->useLineRaster(mode);
    ProgramInfo &programInfo =
        enableLineRasterEmulation ? mLineRasterProgramInfo : mDefaultProgramInfo;
    vk::ShaderProgramHelper *shaderProgram = programInfo.getShaderProgram();
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        gl::Program *glProgram =
            const_cast<gl::Program *>(glPipeline->getShaderProgram(shaderType));
        if (glProgram)
        {
            ProgramVk *programVk = getShaderProgram(glState, shaderType);
            ANGLE_TRY(programVk->initGraphicsProgram(contextVk, shaderType, mode,
                                                     enableLineRasterEmulation, programInfo,
                                                     &shaderProgram));
        }
    }
    ASSERT(shaderProgram->isGraphicsProgram());
    RendererVk *renderer             = contextVk->getRenderer();
    vk::PipelineCache *pipelineCache = nullptr;
    ANGLE_TRY(renderer->getPipelineCache(&pipelineCache));
    const ProgramVk *vertProgram = getShaderProgram(glState, gl::ShaderType::Vertex);
    return shaderProgram->getGraphicsPipeline(
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
    uint32_t shaderStageCount =
        static_cast<uint32_t>(getLinkedShaderStageCount(contextVk->getState()));
    ASSERT(shaderStageCount <= kReservedDefaultUniformBindingCount);

    gl::ShaderVector<VkDescriptorBufferInfo> descriptorBufferInfo(shaderStageCount);
    gl::ShaderVector<VkWriteDescriptorSet> writeDescriptorInfo(shaderStageCount);

    uint32_t bindingIndex = 0;

    mDescriptorBuffersCache.clear();

    // Write default uniforms for each shader type.
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        ProgramVk *program = getShaderProgram(contextVk->getState(), shaderType);
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
            mEmptyBuffer.onResourceAccess(&contextVk->getResourceUseList());
            bufferInfo.buffer = mEmptyBuffer.getBuffer().getHandle();
            mDescriptorBuffersCache.emplace_back(&mEmptyBuffer);
        }

        bufferInfo.offset = 0;
        bufferInfo.range  = VK_WHOLE_SIZE;

        ProgramVk *programVk = getShaderProgram(contextVk->getState(), shaderType);
        ShaderInterfaceVariableInfoMap &variableInfoMap =
            programVk->getShaderInterfaceVariableInfoMap()[shaderType];
        const std::string uniformBlockName = kDefaultUniformNames[shaderType];
        ShaderInterfaceVariableInfo &shaderInterfaceVariableInfo =
            variableInfoMap[uniformBlockName];

        writeInfo.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeInfo.pNext            = nullptr;
        writeInfo.dstSet           = mDescriptorSets[kUniformsAndXfbDescriptorSetIndex];
        writeInfo.dstBinding       = shaderInterfaceVariableInfo.binding;
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

bool ProgramPipelineVk::hasAtomicCounterBuffers(const gl::State &glState)
{
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const ProgramVk *program = getShaderProgram(glState, shaderType);
        if (program && program->hasAtomicCounterBuffers())
        {
            return true;
        }
    }

    return false;
}

bool ProgramPipelineVk::hasImages(const gl::State &glState)
{
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const ProgramVk *program = getShaderProgram(glState, shaderType);
        if (program && program->hasImages())
        {
            return true;
        }
    }

    return false;
}

bool ProgramPipelineVk::hasStorageBuffers(const gl::State &glState)
{
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const ProgramVk *program = getShaderProgram(glState, shaderType);
        if (program && program->hasStorageBuffers())
        {
            return true;
        }
    }

    return false;
}

bool ProgramPipelineVk::hasTransformFeedbackOutput(const gl::State &glState)
{
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const ProgramVk *program = getShaderProgram(glState, shaderType);
        if (program && program->hasTransformFeedbackOutput())
        {
            return true;
        }
    }

    return false;
}

bool ProgramPipelineVk::hasUniformBuffers(const gl::State &glState)
{
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const ProgramVk *program = getShaderProgram(glState, shaderType);
        if (program && program->hasUniformBuffers())
        {
            return true;
        }
    }

    return false;
}

bool ProgramPipelineVk::hasDefaultUniforms(const gl::State &glState)
{
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const ProgramVk *program = getShaderProgram(glState, shaderType);
        if (program && program->hasDefaultUniforms())
        {
            return true;
        }
    }

    return false;
}

// TODO(timvp): Issue 3570: Support separable programs
size_t ProgramPipelineVk::getTransformFeedbackBufferCount(const gl::State &glState)
{
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const ProgramVk *program = getShaderProgram(glState, shaderType);
        if (program)
        {
            // TODO(timvp): Issue 3570: Support separable programs
            // Assume the program pipeline only has a single monolithic program
            return program->getState().getTransformFeedbackBufferCount();
        }
    }

    return 0;
}

// TODO(timvp): Issue 3570: Support separable programs
const gl::ActiveTextureMask &ProgramPipelineVk::getActiveSamplersMask(
    const gl::State &glState) const
{
    const ProgramVk *program = getShaderProgram(glState, gl::ShaderType::Fragment);
    return program->getState().getActiveSamplersMask();
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
    const size_t descriptorSetStart = kUniformsAndXfbDescriptorSetIndex;
    size_t descriptorSetRange       = 0;
    for (size_t descriptorSetIndex = descriptorSetStart;
         descriptorSetIndex < mDescriptorSets.size(); ++descriptorSetIndex)
    {
        if (mDescriptorSets[descriptorSetIndex] != VK_NULL_HANDLE)
        {
            descriptorSetRange = descriptorSetIndex + 1;
        }
    }

    const VkPipelineBindPoint pipelineBindPoint =
        mState.isCompute() ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS;

    for (uint32_t descriptorSetIndex = descriptorSetStart; descriptorSetIndex < descriptorSetRange;
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
        buffer->onResourceAccess(&contextVk->getResourceUseList());
    }

    return angle::Result::Continue;
}

angle::Result ProgramPipelineVk::updateShaderResourcesDescriptorSet(
    ContextVk *contextVk,
    vk::ResourceUseList *resourceUseList,
    CommandBufferHelper *commandBufferHelper,
    vk::CommandGraphResource *recorder)
{
#if 0  // TIMTIM
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        ANGLE_TRY(allocateDescriptorSet(contextVk, kShaderResourceDescriptorSetIndex));

        updateBuffersDescriptorSet(contextVk, resourceUseList, renderPassCommands, recorder,
                                   mState.getUniformBlocks(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        updateBuffersDescriptorSet(contextVk, resourceUseList, renderPassCommands, recorder,
                                   mState.getShaderStorageBlocks(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        updateAtomicCounterBuffersDescriptorSet(contextVk, recorder);
        return updateImagesDescriptorSet(contextVk, recorder);
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
        ProgramVk *programVk = getShaderProgram(contextVk->getState(), shaderType);
        if (programVk && programVk->dirtyUniforms())
        {
            ANGLE_TRY(programVk->updateUniforms(contextVk, shaderType,
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
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        ProgramVk *programVk = getShaderProgram(contextVk->getState(), shaderType);
        if (programVk)
        {
            contextVk->invalidateCurrentShaderResources(programVk);
        }
    }
}

unsigned int ProgramPipelineVk::getMaxActiveAttribLocation(const gl::State &glState) const
{
    const ProgramVk *program = getShaderProgram(glState, gl::ShaderType::Vertex);
    return program->getState().getMaxActiveAttribLocation();
}

const gl::AttributesMask &ProgramPipelineVk::getActiveAttribLocationsMask(
    const gl::State &glState) const
{
    const ProgramVk *program = getShaderProgram(glState, gl::ShaderType::Vertex);
    return program->getState().getActiveAttribLocationsMask();
}

const gl::AttributesMask &ProgramPipelineVk::getNonBuiltinAttribLocationsMask(
    const gl::State &glState) const
{
    const ProgramVk *program = getShaderProgram(glState, gl::ShaderType::Vertex);
    return program->getState().getNonBuiltinAttribLocationsMask();
}

bool ProgramPipelineVk::hasTextures(const gl::State &glState)
{
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const ProgramVk *program = getShaderProgram(glState, shaderType);
        if (program && program->hasTextures())
        {
            return true;
        }
    }

    return false;
}

angle::Result ProgramPipelineVk::updateTexturesDescriptorSet(ContextVk *contextVk)
{
    if (!hasTextures(contextVk->getState()))
    {
        return angle::Result::Continue;
    }

    const vk::TextureDescriptorDesc &texturesDesc = contextVk->getActiveTexturesDesc();

    auto iter = mTextureDescriptorsCache.find(texturesDesc);
    if (iter != mTextureDescriptorsCache.end())
    {
        mDescriptorSets[kTextureDescriptorSetIndex] = iter->second;
        return angle::Result::Continue;
    }

    bool newPoolAllocated;
    ANGLE_TRY(
        allocateDescriptorSetAndGetInfo(contextVk, kTextureDescriptorSetIndex, &newPoolAllocated));

    // Clear descriptor set cache. It may no longer be valid.
    if (newPoolAllocated)
    {
        mTextureDescriptorsCache.clear();
    }

    VkDescriptorSet descriptorSet = mDescriptorSets[kTextureDescriptorSetIndex];

    gl::ActiveTextureArray<VkDescriptorImageInfo> descriptorImageInfo;
    gl::ActiveTextureArray<VkWriteDescriptorSet> writeDescriptorInfo;
    uint32_t writeCount = 0;

    const gl::ActiveTextureArray<vk::TextureUnit> &activeTextures = contextVk->getActiveTextures();

    bool emulateSeamfulCubeMapSampling = contextVk->emulateSeamfulCubeMapSampling();
    bool useOldRewriteStructSamplers   = contextVk->useOldRewriteStructSamplers();

    std::unordered_map<std::string, uint32_t> mappedSamplerNameToArrayOffset;

    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const ProgramVk *program = getShaderProgram(contextVk->getState(), shaderType);
        if (program)
        {
            for (uint32_t textureIndex = 0;
                 textureIndex < program->getState().getSamplerBindings().size(); ++textureIndex)
            {
                const gl::SamplerBinding &samplerBinding =
                    program->getState().getSamplerBindings()[textureIndex];

                ASSERT(!samplerBinding.unreferenced);

                uint32_t uniformIndex =
                    program->getState().getUniformIndexFromSamplerIndex(textureIndex);
                const gl::LinkedUniform &samplerUniform =
                    program->getState().getUniforms()[uniformIndex];
                std::string mappedSamplerName = GlslangGetMappedSamplerName(samplerUniform.name);

                if (!samplerUniform.isActive(shaderType))
                {
                    continue;
                }

                uint32_t arrayOffset = 0;
                uint32_t arraySize = static_cast<uint32_t>(samplerBinding.boundTextureUnits.size());

                if (!useOldRewriteStructSamplers)
                {
                    arrayOffset = mappedSamplerNameToArrayOffset[mappedSamplerName];
                    // Front-end generates array elements in order, so we can just increment
                    // the offset each time we process a nested array.
                    mappedSamplerNameToArrayOffset[mappedSamplerName] += arraySize;
                }

                for (uint32_t arrayElement = 0; arrayElement < arraySize; ++arrayElement)
                {
                    GLuint textureUnit   = samplerBinding.boundTextureUnits[arrayElement];
                    TextureVk *textureVk = activeTextures[textureUnit].texture;
                    SamplerVk *samplerVk = activeTextures[textureUnit].sampler;

                    vk::ImageHelper &image = textureVk->getImage();

                    VkDescriptorImageInfo &imageInfo = descriptorImageInfo[writeCount];

                    // Use bound sampler object if one present, otherwise use texture's sampler
                    const vk::Sampler &sampler =
                        (samplerVk != nullptr) ? samplerVk->getSampler() : textureVk->getSampler();

                    imageInfo.sampler     = sampler.getHandle();
                    imageInfo.imageLayout = image.getCurrentLayout();

                    if (emulateSeamfulCubeMapSampling)
                    {
                        // If emulating seamful cubemapping, use the fetch image view.  This is
                        // basically the same image view as read, except it's a 2DArray view for
                        // cube maps.
                        imageInfo.imageView =
                            textureVk->getFetchImageViewAndRecordUse(contextVk).getHandle();
                    }
                    else
                    {
                        imageInfo.imageView =
                            textureVk->getReadImageViewAndRecordUse(contextVk).getHandle();
                    }

                    ProgramVk *programVk = getShaderProgram(contextVk->getState(), shaderType);
                    ShaderInterfaceVariableInfoMap &variableInfoMap =
                        programVk->getShaderInterfaceVariableInfoMap()[shaderType];
                    const std::string samplerName =
                        contextVk->getRenderer()
                                ->getFeatures()
                                .forceOldRewriteStructSamplers.enabled
                            ? GetMappedSamplerNameOld(samplerUniform.name)
                            : GlslangGetMappedSamplerName(samplerUniform.name);
                    ShaderInterfaceVariableInfo &shaderInterfaceVariableInfo =
                        variableInfoMap[samplerName];

                    VkWriteDescriptorSet &writeInfo = writeDescriptorInfo[writeCount];

                    writeInfo.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    writeInfo.pNext            = nullptr;
                    writeInfo.dstSet           = descriptorSet;
                    writeInfo.dstBinding       = shaderInterfaceVariableInfo.binding;
                    writeInfo.dstArrayElement  = arrayOffset + arrayElement;
                    writeInfo.descriptorCount  = 1;
                    writeInfo.descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    writeInfo.pImageInfo       = &imageInfo;
                    writeInfo.pBufferInfo      = nullptr;
                    writeInfo.pTexelBufferView = nullptr;

                    ++writeCount;
                }
            }
        }
    }

    VkDevice device = contextVk->getDevice();

    ASSERT(writeCount > 0);

    vkUpdateDescriptorSets(device, writeCount, writeDescriptorInfo.data(), 0, nullptr);

    mTextureDescriptorsCache.emplace(texturesDesc, descriptorSet);

    return angle::Result::Continue;
}

GLboolean ProgramPipelineVk::validate(const gl::Caps &caps, gl::InfoLog *infoLog)
{
#if 0  // TIMTIM
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const ProgramVk *programVk = getShaderProgram(glState, shaderType);
        if (programVk)
        {
            GLboolean result = programVk->validate(caps, infoLog);
            if (!result)
            {
                return GL_FALSE;
            }
        }
    }
#endif

    return GL_TRUE;
}

}  // namespace rx
