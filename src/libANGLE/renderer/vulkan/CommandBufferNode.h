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

// TODO(jmadill): Replace ref counting with pool allocation.
class CommandBufferNode final : public gl::RefCountObjectNoID
{
  public:
    CommandBufferNode();
    ~CommandBufferNode();

    // Immutable queries for when we're walking the commands tree.
    const CommandBufferAndState &getOutsideRenderPassCommands() const;
    const CommandBufferAndState &getInsideRenderPassCommands() const;

    // For outside the render pass (copies, transitions, etc).
    vk::Error startRecording(VkDevice device,
                             const vk::CommandPool &commandPool,
                             CommandBufferAndState **commandsOut);

    // For rendering commands (draws).
    vk::Error startRenderPassRecording(VkDevice device,
                                       const vk::CommandPool &commandPool,
                                       CommandBufferAndState **commandsOut);

    // Commands for storing info relevant to the RenderPass.
    // RenderTargets must be added in order, with the depth/stencil being added last.
    void storeRenderPassFramebuffer(const vk::Framebuffer &framebuffer,
                                    const gl::Rectangle renderArea);
    void storeRenderPassClearValues(const std::vector<VkClearValue> &clearValues);
    void appendColorRenderTarget(const gl::Context *context, RenderTargetVk *colorRenderTarget);
    void appendDepthStencilRenderTarget(const gl::Context *context,
                                        RenderTargetVk *depthStencilRenderTarget);

    // Commands for querying RenderPass related info.
    bool usesRenderPass() const;
    const RenderPassDesc &getRenderPassDesc() const;
    const vk::Framebuffer &getRenderPassFramebuffer() const;
    const gl::Rectangle &getRenderPassRenderArea() const;
    const gl::AttachmentArray<VkClearValue> &getRenderPassClearValues() const;

    // Commands for chaining nodes in the dependency graph.
    void addBeforeDependency(const gl::Context *context, CommandBufferNode *beforeCommands);
    void setAfterDependency(const gl::Context *context, CommandBufferNode *afterCommands);

    // Commands for traversing the node on a flush operation.
    bool hasBeenFlushed() const;
    void markFlushedAndReleaseDependencies(const gl::Context *context);

    // Returns true if there were any before dependencies.
    bool addBeforeDependenciesToStack(std::vector<CommandBufferNode *> *stack);

  private:
    void initAttachmentDesc(VkAttachmentDescription *desc);

    // Only used if we need a RenderPass for these commands.
    RenderPassDesc mRenderPassDesc;
    vk::Framebuffer mRenderPassFramebuffer;
    gl::Rectangle mRenderPassRenderArea;
    gl::AttachmentArray<VkClearValue> mRenderPassClearValues;
    vk::CommandBufferAndState mOutsideRenderPassCommands;
    vk::CommandBufferAndState mInsideRenderPassCommands;
    std::vector<gl::BindingPointer<CommandBufferNode>> mBeforeDependencies;
    gl::BindingPointer<CommandBufferNode> mAfterDependency;
    bool mHasBeenFlushed;
};
}  // namespace vk
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_COMMAND_BUFFER_NODE_H_
