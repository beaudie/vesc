//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DrawUtilsVk.cpp:
//    Implements the DrawUtilsVk class.
//

#include "libANGLE/renderer/vulkan/DrawUtilsVk.h"

#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/FramebufferVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"

namespace rx
{

namespace ImageCopy_frag = vk::InternalShader::ImageCopy_frag;

namespace
{
// All internal shaders assume there is only one descriptor set, indexed at 0
constexpr uint32_t kSetIndex = 0;

constexpr uint32_t kImageCopySourceBinding = 0;

uint32_t GetImageCopyFlags(const vk::Format &srcFormat, const vk::Format &destFormat)
{
    const angle::Format &srcAngleFormat  = srcFormat.angleFormat();
    const angle::Format &destAngleFormat = destFormat.angleFormat();

    bool srcIsInt  = srcAngleFormat.componentType == GL_INT;
    bool srcIsUint = srcAngleFormat.componentType == GL_UNSIGNED_INT;

    bool destIsInt  = destAngleFormat.componentType == GL_INT;
    bool destIsUint = destAngleFormat.componentType == GL_UNSIGNED_INT;

    uint32_t flags = 0;

    flags |= srcIsInt ? ImageCopy_frag::kSrcIsInt
                      : srcIsUint ? ImageCopy_frag::kSrcIsUint : ImageCopy_frag::kSrcIsFloat;
    flags |= destIsInt ? ImageCopy_frag::kDestIsInt
                       : destIsUint ? ImageCopy_frag::kDestIsUint : ImageCopy_frag::kDestIsFloat;

    return flags;
}
}  // namespace

DrawUtilsVk::DrawUtilsVk() = default;

DrawUtilsVk::~DrawUtilsVk() = default;

void DrawUtilsVk::destroy(VkDevice device)
{
    for (Function f : angle::AllEnums<Function>())
    {
        for (auto &descriptorSetLayout : mDescriptorSetLayouts[f])
        {
            descriptorSetLayout.reset();
        }
        mPipelineLayouts[f].reset();
        mDescriptorPools[f].destroy(device);
    }

    for (vk::ShaderProgramHelper &program : mImageClearPrograms)
    {
        program.destroy(device);
    }
    for (vk::ShaderProgramHelper &program : mImageCopyPrograms)
    {
        program.destroy(device);
    }
}

angle::Result DrawUtilsVk::ensureResourcesInitialized(vk::Context *context,
                                                      Function function,
                                                      VkDescriptorPoolSize *setSizes,
                                                      size_t setSizesCount,
                                                      size_t pushConstantsSize)
{
    RendererVk *renderer = context->getRenderer();

    vk::DescriptorSetLayoutDesc descriptorSetDesc;

    uint32_t currentBinding = 0;
    for (size_t i = 0; i < setSizesCount; ++i)
    {
        descriptorSetDesc.update(currentBinding, setSizes[i].type, setSizes[i].descriptorCount);
        currentBinding += setSizes[i].descriptorCount;
    }

    ANGLE_TRY(renderer->getDescriptorSetLayout(context, descriptorSetDesc,
                                               &mDescriptorSetLayouts[function][kSetIndex]));

    // Corresponding pipeline layouts:
    vk::PipelineLayoutDesc pipelineLayoutDesc;

    pipelineLayoutDesc.updateDescriptorSetLayout(kSetIndex, descriptorSetDesc);
    pipelineLayoutDesc.updatePushConstantRange(gl::ShaderType::Fragment, 0, pushConstantsSize);

    ANGLE_TRY(renderer->getPipelineLayout(
        context, pipelineLayoutDesc, mDescriptorSetLayouts[function], &mPipelineLayouts[function]));

    if (setSizesCount > 0)
    {
        ANGLE_TRY(mDescriptorPools[function].init(context, setSizes, setSizesCount));
    }

    return angle::Result::Continue;
}

angle::Result DrawUtilsVk::ensureImageClearInitialized(vk::Context *context)
{
    if (mPipelineLayouts[Function::ImageClear].valid())
    {
        return angle::Result::Continue;
    }

    // The shader does not use any descriptor sets.
    return ensureResourcesInitialized(context, Function::ImageClear, nullptr, 0,
                                      sizeof(ImageClearShaderParams));
}

angle::Result DrawUtilsVk::ensureImageCopyInitialized(vk::Context *context)
{
    if (mPipelineLayouts[Function::ImageCopy].valid())
    {
        return angle::Result::Continue;
    }

    VkDescriptorPoolSize setSizes[1] = {
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1},
    };

