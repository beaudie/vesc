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

namespace BufferUtils_comp = vk::InternalShader::BufferUtils_comp;

DispatchUtilsVk::DispatchUtilsVk() {}

DispatchUtilsVk::~DispatchUtilsVk() {}

void DispatchUtilsVk::destroy(VkDevice device)
{
    for (uint32_t i = 0; i < kFunctionCount; ++i)
    {
        for (auto &descriptorSetLayout : mDescriptorSetLayouts[i])
        {
            descriptorSetLayout.reset();
        }
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

    std::array<vk::DescriptorSetLayoutDesc, kFunctionCount> descriptorSetDesc;

    descriptorSetDesc[kBufferClear].update(kClearOutputBinding,
                                           VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1);

    descriptorSetDesc[kBufferCopy].update(kCopyDestinationBinding,
                                          VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1);
    descriptorSetDesc[kBufferCopy].update(kCopySourceBinding,
                                          VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1);

    for (uint32_t i = 0; i < kFunctionCount; ++i)
    {
        ANGLE_TRY(renderer->getDescriptorSetLayout(context, descriptorSetDesc[i],
                                                   &mDescriptorSetLayouts[i][kClearCopySetIndex]));
    }

    // Corresponding pipeline layouts:
    for (uint32_t i = 0; i < kFunctionCount; ++i)
    {
        vk::PipelineLayoutDesc pipelineLayoutDesc;

        pipelineLayoutDesc.updateDescriptorSetLayout(kClearCopySetIndex, descriptorSetDesc[i]);
        pipelineLayoutDesc.updatePushConstantRange(gl::ShaderType::Compute, 0,
                                                   sizeof(ShaderParams));

        ANGLE_TRY(renderer->getPipelineLayout(context, pipelineLayoutDesc, mDescriptorSetLayouts[i],
                                              &mPipelineLayouts[i]));
    }

    VkDescriptorPoolSize bufferSetSizes[2] = {
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1},
    };
    ANGLE_TRY(mDescriptorPools[kBufferClear].init(context, bufferSetSizes, 1));
    ANGLE_TRY(mDescriptorPools[kBufferCopy].init(context, bufferSetSizes, 2));

    return angle::Result::Continue();
}

