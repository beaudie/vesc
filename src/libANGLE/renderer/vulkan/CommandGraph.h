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

#include "libANGLE/renderer/vulkan/SecondaryCommandBuffer.h"
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

enum class CommandGraphResourceType
{
    Buffer,
    Framebuffer,
    Image,
    Query,
    FenceSync,
    DebugMarker,
};

// Certain functionality cannot be put in secondary command buffers, so they are special-cased in
// the node.
enum class CommandGraphNodeFunction
{
    Generic,
    BeginQuery,
    EndQuery,
    WriteTimestamp,
    SetFenceSync,
    WaitFenceSync,
    InsertDebugMarker,
    PushDebugMarker,
    PopDebugMarker,
};

// Receives notifications when a command buffer is no longer able to record. Can be used with
// inheritance. Faster than using an interface class since it has inlined methods. Could be used
// with composition by adding a getCommandBuffer method.
class CommandBufferOwner
{
  public:
    CommandBufferOwner() = default;
    virtual ~CommandBufferOwner() {}

    ANGLE_INLINE void onCommandBufferFinished() { mCommandBuffer = nullptr; }

  protected:
    CommandBuffer *mCommandBuffer = nullptr;
};

// Only used internally in the command graph. Kept in the header for better inlining performance.
class CommandGraphNode final : angle::NonCopyable
{
  public:
    CommandGraphNode(CommandGraphNodeFunction function, angle::PoolAllocator *poolAllocator);
    ~CommandGraphNode();

    // Immutable queries for when we're walking the commands tree.
    CommandBuffer *getOutsideRenderPassCommands()
    {
        ASSERT(!hasChild());
        return &mOutsideRenderPassCommands;
    }

    CommandBuffer *getInsideRenderPassCommands()
    {
        ASSERT(!hasChild());
        return &mInsideRenderPassCommands;
    }

    // For outside the render pass (copies, transitions, etc).
    angle::Result beginOutsideRenderPassRecording(ContextVk *context,
                                                  const CommandPool &commandPool,
                                                  CommandBuffer **commandsOut);

    // For rendering commands (draws).
    angle::Result beginInsideRenderPassRecording(ContextVk *context, CommandBuffer **commandsOut);

    // storeRenderPassInfo and append*RenderTarget store info relevant to the RenderPass.
    void storeRenderPassInfo(const Framebuffer &framebuffer,
                             const gl::Rectangle renderArea,
                             const vk::RenderPassDesc &renderPassDesc,
                             const AttachmentOpsArray &renderPassAttachmentOps,
                             const std::vector<VkClearValue> &clearValues);
    const Framebuffer &getRenderPassFramebuffer() const { return mRenderPassFramebuffer; }

    void clearRenderPassColorAttachment(size_t attachmentIndex, const VkClearColorValue &clearValue)
    {
        mRenderPassAttachmentOps[attachmentIndex].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        mRenderPassClearValues[attachmentIndex].color    = clearValue;
    }

    void clearRenderPassDepthAttachment(size_t attachmentIndex, float depth)
    {
        mRenderPassAttachmentOps[attachmentIndex].loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
        mRenderPassClearValues[attachmentIndex].depthStencil.depth = depth;
    }

    void clearRenderPassStencilAttachment(size_t attachmentIndex, uint32_t stencil)
    {
        mRenderPassAttachmentOps[attachmentIndex].stencilLoadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
        mRenderPassClearValues[attachmentIndex].depthStencil.stencil = stencil;
    }

    // Dependency commands order node execution in the command graph.
    // Once a node has commands that must happen after it, recording is stopped and the node is
    // frozen forever.
    static void SetHappensBeforeDependency(CommandGraphNode *beforeNode,
                                           CommandGraphNode *afterNode)
    {
        ASSERT(beforeNode != afterNode && !beforeNode->isChildOf(afterNode));
        ASSERT(beforeNode->mFunction != CommandGraphNodeFunction::Generic ||
               beforeNode->mDescendant == nullptr);
        afterNode->mParents.emplace_back(beforeNode);
        beforeNode->mDescendant = afterNode;
        beforeNode->onSetChild();
    }

