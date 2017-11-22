//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CommandBufferNode:
//    Deferred work constructed by GL calls, that will later be flushed to Vulkan.
//

#include "libANGLE/renderer/vulkan/CommandBufferNode.h"

#include "libANGLE/renderer/vulkan/RenderTargetVk.h"
#include "libANGLE/renderer/vulkan/formatutilsvk.h"

namespace rx
{

namespace vk
{

namespace
{

Error InitAndBeginCommandBuffer(VkDevice device,
                                const CommandPool &commandPool,
                                const VkCommandBufferInheritanceInfo &inheritanceInfo,
                                CommandBuffer *commandBuffer)
{
    ASSERT(!commandBuffer->valid());

    VkCommandBufferAllocateInfo createInfo;
    createInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    createInfo.pNext              = nullptr;
    createInfo.commandPool        = commandPool.getHandle();
    createInfo.level              = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    createInfo.commandBufferCount = 1;

    ANGLE_TRY(commandBuffer->init(device, createInfo));

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext            = nullptr;
    beginInfo.flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = &inheritanceInfo;

    ANGLE_TRY(commandBuffer->begin(beginInfo));
    return NoError();
}

}  // anonymous namespace

// CommandBufferNode implementation.

CommandBufferNode::CommandBufferNode() : mHasBeenFlushed(false)
{
}

CommandBufferNode::~CommandBufferNode()
{
    mRenderPassFramebuffer.setHandle(VK_NULL_HANDLE);

    // Command buffers are managed by the command pool, so don't need to be freed.
    mOutsideRenderPassCommands.releaseHandle();
    mInsideRenderPassCommands.releaseHandle();
}

CommandBuffer *CommandBufferNode::getOutsideRenderPassCommands()
{
    return &mOutsideRenderPassCommands;
}

CommandBuffer *CommandBufferNode::getInsideRenderPassCommands()
{
    return &mInsideRenderPassCommands;
}

Error CommandBufferNode::startRecording(VkDevice device,
                                        const CommandPool &commandPool,
                                        CommandBuffer **commandsOut)
{
    VkCommandBufferInheritanceInfo inheritanceInfo;
    inheritanceInfo.sType                = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.pNext                = nullptr;
    inheritanceInfo.renderPass           = VK_NULL_HANDLE;
    inheritanceInfo.subpass              = 0;
    inheritanceInfo.framebuffer          = VK_NULL_HANDLE;
    inheritanceInfo.occlusionQueryEnable = VK_FALSE;
    inheritanceInfo.queryFlags           = 0;
    inheritanceInfo.pipelineStatistics   = 0;

    ANGLE_TRY(InitAndBeginCommandBuffer(device, commandPool, inheritanceInfo,
                                        &mOutsideRenderPassCommands));

    *commandsOut = &mOutsideRenderPassCommands;
    return NoError();
}

Error CommandBufferNode::startRenderPassRecording(VkDevice device,
                                                  const CommandPool &commandPool,
                                                  const RenderPass &compatibleRenderPass,
                                                  CommandBuffer **commandsOut)
{
    VkCommandBufferInheritanceInfo inheritanceInfo;
    inheritanceInfo.sType                = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.pNext                = nullptr;
    inheritanceInfo.renderPass           = compatibleRenderPass.getHandle();
    inheritanceInfo.subpass              = 0;
    inheritanceInfo.framebuffer          = mRenderPassFramebuffer.getHandle();
    inheritanceInfo.occlusionQueryEnable = VK_FALSE;
    inheritanceInfo.queryFlags           = 0;
    inheritanceInfo.pipelineStatistics   = 0;

    ANGLE_TRY(InitAndBeginCommandBuffer(device, commandPool, inheritanceInfo,
                                        &mInsideRenderPassCommands));

    *commandsOut = &mInsideRenderPassCommands;
    return NoError();
}

void CommandBufferNode::storeRenderPassFramebuffer(const Framebuffer &framebuffer,
                                                   const gl::Rectangle renderArea)
{
    mRenderPassFramebuffer.setHandle(framebuffer.getHandle());
    mRenderPassRenderArea = renderArea;
}

void CommandBufferNode::storeRenderPassClearValues(const std::vector<VkClearValue> &clearValues)
{
    std::copy(clearValues.begin(), clearValues.end(), mRenderPassClearValues.begin());
}

void CommandBufferNode::appendColorRenderTarget(const gl::Context *context,
                                                RenderTargetVk *colorRenderTarget)
{
    // TODO(jmadill): Layout transition?
    VkAttachmentDescription *desc = mRenderPassDesc.nextColorAttachment();
    initAttachmentDesc(desc);
    desc->format      = colorRenderTarget->format->vkTextureFormat;
    desc->samples     = colorRenderTarget->samples;
    desc->finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    colorRenderTarget->resource->chainNewCommands(context, this);
}

void CommandBufferNode::appendDepthStencilRenderTarget(const gl::Context *context,
                                                       RenderTargetVk *depthStencilRenderTarget)
{
    // TODO(jmadill): Layout transition?
    VkAttachmentDescription *desc = mRenderPassDesc.nextDepthStencilAttachment();
    initAttachmentDesc(desc);
    desc->format      = depthStencilRenderTarget->format->vkTextureFormat;
    desc->samples     = depthStencilRenderTarget->samples;
    desc->finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    depthStencilRenderTarget->resource->chainNewCommands(context, this);
}

void CommandBufferNode::initAttachmentDesc(VkAttachmentDescription *desc)
{
    desc->flags          = 0;
    desc->format         = VK_FORMAT_UNDEFINED;
    desc->samples        = static_cast<VkSampleCountFlagBits>(0);
    desc->loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    desc->storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    desc->stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    desc->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    desc->initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    desc->finalLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
}

bool CommandBufferNode::usesRenderPass() const
{
    return mInsideRenderPassCommands.valid();
}

const Framebuffer &CommandBufferNode::getRenderPassFramebuffer() const
{
    return mRenderPassFramebuffer;
}

const gl::Rectangle &CommandBufferNode::getRenderPassRenderArea() const
{
    return mRenderPassRenderArea;
}

const gl::AttachmentArray<VkClearValue> &CommandBufferNode::getRenderPassClearValues() const
{
    return mRenderPassClearValues;
}

void CommandBufferNode::addBeforeDependency(const gl::Context *context,
                                            CommandBufferNode *beforeCommands)
{
    mBeforeDependencies.emplace_back(beforeCommands);
}

void CommandBufferNode::setAfterDependency(const gl::Context *context,
                                           CommandBufferNode *afterCommands)
{
    ASSERT(!mAfterDependency.valid());
    mAfterDependency.set(context, afterCommands);
}

bool CommandBufferNode::hasBeenFlushed() const
{
    return mHasBeenFlushed;
}

void CommandBufferNode::markFlushedAndReleaseDependencies(const gl::Context *context)
{
    ASSERT(!mHasBeenFlushed);
    mHasBeenFlushed = true;

    for (auto &beforeDependency : mBeforeDependencies)
    {
        beforeDependency.set(context, nullptr);
    }
    mBeforeDependencies.clear();

    mAfterDependency.set(context, nullptr);
}

bool CommandBufferNode::addBeforeDependenciesToStack(std::vector<CommandBufferNode *> *stack)
{
    for (auto &beforeDependency : mBeforeDependencies)
    {
        stack->push_back(beforeDependency.get());
    }

    return !mBeforeDependencies.empty();
}

const RenderPassDesc &CommandBufferNode::getRenderPassDesc() const
{
    return mRenderPassDesc;
}
}  // namespace vk
}  // namespace rx