angle::Result DispatchUtilsVk::setupProgram(vk::Context *context,
                                            uint32_t function,
                                            const VkDescriptorSet &descriptorSet,
                                            const ShaderParams &params,
                                            vk::CommandBuffer *commandBuffer)
{
    RendererVk *renderer             = context->getRenderer();
    vk::ShaderLibrary *shaderLibrary = renderer->getShaderLibrary();

    vk::RefCounted<vk::ShaderAndSerial> *shader = nullptr;
    ANGLE_TRY(shaderLibrary->getBufferUtils_comp(context, function, &shader));

    mPrograms[function].setShader(gl::ShaderType::Compute, shader);

    bool isClear = (function & BufferUtils_comp::kFunctionMask) == BufferUtils_comp::kIsClear;
    uint32_t pipelineLayoutIndex = isClear ? kBufferClear : kBufferCopy;
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

angle::Result DispatchUtilsVk::clearBuffer(vk::Context *context,
                                           vk::BufferHelper *dest,
                                           const ClearParameters &params)
{
    vk::CommandBuffer *commandBuffer;
    ANGLE_TRY(dest->recordCommands(context, &commandBuffer));

    // Tell dest it's being written to.
    dest->onWrite(VK_ACCESS_SHADER_WRITE_BIT);

    const vk::Format &destFormat = dest->getViewFormat();

    uint32_t function = BufferUtils_comp::kIsClear;

    function |= params.size % 64 == 0 ? BufferUtils_comp::kIsAligned : 0;

    function |=
        !destFormat.vkFormatIsInt
            ? BufferUtils_comp::kIsFloat
            : destFormat.vkFormatIsUnsigned ? BufferUtils_comp::kIsUint : BufferUtils_comp::kIsInt;

    ShaderParams shaderParams = {};
    shaderParams.destOffset   = params.offset;
    shaderParams.size         = params.size;
    shaderParams.clearValue   = params.clearValue;

    VkDescriptorSet descriptorSet;
    vk::SharedDescriptorPoolBinding descriptorPoolBinding;
    ANGLE_TRY(mDescriptorPools[kBufferCopy].allocateSets(
        context, mDescriptorSetLayouts[kBufferCopy][kClearCopySetIndex].get().ptr(), 1,
        &descriptorPoolBinding, &descriptorSet));

    VkWriteDescriptorSet writeInfo = {};

    writeInfo.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.dstSet           = descriptorSet;
    writeInfo.dstBinding       = kCopyDestinationBinding;
    writeInfo.descriptorCount  = 1;
    writeInfo.descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    writeInfo.pTexelBufferView = dest->getBufferView().ptr();

    vkUpdateDescriptorSets(context->getDevice(), 1, &writeInfo, 0, nullptr);

    ANGLE_TRY(setupProgram(context, function, descriptorSet, shaderParams, commandBuffer));

    commandBuffer->dispatch((params.size + 63) / 64, 1, 1);

    descriptorPoolBinding.reset();

    return angle::Result::Continue();
}

angle::Result DispatchUtilsVk::copyBuffer(vk::Context *context,
                                          vk::BufferHelper *dest,
                                          vk::BufferHelper *src,
                                          const CopyParameters &params)
{
    vk::CommandBuffer *commandBuffer;
    ANGLE_TRY(dest->recordCommands(context, &commandBuffer));

    // Tell src we are going to read from it.
    src->onRead(dest, VK_ACCESS_SHADER_READ_BIT);
    // Tell dest it's being written to.
    dest->onWrite(VK_ACCESS_SHADER_WRITE_BIT);

    const vk::Format &destFormat = dest->getViewFormat();
    const vk::Format &srcFormat  = src->getViewFormat();

    ASSERT(destFormat.vkFormatIsInt == srcFormat.vkFormatIsInt);
    ASSERT(destFormat.vkFormatIsUnsigned == srcFormat.vkFormatIsUnsigned);

    uint32_t function = BufferUtils_comp::kIsCopy;

    function |= params.size % 64 == 0 ? BufferUtils_comp::kIsAligned : 0;

    function |=
        !destFormat.vkFormatIsInt
            ? BufferUtils_comp::kIsFloat
            : destFormat.vkFormatIsUnsigned ? BufferUtils_comp::kIsUint : BufferUtils_comp::kIsInt;

    ShaderParams shaderParams = {};
    shaderParams.destOffset   = params.destOffset;
    shaderParams.size         = params.size;
    shaderParams.srcOffset    = params.srcOffset;

    VkDescriptorSet descriptorSet;
    vk::SharedDescriptorPoolBinding descriptorPoolBinding;
    ANGLE_TRY(mDescriptorPools[kBufferCopy].allocateSets(
        context, mDescriptorSetLayouts[kBufferCopy][kClearCopySetIndex].get().ptr(), 1,
        &descriptorPoolBinding, &descriptorSet));

    VkWriteDescriptorSet writeInfo[2] = {};

    writeInfo[0].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo[0].dstSet           = descriptorSet;
    writeInfo[0].dstBinding       = kCopyDestinationBinding;
    writeInfo[0].descriptorCount  = 1;
    writeInfo[0].descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    writeInfo[0].pTexelBufferView = dest->getBufferView().ptr();

    writeInfo[1].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo[1].dstSet           = descriptorSet;
    writeInfo[1].dstBinding       = kCopySourceBinding;
    writeInfo[1].descriptorCount  = 1;
    writeInfo[1].descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    writeInfo[1].pTexelBufferView = src->getBufferView().ptr();

    vkUpdateDescriptorSets(context->getDevice(), 2, writeInfo, 0, nullptr);

    ANGLE_TRY(setupProgram(context, function, descriptorSet, shaderParams, commandBuffer));

    commandBuffer->dispatch((params.size + 63) / 64, 1, 1);

    descriptorPoolBinding.reset();

    return angle::Result::Continue();
}

}  // namespace rx