    // Specialized versions of SetHappensBeforeDependency where one of the nodes is a barrier node.
    // To avoid having a barrier node serialize the rest of the commands (by unifying the disjoint
    // sets of nodes), it does not participate in the descendant chain, and only affects the graph
    // to constrain the traversal.
    static void SetHappensBeforeBarrierDependency(CommandGraphNode *beforeNode,
                                                  CommandGraphNode *barrierNode)
    {
        ASSERT(beforeNode != barrierNode && !beforeNode->isChildOf(barrierNode));
        ASSERT(!beforeNode->hasChild());
        // Make sure nothing gets appended to the node.
        beforeNode->mForceNewNode = true;
        beforeNode->forceNewRenderPass();
        // Make sure this node is traversed before the barrier node.
        barrierNode->mParents.emplace_back(beforeNode);
    }

    static void SetHappensAfterBarrierDependency(CommandGraphNode *barrierNode,
                                                 CommandGraphNode *afterNode)
    {
        ASSERT(barrierNode != afterNode && !barrierNode->isChildOf(afterNode));
        // Make sure this node is traversed after the barrier node.
        afterNode->mParents.emplace_back(barrierNode);
    }

    CommandGraphNode *getDescendant();

    bool hasParents() const;
    bool hasChild() const { return mDescendant != nullptr || mForceNewNode; }

    // Commands for traversing the node on a flush operation.
    VisitedState visitedState() const;
    void visitParents(std::vector<CommandGraphNode *> *stack);
    angle::Result visitAndExecute(Context *context,
                                  Serial serial,
                                  RenderPassCache *renderPassCache,
                                  PrimaryCommandBuffer *primaryCommandBuffer);

    // Only used in the command graph diagnostics.
    const std::vector<CommandGraphNode *> &getParentsForDiagnostics() const;
    void setDiagnosticInfo(CommandGraphResourceType resourceType, uintptr_t resourceID);

    CommandGraphResourceType getResourceTypeForDiagnostics() const { return mResourceType; }
    uintptr_t getResourceIDForDiagnostics() const { return mResourceID; }
    std::string dumpCommandsForDiagnostics(const char *separator) const;

    const gl::Rectangle &getRenderPassRenderArea() const { return mRenderPassRenderArea; }

    CommandGraphNodeFunction getFunction() const { return mFunction; }

    void setQueryPool(const QueryPool *queryPool, uint32_t queryIndex);
    VkQueryPool getQueryPool() const { return mQueryPool; }
    uint32_t getQueryIndex() const { return mQueryIndex; }
    void setFenceSync(const vk::Event &event);
    void setDebugMarker(GLenum source, std::string &&marker);
    const std::string &getDebugMarker() const { return mDebugMarker; }

    ANGLE_INLINE void addGlobalMemoryBarrier(VkFlags srcAccess, VkFlags dstAccess)
    {
        mGlobalMemoryBarrierSrcAccess |= srcAccess;
        mGlobalMemoryBarrierDstAccess |= dstAccess;
    }

    void forceNewRenderPass()
    {
        // Invalidate the owner's command buffer so it would create a new render pass.
        invalidateOwnerCommandBuffer();
    }

    // This can only be set for RenderPass nodes. Each RenderPass node can have at most one owner.
    void setCommandBufferOwner(CommandBufferOwner *owner)
    {
        ASSERT(mCommandBufferOwner == nullptr);
        mCommandBufferOwner = owner;
    }

  private:
    ANGLE_INLINE void invalidateOwnerCommandBuffer()
    {
        if (mCommandBufferOwner)
        {
            mCommandBufferOwner->onCommandBufferFinished();
        }
    }

    ANGLE_INLINE void onSetChild() { invalidateOwnerCommandBuffer(); }

    // Used for testing only.
    bool isChildOf(CommandGraphNode *parent);

    // Only used if we need a RenderPass for these commands.
    RenderPassDesc mRenderPassDesc;
    AttachmentOpsArray mRenderPassAttachmentOps;
    Framebuffer mRenderPassFramebuffer;
    gl::Rectangle mRenderPassRenderArea;
    gl::AttachmentArray<VkClearValue> mRenderPassClearValues;