    return ensureResourcesInitialized(context, Function::ImageCopy, setSizes, ArraySize(setSizes),
                                      sizeof(ImageCopyShaderParams));
}

angle::Result DrawUtilsVk::setupProgramCommon(vk::Context *context,
                                              Function function,
                                              vk::RefCounted<vk::ShaderAndSerial> *vertexShader,
                                              vk::RefCounted<vk::ShaderAndSerial> *fragmentShader,
                                              vk::ShaderProgramHelper *program,
                                              const vk::GraphicsPipelineDesc &pipelineDesc,
                                              const VkDescriptorSet descriptorSet,
                                              const void *pushConstants,
                                              size_t pushConstantsSize,
                                              vk::CommandBuffer *commandBuffer)
{
    RendererVk *renderer = context->getRenderer();

    program->setShader(gl::ShaderType::Vertex, vertexShader);
    program->setShader(gl::ShaderType::Fragment, fragmentShader);

    const vk::BindingPointer<vk::PipelineLayout> &pipelineLayout = mPipelineLayouts[function];

    vk::PipelineAndSerial *pipelineAndSerial;
    ANGLE_TRY(program->getGraphicsPipeline(context, pipelineLayout.get(), pipelineDesc,
                                           gl::AttributesMask(), &pipelineAndSerial));

    commandBuffer->bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineAndSerial->get());
    pipelineAndSerial->updateSerial(renderer->getCurrentQueueSerial());

    if (descriptorSet != VK_NULL_HANDLE)
    {
        commandBuffer->bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.get(), 0,
                                          1, &descriptorSet, 0, nullptr);
    }

    commandBuffer->pushConstants(pipelineLayout.get(), VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                 pushConstantsSize, pushConstants);

    return angle::Result::Continue;
}

template <DrawUtilsVk::GetShader getVS,
          DrawUtilsVk::GetShader getFS,
          DrawUtilsVk::Function function,
          typename ShaderParams>
angle::Result DrawUtilsVk::setupProgram(vk::Context *context,
                                        vk::ShaderProgramHelper *program,
                                        uint32_t fsFlags,
                                        const vk::GraphicsPipelineDesc &pipelineDesc,
                                        const VkDescriptorSet descriptorSet,
                                        const ShaderParams &params,
                                        vk::CommandBuffer *commandBuffer)
{
    RendererVk *renderer             = context->getRenderer();
    vk::ShaderLibrary &shaderLibrary = renderer->getShaderLibrary();

    vk::RefCounted<vk::ShaderAndSerial> *vertexShader   = nullptr;
    vk::RefCounted<vk::ShaderAndSerial> *fragmentShader = nullptr;
    ANGLE_TRY((shaderLibrary.*getVS)(context, 0, &vertexShader));
    ANGLE_TRY((shaderLibrary.*getFS)(context, fsFlags, &fragmentShader));

    ANGLE_TRY(setupProgramCommon(context, function, vertexShader, fragmentShader, program,
                                 pipelineDesc, descriptorSet, &params, sizeof(params),
                                 commandBuffer));

    return angle::Result::Continue;
}

