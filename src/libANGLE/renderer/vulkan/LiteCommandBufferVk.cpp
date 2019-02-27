//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// LiteCommandBuffer:
//    CPU-side storage of commands to delay GPU-side allocation until commands are submitted.
//

#include "libANGLE/renderer/vulkan/LiteCommandBufferVk.h"

namespace rx
{
namespace vk
{

// Allocate/initialize memory for the command and return pointer to Cmd Header
template <class structType>
structType *LiteCommandBuffer::initCommand(CommandID cmdID, size_t variableSize)
{
    size_t paramSize      = sizeof(structType);
    size_t completeSize   = sizeof(CommandHeader) + paramSize + variableSize;
    CommandHeader *header = static_cast<CommandHeader *>(mAllocator->allocate(completeSize));
    // Update cmd ID in header
    header->id = cmdID;
    // Update mHead ptr
    mHead = (mHead == nullptr) ? header : mHead;
    // Update prev cmd's "next" ptr and mLast ptr
    if (mLast)
    {
        mLast->next = header;
    }
    // Update mLast ptr
    mLast = header;

    return reinterpret_cast<structType *>(reinterpret_cast<char *>(header) + sizeof(CommandHeader));
}

void *LiteCommandBuffer::getCmdVariableDataPtr(CommandID cmdID, void *paramPtr)
{
    return static_cast<void *>(reinterpret_cast<char *>(paramPtr) +
                               CommandParamSizes[static_cast<uint32_t>(cmdID)]);
}

void LiteCommandBuffer::bindDescriptorSets(VkPipelineBindPoint bindPoint,
                                           VkPipelineLayout layout,
                                           uint32_t firstSet,
                                           uint32_t descriptorSetCount,
                                           const VkDescriptorSet *descriptorSets,
                                           uint32_t dynamicOffsetCount,
                                           const uint32_t *dynamicOffsets)
{
    size_t descSize   = descriptorSetCount * sizeof(VkDescriptorSet);
    size_t offsetSize = dynamicOffsetCount * sizeof(uint32_t);
    size_t varSize    = descSize + offsetSize;
    BindDescriptorSetParams *paramStruct =
        initCommand<BindDescriptorSetParams>(CommandID::BindDescriptorSets, varSize);
    // Copy params into memory
    paramStruct->bindPoint          = bindPoint;
    paramStruct->layout             = layout;
    paramStruct->firstSet           = firstSet;
    paramStruct->descriptorSetCount = descriptorSetCount;
    paramStruct->dynamicOffsetCount = dynamicOffsetCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::BindDescriptorSets, paramStruct);
    memcpy(ptrData, descriptorSets, descSize);
    paramStruct->descriptorSets = static_cast<const VkDescriptorSet *>(ptrData);
    ptrData                     = reinterpret_cast<char *>(ptrData) + descSize;
    memcpy(ptrData, dynamicOffsets, offsetSize);
    paramStruct->dynamicOffsets = static_cast<const uint32_t *>(ptrData);
}

void LiteCommandBuffer::bindIndexBuffer(const VkBuffer &buffer,
                                        VkDeviceSize offset,
                                        VkIndexType indexType)
{
    BindIndexBufferParams *paramStruct =
        initCommand<BindIndexBufferParams>(CommandID::BindIndexBuffer, 0);
    paramStruct->buffer    = buffer;
    paramStruct->offset    = offset;
    paramStruct->indexType = indexType;
}

void LiteCommandBuffer::bindPipeline(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline)
{
    BindPipelineParams *paramStruct = initCommand<BindPipelineParams>(CommandID::BindPipeline, 0);
    paramStruct->pipelineBindPoint  = pipelineBindPoint;
    paramStruct->pipeline           = pipeline;
}

void LiteCommandBuffer::bindVertexBuffers(uint32_t firstBinding,
                                          uint32_t bindingCount,
                                          const VkBuffer *buffers,
                                          const VkDeviceSize *offsets)
{
    size_t buffSize   = bindingCount * sizeof(VkBuffer);
    size_t offsetSize = bindingCount * sizeof(VkDeviceSize);
    size_t varSize    = buffSize + offsetSize;
    BindVertexBuffersParams *paramStruct =
        initCommand<BindVertexBuffersParams>(CommandID::BindVertexBuffers, varSize);
    // Copy params
    paramStruct->firstBinding = firstBinding;
    paramStruct->bindingCount = bindingCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::BindVertexBuffers, paramStruct);
    memcpy(ptrData, buffers, buffSize);
    paramStruct->buffers = static_cast<const VkBuffer *>(ptrData);
    ptrData              = reinterpret_cast<char *>(ptrData) + buffSize;
    memcpy(ptrData, offsets, offsetSize);
    paramStruct->offsets = static_cast<const VkDeviceSize *>(ptrData);
}

