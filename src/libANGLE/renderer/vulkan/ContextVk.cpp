//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ContextVk.cpp:
//    Implements the class methods for ContextVk.
//

#include "libANGLE/renderer/vulkan/ContextVk.h"

#include "common/BitSetIterator.h"
#include "common/debug.h"
#include "libANGLE/Program.h"
#include "libANGLE/renderer/vulkan/BufferVk.h"
#include "libANGLE/renderer/vulkan/CompilerVk.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/DeviceVk.h"
#include "libANGLE/renderer/vulkan/FenceNVVk.h"
#include "libANGLE/renderer/vulkan/FenceSyncVk.h"
#include "libANGLE/renderer/vulkan/FramebufferVk.h"
#include "libANGLE/renderer/vulkan/ImageVk.h"
#include "libANGLE/renderer/vulkan/ProgramVk.h"
#include "libANGLE/renderer/vulkan/QueryVk.h"
#include "libANGLE/renderer/vulkan/RenderbufferVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/renderer/vulkan/SamplerVk.h"
#include "libANGLE/renderer/vulkan/ShaderVk.h"
#include "libANGLE/renderer/vulkan/TextureVk.h"
#include "libANGLE/renderer/vulkan/TransformFeedbackVk.h"
#include "libANGLE/renderer/vulkan/VertexArrayVk.h"
#include "libANGLE/renderer/vulkan/formatutilsvk.h"

