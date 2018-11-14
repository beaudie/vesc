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
#include "libANGLE/renderer/vulkan/vk_internal_shaders_autogen.h"

namespace rx
{

namespace priv
{
// There are a variety of shaders specialized for each operation, and they are queried by providing
// a bit-wise or of the following values.  Only one bit value from any enum is acceptable, except
// for FunctionFlags, where any bit can be set.
enum FunctionFlags
{
    kAligned           = 0x0001,
    kInt               = 0x0002,
    kFunctionFlagsMask = 0x0003,
};
enum Function
{
    kClear        = 0x0000,
    kCopy         = 0x0004,
    kFunctionMask = 0x0004,
};
enum ResourceType
{
    kBuffer           = 0x0000,
    kTexture1D        = 0x0008,
    kTexture2D        = 0x0010,
    kTexture3D        = 0x0018,
    kTexture2DArray   = 0x0020,
    kResourceTypeMask = 0x0038,
};

using SID                                                                 = vk::InternalShaderID;
static constexpr vk::InternalShaderID kShaderIDs[priv::kFunctionFlagsMask | priv::kFunctionMask |
                                                 priv::kResourceTypeMask] = {
    // Note(syoussefi): uncomment after C++20 which finally adds designated initializers.
    /* [               |            | priv::kClear | priv::kBuffer        ] = */ SID::ClearBuffer,
    /* [priv::kAligned |            | priv::kClear | priv::kBuffer        ] = */
    SID::AlignedClearBuffer,
    /* [               | priv::kInt | priv::kClear | priv::kBuffer        ] = */
    SID::IntClearBuffer,
    /* [priv::kAligned | priv::kInt | priv::kClear | priv::kBuffer        ] = */
    SID::AlignedIntClearBuffer,
    /* [               |            | priv::kCopy  | priv::kBuffer        ] = */ SID::CopyBuffer,
    /* [priv::kAligned |            | priv::kCopy  | priv::kBuffer        ] = */
    SID::AlignedCopyBuffer,
    /* [               | priv::kInt | priv::kCopy  | priv::kBuffer        ] = */ SID::IntCopyBuffer,
    /* [priv::kAligned | priv::kInt | priv::kCopy  | priv::kBuffer        ] = */
    SID::AlignedIntCopyBuffer,

    /* [               |            | priv::kClear | priv::kTexture1D     ] = */
    SID::ClearTexture1D,
    /* [priv::kAligned |            | priv::kClear | priv::kTexture1D     ] = */
    SID::AlignedClearTexture1D,
    /* [               | priv::kInt | priv::kClear | priv::kTexture1D     ] = */
    SID::IntClearTexture1D,
    /* [priv::kAligned | priv::kInt | priv::kClear | priv::kTexture1D     ] = */
    SID::AlignedIntClearTexture1D,
    /* [               |            | priv::kCopy  | priv::kTexture1D     ] = */ SID::CopyTexture1D,
    /* [priv::kAligned |            | priv::kCopy  | priv::kTexture1D     ] = */
    SID::AlignedCopyTexture1D,
    /* [               | priv::kInt | priv::kCopy  | priv::kTexture1D     ] = */
    SID::IntCopyTexture1D,
    /* [priv::kAligned | priv::kInt | priv::kCopy  | priv::kTexture1D     ] = */
    SID::AlignedIntCopyTexture1D,

    /* [               |            | priv::kClear | priv::kTexture2D     ] = */
    SID::ClearTexture2D,
    /* [priv::kAligned |            | priv::kClear | priv::kTexture2D     ] = */
    SID::AlignedClearTexture2D,
    /* [               | priv::kInt | priv::kClear | priv::kTexture2D     ] = */
    SID::IntClearTexture2D,
    /* [priv::kAligned | priv::kInt | priv::kClear | priv::kTexture2D     ] = */
    SID::AlignedIntClearTexture2D,
    /* [               |            | priv::kCopy  | priv::kTexture2D     ] = */ SID::CopyTexture2D,
    /* [priv::kAligned |            | priv::kCopy  | priv::kTexture2D     ] = */
    SID::AlignedCopyTexture2D,
    /* [               | priv::kInt | priv::kCopy  | priv::kTexture2D     ] = */
    SID::IntCopyTexture2D,
    /* [priv::kAligned | priv::kInt | priv::kCopy  | priv::kTexture2D     ] = */
    SID::AlignedIntCopyTexture2D,

    /* [               |            | priv::kClear | priv::kTexture3D     ] = */
    SID::ClearTexture3D,
    /* [priv::kAligned |            | priv::kClear | priv::kTexture3D     ] = */
    SID::AlignedClearTexture3D,
    /* [               | priv::kInt | priv::kClear | priv::kTexture3D     ] = */
    SID::IntClearTexture3D,
    /* [priv::kAligned | priv::kInt | priv::kClear | priv::kTexture3D     ] = */
    SID::AlignedIntClearTexture3D,
    /* [               |            | priv::kCopy  | priv::kTexture3D     ] = */ SID::CopyTexture3D,
    /* [priv::kAligned |            | priv::kCopy  | priv::kTexture3D     ] = */
    SID::AlignedCopyTexture3D,
    /* [               | priv::kInt | priv::kCopy  | priv::kTexture3D     ] = */
    SID::IntCopyTexture3D,
    /* [priv::kAligned | priv::kInt | priv::kCopy  | priv::kTexture3D     ] = */
    SID::AlignedIntCopyTexture3D,

    /* [               |            | priv::kClear | priv::kTexture2DArray] = */
    SID::ClearTexture2DArray,
    /* [priv::kAligned |            | priv::kClear | priv::kTexture2DArray] = */
    SID::AlignedClearTexture2DArray,
    /* [               | priv::kInt | priv::kClear | priv::kTexture2DArray] = */
    SID::IntClearTexture2DArray,
    /* [priv::kAligned | priv::kInt | priv::kClear | priv::kTexture2DArray] = */
    SID::AlignedIntClearTexture2DArray,
    /* [               |            | priv::kCopy  | priv::kTexture2DArray] = */
    SID::CopyTexture2DArray,
    /* [priv::kAligned |            | priv::kCopy  | priv::kTexture2DArray] = */
    SID::AlignedCopyTexture2DArray,
    /* [               | priv::kInt | priv::kCopy  | priv::kTexture2DArray] = */
    SID::IntCopyTexture2DArray,
    /* [priv::kAligned | priv::kInt | priv::kCopy  | priv::kTexture2DArray] = */
    SID::AlignedIntCopyTexture2DArray,

    // The rest of the array will be filled with 0, i.e. SID::Invalid.
};

}  // namespace priv

DispatchUtilsVk::DispatchUtilsVk()
{
}

~DispatchUtilsVk::DispatchUtilsVk()
{
}

static constexpr uint32_t kTextureClearSetIndex 0 static constexpr uint32_t
    kTextureClearOutputBinding 0