    bool mForceNewNode;

    CommandGraphNodeFunction mFunction;
    angle::PoolAllocator *mPoolAllocator;
    // Keep separate buffers for commands inside and outside a RenderPass.
    // TODO(jmadill): We might not need inside and outside RenderPass commands separate.
    CommandBuffer mOutsideRenderPassCommands;
    CommandBuffer mInsideRenderPassCommands;

    // Special-function additional data:
    // Queries:
    VkQueryPool mQueryPool;
    uint32_t mQueryIndex;
    // GLsync and EGLSync:
    VkEvent mFenceSyncEvent;
    // Debug markers:
    GLenum mDebugMarkerSource;
    std::string mDebugMarker;

    // Parents are commands that must be submitted before 'this' CommandNode can be submitted.
    std::vector<CommandGraphNode *> mParents;

    // When a dependency is created between two nodes, through B.addDependency(A), any future user
    // of A must add an edge to B's node, so it can be executed after B has finished using A (even
    // if all users are reads, because the image and buffer barriers set when recording the readers'
    // nodes assume same-order execution).  As a result, A is never going to have multiple children.
    //
    // This variable is set to whichever node is the child of this node when creating a dependency.
    // When a new user of this node appears, the dependency is instead created on the child.
    //
    // Note that if the child itself has a child, the dependency is created on the descendant with
    // no child.  This pointer is updated to the latest child-less descendant whenever queried.
    CommandGraphNode *mDescendant;

    // Used when traversing the dependency graph.
    VisitedState mVisitedState;

    // Additional diagnostic information.
    CommandGraphResourceType mResourceType;
    uintptr_t mResourceID;

    // For global memory barriers.
    VkFlags mGlobalMemoryBarrierSrcAccess;
    VkFlags mGlobalMemoryBarrierDstAccess;

    // Command buffer notifications.
    CommandBufferOwner *mCommandBufferOwner;
};

// This is a helper class for back-end objects used in Vk command buffers. It records a serial
// at command recording times indicating an order in the queue. We use Fences to detect when
// commands finish, and then release any unreferenced and deleted resources based on the stored
// queue serial in a special 'garbage' queue. Resources also track current read and write
// dependencies. Only one command buffer node can be writing to the Resource at a time, but many
// can be reading from it. Together the dependencies will form a command graph at submission time.
class CommandGraphResource : angle::NonCopyable
{
  public:
    virtual ~CommandGraphResource();

    // Returns true if the resource is in use by the renderer.
    bool isResourceInUse(ContextVk *context) const;

    // Get the current queue serial for this resource. Used to release resources, and for
    // queries, to know if the queue they are submitted on has finished execution.
    Serial getStoredQueueSerial() const { return mStoredQueueSerial; }

    // Sets up dependency relations.  |this| resource is the resource that's recording commands.
    // |used| is the resource being used in the commands, be it to read from or to write to.  May
    // create a new node for |this| if necessary.  As there would be a dependency from |used| to
    // |this|, any command buffer previously allocated from |used| should no longer be used.  The
    // usage pattern is:
    //
    //     A.recordCommands(&commandBufferA);
    //     A.render(commandBufferA);
    //
    //     B.addDependency(A)
    //
    //     B.recordCommands(&commandBufferB);
    //     B.render(commandBufferB);
    //
    // or:
    //
    //     B.addDependency(A)
    //
    //     B.recordCommands(&commandBufferB);
    //     A.render(commandBufferB);
    //     B.render(commandBufferB);
    //
    // Note again that after B.addDependency(A), both A's and B's previously allocated command
    // buffers may be invalid.  So, if B uses multiple resources, it would be best to set the
    // dependencies first, and render all at once instead of setting dependencies and re-acquiring
    // the command buffer after every addDependency():
    //
    //     B.addDependency(M)
    //     B.addDependency(N)
    //     B.addDependency(O)
    //     B.addDependency(P)
    //
    //     B.recordCommands(&commandBufferB);
    //     M.render(commandBufferB);
    //     N.render(commandBufferB);
    //     O.render(commandBufferB);
    //     P.render(commandBufferB);
    //     B.render(commandBufferB);
    //
    // The |addDependency| function effectively creates a weak execution ordering; all commands
    // previously recorded in A will be executed before all commands to be recorded in B.
    void addDependency(ContextVk *contextVk, CommandGraphResource *used);