namespace rx
{

ContextVk::ContextVk(const gl::ContextState &state, RendererVk *renderer)
    : ContextImpl(state), mRenderer(renderer)
{
}

ContextVk::~ContextVk()
{
}

gl::Error ContextVk::initialize()
{
    return gl::NoError();
}

gl::Error ContextVk::flush()
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error ContextVk::finish()
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error ContextVk::prepareVertexInput(GLsizei count,
                                        GLsizei instanceCount,
                                        gl::BufferState &dummyState,
                                        std::vector<VkVertexInputBindingDescription> *outBindings,
                                        std::vector<VkVertexInputAttributeDescription> *outAttribs,
                                        std::vector<VkBuffer> *outHandles,
                                        std::vector<VkDeviceSize> *outOffsets,
                                        std::vector<BufferVk *> *outInterimBuffers)
{
    const auto &state     = mState.getState();
    const auto &programGL = state.getProgram();
    const auto &vao       = state.getVertexArray();
    const auto &attribs   = vao->getVertexAttributes();

    for (auto attribIndex : angle::IterateBitSet(programGL->getActiveAttribLocationsMask()))
    {
        const auto &attrib = attribs[attribIndex];
        if (attrib.enabled)
        {
            size_t typeSize = gl::ComputeVertexAttributeTypeSize(attrib);

            VkVertexInputBindingDescription bindingDesc;
            bindingDesc.binding = static_cast<uint32_t>((*outBindings).size());
            bindingDesc.stride  = (attrib.stride == 0 ? typeSize : attrib.stride);
            bindingDesc.inputRate =
                (attrib.divisor > 0 ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX);

            gl::VertexFormatType vertexFormatType = gl::GetVertexFormatType(attrib);

            VkVertexInputAttributeDescription attribDesc;
            attribDesc.binding  = bindingDesc.binding;
            attribDesc.format   = vk::GetNativeVertexFormat(vertexFormatType);
            attribDesc.location = static_cast<uint32_t>(attribIndex);
            attribDesc.offset   = attrib.buffer.get() ? static_cast<uint32_t>(attrib.offset) : 0u;

            gl::Buffer *bufferGL = attrib.buffer.get();
            BufferVk *bufferVk;
            // Vulkan only support divisor of vaule 1
            if (bufferGL == nullptr || attrib.divisor > 1)
            {
                size_t elementCount = (attrib.divisor > 0 ? instanceCount : count);
                BufferVk *buffer    = new BufferVk(dummyState);
                ANGLE_TRY(buffer->setData(this, GL_ARRAY_BUFFER, nullptr, typeSize * elementCount,
                                          GL_DYNAMIC_DRAW));
                GLvoid *data;
                ANGLE_TRY(buffer->map(GL_MAP_WRITE_BIT, &data));
                uint8_t *dst   = static_cast<uint8_t *>(data);
                size_t stride  = (attrib.stride == 0 ? typeSize : attrib.stride);
                size_t divisor = (attrib.divisor > 1 ? attrib.divisor : 1);
                GLboolean result;

                if (bufferGL == nullptr)
                {
                    const uint8_t *src = static_cast<const uint8_t *>(attrib.pointer);
                    for (size_t i = 0; i < elementCount; i++, dst += typeSize)
                        memcpy(dst, src + (i / divisor) * stride, typeSize);
                }
                else
                {
                    GLvoid *read;
                    ANGLE_TRY(GetImplAs<BufferVk>(bufferGL)->map(GL_MAP_READ_BIT, &read));
                    uint8_t *src = static_cast<uint8_t *>(read) + attrib.offset;
                    for (size_t i = 0; i < elementCount; i++, dst += typeSize)
                        memcpy(dst, src + (i / divisor) * stride, typeSize);
                    ANGLE_TRY(GetImplAs<BufferVk>(bufferGL)->unmap(&result));
                }
                ANGLE_TRY(buffer->unmap(&result));
                bindingDesc.stride = typeSize;
                attribDesc.offset  = 0;
                (*outInterimBuffers).push_back(buffer);
                bufferVk = buffer;
            }
            else
            {
                bufferVk = GetImplAs<BufferVk>(bufferGL);
            }

            (*outBindings).push_back(bindingDesc);
            (*outAttribs).push_back(attribDesc);

            (*outHandles).push_back(bufferVk->getVkBuffer().getHandle());
            (*outOffsets).push_back(0);
        }
        else
        {
            UNIMPLEMENTED();
        }
    }

    return gl::NoError();
}

gl::Error ContextVk::prepareIndexInput(GLsizei count,
                                       GLenum type,
                                       const GLvoid *indices,
                                       gl::BufferState &dummyState,
                                       BufferVk **outBuffer,
                                       VkDeviceSize *outOffset,
                                       VkIndexType *outType,
                                       bool *outNewBufferCreated)
{
    const auto &state = mState.getState();
    const auto &vao   = state.getVertexArray();

    gl::Buffer *elementArrayBuffer = vao->getElementArrayBuffer().get();

    // Vulkan doesn't support GL_UNSIGNED_BYTE type
    if (elementArrayBuffer == nullptr || type == GL_UNSIGNED_BYTE)
    {
        GLboolean result;
        const uint8_t *src;
        uint32_t typeSize;

        if (elementArrayBuffer != nullptr)
        {
            GLvoid *read;
            ANGLE_TRY(GetImplAs<BufferVk>(elementArrayBuffer)->map(GL_MAP_READ_BIT, &read));
            uint64_t offset = reinterpret_cast<uint64_t>(indices);
            src             = static_cast<uint8_t *>(read) + offset;
        }
        else
        {
            src = static_cast<const uint8_t *>(indices);
        }

        if (type == GL_UNSIGNED_BYTE || type == GL_UNSIGNED_SHORT)
        {
            typeSize = 2;
        }
        else if (type == GL_UNSIGNED_INT)
        {
            typeSize = 4;
        }
        BufferVk *buffer = new BufferVk(dummyState);
        ANGLE_TRY(buffer->setData(this, GL_ELEMENT_ARRAY_BUFFER, nullptr, typeSize * count,
                                  GL_DYNAMIC_DRAW));
        GLvoid *data;
        ANGLE_TRY(buffer->map(GL_MAP_WRITE_BIT, &data));
        if (type == GL_UNSIGNED_BYTE)
        {
            uint16_t *dst = static_cast<uint16_t *>(data);
            for (size_t i = 0; i < count; i++)
            {
                if (src[i] == 0xFFu)
                    dst[i] = 0xFFFFu;
                else
                    dst[i] = src[i];
            }
        }
        else
        {
            memcpy(data, src, typeSize * count);
        }
        ANGLE_TRY(buffer->unmap(&result));
        if (elementArrayBuffer != nullptr)
        {
            ANGLE_TRY(GetImplAs<BufferVk>(elementArrayBuffer)->unmap(&result));
        }
        *outBuffer           = buffer;
        *outOffset           = 0;
        *outNewBufferCreated = true;
    }
    else
    {
        *outBuffer           = GetImplAs<BufferVk>(elementArrayBuffer);
        *outOffset           = reinterpret_cast<VkDeviceSize>(indices);
        *outNewBufferCreated = false;
    }
    *outType = (type == GL_UNSIGNED_INT ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);

    return gl::NoError();
}

gl::Error ContextVk::preparePipeline(GLenum mode,
                                     std::vector<VkVertexInputBindingDescription> &vertexBindings,
                                     std::vector<VkVertexInputAttributeDescription> &vertexAttribs,
                                     vk::RenderPass *renderPass)
{
    VkDevice device       = mRenderer->getDevice();
    const auto &state     = mState.getState();
    const auto &programGL = state.getProgram();
    const auto &programVk = GetImplAs<ProgramVk>(programGL);

    // { vertex, fragment }
    VkPipelineShaderStageCreateInfo shaderStages[2];

    shaderStages[0].sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].pNext               = nullptr;
    shaderStages[0].flags               = 0;
    shaderStages[0].stage               = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module              = programVk->getLinkedVertexModule().getHandle();
    shaderStages[0].pName               = "main";
    shaderStages[0].pSpecializationInfo = nullptr;