void LiteCommandBuffer::blitImage(VkImage srcImage,
                                  VkImageLayout srcImageLayout,
                                  VkImage dstImage,
                                  VkImageLayout dstImageLayout,
                                  uint32_t regionCount,
                                  VkImageBlit *pRegions,
                                  VkFilter filter)
{
    size_t regionSize            = regionCount * sizeof(VkImageBlit);
    BlitImageParams *paramStruct = initCommand<BlitImageParams>(CommandID::BlitImage, regionSize);
    paramStruct->srcImage        = srcImage;
    paramStruct->srcImageLayout  = srcImageLayout;
    paramStruct->dstImage        = dstImage;
    paramStruct->dstImageLayout  = dstImageLayout;
    paramStruct->regionCount     = regionCount;
    paramStruct->filter          = filter;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::BlitImage, paramStruct);
    memcpy(ptrData, pRegions, regionSize);
    paramStruct->pRegions = reinterpret_cast<VkImageBlit *>(ptrData);
}

void LiteCommandBuffer::copyBuffer(const VkBuffer &srcBuffer,
                                   const VkBuffer &destBuffer,
                                   uint32_t regionCount,
                                   const VkBufferCopy *regions)
{
    size_t regionSize = regionCount * sizeof(VkBufferCopy);
    CopyBufferParams *paramStruct =
        initCommand<CopyBufferParams>(CommandID::CopyBuffer, regionSize);
    paramStruct->srcBuffer   = srcBuffer;
    paramStruct->destBuffer  = destBuffer;
    paramStruct->regionCount = regionCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::CopyBuffer, paramStruct);
    memcpy(ptrData, regions, regionSize);
    paramStruct->regions = static_cast<const VkBufferCopy *>(ptrData);
}
#if 0
void LiteCommandBuffer::copyBufferToImage(VkBuffer srcBuffer,
                                          VkImage dstImage,
                                          VkImageLayout dstImageLayout,
                                          uint32_t regionCount,
                                          const VkBufferImageCopy *regions)
{
    size_t regionSize = regionCount * sizeof(VkBufferImageCopy);
    void *basePtr     = initCommand(CommandID::CopyBufferToImage, regionSize);
    CopyBufferToImageParams *paramStruct = reinterpret_cast<CopyBufferToImageParams *>(basePtr);
    paramStruct->srcBuffer               = srcBuffer;
    paramStruct->dstImage                = dstImage;
    paramStruct->dstImageLayout          = dstImageLayout;
    paramStruct->regionCount             = regionCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::CopyBufferToImage, basePtr);
    memcpy(ptrData, regions, regionSize);
    paramStruct->regions = static_cast<const VkBufferImageCopy *>(ptrData);
}

void LiteCommandBuffer::copyImage(VkImage srcImage,
                                  VkImageLayout srcImageLayout,
                                  VkImage dstImage,
                                  VkImageLayout dstImageLayout,
                                  uint32_t regionCount,
                                  const VkImageCopy *regions)
{
    size_t regionSize            = regionCount * sizeof(VkImageCopy);
    void *basePtr                = initCommand(CommandID::CopyImage, regionSize);
    CopyImageParams *paramStruct = reinterpret_cast<CopyImageParams *>(basePtr);
    paramStruct->srcImage        = srcImage;
    paramStruct->srcImageLayout  = srcImageLayout;
    paramStruct->dstImage        = dstImage;
    paramStruct->dstImageLayout  = dstImageLayout;
    paramStruct->regionCount     = regionCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::CopyImage, basePtr);
    memcpy(ptrData, regions, regionSize);
    paramStruct->regions = static_cast<const VkImageCopy *>(ptrData);
}

