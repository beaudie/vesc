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
namespace
{
// All internal shaders assume there is only one descriptor set, indexed at 0
constexpr uint32_t kSetIndex = 0;
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

}  // namespace rx
