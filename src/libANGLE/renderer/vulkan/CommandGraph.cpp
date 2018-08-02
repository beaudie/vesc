//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CommandGraph:
//    Deferred work constructed by GL calls, that will later be flushed to Vulkan.
//

#include "libANGLE/renderer/vulkan/CommandGraph.h"

#include "libANGLE/renderer/vulkan/RenderTargetVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/renderer/vulkan/vk_format_utils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{

namespace vk
{

namespace
{

angle::Result InitAndBeginCommandBuffer(vk::Context *context,
                                        const CommandPool &commandPool,
                                        const VkCommandBufferInheritanceInfo &inheritanceInfo,
                                        VkCommandBufferUsageFlags flags,
                                        CommandBuffer *commandBuffer)
{
    ASSERT(!commandBuffer->valid());

    VkCommandBufferAllocateInfo createInfo;
    createInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    createInfo.pNext              = nullptr;
    createInfo.commandPool        = commandPool.getHandle();
    createInfo.level              = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    createInfo.commandBufferCount = 1;

    ANGLE_TRY(commandBuffer->init(context, createInfo));

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext            = nullptr;
    beginInfo.flags            = flags | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = &inheritanceInfo;

    ANGLE_TRY(commandBuffer->begin(context, beginInfo));
    return angle::Result::Continue();
}

}  // anonymous namespace

class CommandGraphNode final : angle::NonCopyable
{
  public:
    CommandGraphNode();
    ~CommandGraphNode();

    // Immutable queries for when we're walking the commands tree.
    CommandBuffer *getOutsideRenderPassCommands();
    CommandBuffer *getInsideRenderPassCommands();

    // For outside the render pass (copies, transitions, etc).
    angle::Result beginOutsideRenderPassRecording(Context *context,
                                                  const CommandPool &commandPool,
                                                  CommandBuffer **commandsOut);

    // For rendering commands (draws).
    angle::Result beginInsideRenderPassRecording(Context *context, CommandBuffer **commandsOut);

    // storeRenderPassInfo and append*RenderTarget store info relevant to the RenderPass.
    void storeRenderPassInfo(const Framebuffer &framebuffer,
                             const gl::Rectangle renderArea,
                             const vk::RenderPassDesc &renderPassDesc,
                             const std::vector<VkClearValue> &clearValues);

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
    angle::Result visitAndExecute(Context *context,
                                  RenderPassCache *renderPassCache,
                                  CommandBuffer *primaryCommandBuffer);

