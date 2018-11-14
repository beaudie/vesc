//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DispatchUtilsVk.cpp:
//    Implements the DispatchUtilsVk class.
//

#include "libANGLE/renderer/vulkan/DispatchUtilsVk.h"

#include "libANGLE/renderer/vulkan/BufferVk.h"
#include "libANGLE/renderer/vulkan/FramebufferVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/renderer/vulkan/TextureVk.h"

namespace rx
{

namespace DispatchUtils_comp = vk::InternalShader::DispatchUtils_comp;

DispatchUtilsVk::DispatchUtilsVk() {}

DispatchUtilsVk::~DispatchUtilsVk() {}

void DispatchUtilsVk::destroy(VkDevice device)
{
    for (uint32_t i = 0; i < kFunctionCount; ++i)
    {
        mDescriptorSetLayouts[i].reset();
        mPipelineLayouts[i].reset();
        mDescriptorPools[i].destroy(device);
    }

    for (vk::ShaderProgramHelper &program : mPrograms)
    {
        program.destroy(device);
    }
}

static constexpr uint32_t kClearCopySetIndex = 0;

static constexpr uint32_t kClearOutputBinding     = 0;
static constexpr uint32_t kCopyDestinationBinding = 0;
static constexpr uint32_t kCopySourceBinding      = 1;

angle::Result DispatchUtilsVk::initialize(vk::Context *context)
{
    RendererVk *renderer = context->getRenderer();

    // Descriptor set layouts:
    //
    //   - BufferClear:
    //     * set 0, binding 0: output buffer
    //   - BufferCopy:
    //     * set 0, binding 0: destination buffer
    //     * set 0, binding 1: source buffer
    //   - TextureClear:
    //     * set 0, binding 0: output texture
    //   - TextureCopy:
    //     * set 0, binding 0: destination texture
    //     * set 0, binding 1: source texture
    //
    std::array<vk::DescriptorSetLayoutDesc, kFunctionCount> descriptorSetDesc;

    descriptorSetDesc[kBufferClear].update(kClearOutputBinding,
                                           VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1);

    descriptorSetDesc[kBufferCopy].update(kCopyDestinationBinding,
                                          VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1);
    descriptorSetDesc[kBufferCopy].update(kCopySourceBinding,
                                          VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1);

    descriptorSetDesc[kTextureClear].update(kClearOutputBinding, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                            1);

    descriptorSetDesc[kTextureCopy].update(kCopyDestinationBinding,
                                           VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1);
    descriptorSetDesc[kTextureCopy].update(kCopySourceBinding, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1);

    for (uint32_t i = 0; i < kFunctionCount; ++i)
    {
        ANGLE_TRY(renderer->getDescriptorSetLayout(context, descriptorSetDesc[i],
                                                   &mDescriptorSetLayouts[i]));
    }

    // Corresponding pipeline layouts:
    for (uint32_t i = 0; i < kFunctionCount; ++i)
    {
        vk::PipelineLayoutDesc pipelineLayoutDesc;

        pipelineLayoutDesc.updateDescriptorSetLayout(kClearCopySetIndex, descriptorSetDesc[i]);
        pipelineLayoutDesc.updatePushConstantRange(gl::ShaderType::Compute, 0, 128);

        ANGLE_TRY(renderer->getPipelineLayout(context, pipelineLayoutDesc,
                                              &mDescriptorSetLayouts[i], 1, &mPipelineLayouts[i]));
    }

    VkDescriptorPoolSize bufferSetSizes[2] = {
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1},
    };
    VkDescriptorPoolSize textureSetSizes[2] = {
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1},
    };
    ANGLE_TRY(mDescriptorPools[kBufferClear].init(context, bufferSetSizes, 1));
    ANGLE_TRY(mDescriptorPools[kBufferCopy].init(context, bufferSetSizes, 2));
    ANGLE_TRY(mDescriptorPools[kTextureClear].init(context, textureSetSizes, 1));
    ANGLE_TRY(mDescriptorPools[kTextureCopy].init(context, textureSetSizes, 2));

    return angle::Result::Continue();
}

angle::Result DispatchUtilsVk::setupProgram(vk::Context *context,
                                            uint32_t function,
                                            const VkDescriptorSet &descriptorSet,
                                            const Params &params,
                                            vk::CommandBuffer *commandBuffer)
{
    RendererVk *renderer             = context->getRenderer();
    vk::ShaderLibrary *shaderLibrary = renderer->getShaderLibrary();

    vk::RefCounted<vk::ShaderAndSerial> *shader = nullptr;
    ANGLE_TRY(shaderLibrary->getDispatchUtils_comp(context, function, &shader));

    mPrograms[function].setShader(gl::ShaderType::Compute, shader);

    bool isClear = (function & DispatchUtils_comp::kFunctionMask) == DispatchUtils_comp::kIsClear;
    bool isBuffer =
        (function & DispatchUtils_comp::kResourceTypeMask) == DispatchUtils_comp::kIsBuffer;
    uint32_t pipelineLayoutIndex =
        isClear ? isBuffer ? kBufferClear : kTextureClear : isBuffer ? kBufferCopy : kTextureCopy;
    const vk::BindingPointer<vk::PipelineLayout> &pipelineLayout =
        mPipelineLayouts[pipelineLayoutIndex];

    vk::PipelineAndSerial *pipelineAndSerial;
    ANGLE_TRY(
        mPrograms[function].getComputePipeline(context, pipelineLayout.get(), &pipelineAndSerial));

    commandBuffer->bindPipeline(VK_PIPELINE_BIND_POINT_COMPUTE, pipelineAndSerial->get());
    pipelineAndSerial->updateSerial(renderer->getCurrentQueueSerial());

    commandBuffer->bindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout.get(), 0, 1,
                                      &descriptorSet, 0, nullptr);
    commandBuffer->pushConstants(pipelineLayout.get(), VK_SHADER_STAGE_COMPUTE_BIT, 0,
                                 sizeof(params), &params);

    return angle::Result::Continue();
}