    shaderStages[1].sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].pNext               = nullptr;
    shaderStages[1].flags               = 0;
    shaderStages[1].stage               = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module              = programVk->getLinkedFragmentModule().getHandle();
    shaderStages[1].pName               = "main";
    shaderStages[1].pSpecializationInfo = nullptr;

    // TODO(jmadill): Validate with ASSERT against physical device limits/caps?
    VkPipelineVertexInputStateCreateInfo vertexInputState;
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.pNext = nullptr;
    vertexInputState.flags = 0;
    vertexInputState.vertexBindingDescriptionCount   = static_cast<uint32_t>(vertexBindings.size());
    vertexInputState.pVertexBindingDescriptions      = vertexBindings.data();
    vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttribs.size());
    vertexInputState.pVertexAttributeDescriptions    = vertexAttribs.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
    inputAssemblyState.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.pNext    = nullptr;
    inputAssemblyState.flags    = 0;
    inputAssemblyState.topology = gl_vk::GetPrimitiveTopology(mode);
    inputAssemblyState.primitiveRestartEnable = state.isPrimitiveRestartEnabled();

    const gl::Rectangle &viewportGL = state.getViewport();
    VkViewport viewportVk;
    viewportVk.x        = static_cast<float>(viewportGL.x);
    viewportVk.y        = static_cast<float>(viewportGL.y);
    viewportVk.width    = static_cast<float>(viewportGL.width);
    viewportVk.height   = static_cast<float>(viewportGL.height);
    viewportVk.minDepth = state.getNearPlane();
    viewportVk.maxDepth = state.getFarPlane();

    // TODO(jmadill): Scissor.
    VkRect2D scissorVk;
    scissorVk.offset.x      = viewportGL.x;
    scissorVk.offset.y      = viewportGL.y;
    scissorVk.extent.width  = viewportGL.width;
    scissorVk.extent.height = viewportGL.height;

    VkPipelineViewportStateCreateInfo viewportState;
    viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext         = nullptr;
    viewportState.flags         = 0;
    viewportState.viewportCount = 1;
    viewportState.pViewports    = &viewportVk;
    viewportState.scissorCount  = 1;
    viewportState.pScissors     = &scissorVk;

    // TODO(jmadill): Extra rasterizer state features.
    VkPipelineRasterizationStateCreateInfo rasterState;
    rasterState.sType            = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterState.pNext            = nullptr;
    rasterState.flags            = 0;
    rasterState.depthClampEnable = VK_FALSE;
    rasterState.rasterizerDiscardEnable = VK_FALSE;
    rasterState.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterState.cullMode                = gl_vk::GetCullMode(state.getRasterizerState());
    rasterState.frontFace               = gl_vk::GetFrontFace(state.getRasterizerState().frontFace);
    rasterState.depthBiasEnable         = VK_FALSE;
    rasterState.depthBiasConstantFactor = 0.0f;
    rasterState.depthBiasClamp          = 0.0f;
    rasterState.depthBiasSlopeFactor    = 0.0f;
    rasterState.lineWidth               = state.getLineWidth();

    // TODO(jmadill): Multisample state.
    VkPipelineMultisampleStateCreateInfo multisampleState;
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.pNext = nullptr;
    multisampleState.flags = 0;
    multisampleState.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampleState.sampleShadingEnable   = VK_FALSE;
    multisampleState.minSampleShading      = 0.0f;
    multisampleState.pSampleMask           = nullptr;
    multisampleState.alphaToCoverageEnable = VK_FALSE;
    multisampleState.alphaToOneEnable      = VK_FALSE;

    // TODO(jmadill): Depth/stencil state.
    VkPipelineDepthStencilStateCreateInfo depthStencilState;
    memset(&depthStencilState, 0, sizeof(depthStencilState));
    depthStencilState.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.pNext            = nullptr;
    depthStencilState.depthTestEnable  = VK_TRUE;
    depthStencilState.depthWriteEnable = VK_TRUE;
    depthStencilState.depthCompareOp   = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.back.failOp           = VK_STENCIL_OP_KEEP;
    depthStencilState.back.passOp           = VK_STENCIL_OP_KEEP;
    depthStencilState.back.compareOp        = VK_COMPARE_OP_ALWAYS;
    depthStencilState.stencilTestEnable     = VK_FALSE;
    depthStencilState.front                 = depthStencilState.back;

    // TODO(jmadill): Blend state/MRT.
    VkPipelineColorBlendAttachmentState blendAttachmentState;
    blendAttachmentState.blendEnable         = VK_FALSE;
    blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    blendAttachmentState.colorBlendOp        = VK_BLEND_OP_ADD;
    blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blendAttachmentState.alphaBlendOp        = VK_BLEND_OP_ADD;
    blendAttachmentState.colorWriteMask = (VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                           VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);

    VkPipelineColorBlendStateCreateInfo blendState;
    blendState.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendState.pNext             = 0;
    blendState.flags             = 0;
    blendState.logicOpEnable     = VK_FALSE;
    blendState.logicOp           = VK_LOGIC_OP_CLEAR;
    blendState.attachmentCount   = 1;
    blendState.pAttachments      = &blendAttachmentState;
    blendState.blendConstants[0] = 0.0f;
    blendState.blendConstants[1] = 0.0f;
    blendState.blendConstants[2] = 0.0f;
    blendState.blendConstants[3] = 0.0f;

    vk::PipelineLayout *pipelineLayout = nullptr;
    ANGLE_TRY_RESULT(programVk->getPipelineLayout(device), pipelineLayout);
    ASSERT(pipelineLayout && pipelineLayout->valid());

    VkGraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext               = nullptr;
    pipelineInfo.flags               = 0;
    pipelineInfo.stageCount          = 2;
    pipelineInfo.pStages             = shaderStages;
    pipelineInfo.pVertexInputState   = &vertexInputState;
    pipelineInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineInfo.pTessellationState  = nullptr;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterState;
    pipelineInfo.pMultisampleState   = &multisampleState;
    pipelineInfo.pDepthStencilState  = &depthStencilState;
    pipelineInfo.pColorBlendState    = &blendState;
    pipelineInfo.pDynamicState       = nullptr;
    pipelineInfo.layout              = pipelineLayout->getHandle();
    pipelineInfo.renderPass          = renderPass->getHandle();
    pipelineInfo.subpass             = 0;
    pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex   = 0;

    vk::Pipeline newPipeline(device);
    ANGLE_TRY(newPipeline.initGraphics(pipelineInfo));

    mCurrentPipeline = std::move(newPipeline);

    return gl::NoError();
}