    const gl::Rectangle &getRenderPassRenderArea() const;

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

// CommmandGraphResource::PerContextData implementation

Serial CommandGraphResource::PerContextData::getStoredQueueSerial() const
{
    return serial;
}

bool CommandGraphResource::PerContextData::isResourceInUse(Context *context) const
{
    return context->isSerialInUse(serial);
}

void CommandGraphResource::PerContextData::addWriteDependency(PerContextData *writingResource)
{
    CommandGraphNode *writingNode = writingResource->currentWritingNode;
    ASSERT(writingNode);

    onWriteImpl(writingNode, writingResource->serial);
}

void CommandGraphResource::PerContextData::addReadDependency(PerContextData *readingResource)
{
    updateQueueSerial(readingResource->serial);

    CommandGraphNode *readingNode = readingResource->currentWritingNode;
    ASSERT(readingNode);

    if (hasChildlessWritingNode())
    {
        // Ensure 'readingNode' happens after the current writing node.
        CommandGraphNode::SetHappensBeforeDependency(currentWritingNode, readingNode);
    }

    // Add the read node to the list of nodes currently reading this resource.
    currentReadingNodes.push_back(readingNode);
}

angle::Result CommandGraphResource::PerContextData::beginWriteResource(
    Context *context,
    CommandBuffer **commandBufferOut)
{
    onResourceChanged(context);
    return currentWritingNode->beginOutsideRenderPassRecording(context, context->getCommandPool(),
                                                               commandBufferOut);
}

angle::Result CommandGraphResource::PerContextData::appendWriteResource(
    Context *context,
    CommandBuffer **commandBufferOut)
{
    updateQueueSerial(context->getCurrentQueueSerial());

    if (!hasChildlessWritingNode())
    {
        return beginWriteResource(context, commandBufferOut);
    }

    CommandBuffer *outsideRenderPassCommands = currentWritingNode->getOutsideRenderPassCommands();
    if (!outsideRenderPassCommands->valid())
    {
        ANGLE_TRY(currentWritingNode->beginOutsideRenderPassRecording(
            context, context->getCommandPool(), commandBufferOut));
    }
    else
    {
        *commandBufferOut = outsideRenderPassCommands;
    }

    return angle::Result::Continue();
}

angle::Result CommandGraphResource::PerContextData::beginRenderPass(
    Context *context,
    const Framebuffer &framebuffer,
    const gl::Rectangle &renderArea,
    const RenderPassDesc &renderPassDesc,
    const std::vector<VkClearValue> &clearValues,
    CommandBuffer **commandBufferOut) const
{
    // Hard-code RenderPass to clear the first render target to the current clear value.
    // TODO(jmadill): Proper clear value implementation. http://anglebug.com/2361
    currentWritingNode->storeRenderPassInfo(framebuffer, renderArea, renderPassDesc, clearValues);

    return currentWritingNode->beginInsideRenderPassRecording(context, commandBufferOut);
}

bool CommandGraphResource::PerContextData::appendToStartedRenderPass(
    Context *context,
    CommandBuffer **commandBufferOut)
{
    updateQueueSerial(context->getCurrentQueueSerial());
    if (hasStartedRenderPass())
    {
        *commandBufferOut = currentWritingNode->getInsideRenderPassCommands();
        return true;
    }
    else
    {
        return false;
    }
}

void CommandGraphResource::PerContextData::onResourceChanged(Context *context)
{
    CommandGraphNode *newCommands = context->getCommandGraph()->allocateNode();
    onWriteImpl(newCommands, context->getCurrentQueueSerial());
}

void CommandGraphResource::PerContextData::onWriteImpl(CommandGraphNode *writingNode,
                                                       Serial currentSerial)
{
    updateQueueSerial(currentSerial);

    // Make sure any open reads and writes finish before we execute 'writingNode'.
    if (!currentReadingNodes.empty())
    {
        CommandGraphNode::SetHappensBeforeDependencies(currentReadingNodes, writingNode);
        currentReadingNodes.clear();
    }

    if (currentWritingNode && currentWritingNode != writingNode)
    {
        CommandGraphNode::SetHappensBeforeDependency(currentWritingNode, writingNode);
    }

    currentWritingNode = writingNode;
}

bool CommandGraphResource::PerContextData::hasChildlessWritingNode() const
{
    return currentWritingNode != nullptr && !currentWritingNode->hasChildren();
}

bool CommandGraphResource::PerContextData::hasStartedRenderPass() const
{
    return hasChildlessWritingNode() && currentWritingNode->getInsideRenderPassCommands()->valid();
}

bool CommandGraphResource::PerContextData::hasStartedWriteResource() const
{
    return hasChildlessWritingNode() && currentWritingNode->getOutsideRenderPassCommands()->valid();
}

void CommandGraphResource::PerContextData::updateQueueSerial(Serial queueSerial)
{
    ASSERT(queueSerial >= serial);

    if (queueSerial > serial)
    {
        currentWritingNode = nullptr;
        currentReadingNodes.clear();
        serial = queueSerial;
    }
}

const gl::Rectangle &CommandGraphResource::PerContextData::getRenderPassRenderArea() const
{
    ASSERT(hasStartedRenderPass());
    return currentWritingNode->getRenderPassRenderArea();
}

// CommandGraphResource implementation.
CommandGraphResource::CommandGraphResource()
{
}

CommandGraphResource::~CommandGraphResource() = default;

bool CommandGraphResource::isResourceInUse(Context *context) const
{
    auto iter = mContextData.find(context);
    return iter != mContextData.end() && iter->second.isResourceInUse(context);
}

Serial CommandGraphResource::getStoredQueueSerial(Context *context) const
{
    auto iter = mContextData.find(context);
    return iter != mContextData.end() ? iter->second.getStoredQueueSerial() : Serial();
}

ContextSerialMap CommandGraphResource::getStoredQueueSerials() const
{
    // TODO: don't build this on the fly, store it so it can be returned by const ref
    ContextSerialMap result;
    for (const auto &contextData : mContextData)
    {
        result[contextData.first] = contextData.second.getStoredQueueSerial();
    }
    return result;
}

bool CommandGraphResource::hasStartedWriteResource(Context *context) const
{
    auto iter = mContextData.find(context);
    return iter != mContextData.end() && iter->second.hasStartedWriteResource();
}

angle::Result CommandGraphResource::beginWriteResource(Context *context,
                                                       CommandBuffer **commandBufferOut)
{
    PerContextData &contextData = mContextData[context];
    return contextData.beginWriteResource(context, commandBufferOut);
}

angle::Result CommandGraphResource::appendWriteResource(Context *context,
                                                        CommandBuffer **commandBufferOut)
{
    PerContextData &contextData = mContextData[context];
    return contextData.appendWriteResource(context, commandBufferOut);
}

bool CommandGraphResource::appendToStartedRenderPass(Context *context,
                                                     CommandBuffer **commandBufferOut)
{
    PerContextData &contextData = mContextData[context];
    return contextData.appendToStartedRenderPass(context, commandBufferOut);
}

const gl::Rectangle &CommandGraphResource::getRenderPassRenderArea(Context *context) const
{
    const PerContextData &contextData = mContextData.at(context);
    return contextData.getRenderPassRenderArea();
}

angle::Result CommandGraphResource::beginRenderPass(Context *context,
                                                    const Framebuffer &framebuffer,
                                                    const gl::Rectangle &renderArea,
                                                    const RenderPassDesc &renderPassDesc,
                                                    const std::vector<VkClearValue> &clearValues,
                                                    CommandBuffer **commandBufferOut) const
{
    const PerContextData &contextData = mContextData.at(context);
    return contextData.beginRenderPass(context, framebuffer, renderArea, renderPassDesc,
                                       clearValues, commandBufferOut);
}

void CommandGraphResource::onResourceChanged(Context *context)
{
    PerContextData &contextData = mContextData[context];
    contextData.onResourceChanged(context);
}

void CommandGraphResource::addWriteDependency(Context *context,
                                              CommandGraphResource *writingResource)
{
    PerContextData &contextData = mContextData[context];
    contextData.addWriteDependency(&writingResource->mContextData[context]);
}

void CommandGraphResource::addReadDependency(Context *context,
                                             CommandGraphResource *readingResource)
{
    PerContextData &contextData = mContextData[context];
    contextData.addReadDependency(&readingResource->mContextData[context]);
}

// CommandGraphNode implementation.
CommandGraphNode::CommandGraphNode()
    : mRenderPassClearValues{}, mHasChildren(false), mVisitedState(VisitedState::Unvisited)
{
}

CommandGraphNode::~CommandGraphNode()
{
    mRenderPassFramebuffer.setHandle(VK_NULL_HANDLE);

    // Command buffers are managed by the command pool, so don't need to be freed.
    mOutsideRenderPassCommands.releaseHandle();
    mInsideRenderPassCommands.releaseHandle();
}

CommandBuffer *CommandGraphNode::getOutsideRenderPassCommands()
{
    ASSERT(!mHasChildren);
    return &mOutsideRenderPassCommands;
}

CommandBuffer *CommandGraphNode::getInsideRenderPassCommands()
{
    ASSERT(!mHasChildren);
    return &mInsideRenderPassCommands;
}

angle::Result CommandGraphNode::beginOutsideRenderPassRecording(Context *context,
                                                                const CommandPool &commandPool,
                                                                CommandBuffer **commandsOut)
{
    ASSERT(!mHasChildren);

    VkCommandBufferInheritanceInfo inheritanceInfo;
    inheritanceInfo.sType                = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.pNext                = nullptr;
    inheritanceInfo.renderPass           = VK_NULL_HANDLE;
    inheritanceInfo.subpass              = 0;
    inheritanceInfo.framebuffer          = VK_NULL_HANDLE;
    inheritanceInfo.occlusionQueryEnable = VK_FALSE;
    inheritanceInfo.queryFlags           = 0;
    inheritanceInfo.pipelineStatistics   = 0;

    ANGLE_TRY(InitAndBeginCommandBuffer(context, commandPool, inheritanceInfo, 0,
                                        &mOutsideRenderPassCommands));

    *commandsOut = &mOutsideRenderPassCommands;
    return angle::Result::Continue();
}

angle::Result CommandGraphNode::beginInsideRenderPassRecording(Context *context,
                                                               CommandBuffer **commandsOut)
{
    ASSERT(!mHasChildren);

    // Get a compatible RenderPass from the cache so we can initialize the inheritance info.
    // TODO(jmadill): Support query for compatible/conformant render pass. htto://anglebug.com/2361
    RenderPass *compatibleRenderPass;
    ANGLE_TRY(context->getCompatibleRenderPass(mRenderPassDesc, &compatibleRenderPass));

    VkCommandBufferInheritanceInfo inheritanceInfo;
    inheritanceInfo.sType                = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.pNext                = nullptr;
    inheritanceInfo.renderPass           = compatibleRenderPass->getHandle();
    inheritanceInfo.subpass              = 0;
    inheritanceInfo.framebuffer          = mRenderPassFramebuffer.getHandle();
    inheritanceInfo.occlusionQueryEnable = VK_FALSE;
    inheritanceInfo.queryFlags           = 0;
    inheritanceInfo.pipelineStatistics   = 0;

    ANGLE_TRY(InitAndBeginCommandBuffer(context, context->getCommandPool(), inheritanceInfo,
                                        VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
                                        &mInsideRenderPassCommands));

    *commandsOut = &mInsideRenderPassCommands;
    return angle::Result::Continue();
}

void CommandGraphNode::storeRenderPassInfo(const Framebuffer &framebuffer,
                                           const gl::Rectangle renderArea,
                                           const vk::RenderPassDesc &renderPassDesc,
                                           const std::vector<VkClearValue> &clearValues)
{
    mRenderPassDesc = renderPassDesc;
    mRenderPassFramebuffer.setHandle(framebuffer.getHandle());
    mRenderPassRenderArea = renderArea;
    std::copy(clearValues.begin(), clearValues.end(), mRenderPassClearValues.begin());
}

// static
void CommandGraphNode::SetHappensBeforeDependency(CommandGraphNode *beforeNode,
                                                  CommandGraphNode *afterNode)
{
    ASSERT(beforeNode != afterNode && !beforeNode->isChildOf(afterNode));
    afterNode->mParents.emplace_back(beforeNode);
    beforeNode->setHasChildren();
}

// static
void CommandGraphNode::SetHappensBeforeDependencies(
    const std::vector<CommandGraphNode *> &beforeNodes,
    CommandGraphNode *afterNode)
{
    afterNode->mParents.insert(afterNode->mParents.end(), beforeNodes.begin(), beforeNodes.end());

    // TODO(jmadill): is there a faster way to do this?
    for (CommandGraphNode *beforeNode : beforeNodes)
    {
        beforeNode->setHasChildren();

        ASSERT(beforeNode != afterNode && !beforeNode->isChildOf(afterNode));
    }
}

bool CommandGraphNode::hasParents() const
{
    return !mParents.empty();
}

void CommandGraphNode::setHasChildren()
{
    mHasChildren = true;
}

bool CommandGraphNode::hasChildren() const
{
    return mHasChildren;
}

// Do not call this in anything but testing code, since it's slow.
bool CommandGraphNode::isChildOf(CommandGraphNode *parent)
{
    std::set<CommandGraphNode *> visitedList;
    std::vector<CommandGraphNode *> openList;
    openList.insert(openList.begin(), mParents.begin(), mParents.end());
    while (!openList.empty())
    {
        CommandGraphNode *current = openList.back();
        openList.pop_back();
        if (visitedList.count(current) == 0)
        {
            if (current == parent)
            {
                return true;
            }
            visitedList.insert(current);
            openList.insert(openList.end(), current->mParents.begin(), current->mParents.end());
        }
    }

    return false;
}

VisitedState CommandGraphNode::visitedState() const
{
    return mVisitedState;
}

void CommandGraphNode::visitParents(std::vector<CommandGraphNode *> *stack)
{
    ASSERT(mVisitedState == VisitedState::Unvisited);
    stack->insert(stack->end(), mParents.begin(), mParents.end());
    mVisitedState = VisitedState::Ready;
}

angle::Result CommandGraphNode::visitAndExecute(vk::Context *context,
                                                RenderPassCache *renderPassCache,
                                                CommandBuffer *primaryCommandBuffer)
{
    if (mOutsideRenderPassCommands.valid())
    {
        ANGLE_TRY(mOutsideRenderPassCommands.end(context));
        primaryCommandBuffer->executeCommands(1, &mOutsideRenderPassCommands);
    }

    if (mInsideRenderPassCommands.valid())
    {
        // Pull a compatible RenderPass from the cache.
        // TODO(jmadill): Insert real ops and layout transitions.
        RenderPass *renderPass = nullptr;
        ANGLE_TRY(renderPassCache->getCompatibleRenderPass(context, mRenderPassDesc, &renderPass));

        ANGLE_TRY(mInsideRenderPassCommands.end(context));

        VkRenderPassBeginInfo beginInfo;
        beginInfo.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.pNext                    = nullptr;
        beginInfo.renderPass               = renderPass->getHandle();
        beginInfo.framebuffer              = mRenderPassFramebuffer.getHandle();
        beginInfo.renderArea.offset.x      = static_cast<uint32_t>(mRenderPassRenderArea.x);
        beginInfo.renderArea.offset.y      = static_cast<uint32_t>(mRenderPassRenderArea.y);
        beginInfo.renderArea.extent.width  = static_cast<uint32_t>(mRenderPassRenderArea.width);
        beginInfo.renderArea.extent.height = static_cast<uint32_t>(mRenderPassRenderArea.height);
        beginInfo.clearValueCount          = mRenderPassDesc.attachmentCount();
        beginInfo.pClearValues             = mRenderPassClearValues.data();

        primaryCommandBuffer->beginRenderPass(beginInfo,
                                              VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
        primaryCommandBuffer->executeCommands(1, &mInsideRenderPassCommands);
        primaryCommandBuffer->endRenderPass();
    }

    mVisitedState = VisitedState::Visited;
    return angle::Result::Continue();
}

const gl::Rectangle &CommandGraphNode::getRenderPassRenderArea() const
{
    return mRenderPassRenderArea;
}

// CommandGraph implementation.
CommandGraph::CommandGraph() = default;

CommandGraph::~CommandGraph()
{
    ASSERT(empty());
}

CommandGraphNode *CommandGraph::allocateNode()
{
    // TODO(jmadill): Use a pool allocator for the CPU node allocations.
    CommandGraphNode *newCommands = new CommandGraphNode();
    mNodes.emplace_back(newCommands);
    return newCommands;
}

angle::Result CommandGraph::submitCommands(Context *context,
                                           RenderPassCache *renderPassCache,
                                           CommandPool *commandPool,
                                           CommandBuffer *primaryCommandBufferOut)
{
    VkCommandBufferAllocateInfo primaryInfo;
    primaryInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    primaryInfo.pNext              = nullptr;
    primaryInfo.commandPool        = commandPool->getHandle();
    primaryInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    primaryInfo.commandBufferCount = 1;

    ANGLE_TRY(primaryCommandBufferOut->init(context, primaryInfo));

    if (mNodes.empty())
    {
        return angle::Result::Continue();
    }

    std::vector<CommandGraphNode *> nodeStack;

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext            = nullptr;
    beginInfo.flags            = 0;
    beginInfo.pInheritanceInfo = nullptr;

    ANGLE_TRY(primaryCommandBufferOut->begin(context, beginInfo));

    for (CommandGraphNode *topLevelNode : mNodes)
    {
        // Only process commands that don't have child commands. The others will be pulled in
        // automatically. Also skip commands that have already been visited.
        if (topLevelNode->hasChildren() || topLevelNode->visitedState() != VisitedState::Unvisited)
            continue;

        nodeStack.push_back(topLevelNode);

        while (!nodeStack.empty())
        {
            CommandGraphNode *node = nodeStack.back();

            switch (node->visitedState())
            {
                case VisitedState::Unvisited:
                    node->visitParents(&nodeStack);
                    break;
                case VisitedState::Ready:
                    ANGLE_TRY(
                        node->visitAndExecute(context, renderPassCache, primaryCommandBufferOut));
                    nodeStack.pop_back();
                    break;
                case VisitedState::Visited:
                    nodeStack.pop_back();
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
        }
    }

    ANGLE_TRY(primaryCommandBufferOut->end(context));

    // TODO(jmadill): Use pool allocation so we don't need to deallocate command graph.
    for (CommandGraphNode *node : mNodes)
    {
        delete node;
    }
    mNodes.clear();

    return angle::Result::Continue();
}

bool CommandGraph::empty() const
{
    return mNodes.empty();
}

}  // namespace vk
}  // namespace rx