bool canClearTexture(const TextureVk &dest)
{
    UNIMPLEMENTED();
    return false;
}

bool canCopyTexture(const TextureVk &dest, const TextureVk &src)
{
    UNIMPLEMENTED();
    return false;
}

angle::Result DispatchUtilsVk::clearBuffer(vk::Context *context,
                                           BufferVk &dest,
                                           const ClearParameters &params)
{
    ANGLE_TRY(context->getRenderer()->finish(context));

    /* TODO(syoussefi): TEST CODE */
    uint32_t function =
        DispatchUtils_comp::kIsBuffer | DispatchUtils_comp::kIsInt | DispatchUtils_comp::kIsClear;
    Params cparams = {
        {0, 0, 0},
        0,
        {100, 0, 0},
        0,
        {0, 0, 0},
        0,
        {0xFFFFFFFF, 2, 3, 4},
        {5.0f, 6.0f, 7.0f, 8.0f},
        {9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20},
    };
    vk::CommandBuffer *commandBuffer;
    ANGLE_TRY(dest.getBuffer().recordCommands(context, &commandBuffer));

    VkDescriptorSet descriptorSet;
    vk::SharedDescriptorPoolBinding descriptorPoolBinding;
    ANGLE_TRY(mDescriptorPools[kBufferClear].allocateSets(
        context, mDescriptorSetLayouts[kBufferClear].get().ptr(), 1, &descriptorPoolBinding,
        &descriptorSet));

    // Create the test buffer:
    uint32_t bufferSize           = 115;
    VkBufferCreateInfo createInfo = {};
    createInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size               = bufferSize * sizeof(float);
    createInfo.usage              = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

    vk::Buffer buffer;
    ANGLE_VK_TRY(context, buffer.init(context->getDevice(), createInfo));

    vk::DeviceMemory bufferMemory;
    VkMemoryPropertyFlags actualMemoryPropertyFlags = 0;
    ANGLE_TRY(vk::AllocateBufferMemory(
        context, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &actualMemoryPropertyFlags, &buffer, &bufferMemory));

    vk::BufferView bufferView;
    VkBufferViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType                  = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    viewCreateInfo.buffer                 = buffer.getHandle();
    viewCreateInfo.format                 = VK_FORMAT_R8_UINT;
    viewCreateInfo.offset                 = 0;
    viewCreateInfo.range                  = createInfo.size;
    ANGLE_VK_TRY(context, bufferView.init(context->getDevice(), viewCreateInfo));

    VkWriteDescriptorSet writeInfo = {};
    writeInfo.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.dstSet               = descriptorSet;
    writeInfo.dstBinding           = kClearOutputBinding;
    writeInfo.descriptorCount      = 1;
    writeInfo.descriptorType       = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    writeInfo.pTexelBufferView     = bufferView.ptr();

    vkUpdateDescriptorSets(context->getDevice(), 1, &writeInfo, 0, nullptr);

    ANGLE_TRY(setupProgram(context, function, descriptorSet, cparams, commandBuffer));

    commandBuffer->dispatch((bufferSize + 63) / 64, 1, 1);

    VkMemoryBarrier memBarrier = {};
    memBarrier.sType           = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memBarrier.srcAccessMask   = VK_ACCESS_MEMORY_WRITE_BIT;
    memBarrier.dstAccessMask   = VK_ACCESS_HOST_READ_BIT;
    commandBuffer->pipelineBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_HOST_BIT,
                                   0, 1, &memBarrier, 0, nullptr, 0, nullptr);

    ANGLE_TRY(context->getRenderer()->finish(context));

    float *memory;
    ANGLE_VK_TRY(context, bufferMemory.map(context->getDevice(), 0, createInfo.size, 0,
                                           (uint8_t **)&memory));

    fprintf(stderr, "Read back:\n");
    for (size_t i = 0; i < bufferSize; ++i)
    {
        fprintf(stderr, "%f(%u) ", memory[i], ((uint32_t *)memory)[i]);
        if (i % 16 == 15)
            fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");

    bufferMemory.unmap(context->getDevice());

    bufferView.destroy(context->getDevice());
    buffer.destroy(context->getDevice());
    bufferMemory.destroy(context->getDevice());
    descriptorPoolBinding.reset();

    UNIMPLEMENTED();
    return angle::Result::Stop();
}

angle::Result DispatchUtilsVk::copyBuffer(vk::Context *context,
                                          const BufferVk &dest,
                                          const BufferVk &src,
                                          const CopyParameters &params,
                                          vk::CommandBuffer *commandBuffer)
{
    UNIMPLEMENTED();
    return angle::Result::Stop();
}

angle::Result DispatchUtilsVk::clearTexture(vk::Context *context,
                                            const TextureVk &dest,
                                            const ClearParameters &params,
                                            vk::CommandBuffer *commandBuffer)
{
    UNIMPLEMENTED();
    return angle::Result::Stop();
}

angle::Result DispatchUtilsVk::copyTexture(vk::Context *context,
                                           const TextureVk &dest,
                                           const TextureVk &src,
                                           const CopyParameters &params,
                                           vk::CommandBuffer *commandBuffer)
{
    UNIMPLEMENTED();
    return angle::Result::Stop();
}

}  // namespace rx