    static constexpr uint32_t kTextureCopySetIndex 0 static constexpr uint32_t
        kTextureCopySourceBinding 0 static constexpr uint32_t kTextureCopyDestinationBinding 1

    angle::Result
    DispatchUtilsVk::initialize(vk::Context *context)
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

    bufferClearSetDesc.update(kBufferClearOutputBinding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);

    bufferCopySetDesc.update(kBufferCopySourceBinding, VK_DESCRIPTOR_TYPE_UNIFRM_BUFFER, 1);
    bufferCopySetDesc.update(kBufferCopyDestinationBinding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);

    textureClearSetDesc.update(kTextureClearOutputBinding, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1);

    textureCopySetDesc.update(kTextureCopySourceBinding, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1);
    textureCopySetDesc.update(kTextureCopyDestinationBinding, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1);

    ANGLE_TRY(
        renderer->getDescriptorSetLayout(contextVk, bufferClearSetDesc, &mBufferClearSetLayout));
    ANGLE_TRY(
        renderer->getDescriptorSetLayout(contextVk, bufferCopySetDesc, &mBufferCopySetLayout));
    ANGLE_TRY(
        renderer->getDescriptorSetLayout(contextVk, textureClearSetDesc, &mTextureClearSetLayout));
    ANGLE_TRY(
        renderer->getDescriptorSetLayout(contextVk, textureCopySetDesc, &mTextureCopySetLayout));

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