angle::Result DrawUtilsVk::startRenderPass(vk::Context *context,
                                           vk::ImageHelper *image,
                                           vk::ImageView *imageView,
                                           const vk::RenderPassDesc &renderPassDesc,
                                           const gl::Rectangle &renderArea,
                                           vk::CommandBuffer **commandBufferOut)
{
    RendererVk *renderer = context->getRenderer();

    vk::RenderPass *renderPass = nullptr;
    ANGLE_TRY(renderer->getCompatibleRenderPass(context, renderPassDesc, &renderPass));

    VkFramebufferCreateInfo framebufferInfo = {};

    framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.flags           = 0;
    framebufferInfo.renderPass      = renderPass->getHandle();
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments    = imageView->ptr();
    framebufferInfo.width           = renderArea.x + renderArea.width;
    framebufferInfo.height          = renderArea.y + renderArea.height;
    framebufferInfo.layers          = 1;

    vk::Framebuffer framebuffer;
    ANGLE_VK_TRY(context, framebuffer.init(context->getDevice(), framebufferInfo));

    // TODO(jmadill): Proper clear value implementation. http://anglebug.com/2361
    std::vector<VkClearValue> clearValues = {{}};
    ASSERT(clearValues.size() == 1);

    ANGLE_TRY(image->beginRenderPass(context, framebuffer, renderArea, renderPassDesc, clearValues,
                                     commandBufferOut));

    renderer->releaseObject(renderer->getCurrentQueueSerial(), &framebuffer);

    return angle::Result::Continue;
}

angle::Result DrawUtilsVk::clearImage(ContextVk *context,
                                      FramebufferVk *framebuffer,
                                      const ClearImageParameters &params)
{
    ANGLE_TRY(ensureImageClearInitialized(context));

    vk::CommandBuffer *commandBuffer;
    if (!framebuffer->appendToStartedRenderPass(context->getRenderer(), &commandBuffer))
    {
        ANGLE_TRY(framebuffer->startNewRenderPass(context, &commandBuffer))
    }

    ImageClearShaderParams shaderParams;
    shaderParams.clearValue = params.clearValue;

    vk::GraphicsPipelineDesc pipelineDesc;
    pipelineDesc.initDefaults();
    pipelineDesc.updateColorWriteMask(params.colorMaskFlags, *params.alphaMask);
    pipelineDesc.updateRenderPassDesc(*params.renderPassDesc);

    ANGLE_TRY(
        (setupProgram<&vk::ShaderLibrary::getFullScreenQuad_vert,
                      &vk::ShaderLibrary::getImageClear_frag, Function::ImageClear,
                      ImageClearShaderParams>(context, &mImageClearPrograms[0], 0, pipelineDesc,
                                              VK_NULL_HANDLE, shaderParams, commandBuffer)));

    VkViewport viewport;
    const gl::Rectangle &renderArea = framebuffer->getFramebuffer()->getRenderPassRenderArea();
    bool invertViewport             = context->isViewportFlipEnabledForDrawFBO();
    gl_vk::GetViewport(renderArea, 0.0f, 1.0f, invertViewport, params.renderAreaHeight, &viewport);

    VkRect2D scissor;
    const gl::State &glState = context->getGLState();
    gl_vk::GetScissor(glState, invertViewport, renderArea, &scissor);

    commandBuffer->setViewport(0, 1, &viewport);
    commandBuffer->setScissor(0, 1, &scissor);
    commandBuffer->draw(6, 1, 0, 0);

    return angle::Result::Continue;
}