void LiteCommandBuffer::copyImageToBuffer(VkImage srcImage,
                                          VkImageLayout srcImageLayout,
                                          VkBuffer dstBuffer,
                                          uint32_t regionCount,
                                          const VkBufferImageCopy *regions)
{
    size_t regionSize = regionCount * sizeof(VkBufferImageCopy);
    void *basePtr     = initCommand(CommandID::CopyImageToBuffer, regionSize);
    CopyImageToBufferParams *paramStruct = reinterpret_cast<CopyImageToBufferParams *>(basePtr);
    paramStruct->srcImage                = srcImage;
    paramStruct->srcImageLayout          = srcImageLayout;
    paramStruct->dstBuffer               = dstBuffer;
    paramStruct->regionCount             = regionCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::CopyImageToBuffer, basePtr);
    memcpy(ptrData, regions, regionSize);
    paramStruct->regions = static_cast<const VkBufferImageCopy *>(ptrData);
}

void LiteCommandBuffer::clearAttachments(uint32_t attachmentCount,
                                         const VkClearAttachment *attachments,
                                         uint32_t rectCount,
                                         const VkClearRect *rects)
{
    size_t attachSize                   = attachmentCount * sizeof(VkClearAttachment);
    size_t rectSize                     = rectCount * sizeof(VkClearRect);
    size_t varSize                      = attachSize + rectSize;
    void *basePtr                       = initCommand(CommandID::ClearAttachments, varSize);
    ClearAttachmentsParams *paramStruct = reinterpret_cast<ClearAttachmentsParams *>(basePtr);
    paramStruct->attachmentCount        = attachmentCount;
    paramStruct->rectCount              = rectCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::ClearAttachments, basePtr);
    memcpy(ptrData, attachments, attachSize);
    paramStruct->attachments = static_cast<const VkClearAttachment *>(ptrData);
    ptrData                  = reinterpret_cast<char *>(ptrData) + attachSize;
    memcpy(ptrData, rects, rectSize);
    paramStruct->rects = static_cast<const VkClearRect *>(ptrData);
}

void LiteCommandBuffer::clearColorImage(VkImage image,
                                        VkImageLayout imageLayout,
                                        const VkClearColorValue &color,
                                        uint32_t rangeCount,
                                        const VkImageSubresourceRange *ranges)
{
    size_t rangeSize = rangeCount * sizeof(VkImageSubresourceRange);
    void *basePtr    = initCommand(CommandID::ClearColorImage, rangeSize);
    ClearColorImageParams *paramStruct = reinterpret_cast<ClearColorImageParams *>(basePtr);
    paramStruct->image                 = image;
    paramStruct->imageLayout           = imageLayout;
    paramStruct->color                 = color;
    paramStruct->rangeCount            = rangeCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::ClearColorImage, basePtr);
    memcpy(ptrData, ranges, rangeSize);
    paramStruct->ranges = static_cast<const VkImageSubresourceRange *>(ptrData);
}

void LiteCommandBuffer::clearDepthStencilImage(VkImage image,
                                               VkImageLayout imageLayout,
                                               const VkClearDepthStencilValue &depthStencil,
                                               uint32_t rangeCount,
                                               const VkImageSubresourceRange *ranges)
{
    size_t rangeSize = rangeCount * sizeof(VkImageSubresourceRange);
    void *basePtr    = initCommand(CommandID::ClearDepthStencilImage, rangeSize);
    ClearDepthStencilImageParams *paramStruct =
        reinterpret_cast<ClearDepthStencilImageParams *>(basePtr);
    paramStruct->image        = image;
    paramStruct->imageLayout  = imageLayout;
    paramStruct->depthStencil = depthStencil;
    paramStruct->rangeCount   = rangeCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::ClearDepthStencilImage, basePtr);
    memcpy(ptrData, ranges, rangeSize);
    paramStruct->ranges = static_cast<const VkImageSubresourceRange *>(ptrData);
}

