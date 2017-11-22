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
#include "libANGLE/renderer/vulkan/RendererVk.h"
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

CommandBufferNode::CommandBufferNode()
    : mAfterDependency(nullptr), mVisitedState(VisitedState::Unvisited)
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

void CommandBufferNode::appendColorRenderTarget(RenderTargetVk *colorRenderTarget)
{
    // TODO(jmadill): Layout transition?
    VkAttachmentDescription *desc = mRenderPassDesc.nextColorAttachment();
    initAttachmentDesc(desc);
    desc->format      = colorRenderTarget->format->vkTextureFormat;
    desc->samples     = colorRenderTarget->samples;
    desc->finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    colorRenderTarget->resource->chainNewCommands(this);
}

void CommandBufferNode::appendDepthStencilRenderTarget(RenderTargetVk *depthStencilRenderTarget)
{
    // TODO(jmadill): Layout transition?
    VkAttachmentDescription *desc = mRenderPassDesc.nextDepthStencilAttachment();
    initAttachmentDesc(desc);
    desc->format      = depthStencilRenderTarget->format->vkTextureFormat;
    desc->samples     = depthStencilRenderTarget->samples;
    desc->finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    depthStencilRenderTarget->resource->chainNewCommands(this);
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

const RenderPassDesc &CommandBufferNode::getRenderPassDesc() const
{
    return mRenderPassDesc;
}

void CommandBufferNode::addBeforeDependency(CommandBufferNode *beforeCommands)
{
    mBeforeDependencies.emplace_back(beforeCommands);
}

void CommandBufferNode::setAfterDependency(CommandBufferNode *afterCommands)
{
    ASSERT(mAfterDependency == nullptr);
    mAfterDependency = afterCommands;
}

bool CommandBufferNode::hasBeforeDependencies() const
{
    return !mBeforeDependencies.empty();
}

bool CommandBufferNode::hasAfterDependency() const
{
    return (mAfterDependency != nullptr);
}

VisitedState CommandBufferNode::visitedState() const
{
    return mVisitedState;
}

void CommandBufferNode::visitBeforeDependencies(std::vector<CommandBufferNode *> *stack)
{
    ASSERT(mVisitedState == VisitedState::Unvisited);
    stack->insert(stack->end(), mBeforeDependencies.begin(), mBeforeDependencies.end());
    mVisitedState = VisitedState::Ready;
}

vk::Error CommandBufferNode::visitAndExecute(RendererVk *renderer,
                                             vk::CommandBuffer *primaryCommandBuffer)
{
    if (mOutsideRenderPassCommands.valid())
    {
        mOutsideRenderPassCommands.end();
        primaryCommandBuffer->executeCommands(1, &mOutsideRenderPassCommands);
    }

    if (mInsideRenderPassCommands.valid())
    {
        // Pull a compatible RenderPass from the cache.
        vk::RenderPass *renderPass = nullptr;
        ANGLE_TRY(renderer->getRenderPass(mRenderPassDesc, &renderPass));

        mInsideRenderPassCommands.end();

        primaryCommandBuffer->beginRenderPass(
            *renderPass, mRenderPassFramebuffer, mRenderPassRenderArea,
            mRenderPassDesc.attachmentCount(), mRenderPassClearValues.data());
        primaryCommandBuffer->executeCommands(1, &mInsideRenderPassCommands);
        primaryCommandBuffer->endRenderPass();
    }

    mVisitedState = VisitedState::Visited;
    return vk::NoError();
}

}  // namespace vk
}  // namespace rx