gl::Error ContextVk::drawArrays(GLenum mode, GLint first, GLsizei count)
{
    return drawArraysInstanced(mode, first, count, 1);
}

gl::Error ContextVk::drawArraysInstanced(GLenum mode,
                                         GLint first,
                                         GLsizei count,
                                         GLsizei instanceCount)
{
    VkDevice device      = mRenderer->getDevice();
    const auto &state    = mState.getState();
    const auto *drawFBO  = state.getDrawFramebuffer();
    FramebufferVk *vkFBO = GetImplAs<FramebufferVk>(drawFBO);

    // Process vertex attributes
    // TODO(jmadill): Caching with dirty bits.
    std::vector<VkVertexInputBindingDescription> vertexBindings;
    std::vector<VkVertexInputAttributeDescription> vertexAttribs;
    std::vector<VkBuffer> vertexHandles;
    std::vector<VkDeviceSize> vertexOffsets;
    std::vector<BufferVk *> interimBuffers;
    gl::BufferState dummyState;

    ANGLE_TRY(prepareVertexInput(count, instanceCount, dummyState, &vertexBindings, &vertexAttribs,
                                 &vertexHandles, &vertexOffsets, &interimBuffers));

    // TODO(jmadill): Dynamic state.
    vk::RenderPass *renderPass = nullptr;
    ANGLE_TRY_RESULT(vkFBO->getRenderPass(device), renderPass);
    ASSERT(renderPass && renderPass->valid());

    ANGLE_TRY(preparePipeline(mode, vertexBindings, vertexAttribs, renderPass));

    vk::CommandBuffer *commandBuffer = mRenderer->getCommandBuffer();
    ANGLE_TRY(vkFBO->beginRenderPass(device, commandBuffer, state));

    commandBuffer->bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, mCurrentPipeline);
    commandBuffer->bindVertexBuffers(0, vertexHandles, vertexOffsets);
    commandBuffer->draw(count, instanceCount, first, 0);
    commandBuffer->endRenderPass();
    ANGLE_TRY(commandBuffer->end());

    ANGLE_TRY(mRenderer->submitAndFinishCommandBuffer(*commandBuffer, 1000));
    ANGLE_TRY(vkFBO->onDrawn());

    for (auto buf : interimBuffers)
        delete buf;

    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error ContextVk::drawElements(GLenum mode,
                                  GLsizei count,
                                  GLenum type,
                                  const GLvoid *indices,
                                  const gl::IndexRange &indexRange)
{
    return drawElementsInstanced(mode, count, type, indices, 1, indexRange);
}