void LiteCommandBuffer::updateBuffer(VkBuffer buffer,
                                     VkDeviceSize dstOffset,
                                     VkDeviceSize dataSize,
                                     const void *data)
{
    void *basePtr                   = initCommand(CommandID::UpdateBuffer, dataSize);
    UpdateBufferParams *paramStruct = reinterpret_cast<UpdateBufferParams *>(basePtr);
    paramStruct->buffer             = buffer;
    paramStruct->dstOffset          = dstOffset;
    paramStruct->dataSize           = dataSize;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::UpdateBuffer, basePtr);
    memcpy(ptrData, data, dataSize);
    paramStruct->data = ptrData;
}

void LiteCommandBuffer::pushConstants(VkPipelineLayout layout,
                                      VkShaderStageFlags flag,
                                      uint32_t offset,
                                      uint32_t size,
                                      const void *data)
{
    void *basePtr                    = initCommand(CommandID::PushConstants, size);
    PushConstantsParams *paramStruct = reinterpret_cast<PushConstantsParams *>(basePtr);
    paramStruct->layout              = layout;
    paramStruct->flag                = flag;
    paramStruct->offset              = offset;
    paramStruct->size                = size;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::PushConstants, basePtr);
    memcpy(ptrData, data, size);
    paramStruct->data = ptrData;
}

void LiteCommandBuffer::setViewport(uint32_t firstViewport,
                                    uint32_t viewportCount,
                                    const VkViewport *viewports)
{
    size_t viewportSize            = viewportCount * sizeof(VkViewport);
    void *basePtr                  = initCommand(CommandID::SetViewport, viewportSize);
    SetViewportParams *paramStruct = reinterpret_cast<SetViewportParams *>(basePtr);
    paramStruct->firstViewport     = firstViewport;
    paramStruct->viewportCount     = viewportCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::SetViewport, basePtr);
    memcpy(ptrData, viewports, viewportSize);
    paramStruct->viewports = static_cast<const VkViewport *>(ptrData);
}

void LiteCommandBuffer::setScissor(uint32_t firstScissor,
                                   uint32_t scissorCount,
                                   const VkRect2D *scissors)
{
    size_t scissorSize            = scissorCount * sizeof(VkRect2D);
    void *basePtr                 = initCommand(CommandID::SetScissor, scissorSize);
    SetScissorParams *paramStruct = reinterpret_cast<SetScissorParams *>(basePtr);
    paramStruct->firstScissor     = firstScissor;
    paramStruct->scissorCount     = scissorCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::SetScissor, basePtr);
    memcpy(ptrData, scissors, scissorSize);
    paramStruct->scissors = static_cast<const VkRect2D *>(ptrData);
}

void LiteCommandBuffer::draw(uint32_t vertexCount,
                             uint32_t instanceCount,
                             uint32_t firstVertex,
                             uint32_t firstInstance)
{
    DrawParams *paramStruct =
        reinterpret_cast<DrawParams *>(initCommand(CommandID::Draw, 0));
    paramStruct->vertexCount   = vertexCount;
    paramStruct->instanceCount = instanceCount;
    paramStruct->firstVertex   = firstVertex;
    paramStruct->firstInstance = firstInstance;
}

void LiteCommandBuffer::drawIndexed(uint32_t indexCount,
                                    uint32_t instanceCount,
                                    uint32_t firstIndex,
                                    int32_t vertexOffset,
                                    uint32_t firstInstance)
{
    DrawIndexedParams *paramStruct =
        reinterpret_cast<DrawIndexedParams *>(initCommand(CommandID::DrawIndexed, 0));
    paramStruct->indexCount    = indexCount;
    paramStruct->instanceCount = instanceCount;
    paramStruct->firstIndex    = firstIndex;
    paramStruct->vertexOffset  = vertexOffset;
    paramStruct->firstInstance = firstInstance;
}

void LiteCommandBuffer::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    DispatchParams *paramStruct =
        reinterpret_cast<DispatchParams *>(initCommand(CommandID::Dispatch, 0));
    paramStruct->groupCountX = groupCountX;
    paramStruct->groupCountY = groupCountY;
    paramStruct->groupCountZ = groupCountZ;
}

