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

DispatchUtilsVk::DispatchUtilsVk()
{
}

DispatchUtilsVk::~DispatchUtilsVk()
{
}

static constexpr uint32_t kClearSetIndex      = 0;
static constexpr uint32_t kClearOutputBinding = 0;

static constexpr uint32_t kCopySetIndex           = 0;
static constexpr uint32_t kCopySourceBinding      = 0;
static constexpr uint32_t kCopyDestinationBinding = 1;

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
    vk::DescriptorSetLayoutDesc bufferClearSetDesc;
    vk::DescriptorSetLayoutDesc bufferCopySetDesc;
    vk::DescriptorSetLayoutDesc textureClearSetDesc;
    vk::DescriptorSetLayoutDesc textureCopySetDesc;

    bufferClearSetDesc.update(kClearOutputBinding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);

    bufferCopySetDesc.update(kCopySourceBinding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    bufferCopySetDesc.update(kCopyDestinationBinding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);

    textureClearSetDesc.update(kClearOutputBinding, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1);

    textureCopySetDesc.update(kCopySourceBinding, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1);
    textureCopySetDesc.update(kCopyDestinationBinding, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1);

    ANGLE_TRY(
        renderer->getDescriptorSetLayout(context, bufferClearSetDesc, &mBufferClearSetLayout));
    ANGLE_TRY(renderer->getDescriptorSetLayout(context, bufferCopySetDesc, &mBufferCopySetLayout));
    ANGLE_TRY(
        renderer->getDescriptorSetLayout(context, textureClearSetDesc, &mTextureClearSetLayout));
    ANGLE_TRY(
        renderer->getDescriptorSetLayout(context, textureCopySetDesc, &mTextureCopySetLayout));

    // Corresponding pipeline layouts:
    //
    //   - BufferClear:
    //     * Push constant range TODO: clear info (offset, size, clear value etc)
    //   - BufferCopy:
    //     * Push constant range TODO: copy info (src_offset, dest_offset, size etc)
    //   - TextureClear:
    //     * Push constant range TODO: clear info (offset, size, clear value etc)
    //   - TextureCopy:
    //     * Push constant range TODO: copy info (src_offset, dest_offset, size, src_lum-alpha hack,
    //     dest_lum-alpha hack etc)
    //
    vk::PipelineLayoutDesc bufferClearPipelineLayoutDesc;
    vk::PipelineLayoutDesc bufferCopyPipelineLayoutDesc;
    vk::PipelineLayoutDesc textureClearPipelineLayoutDesc;
    vk::PipelineLayoutDesc textureCopyPipelineLayoutDesc;

    bufferClearPipelineLayoutDesc.updateDescriptorSetLayout(kClearSetIndex, bufferClearSetDesc);
    bufferClearPipelineLayoutDesc.updatePushConstantRange(gl::ShaderType::Compute, 0, 16);

    bufferCopyPipelineLayoutDesc.updateDescriptorSetLayout(kCopySetIndex, bufferCopySetDesc);
    bufferCopyPipelineLayoutDesc.updatePushConstantRange(gl::ShaderType::Compute, 0, 32);

    textureClearPipelineLayoutDesc.updateDescriptorSetLayout(kClearSetIndex, textureClearSetDesc);
    textureClearPipelineLayoutDesc.updatePushConstantRange(gl::ShaderType::Compute, 0, 16);

    textureCopyPipelineLayoutDesc.updateDescriptorSetLayout(kCopySetIndex, textureCopySetDesc);
    textureCopyPipelineLayoutDesc.updatePushConstantRange(gl::ShaderType::Compute, 0, 32);

    ANGLE_TRY(renderer->getPipelineLayout(context, bufferClearPipelineLayoutDesc,
                                          &mBufferClearSetLayout, 1, &mBufferClearPipelineLayout));
    ANGLE_TRY(renderer->getPipelineLayout(context, bufferCopyPipelineLayoutDesc,
                                          &mBufferCopySetLayout, 1, &mBufferCopyPipelineLayout));
    ANGLE_TRY(renderer->getPipelineLayout(context, textureClearPipelineLayoutDesc,
                                          &mTextureClearSetLayout, 1,
                                          &mTextureClearPipelineLayout));
    ANGLE_TRY(renderer->getPipelineLayout(context, textureCopyPipelineLayoutDesc,
                                          &mTextureCopySetLayout, 1, &mTextureCopyPipelineLayout));

    return angle::Result::Continue();
}

angle::Result DispatchUtilsVk::bindPipeline(vk::Context *context,
                                            uint32_t function,
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
    const vk::BindingPointer<vk::PipelineLayout> &pipelineLayout =
        isClear ? isBuffer ? mBufferClearPipelineLayout : mTextureClearPipelineLayout
                : isBuffer ? mBufferCopyPipelineLayout : mTextureCopyPipelineLayout;

    vk::PipelineAndSerial *pipelineAndSerial;
    ANGLE_TRY(
        mPrograms[function].getComputePipeline(context, pipelineLayout.get(), &pipelineAndSerial));

    commandBuffer->bindPipeline(VK_PIPELINE_BIND_POINT_COMPUTE, pipelineAndSerial->get());
    pipelineAndSerial->updateSerial(renderer->getCurrentQueueSerial());

    return angle::Result::Continue();
}

angle::Result DispatchUtilsVk::ClearBuffer(const BufferVk &dest,
                                           const ClearParameters &params,
                                           vk::CommandBuffer *commandBuffer)
{
    UNIMPLEMENTED();
    return angle::Result::Stop();
}

angle::Result DispatchUtilsVk::CopyBuffer(const BufferVk &dest,
                                          const BufferVk &src,
                                          const CopyParameters &params,
                                          vk::CommandBuffer *commandBuffer)
{
    UNIMPLEMENTED();
    return angle::Result::Stop();
}

angle::Result DispatchUtilsVk::ClearTexture(const TextureVk &dest,
                                            const ClearParameters &params,
                                            vk::CommandBuffer *commandBuffer)
{
    UNIMPLEMENTED();
    return angle::Result::Stop();
}

angle::Result DispatchUtilsVk::CopyTexture(const TextureVk &dest,
                                           const TextureVk &src,
                                           const CopyParameters &params,
                                           vk::CommandBuffer *commandBuffer)
{
    UNIMPLEMENTED();
    return angle::Result::Stop();
}

}  // namespace rx