    // The global memory barriers (for the sake of buffers) are executed at the beginning of the
    // node that uses the buffers.  If a node depends on multiple buffers, their memory barrier
    // access masks can be accumulated and a single memory barrier generated.
    //
    // If a resource accesses the same buffer in different ways, its graph node needs to break off
    // between each use so the memory barriers would be placed in between.  Setting the memory
    // barrier access masks is thus paired with setting dependency, so the graph node can be broken
    // off if necessary and the barrier set on the new node.
    void addDependencyAndMemoryBarrier(ContextVk *contextVk,
                                       CommandGraphResource *used,
                                       VkFlags srcAccess,
                                       VkFlags dstAccess);

    // Updates the in-use serial tracked for this resource. Will clear dependencies if the resource
    // was not used in this set of command nodes.
    ANGLE_INLINE void updateQueueSerial(Serial queueSerial)
    {
        ASSERT(queueSerial >= mStoredQueueSerial);

        if (queueSerial > mStoredQueueSerial)
        {
            mCurrentWritingNode = nullptr;
            mStoredQueueSerial = queueSerial;
        }
    }

    // Reset the current queue serial for this resource. Will clear dependencies if the resource
    // was not used in this set of command nodes.
    void resetQueueSerial();

    // Allocates a write node via getNewWriteNode and returns a started command buffer.
    // The started command buffer will render outside of a RenderPass.
    // Will append to an existing command buffer/graph node if possible.
    angle::Result recordCommands(ContextVk *contextVk, CommandBuffer **commandBufferOut);

    // Begins a command buffer on the current graph node for in-RenderPass rendering.
    // Called from FramebufferVk::startNewRenderPass and UtilsVk functions.
    angle::Result beginRenderPass(ContextVk *contextVk,
                                  const Framebuffer &framebuffer,
                                  const gl::Rectangle &renderArea,
                                  const RenderPassDesc &renderPassDesc,
                                  const AttachmentOpsArray &renderPassAttachmentOps,
                                  const std::vector<VkClearValue> &clearValues,
                                  CommandBuffer **commandBufferOut);

    // Checks if we're in a RenderPass without children.
    bool hasStartedRenderPass() const
    {
        return !mForceNewRenderPass && hasChildlessWritingNode() &&
               mCurrentWritingNode->getInsideRenderPassCommands()->valid();
    }

    // Checks if we're in a useable render pass for this particular framebuffer.  When modifying or
    // appending to the render pass of this node, we have to make sure it belongs to the correct
    // framebuffer.
    bool hasStartedFramebufferRenderPass(const Framebuffer &framebuffer) const
    {
        return hasStartedRenderPass() &&
               framebuffer.getHandle() ==
                   mCurrentWritingNode->getRenderPassFramebuffer().getHandle();
    }

    // Checks if we're in a RenderPass that encompasses renderArea, returning true if so. Updates
    // serial internally. Returns the started command buffer in commandBufferOut.
    ANGLE_INLINE bool appendToStartedRenderPass(Serial currentQueueSerial,
                                                const Framebuffer &framebuffer,
                                                const gl::Rectangle &renderArea,
                                                CommandBuffer **commandBufferOut)
    {
        updateQueueSerial(currentQueueSerial);
        if (hasStartedFramebufferRenderPass(framebuffer))
        {
            if (mCurrentWritingNode->getRenderPassRenderArea().encloses(renderArea))
            {
                *commandBufferOut = mCurrentWritingNode->getInsideRenderPassCommands();
                return true;
            }
        }

        return false;
    }

    // Returns true if the render pass is started, but there are no commands yet recorded in it.
    // This is useful to know if the render pass ops can be modified.
    bool renderPassStartedButEmpty(const Framebuffer &framebuffer) const
    {
        return hasStartedFramebufferRenderPass(framebuffer) &&
               (!vk::CommandBuffer::CanKnowIfEmpty() ||
                mCurrentWritingNode->getInsideRenderPassCommands()->empty());
    }