void LiteCommandBuffer::pipelineBarrier(VkPipelineStageFlags srcStageMask,
                                        VkPipelineStageFlags dstStageMask,
                                        VkDependencyFlags dependencyFlags,
                                        uint32_t memoryBarrierCount,
                                        const VkMemoryBarrier *memoryBarriers,
                                        uint32_t bufferMemoryBarrierCount,
                                        const VkBufferMemoryBarrier *bufferMemoryBarriers,
                                        uint32_t imageMemoryBarrierCount,
                                        const VkImageMemoryBarrier *imageMemoryBarriers)
{
    size_t memBarrierSize              = memoryBarrierCount * sizeof(VkMemoryBarrier);
    size_t buffBarrierSize             = bufferMemoryBarrierCount * sizeof(VkBufferMemoryBarrier);
    size_t imgBarrierSize              = imageMemoryBarrierCount * sizeof(VkImageMemoryBarrier);
    size_t totalSize                   = memBarrierSize + buffBarrierSize + imgBarrierSize;
    void *basePtr                      = initCommand(CommandID::PipelinBarrier, totalSize);
    PipelineBarrierParams *paramStruct = reinterpret_cast<PipelineBarrierParams *>(basePtr);
    paramStruct->srcStageMask          = srcStageMask;
    paramStruct->dstStageMask          = dstStageMask;
    paramStruct->memoryBarrierCount    = memoryBarrierCount;
    paramStruct->bufferMemoryBarrierCount = bufferMemoryBarrierCount;
    paramStruct->imageMemoryBarrierCount  = imageMemoryBarrierCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::PipelinBarrier, basePtr);
    memcpy(ptrData, memoryBarriers, memBarrierSize);
    paramStruct->memoryBarriers = static_cast<const VkMemoryBarrier *>(ptrData);
    ptrData                     = reinterpret_cast<char *>(ptrData) + memBarrierSize;
    memcpy(ptrData, bufferMemoryBarriers, buffBarrierSize);
    paramStruct->bufferMemoryBarriers = static_cast<const VkBufferMemoryBarrier *>(ptrData);
    ptrData                           = reinterpret_cast<char *>(ptrData) + buffBarrierSize;
    memcpy(ptrData, imageMemoryBarriers, imgBarrierSize);
    paramStruct->imageMemoryBarriers = static_cast<const VkImageMemoryBarrier *>(ptrData);
}

void LiteCommandBuffer::setEvent(VkEvent event, VkPipelineStageFlags stageMask)
{
    SetEventParams *paramStruct =
        reinterpret_cast<SetEventParams *>(initCommand(CommandID::SetEvent, 0));
    paramStruct->event     = event;
    paramStruct->stageMask = stageMask;
}

void LiteCommandBuffer::resetEvent(VkEvent event, VkPipelineStageFlags stageMask)
{
    ResetEventParams *paramStruct =
        reinterpret_cast<ResetEventParams *>(initCommand(CommandID::ResetEvent, 0));
    paramStruct->event     = event;
    paramStruct->stageMask = stageMask;
}