gl::Error ContextVk::drawElementsInstanced(GLenum mode,
                                           GLsizei count,
                                           GLenum type,
                                           const GLvoid *indices,
                                           GLsizei instanceCount,
                                           const gl::IndexRange &indexRange)
{
    VkDevice device      = mRenderer->getDevice();
    const auto &state    = mState.getState();
    const auto *drawFBO  = state.getDrawFramebuffer();
    FramebufferVk *vkFBO = GetImplAs<FramebufferVk>(drawFBO);

    // Process vertex attributes
    // TODO(jmadill): Caching with dirty bits.
    std::vector<VkVertexInputBindingDescription> vertexBindings;
    std::vector<VkVertexInputAttributeDescription> vertexAttribs;
    std::vector<VkBuffer> vertexHandles;
    std::vector<VkDeviceSize> vertexOffsets;
    std::vector<BufferVk *> interimBuffers;
    gl::BufferState dummyState;

    ANGLE_TRY(prepareVertexInput(indexRange.end + 1, instanceCount, dummyState, &vertexBindings,
                                 &vertexAttribs, &vertexHandles, &vertexOffsets, &interimBuffers));

    BufferVk *indexBuffer;
    VkDeviceSize indexOffset;
    VkIndexType indexType;
    bool newBufferCreated;
    ANGLE_TRY(prepareIndexInput(count, type, indices, dummyState, &indexBuffer, &indexOffset,
                                &indexType, &newBufferCreated));

    // TODO(jmadill): Dynamic state.
    vk::RenderPass *renderPass = nullptr;
    ANGLE_TRY_RESULT(vkFBO->getRenderPass(device), renderPass);
    ASSERT(renderPass && renderPass->valid());

    ANGLE_TRY(preparePipeline(mode, vertexBindings, vertexAttribs, renderPass));

    vk::CommandBuffer *commandBuffer = mRenderer->getCommandBuffer();
    ANGLE_TRY(vkFBO->beginRenderPass(device, commandBuffer, state));

    commandBuffer->bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, mCurrentPipeline);
    commandBuffer->bindVertexBuffers(0, vertexHandles, vertexOffsets);
    commandBuffer->bindIndexBuffer(indexBuffer->getVkBuffer().getHandle(), indexOffset, indexType);
    commandBuffer->drawIndexed(count, instanceCount, 0, 0, 0);
    commandBuffer->endRenderPass();
    ANGLE_TRY(commandBuffer->end());

    ANGLE_TRY(mRenderer->submitAndFinishCommandBuffer(*commandBuffer, 1000));
    ANGLE_TRY(vkFBO->onDrawn());

    for (auto buf : interimBuffers)
        delete buf;
    if (newBufferCreated)
        delete indexBuffer;

    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error ContextVk::drawRangeElements(GLenum mode,
                                       GLuint start,
                                       GLuint end,
                                       GLsizei count,
                                       GLenum type,
                                       const GLvoid *indices,
                                       const gl::IndexRange &indexRange)
{
    return gl::NoError();
}