    bufferClearPipelineLayoutDesc.updateDescriptorSetLayout(kBufferClearSetIndex,
                                                            bufferClearSetDesc);
    bufferClearPipelineLayoutDesc.updatePushConstantRange(gl::ShaderType::Compute, 0, 16);

    bufferCopyPipelineLayoutDesc.updateDescriptorSetLayout(kBufferCopySetIndex, bufferCopySetDesc);
    bufferCopyPipelineLayoutDesc.updatePushConstantRange(gl::ShaderType::Compute, 0, 32);

    textureClearPipelineLayoutDesc.updateDescriptorSetLayout(kTextureClearSetIndex,
                                                             textureClearSetDesc);
    textureClearPipelineLayoutDesc.updatePushConstantRange(gl::ShaderType::Compute, 0, 16);

    textureCopyPipelineLayoutDesc.updateDescriptorSetLayout(kTextureCopySetIndex,
                                                            textureCopySetDesc);
    textureCopyPipelineLayoutDesc.updatePushConstantRange(gl::ShaderType::Compute, 0, 32);

    ANGLE_TRY(renderer->getPipelineLayout(contextVk, bufferClearPipelineLayoutDesc,
                                          &mBufferClearSetLayout, 1, &mBufferClearPipelineLayout));
    ANGLE_TRY(renderer->getPipelineLayout(contextVk, bufferCopyPipelineLayoutDesc,
                                          &mBufferCopySetLayout, 1, &mBufferCopyPipelineLayout));
    ANGLE_TRY(renderer->getPipelineLayout(contextVk, textureClearPipelineLayoutDesc,
                                          &mTextureClearSetLayout, 1,
                                          &mTextureClearPipelineLayout));
    ANGLE_TRY(renderer->getPipelineLayout(contextVk, textureCopyPipelineLayoutDesc,
                                          &mTextureCopySetLayout, 1, &mTextureCopyPipelineLayout));
}

angle::Result DispatchUtilsVk::bindPipeline(vk::Context *context,
                                            uint32_t function,
                                            vk::CommandBuffer *commandBuffer)
{
    RendererVk *renderer = context->getRenderer();

    const vk::ShaderAndSerial *shader = nullptr;
    ANGLE_TRY(renderer->getShaderLibrary(context, priv::kShaderIDS[function], &shader));

    vk::ComputePipelineDesc pipelineDesc;
    pipelineDesc.initDefaults();
    pipelineDesc.updateShaders(shader->getSerial());

    bool isClear  = (function & priv::kFunctionMask) == priv::kFunctionClear;
    bool isBuffer = (function & priv::kResourceTypeMask) == priv::kBuffer;
    const vk::BindingPointer<vk::PipelineLayout> &pipelineLayout =
        isClear ? isBuffer ? mBufferClearPipelineLayout : mTextureClearPipelineLayout
                : isBuffer ? mBufferCopyPipelineLayout : mTextureCopyPipelineLayout;

    vk::PipelineAndSerial *pipelineAndSerial;
    ANGLE_TRY(
        renderer->getPipeline(this, *shader, *pipelineLayout, *pipelineDesc, &pipelineAndSerial));

    commandBuffer->bindPipeline(VK_PIPELINE_BIND_POINT_COMPUTE, pipelineAndSerial->get());
    pipelineAndSerial->updateSerial(renderer->getCurrentQueueSerial);

    return Angle::Result::Continue();
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