void LiteCommandBuffer::waitEvents(uint32_t eventCount,
                                   const VkEvent *events,
                                   VkPipelineStageFlags srcStageMask,
                                   VkPipelineStageFlags dstStageMask,
                                   uint32_t memoryBarrierCount,
                                   const VkMemoryBarrier *memoryBarriers,
                                   uint32_t bufferMemoryBarrierCount,
                                   const VkBufferMemoryBarrier *bufferMemoryBarriers,
                                   uint32_t imageMemoryBarrierCount,
                                   const VkImageMemoryBarrier *imageMemoryBarriers)
{
    size_t eventSize                = eventCount * sizeof(VkEvent);
    size_t memBarrierSize           = memoryBarrierCount * sizeof(VkMemoryBarrier);
    size_t buffBarrierSize          = bufferMemoryBarrierCount * sizeof(VkBufferMemoryBarrier);
    size_t imgBarrierSize           = imageMemoryBarrierCount * sizeof(VkImageMemoryBarrier);
    size_t totalSize                = eventSize + memBarrierSize + buffBarrierSize + imgBarrierSize;
    void *basePtr                   = initCommand(CommandID::WaitEvents, totalSize);
    WaitEventsParams *paramStruct   = reinterpret_cast<WaitEventsParams *>(basePtr);
    paramStruct->eventCount         = eventCount;
    paramStruct->srcStageMask       = srcStageMask;
    paramStruct->dstStageMask       = dstStageMask;
    paramStruct->memoryBarrierCount = memoryBarrierCount;
    paramStruct->bufferMemoryBarrierCount = bufferMemoryBarrierCount;
    paramStruct->imageMemoryBarrierCount  = imageMemoryBarrierCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::PipelinBarrier, basePtr);
    memcpy(ptrData, events, eventSize);
    paramStruct->events = static_cast<const VkEvent *>(ptrData);
    ptrData             = reinterpret_cast<char *>(ptrData) + eventSize;
    memcpy(ptrData, memoryBarriers, memBarrierSize);
    paramStruct->memoryBarriers = static_cast<const VkMemoryBarrier *>(ptrData);
    ptrData                     = reinterpret_cast<char *>(ptrData) + memBarrierSize;
    memcpy(ptrData, bufferMemoryBarriers, buffBarrierSize);
    paramStruct->bufferMemoryBarriers = static_cast<const VkBufferMemoryBarrier *>(ptrData);
    ptrData                           = reinterpret_cast<char *>(ptrData) + buffBarrierSize;
    memcpy(ptrData, imageMemoryBarriers, imgBarrierSize);
    paramStruct->imageMemoryBarriers = static_cast<const VkImageMemoryBarrier *>(ptrData);
}

void LiteCommandBuffer::resetQueryPool(VkQueryPool queryPool,
                                       uint32_t firstQuery,
                                       uint32_t queryCount)
{
    ResetQueryPoolParams *paramStruct = reinterpret_cast<ResetQueryPoolParams *>(
        initCommand(CommandID::ResetQueryPool, 0));
    paramStruct->queryPool  = queryPool;
    paramStruct->firstQuery = firstQuery;
    paramStruct->queryCount = queryCount;
}

void LiteCommandBuffer::beginQuery(VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags)
{
    BeginQueryParams *paramStruct =
        reinterpret_cast<BeginQueryParams *>(initCommand(CommandID::BeginQuery, 0));
    paramStruct->queryPool = queryPool;
    paramStruct->query     = query;
    paramStruct->flags     = flags;
}

void LiteCommandBuffer::endQuery(VkQueryPool queryPool, uint32_t query)
{
    EndQueryParams *paramStruct =
        reinterpret_cast<EndQueryParams *>(initCommand(CommandID::EndQuery, 0));
    paramStruct->queryPool = queryPool;
    paramStruct->query     = query;
}

void LiteCommandBuffer::writeTimestamp(VkPipelineStageFlagBits pipelineStage,
                                       VkQueryPool queryPool,
                                       uint32_t query)
{
    WriteTimestampParams *paramStruct =
        reinterpret_cast<WriteTimestampParams *>(initCommand(CommandID::WriteTimestamp, 0));
    paramStruct->pipelineStage = pipelineStage;
    paramStruct->queryPool     = queryPool;
    paramStruct->query         = query;
}