angle::Result DrawUtilsVk::copyImage(vk::Context *context,
                                     vk::ImageHelper *dest,
                                     vk::ImageView *destView,
                                     vk::ImageHelper *src,
                                     vk::ImageView *srcView,
                                     const CopyImageParameters &params)
{
    ANGLE_TRY(ensureImageCopyInitialized(context));

    const vk::Format &srcFormat  = src->getFormat();
    const vk::Format &destFormat = dest->getFormat();

    ImageCopyShaderParams shaderParams;
    shaderParams.flipY      = params.flipY;
    shaderParams.destIsLuma = destFormat.angleFormat().luminanceBits > 0;
    shaderParams.destIsAlpha =
        destFormat.angleFormat().hasLuminanceAlphaBits() && destFormat.angleFormat().alphaBits > 0;
    shaderParams.srcMip        = params.srcMip;
    shaderParams.srcOffset[0]  = params.srcOffset[0];
    shaderParams.srcOffset[1]  = params.srcOffset[1];
    shaderParams.destOffset[0] = params.destOffset[0];
    shaderParams.destOffset[1] = params.destOffset[1];

    if (params.flipY)
    {
        // If viewport is flipped, the shader expects srcOffset[1] to have the
        // last row's index instead of the first's.
        shaderParams.srcOffset[1] = params.srcHeight - shaderParams.srcOffset[1] - 1;
    }

    uint32_t flags = GetImageCopyFlags(srcFormat, destFormat);

    VkDescriptorSet descriptorSet;
    vk::SharedDescriptorPoolBinding descriptorPoolBinding;
    ANGLE_TRY(mDescriptorPools[Function::ImageCopy].allocateSets(
        context, mDescriptorSetLayouts[Function::ImageCopy][kSetIndex].get().ptr(), 1,
        &descriptorPoolBinding, &descriptorSet));
    descriptorPoolBinding.get().updateSerial(context->getRenderer()->getCurrentQueueSerial());

    vk::RenderPassDesc renderPassDesc;
    renderPassDesc.setSamples(dest->getSamples());
    renderPassDesc.packAttachment(destFormat);

    vk::GraphicsPipelineDesc pipelineDesc;
    pipelineDesc.initDefaults();
    pipelineDesc.updateRenderPassDesc(renderPassDesc);

    gl::Rectangle renderArea;
    renderArea.x      = params.destOffset[0];
    renderArea.y      = params.destOffset[1];
    renderArea.width  = params.srcExtents[0];
    renderArea.height = params.srcExtents[1];

    // Change source layout outside render pass
    vk::CommandBuffer *srcLayoutChange;
    ANGLE_TRY(src->recordCommands(context, &srcLayoutChange));

    src->changeLayoutWithStages(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, srcLayoutChange);

    // Change destination layout outside render pass as well
    vk::CommandBuffer *destLayoutChange;
    ANGLE_TRY(dest->recordCommands(context, &destLayoutChange));

    dest->changeLayoutWithStages(VK_IMAGE_ASPECT_COLOR_BIT,
                                 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, destLayoutChange);

    vk::CommandBuffer *commandBuffer;
    ANGLE_TRY(startRenderPass(context, dest, destView, renderPassDesc, renderArea, &commandBuffer));

    // Source's layout change should happen before rendering
    src->addReadDependency(dest);

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageView             = srcView->getHandle();
    imageInfo.imageLayout           = src->getCurrentLayout();

    VkWriteDescriptorSet writeInfo = {};
    writeInfo.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.dstSet               = descriptorSet;
    writeInfo.dstBinding           = kImageCopySourceBinding;
    writeInfo.descriptorCount      = 1;
    writeInfo.descriptorType       = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    writeInfo.pImageInfo           = &imageInfo;

    vkUpdateDescriptorSets(context->getDevice(), 1, &writeInfo, 0, nullptr);

    ANGLE_TRY((setupProgram<&vk::ShaderLibrary::getFullScreenQuad_vert,
                            &vk::ShaderLibrary::getImageCopy_frag, Function::ImageCopy,
                            ImageCopyShaderParams>(context, &mImageCopyPrograms[flags], flags,
                                                   pipelineDesc, descriptorSet, shaderParams,
                                                   commandBuffer)));

    VkViewport viewport;
    gl_vk::GetViewport(renderArea, 0.0f, 1.0f, false, dest->getExtents().height, &viewport);

    VkRect2D scissor = gl_vk::GetRect(renderArea);

    commandBuffer->setViewport(0, 1, &viewport);
    commandBuffer->setScissor(0, 1, &scissor);
    commandBuffer->draw(6, 1, 0, 0);

    descriptorPoolBinding.reset();

    return angle::Result::Continue;
}

}  // namespace rx
