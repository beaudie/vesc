//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CommandGraph:
//    Deferred work constructed by GL calls, that will later be flushed to Vulkan.
//

#ifndef LIBANGLE_RENDERER_VULKAN_COMMAND_GRAPH_H_
#define LIBANGLE_RENDERER_VULKAN_COMMAND_GRAPH_H_

#include "libANGLE/renderer/vulkan/vk_cache_utils.h"

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

// A command graph node contains some work to be done in Vulkan. The work is stored into
// secondary vk::CommandBuffers that we then compose together with vk::RenderPasses as necessary
// when we need to submit the work. We submit on a read operation - such as a SwapBuffers call
// or ReadPixels.
class CommandGraphNode final : angle::NonCopyable
{
  public:
    CommandGraphNode();
    ~CommandGraphNode();

    // Immutable queries for when we're walking the commands tree.
    CommandBuffer *getOutsideRenderPassCommands();
    CommandBuffer *getInsideRenderPassCommands();

    // For outside the render pass (copies, transitions, etc).
    Error beginOutsideRenderPassRecording(VkDevice device,
                                          const CommandPool &commandPool,
                                          CommandBuffer **commandsOut);

    // For rendering commands (draws).
    Error beginInsideRenderPassRecording(RendererVk *renderer, CommandBuffer **commandsOut);

    // storeRenderPassInfo and append*RenderTarget store info relevant to the RenderPass.
    void storeRenderPassInfo(const Framebuffer &framebuffer,
                             const gl::Rectangle renderArea,
                             const std::vector<VkClearValue> &clearValues);

    // storeRenderPassInfo and append*RenderTarget store info relevant to the RenderPass.
    // Note: RenderTargets must be added in order, with the depth/stencil being added last.
    void appendColorRenderTarget(Serial serial, RenderTargetVk *colorRenderTarget);
    void appendDepthStencilRenderTarget(Serial serial, RenderTargetVk *depthStencilRenderTarget);

    // Dependency commands order node execution in the command graph.
    // Once a node has commands that must happen after it, recording is stopped and the node is
    // frozen forever.
    static void SetHappensBeforeDependency(CommandGraphNode *beforeNode,
                                           CommandGraphNode *afterNode);
    static void SetHappensBeforeDependencies(const std::vector<CommandGraphNode *> &beforeNodes,
                                             CommandGraphNode *afterNode);
    bool hasParents() const;
    bool hasChildren() const;

    // Commands for traversing the node on a flush operation.
    VisitedState visitedState() const;
    void visitParents(std::vector<CommandGraphNode *> *stack);
    Error visitAndExecute(VkDevice device,
                          Serial serial,
                          RenderPassCache *renderPassCache,
                          CommandBuffer *primaryCommandBuffer);

  private:
    void setHasChildren();

    // Used for testing only.
    bool isChildOf(CommandGraphNode *parent);

    // Only used if we need a RenderPass for these commands.
    RenderPassDesc mRenderPassDesc;
    Framebuffer mRenderPassFramebuffer;
    gl::Rectangle mRenderPassRenderArea;
    gl::AttachmentArray<VkClearValue> mRenderPassClearValues;

    // Keep a separate buffers for commands inside and outside a RenderPass.
    // TODO(jmadill): We might not need inside and outside RenderPass commands separate.
    CommandBuffer mOutsideRenderPassCommands;
    CommandBuffer mInsideRenderPassCommands;

    // Parents are commands that must be submitted before 'this' CommandNode can be submitted.
    std::vector<CommandGraphNode *> mParents;

    // If this is true, other commands exist that must be submitted after 'this' command.
    bool mHasChildren;

    // Used when traversing the dependency graph.
    VisitedState mVisitedState;
};

// The Command Graph consists of an array of open Command Graph Nodes. It supports allocating new
// nodes for the graph, which are linked via dependency relation calls in CommandGraphNode, and
// also submitting the whole command graph via submitCommands.
class CommandGraph final : angle::NonCopyable
{
  public:
    CommandGraph();
    ~CommandGraph();

    // Allocates a new CommandGraphNode and adds it to the list of current open nodes. No ordering
    // relations exist in the node by default. Call CommandGraphNode::SetHappensBeforeDependency
    // to set up dependency relations.
    CommandGraphNode *allocateNode();

    Error submitCommands(VkDevice device,
                         Serial serial,
                         RenderPassCache *renderPassCache,
                         CommandPool *commandPool,
                         CommandBuffer *primaryCommandBufferOut);
    bool empty() const;

  private:
    std::vector<CommandGraphNode *> mNodes;
};

}  // namespace vk
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_COMMAND_GRAPH_H_