// Parse the cmds in this cmd buffer into given primary cmd buffer
void LiteCommandBuffer::parse(VkCommandBuffer cmdBuffer)
{
    CommandHeader *currentCommand = mHead;
    while (currentCommand)
    {
        switch (currentCommand->id)
        {
            case CommandID::BindDescriptorSets:
            {
                BindDescriptorSetParams *params =
                    getParamPtr<BindDescriptorSetParams>(currentCommand);
                vkCmdBindDescriptorSets(cmdBuffer, params->bindPoint, params->layout,
                                        params->firstSet, params->descriptorSetCount,
                                        params->descriptorSets, params->dynamicOffsetCount,
                                        params->dynamicOffsets);
                break;
            }
            case CommandID::BindIndexBuffer:
            {
                BindIndexBufferParams *params = getParamPtr<BindIndexBufferParams>(currentCommand);
                vkCmdBindIndexBuffer(cmdBuffer, params->buffer, params->offset, params->indexType);
                break;
            }
            case CommandID::BindPipeline:
            {
                BindPipelineParams *params = getParamPtr<BindPipelineParams>(currentCommand);
                vkCmdBindPipeline(cmdBuffer, params->pipelineBindPoint, params->pipeline);
                break;
            }
            case CommandID::BindVertexBuffers:
            {
                BindVertexBuffersParams *params =
                    getParamPtr<BindVertexBuffersParams>(currentCommand);
                vkCmdBindVertexBuffers(cmdBuffer, params->firstBinding, params->bindingCount,
                                       params->buffers, params->offsets);
                break;
            }
            case CommandID::BlitImage:
            {
                BlitImageParams *params = getParamPtr<BlitImageParams>(currentCommand);
                vkCmdBlitImage(cmdBuffer, params->srcImage, params->srcImageLayout,
                               params->dstImage, params->dstImageLayout, params->regionCount,
                               params->pRegions, params->filter);
                break;
            }
            case CommandID::CopyBuffer:
            {
                CopyBufferParams *params = getParamPtr<CopyBufferParams>(currentCommand);
                vkCmdCopyBuffer(cmdBuffer, params->srcBuffer, params->destBuffer,
                                params->regionCount, params->regions);
                break;
            }
            case CommandID::CopyBufferToImage:
            {
                CopyBufferToImageParams *params =
                    getParamPtr<CopyBufferToImageParams>(currentCommand);
                vkCmdCopyBufferToImage(cmdBuffer, params->srcBuffer, params->dstImage,
                                       params->dstImageLayout, params->regionCount,
                                       params->regions);
                break;
            }
            case CommandID::CopyImage:
            {
                CopyImageParams *params = getParamPtr<CopyImageParams>(currentCommand);
                vkCmdCopyImage(cmdBuffer, params->srcImage, params->srcImageLayout,
                               params->dstImage, params->dstImageLayout, params->regionCount,
                               params->regions);
                break;
            }
            case CommandID::CopyImageToBuffer:
            {
                CopyImageToBufferParams *params =
                    getParamPtr<CopyImageToBufferParams>(currentCommand);
                vkCmdCopyImageToBuffer(cmdBuffer, params->srcImage, params->srcImageLayout,
                                       params->dstBuffer, params->regionCount, params->regions);
                break;
            }
            case CommandID::ClearAttachments:
            {
                ClearAttachmentsParams *params =
                    getParamPtr<ClearAttachmentsParams>(currentCommand);
                vkCmdClearAttachments(cmdBuffer, params->attachmentCount, params->attachments,
                                      params->rectCount, params->rects);
                break;
            }
            case CommandID::ClearColorImage:
            {
                ClearColorImageParams *params = getParamPtr<ClearColorImageParams>(currentCommand);
                vkCmdClearColorImage(cmdBuffer, params->image, params->imageLayout, &params->color,
                                     params->rangeCount, params->ranges);
                break;
            }
            case CommandID::ClearDepthStencilImage:
            {
                ClearDepthStencilImageParams *params =
                    getParamPtr<ClearDepthStencilImageParams>(currentCommand);
                vkCmdClearDepthStencilImage(cmdBuffer, params->image, params->imageLayout,
                                            &params->depthStencil, params->rangeCount,
                                            params->ranges);
                break;
            }
            case CommandID::UpdateBuffer:
            {
                UpdateBufferParams *params = getParamPtr<UpdateBufferParams>(currentCommand);
                vkCmdUpdateBuffer(cmdBuffer, params->buffer, params->dstOffset, params->dataSize,
                                  params->data);
                break;
            }
            case CommandID::PushConstants:
            {
                PushConstantsParams *params = getParamPtr<PushConstantsParams>(currentCommand);
                vkCmdPushConstants(cmdBuffer, params->layout, params->flag, params->offset,
                                   params->size, params->data);
                break;
            }
            case CommandID::SetViewport:
            {
                SetViewportParams *params = getParamPtr<SetViewportParams>(currentCommand);
                vkCmdSetViewport(cmdBuffer, params->firstViewport, params->viewportCount,
                                 params->viewports);
                break;
            }
            case CommandID::SetScissor:
            {
                SetScissorParams *params = getParamPtr<SetScissorParams>(currentCommand);
                vkCmdSetScissor(cmdBuffer, params->firstScissor, params->scissorCount,
                                params->scissors);
                break;
            }
            case CommandID::Draw:
            {
                DrawParams *params = getParamPtr<DrawParams>(currentCommand);
                vkCmdDraw(cmdBuffer, params->vertexCount, params->instanceCount,
                          params->firstVertex, params->firstInstance);
                break;
            }
            case CommandID::DrawIndexed:
            {
                DrawIndexedParams *params = getParamPtr<DrawIndexedParams>(currentCommand);
                vkCmdDrawIndexed(cmdBuffer, params->indexCount, params->instanceCount,
                                 params->firstIndex, params->vertexOffset, params->firstInstance);
                break;
            }
            case CommandID::Dispatch:
            {
                DispatchParams *params = getParamPtr<DispatchParams>(currentCommand);
                vkCmdDispatch(cmdBuffer, params->groupCountX, params->groupCountY,
                              params->groupCountZ);
                break;
            }
            case CommandID::PipelinBarrier:
            {
                PipelineBarrierParams *params = getParamPtr<PipelineBarrierParams>(currentCommand);
                vkCmdPipelineBarrier(cmdBuffer, params->srcStageMask, params->dstStageMask,
                                     params->dependencyFlags, params->memoryBarrierCount,
                                     params->memoryBarriers, params->bufferMemoryBarrierCount,
                                     params->bufferMemoryBarriers, params->imageMemoryBarrierCount,
                                     params->imageMemoryBarriers);
                break;
            }
            case CommandID::SetEvent:
            {
                SetEventParams *params = getParamPtr<SetEventParams>(currentCommand);
                vkCmdSetEvent(cmdBuffer, params->event, params->stageMask);
                break;
            }
            case CommandID::ResetEvent:
            {
                ResetEventParams *params = getParamPtr<ResetEventParams>(currentCommand);
                vkCmdResetEvent(cmdBuffer, params->event, params->stageMask);
                break;
            }
            case CommandID::WaitEvents:
            {
                WaitEventsParams *params = getParamPtr<WaitEventsParams>(currentCommand);
                vkCmdWaitEvents(cmdBuffer, params->eventCount, params->events, params->srcStageMask,
                                params->dstStageMask, params->memoryBarrierCount,
                                params->memoryBarriers, params->bufferMemoryBarrierCount,
                                params->bufferMemoryBarriers, params->imageMemoryBarrierCount,
                                params->imageMemoryBarriers);
                break;
            }
            case CommandID::ResetQueryPool:
            {
                ResetQueryPoolParams *params = getParamPtr<ResetQueryPoolParams>(currentCommand);
                vkCmdResetQueryPool(cmdBuffer, params->queryPool, params->firstQuery,
                                    params->queryCount);
                break;
            }
            case CommandID::BeginQuery:
            {
                BeginQueryParams *params = getParamPtr<BeginQueryParams>(currentCommand);
                vkCmdBeginQuery(cmdBuffer, params->queryPool, params->query, params->flags);
                break;
            }
            case CommandID::EndQuery:
            {
                EndQueryParams *params = getParamPtr<EndQueryParams>(currentCommand);
                vkCmdEndQuery(cmdBuffer, params->queryPool, params->query);
                break;
            }
            case CommandID::WriteTimestamp:
            {
                WriteTimestampParams *params = getParamPtr<WriteTimestampParams>(currentCommand);
                vkCmdWriteTimestamp(cmdBuffer, params->pipelineStage, params->queryPool,
                                    params->query);
                break;
            }
            default:
            {
                break;
            }
        }
        currentCommand = currentCommand->next;
    }
}
#endif
}  // namespace vk
}  // namespace rx