    // The following modify the render pass ops, and must be called only if
    // renderPassStartedButEmpty().
    void clearRenderPassColorAttachment(size_t attachmentIndex, const VkClearColorValue &clearValue)
    {
        mCurrentWritingNode->clearRenderPassColorAttachment(attachmentIndex, clearValue);
    }
    void clearRenderPassDepthAttachment(size_t attachmentIndex, float depth)
    {
        mCurrentWritingNode->clearRenderPassDepthAttachment(attachmentIndex, depth);
    }
    void clearRenderPassStencilAttachment(size_t attachmentIndex, uint32_t stencil)
    {
        mCurrentWritingNode->clearRenderPassStencilAttachment(attachmentIndex, stencil);
    }

    // Accessor for RenderPass RenderArea.
    const gl::Rectangle &getRenderPassRenderArea() const
    {
        ASSERT(hasStartedRenderPass());
        return mCurrentWritingNode->getRenderPassRenderArea();
    }

    // Called when 'this' object changes, and we'd like to restart the render pass later.
    void forceNewRenderPass(ContextVk *contextVk);

  protected:
    explicit CommandGraphResource(CommandGraphResourceType resourceType);

  private:
    // Returns true if this node has a current writing node with no children.
    ANGLE_INLINE bool hasChildlessWritingNode() const
    {
        // Note: currently, we don't have a resource that can issue both generic and special
        // commands.  We don't create read/write dependencies between mixed generic/special
        // resources either.  As such, we expect the function to always be generic here.  If such a
        // resource is added in the future, this can add a check for function == generic and fail if
        // false.
        ASSERT(mCurrentWritingNode == nullptr ||
               mCurrentWritingNode->getFunction() == CommandGraphNodeFunction::Generic);
        return (mCurrentWritingNode != nullptr && !mCurrentWritingNode->hasChild());
    }

    void startNewCommands(ContextVk *contextVk);

    void addDependencyImpl(ContextVk *contextVk, CommandGraphResource *used);

    Serial mStoredQueueSerial;

    // Current command graph writing node.
    CommandGraphNode *mCurrentWritingNode;

    // Set if the framebuffer or scissor has changed in such a way that would require the render
    // pass to restart.  If the render pass isn't automatically restarted for some other reason
    // (such as a dependency), this would ensure it does on next draw call.
    bool mForceNewRenderPass;

    // Additional diagnostic information.
    CommandGraphResourceType mResourceType;
};

// Translating OpenGL commands into Vulkan and submitting them immediately loses out on some
// of the powerful flexiblity Vulkan offers in RenderPasses. Load/Store ops can automatically
// clear RenderPass attachments, or preserve the contents. RenderPass automatic layout transitions
// can improve certain performance cases. Also, we can remove redundant RenderPass Begin and Ends
// when processing interleaved draw operations on independent Framebuffers.
//
// ANGLE's CommandGraph (and CommandGraphNode) attempt to solve these problems using deferred
// command submission. We also sometimes call this command re-ordering. A brief summary:
//
// During GL command processing, we record Vulkan commands into SecondaryCommandBuffers, which
// are stored in CommandGraphNodes, and these nodes are chained together via dependencies to
// form a directed acyclic CommandGraph. When we need to submit the CommandGraph, say during a
// SwapBuffers or ReadPixels call, we begin a primary Vulkan CommandBuffer, and walk the
// CommandGraph, starting at the most senior nodes, recording SecondaryCommandBuffers inside
// and outside RenderPasses as necessary, filled with the right load/store operations. Once
// the primary CommandBuffer has recorded all of the SecondaryCommandBuffers from all the open
// CommandGraphNodes, we submit the primary CommandBuffer to the VkQueue on the device.
//
// The Command Graph consists of an array of open Command Graph Nodes. It supports allocating new
// nodes for the graph, which are linked via dependency relation calls in CommandGraphNode, and
// also submitting the whole command graph via submitCommands.
class CommandGraph final : angle::NonCopyable
{
  public:
    explicit CommandGraph(bool enableGraphDiagnostics, angle::PoolAllocator *poolAllocator);
    ~CommandGraph();

