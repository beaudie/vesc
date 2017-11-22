//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CommandBufferNode:
//    Deferred work constructed by GL calls, that will later be flushed to Vulkan.
//

#ifndef LIBANGLE_RENDERER_VULKAN_COMMAND_BUFFER_NODE_H_
#define LIBANGLE_RENDERER_VULKAN_COMMAND_BUFFER_NODE_H_

#include "libANGLE/renderer/vulkan/renderervk_utils.h"

namespace rx
{

namespace vk
{

enum class VisitedState
{
    Unvisited,
    Ready,
    Visited,
};

// TODO(jmadill): Replace ref counting with pool allocation.
class CommandBufferNode final : angle::NonCopyable
{
  public:
    CommandBufferNode();
    ~CommandBufferNode();

    // Immutable queries for when we're walking the commands tree.
    CommandBuffer *getOutsideRenderPassCommands();
    CommandBuffer *getInsideRenderPassCommands();

    // For outside the render pass (copies, transitions, etc).
    vk::Error startRecording(VkDevice device,
                             const vk::CommandPool &commandPool,
                             CommandBuffer **commandsOut);

    // For rendering commands (draws).
    vk::Error startRenderPassRecording(VkDevice device,
                                       const vk::CommandPool &commandPool,
                                       const vk::RenderPass &compatibleRenderPass,
                                       CommandBuffer **commandsOut);

    // Commands for storing info relevant to the RenderPass.
    // RenderTargets must be added in order, with the depth/stencil being added last.
    void storeRenderPassFramebuffer(const vk::Framebuffer &framebuffer,
                                    const gl::Rectangle renderArea);
    void storeRenderPassClearValues(const std::vector<VkClearValue> &clearValues);
    void appendColorRenderTarget(RenderTargetVk *colorRenderTarget);
    void appendDepthStencilRenderTarget(RenderTargetVk *depthStencilRenderTarget);

    // Commands for querying RenderPass related info.
    const RenderPassDesc &getRenderPassDesc() const;

    // Commands for chaining nodes in the dependency graph.
    void addBeforeDependency(CommandBufferNode *beforeCommands);
    void setAfterDependency(CommandBufferNode *afterCommands);
    bool hasBeforeDependencies() const;
    bool hasAfterDependency() const;

    // Commands for traversing the node on a flush operation.
    VisitedState visitedState() const;
    void visitBeforeDependencies(std::vector<CommandBufferNode *> *stack);
    vk::Error visitAndExecute(RendererVk *renderer, vk::CommandBuffer *primaryCommandBuffer);

  private:
    void initAttachmentDesc(VkAttachmentDescription *desc);

    // Only used if we need a RenderPass for these commands.
    RenderPassDesc mRenderPassDesc;
    vk::Framebuffer mRenderPassFramebuffer;
    gl::Rectangle mRenderPassRenderArea;
    gl::AttachmentArray<VkClearValue> mRenderPassClearValues;
    // Keep a separate buffers for commands inside and outside a RenderPass.
    // TODO(jmadill): We might not need inside and outside RenderPass commands separate.
    vk::CommandBuffer mOutsideRenderPassCommands;
    vk::CommandBuffer mInsideRenderPassCommands;
    std::vector<CommandBufferNode *> mBeforeDependencies;
    CommandBufferNode *mAfterDependency;
    VisitedState mVisitedState;
};
}  // namespace vk
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_COMMAND_BUFFER_NODE_H_
