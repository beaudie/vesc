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
                                            const ShaderParams &params,
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
                                           vk::BufferHelper *dest,
                                           const ClearParameters &params)
{
    UNIMPLEMENTED();
    return angle::Result::Stop();
}

angle::Result DispatchUtilsVk::copyBuffer(vk::Context *context,
                                          vk::BufferHelper *dest,
                                          vk::BufferHelper *src,
                                          const CopyParameters &params)
{
    // TODO(syoussefi): test to see which test actually gets here
    ASSERT(false);
    // Tell src we are going to read from it.
    src->onRead(dest, VK_ACCESS_SHADER_READ_BIT);
    // Tell dest it's being written to.
    dest->onWrite(VK_ACCESS_SHADER_WRITE_BIT);

    const vk::Format &destFormat = dest->getViewFormat();
    const vk::Format &srcFormat  = src->getViewFormat();

    ASSERT(destFormat.vkFormatIsInt == srcFormat.vkFormatIsInt);
    ASSERT(destFormat.vkFormatIsUnsigned == srcFormat.vkFormatIsUnsigned);

    uint32_t function = DispatchUtils_comp::kIsBuffer | DispatchUtils_comp::kIsCopy;

    function |= params.extents[0] % 64 == 0 ? DispatchUtils_comp::kIsAligned : 0;

    function |= !destFormat.vkFormatIsInt
                    ? DispatchUtils_comp::kIsFloat
                    : destFormat.vkFormatIsUnsigned ? DispatchUtils_comp::kIsUint
                                                    : DispatchUtils_comp::kIsInt;

    ShaderParams shaderParams  = {};
    shaderParams.destOffset[0] = params.destOffset[0];
    shaderParams.extents[0]    = params.extents[0];
    shaderParams.srcOffset[0]  = params.srcOffset[0];

    vk::CommandBuffer *commandBuffer;
    ANGLE_TRY(dest->recordCommands(context, &commandBuffer));

    VkDescriptorSet descriptorSet;
    vk::SharedDescriptorPoolBinding descriptorPoolBinding;
    ANGLE_TRY(mDescriptorPools[kBufferCopy].allocateSets(
        context, mDescriptorSetLayouts[kBufferCopy].get().ptr(), 1, &descriptorPoolBinding,
        &descriptorSet));

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

    commandBuffer->dispatch((params.extents[0] + 63) / 64, 1, 1);

    descriptorPoolBinding.reset();

    return angle::Result::Continue();
}

angle::Result DispatchUtilsVk::clearTexture(vk::Context *context,
                                            vk::ImageHelper *dest,
                                            const ClearParameters &params)
{
    UNIMPLEMENTED();
    return angle::Result::Stop();
}

angle::Result DispatchUtilsVk::copyTexture(vk::Context *context,
                                           vk::ImageHelper *dest,
                                           vk::ImageHelper *src,
                                           const CopyParameters &params)
{
    UNIMPLEMENTED();
    return angle::Result::Stop();
}

}  // namespace rx