    // Allocates a new CommandGraphNode and adds it to the list of current open nodes. No ordering
    // relations exist in the node by default. Call CommandGraphNode::SetHappensBeforeDependency
    // to set up dependency relations. If the node is a barrier, it will automatically add
    // dependencies between the previous barrier, the new barrier and all nodes in between.
    CommandGraphNode *allocateNode(CommandGraphNodeFunction function);

    angle::Result submitCommands(ContextVk *context,
                                 Serial serial,
                                 RenderPassCache *renderPassCache,
                                 CommandPool *commandPool,
                                 PrimaryCommandBuffer *primaryCommandBufferOut);
    bool empty() const;
    void clear();

    // The following create special-function nodes that don't require a graph resource.
    // Queries:
    void beginQuery(const QueryPool *queryPool, uint32_t queryIndex);
    void endQuery(const QueryPool *queryPool, uint32_t queryIndex);
    void writeTimestamp(const QueryPool *queryPool, uint32_t queryIndex);
    // GLsync and EGLSync:
    void setFenceSync(const vk::Event &event);
    void waitFenceSync(const vk::Event &event);
    // Debug markers:
    void insertDebugMarker(GLenum source, std::string &&marker);
    void pushDebugMarker(GLenum source, std::string &&marker);
    void popDebugMarker();

  private:
    CommandGraphNode *allocateBarrierNode(CommandGraphNodeFunction function,
                                          CommandGraphResourceType resourceType,
                                          uintptr_t resourceID);
    void setNewBarrier(CommandGraphNode *newBarrier);
    CommandGraphNode *getLastBarrierNode(size_t *indexOut);
    void addDependenciesFromPreviousBarrier(size_t begin,
                                            size_t end,
                                            CommandGraphNode *previousBarrier);
    void addDependenciesToNextBarrier(size_t begin, size_t end, CommandGraphNode *nextBarrier);

    void dumpGraphDotFile(std::ostream &out) const;

    std::vector<CommandGraphNode *> mNodes;
    bool mEnableGraphDiagnostics;
    angle::PoolAllocator *mPoolAllocator;

    // A set of nodes (eventually) exist that act as barriers to guarantee submission order.  For
    // example, a glMemoryBarrier() calls would lead to such a barrier or beginning and ending a
    // query. This is because the graph can reorder operations if it sees fit.  Let's call a barrier
    // node Bi, and the other nodes Ni. The edges between Ni don't interest us.  Before a barrier is
    // inserted, we have:
    //
    // N0 N1 ... Na
    // \___\__/_/     (dependency egdes, which we don't care about so I'll stop drawing them.
    //      \/
    //
    // When the first barrier is inserted, we will have:
    //
    //     ______
    //    /  ____\
    //   /  /     \
    //  /  /      /\
    // N0 N1 ... Na B0
    //
    // This makes sure all N0..Na are called before B0.  From then on, B0 will be the current
    // "barrier point" which extends an edge to every next node:
    //
    //     ______
    //    /  ____\
    //   /  /     \
    //  /  /      /\
    // N0 N1 ... Na B0 Na+1 ... Nb
    //                \/       /
    //                 \______/
    //
    //
    // When the next barrier B1 is met, all nodes between B0 and B1 will add a depenency on B1 as
    // well, and the "barrier point" is updated.
    //
    //     ______
    //    /  ____\         ______         ______
    //   /  /     \       /      \       /      \
    //  /  /      /\     /       /\     /       /\
    // N0 N1 ... Na B0 Na+1 ... Nb B1 Nb+1 ... Nc B2 ...
    //                \/       /  /  \/       /  /
    //                 \______/  /    \______/  /
    //                  \_______/      \_______/
    //
    //
    // When barrier Bi is introduced, all nodes added since Bi-1 need to add a dependency to Bi
    // (including Bi-1). We therefore keep track of the node index of the last barrier that was
    // issued.
    static constexpr size_t kInvalidNodeIndex = std::numeric_limits<std::size_t>::max();
    size_t mLastBarrierIndex;
};
}  // namespace vk
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_COMMAND_GRAPH_H_