VkDevice ContextVk::getDevice() const
{
    return mRenderer->getDevice();
}

vk::CommandBuffer *ContextVk::getCommandBuffer()
{
    return mRenderer->getCommandBuffer();
}

vk::Error ContextVk::submitCommands(const vk::CommandBuffer &commandBuffer)
{
    // FIXME(jmadill): timeout? or semaphore
    ANGLE_TRY(mRenderer->submitAndFinishCommandBuffer(commandBuffer, 1000u));
    return vk::NoError();
}

gl::Error ContextVk::drawArraysIndirect(GLenum mode, const GLvoid *indirect)
{
    UNIMPLEMENTED();
    return gl::InternalError() << "DrawArraysIndirect hasn't been implemented for vulkan backend.";
}

gl::Error ContextVk::drawElementsIndirect(GLenum mode, GLenum type, const GLvoid *indirect)
{
    UNIMPLEMENTED();
    return gl::InternalError()
           << "DrawElementsIndirect hasn't been implemented for vulkan backend.";
}

GLenum ContextVk::getResetStatus()
{
    UNIMPLEMENTED();
    return GL_NO_ERROR;
}

std::string ContextVk::getVendorString() const
{
    UNIMPLEMENTED();
    return std::string();
}

std::string ContextVk::getRendererDescription() const
{
    return mRenderer->getRendererDescription();
}

void ContextVk::insertEventMarker(GLsizei length, const char *marker)
{
    UNIMPLEMENTED();
}

void ContextVk::pushGroupMarker(GLsizei length, const char *marker)
{
    UNIMPLEMENTED();
}

void ContextVk::popGroupMarker()
{
    UNIMPLEMENTED();
}

void ContextVk::syncState(const gl::State & /*state*/, const gl::State::DirtyBits & /*dirtyBits*/)
{
    // TODO(jmadill): Vulkan dirty bits.
}

GLint ContextVk::getGPUDisjoint()
{
    UNIMPLEMENTED();
    return GLint();
}

GLint64 ContextVk::getTimestamp()
{
    UNIMPLEMENTED();
    return GLint64();
}

void ContextVk::onMakeCurrent(const gl::ContextState & /*data*/)
{
}

const gl::Caps &ContextVk::getNativeCaps() const
{
    return mRenderer->getNativeCaps();
}

const gl::TextureCapsMap &ContextVk::getNativeTextureCaps() const
{
    return mRenderer->getNativeTextureCaps();
}

const gl::Extensions &ContextVk::getNativeExtensions() const
{
    return mRenderer->getNativeExtensions();
}

const gl::Limitations &ContextVk::getNativeLimitations() const
{
    return mRenderer->getNativeLimitations();
}

CompilerImpl *ContextVk::createCompiler()
{
    return new CompilerVk();
}

ShaderImpl *ContextVk::createShader(const gl::ShaderState &state)
{
    return new ShaderVk(state);
}

ProgramImpl *ContextVk::createProgram(const gl::ProgramState &state)
{
    return new ProgramVk(state);
}

FramebufferImpl *ContextVk::createFramebuffer(const gl::FramebufferState &state)
{
    return FramebufferVk::CreateUserFBO(state);
}

TextureImpl *ContextVk::createTexture(const gl::TextureState &state)
{
    return new TextureVk(state);
}

RenderbufferImpl *ContextVk::createRenderbuffer()
{
    return new RenderbufferVk();
}

BufferImpl *ContextVk::createBuffer(const gl::BufferState &state)
{
    return new BufferVk(state);
}

VertexArrayImpl *ContextVk::createVertexArray(const gl::VertexArrayState &state)
{
    return new VertexArrayVk(state);
}

QueryImpl *ContextVk::createQuery(GLenum type)
{
    return new QueryVk(type);
}

FenceNVImpl *ContextVk::createFenceNV()
{
    return new FenceNVVk();
}

FenceSyncImpl *ContextVk::createFenceSync()
{
    return new FenceSyncVk();
}

TransformFeedbackImpl *ContextVk::createTransformFeedback(const gl::TransformFeedbackState &state)
{
    return new TransformFeedbackVk(state);
}

SamplerImpl *ContextVk::createSampler()
{
    return new SamplerVk();
}

std::vector<PathImpl *> ContextVk::createPaths(GLsizei)
{
    return std::vector<PathImpl *>();
}

}  // namespace rx
