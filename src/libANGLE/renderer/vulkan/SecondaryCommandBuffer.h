//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SecondaryCommandBuffer:
//    Lightweight, CPU-Side command buffers used to hold command state until
//    it has to be submitted to GPU.
//

#ifndef LIBANGLE_RENDERER_VULKAN_SECONDARYCOMMANDBUFFERVK_H_
#define LIBANGLE_RENDERER_VULKAN_SECONDARYCOMMANDBUFFERVK_H_

#include <vulkan/vulkan.h>

#include "common/PoolAlloc.h"
#include "libANGLE/renderer/vulkan/vk_wrapper.h"

namespace rx
{

namespace vk
{

enum class CommandID
{
    // State update cmds
    BindDescriptorSets = 0,
    BindIndexBuffer    = 1,
    // Specialized to Gfx & Compute BindPipeline cmds below
    // BindPipeline           = 2,
    BindVertexBuffers      = 3,
    BlitImage              = 4,
    CopyBuffer             = 5,
    CopyBufferToImage      = 6,
    CopyImage              = 7,
    CopyImageToBuffer      = 8,
    ClearAttachments       = 9,
    ClearColorImage        = 10,
    ClearDepthStencilImage = 11,
    UpdateBuffer           = 12,
    PushConstants          = 13,
    SetViewport            = 14,
    SetScissor             = 15,
    // Draw/dispatch cmds
    Draw        = 16,
    DrawIndexed = 17,
    Dispatch    = 18,
    // Sync & Query cmds
    PipelineBarrier = 19,
    ResetEvent      = 20,
    SetEvent        = 21,
    WaitEvents      = 22,
    ResetQueryPool  = 23,
    BeginQuery      = 24,
    EndQuery        = 25,
    WriteTimestamp  = 26,
    // Custom commands for optimization
    ImageBarrier         = 27,
    BindGraphicsPipeline = 28,
    BindComputePipeline  = 29,
    DrawInstanced        = 30,
    DrawIndexedInstanced = 31,
};

// Structs to encapsulate parameters for different commands
// This makes it easy to know the size of params & to copy params
// TODO: Could optimize the size of some of these structs through bit-packing
//  and customizing sizing based on limited parameter sets used by ANGLE
struct BindDescriptorSetParams
{
    VkPipelineBindPoint bindPoint;
    VkPipelineLayout layout;
    uint32_t firstSet;
    uint32_t descriptorSetCount;
    const VkDescriptorSet *descriptorSets;
    uint32_t dynamicOffsetCount;
    const uint32_t *dynamicOffsets;
};

struct BindIndexBufferParams
{
    VkBuffer buffer;
    VkDeviceSize offset;
    VkIndexType indexType;
};

struct BindPipelineParams
{
    VkPipeline pipeline;
};

struct BindVertexBuffersParams
{
    // ANGLE always has firstBinding of 0 so not storing that currently
    uint32_t bindingCount;
    const VkBuffer *buffers;
    const VkDeviceSize *offsets;
};

struct BlitImageParams
{
    VkImage srcImage;
    VkImageLayout srcImageLayout;
    VkImage dstImage;
    VkImageLayout dstImageLayout;
    uint32_t regionCount;
    const VkImageBlit *pRegions;
    VkFilter filter;
};

struct CopyBufferParams
{
    VkBuffer srcBuffer;
    VkBuffer destBuffer;
    uint32_t regionCount;
    const VkBufferCopy *regions;
};

struct CopyBufferToImageParams
{
    VkBuffer srcBuffer;
    VkImage dstImage;
    VkImageLayout dstImageLayout;
    uint32_t regionCount;
    const VkBufferImageCopy *regions;
};

struct CopyImageParams
{
    VkImage srcImage;
    VkImageLayout srcImageLayout;
    VkImage dstImage;
    VkImageLayout dstImageLayout;
    uint32_t regionCount;
    const VkImageCopy *regions;
};

struct CopyImageToBufferParams
{
    VkImage srcImage;
    VkImageLayout srcImageLayout;
    VkBuffer dstBuffer;
    uint32_t regionCount;
    const VkBufferImageCopy *regions;
};

struct ClearAttachmentsParams
{
    uint32_t attachmentCount;
    const VkClearAttachment *attachments;
    uint32_t rectCount;
    const VkClearRect *rects;
};

struct ClearColorImageParams
{
    VkImage image;
    VkImageLayout imageLayout;
    VkClearColorValue color;
    uint32_t rangeCount;
    const VkImageSubresourceRange *ranges;
};

struct ClearDepthStencilImageParams
{
    VkImage image;
    VkImageLayout imageLayout;
    VkClearDepthStencilValue depthStencil;
    uint32_t rangeCount;
    const VkImageSubresourceRange *ranges;
};

struct UpdateBufferParams
{
    VkBuffer buffer;
    VkDeviceSize dstOffset;
    VkDeviceSize dataSize;
    const void *data;
};

struct PushConstantsParams
{
    VkPipelineLayout layout;
    VkShaderStageFlags flag;
    uint32_t offset;
    uint32_t size;
    const void *data;
};

struct SetViewportParams
{
    uint32_t firstViewport;
    uint32_t viewportCount;
    const VkViewport *viewports;
};

struct SetScissorParams
{
    uint32_t firstScissor;
    uint32_t scissorCount;
    const VkRect2D *scissors;
};

struct DrawParams
{
    uint32_t vertexCount;
    uint32_t firstVertex;
};

struct DrawInstancedParams
{
    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
};

struct DrawIndexedParams
{
    uint32_t indexCount;
};

struct DrawIndexedInstancedParams
{
    uint32_t indexCount;
    uint32_t instanceCount;
};

struct DispatchParams
{
    uint32_t groupCountX;
    uint32_t groupCountY;
    uint32_t groupCountZ;
};

struct PipelineBarrierParams
{
    VkPipelineStageFlags srcStageMask;
    VkPipelineStageFlags dstStageMask;
    VkDependencyFlags dependencyFlags;
    uint32_t memoryBarrierCount;
    const VkMemoryBarrier *memoryBarriers;
    uint32_t bufferMemoryBarrierCount;
    const VkBufferMemoryBarrier *bufferMemoryBarriers;
    uint32_t imageMemoryBarrierCount;
    const VkImageMemoryBarrier *imageMemoryBarriers;
};

struct ImageBarrierParams
{
    VkPipelineStageFlags srcStageMask;
    VkPipelineStageFlags dstStageMask;
    VkImageMemoryBarrier imageMemoryBarrier;
};

struct SetEventParams
{
    VkEvent event;
    VkPipelineStageFlags stageMask;
};

struct ResetEventParams
{
    VkEvent event;
    VkPipelineStageFlags stageMask;
};

struct WaitEventsParams
{
    uint32_t eventCount;
    const VkEvent *events;
    VkPipelineStageFlags srcStageMask;
    VkPipelineStageFlags dstStageMask;
    uint32_t memoryBarrierCount;
    const VkMemoryBarrier *memoryBarriers;
    uint32_t bufferMemoryBarrierCount;
    const VkBufferMemoryBarrier *bufferMemoryBarriers;
    uint32_t imageMemoryBarrierCount;
    const VkImageMemoryBarrier *imageMemoryBarriers;
};

struct ResetQueryPoolParams
{
    VkQueryPool queryPool;
    uint32_t firstQuery;
    uint32_t queryCount;
};

struct BeginQueryParams
{
    VkQueryPool queryPool;
    uint32_t query;
    VkQueryControlFlags flags;
};

struct EndQueryParams
{
    VkQueryPool queryPool;
    uint32_t query;
};

struct WriteTimestampParams
{
    VkPipelineStageFlagBits pipelineStage;
    VkQueryPool queryPool;
    uint32_t query;
};

// Header for every cmd in custom cmd buffer
struct CommandHeader
{
    CommandID id;
    CommandHeader *next;
};

class SecondaryCommandBuffer final : angle::NonCopyable
{
  public:
    SecondaryCommandBuffer() : mHead(nullptr), mLast(nullptr), mAllocator(nullptr) {}
    ~SecondaryCommandBuffer() {}

    // Add commands
    ANGLE_INLINE void bindDescriptorSets(VkPipelineBindPoint bindPoint,
                                         const PipelineLayout &layout,
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
        paramStruct->layout             = layout.getHandle();
        paramStruct->firstSet           = firstSet;
        paramStruct->descriptorSetCount = descriptorSetCount;
        paramStruct->dynamicOffsetCount = dynamicOffsetCount;
        // Copy variable sized data
        storePointerParameter(descriptorSets, &paramStruct->descriptorSets, descSize);
        storePointerParameter(dynamicOffsets, &paramStruct->dynamicOffsets, offsetSize);
    }

    ANGLE_INLINE void bindIndexBuffer(const Buffer &buffer,
                                      VkDeviceSize offset,
                                      VkIndexType indexType)
    {
        BindIndexBufferParams *paramStruct =
            initCommand<BindIndexBufferParams>(CommandID::BindIndexBuffer);
        paramStruct->buffer    = buffer.getHandle();
        paramStruct->offset    = offset;
        paramStruct->indexType = indexType;
    }

    ANGLE_INLINE void bindGraphicsPipeline(const Pipeline &pipeline)
    {
        BindPipelineParams *paramStruct =
            initCommand<BindPipelineParams>(CommandID::BindGraphicsPipeline);
        paramStruct->pipeline = pipeline.getHandle();
    }

    ANGLE_INLINE void bindComputePipeline(const Pipeline &pipeline)
    {
        BindPipelineParams *paramStruct =
            initCommand<BindPipelineParams>(CommandID::BindComputePipeline);
        paramStruct->pipeline = pipeline.getHandle();
    }

    ANGLE_INLINE void bindVertexBuffers(uint32_t firstBinding,
                                        uint32_t bindingCount,
                                        const VkBuffer *buffers,
                                        const VkDeviceSize *offsets)
    {
        ASSERT(firstBinding == 0);
        size_t buffSize                      = bindingCount * sizeof(VkBuffer);
        size_t offsetSize                    = bindingCount * sizeof(VkDeviceSize);
        BindVertexBuffersParams *paramStruct = initCommand<BindVertexBuffersParams>(
            CommandID::BindVertexBuffers, buffSize + offsetSize);
        // Copy params
        paramStruct->bindingCount = bindingCount;
        // Copy variable sized data
        storePointerParameter(buffers, &paramStruct->buffers, buffSize);
        storePointerParameter(offsets, &paramStruct->offsets, offsetSize);
    }

    void blitImage(const Image &srcImage,
                   VkImageLayout srcImageLayout,
                   const Image &dstImage,
                   VkImageLayout dstImageLayout,
                   uint32_t regionCount,
                   const VkImageBlit *pRegions,
                   VkFilter filter);

    void copyBuffer(const Buffer &srcBuffer,
                    const Buffer &destBuffer,
                    uint32_t regionCount,
                    const VkBufferCopy *regions);

    void copyBufferToImage(VkBuffer srcBuffer,
                           const Image &dstImage,
                           VkImageLayout dstImageLayout,
                           uint32_t regionCount,
                           const VkBufferImageCopy *regions);

    void copyImage(const Image &srcImage,
                   VkImageLayout srcImageLayout,
                   const Image &dstImage,
                   VkImageLayout dstImageLayout,
                   uint32_t regionCount,
                   const VkImageCopy *regions);

    void copyImageToBuffer(const Image &srcImage,
                           VkImageLayout srcImageLayout,
                           VkBuffer dstBuffer,
                           uint32_t regionCount,
                           const VkBufferImageCopy *regions);

    void clearAttachments(uint32_t attachmentCount,
                          const VkClearAttachment *attachments,
                          uint32_t rectCount,
                          const VkClearRect *rects);

    void clearColorImage(const Image &image,
                         VkImageLayout imageLayout,
                         const VkClearColorValue &color,
                         uint32_t rangeCount,
                         const VkImageSubresourceRange *ranges);

    void clearDepthStencilImage(const Image &image,
                                VkImageLayout imageLayout,
                                const VkClearDepthStencilValue &depthStencil,
                                uint32_t rangeCount,
                                const VkImageSubresourceRange *ranges);

    void updateBuffer(const Buffer &buffer,
                      VkDeviceSize dstOffset,
                      VkDeviceSize dataSize,
                      const void *data);

    void pushConstants(const PipelineLayout &layout,
                       VkShaderStageFlags flag,
                       uint32_t offset,
                       uint32_t size,
                       const void *data);

    void setViewport(uint32_t firstViewport, uint32_t viewportCount, const VkViewport *viewports);
    void setScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D *scissors);

    ANGLE_INLINE void draw(uint32_t vertexCount, uint32_t firstVertex)
    {
        DrawParams *paramStruct  = initCommand<DrawParams>(CommandID::Draw);
        paramStruct->vertexCount = vertexCount;
        paramStruct->firstVertex = firstVertex;
    }

    ANGLE_INLINE void drawInstanced(uint32_t vertexCount,
                                    uint32_t instanceCount,
                                    uint32_t firstVertex)
    {
        DrawInstancedParams *paramStruct =
            initCommand<DrawInstancedParams>(CommandID::DrawInstanced);
        paramStruct->vertexCount   = vertexCount;
        paramStruct->instanceCount = instanceCount;
        paramStruct->firstVertex   = firstVertex;
    }

    ANGLE_INLINE void drawIndexed(uint32_t indexCount)
    {
        DrawIndexedParams *paramStruct = initCommand<DrawIndexedParams>(CommandID::DrawIndexed);
        paramStruct->indexCount        = indexCount;
    }

    ANGLE_INLINE void drawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount)
    {
        DrawIndexedInstancedParams *paramStruct =
            initCommand<DrawIndexedInstancedParams>(CommandID::DrawIndexedInstanced);
        paramStruct->indexCount    = indexCount;
        paramStruct->instanceCount = instanceCount;
    }

    void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

    void pipelineBarrier(VkPipelineStageFlags srcStageMask,
                         VkPipelineStageFlags dstStageMask,
                         VkDependencyFlags dependencyFlags,
                         uint32_t memoryBarrierCount,
                         const VkMemoryBarrier *memoryBarriers,
                         uint32_t bufferMemoryBarrierCount,
                         const VkBufferMemoryBarrier *bufferMemoryBarriers,
                         uint32_t imageMemoryBarrierCount,
                         const VkImageMemoryBarrier *imageMemoryBarriers);

    void imageBarrier(VkPipelineStageFlags srcStageMask,
                      VkPipelineStageFlags dstStageMask,
                      VkImageMemoryBarrier *imageMemoryBarrier);

    void setEvent(VkEvent event, VkPipelineStageFlags stageMask);
    void resetEvent(VkEvent event, VkPipelineStageFlags stageMask);
    void waitEvents(uint32_t eventCount,
                    const VkEvent *events,
                    VkPipelineStageFlags srcStageMask,
                    VkPipelineStageFlags dstStageMask,
                    uint32_t memoryBarrierCount,
                    const VkMemoryBarrier *memoryBarriers,
                    uint32_t bufferMemoryBarrierCount,
                    const VkBufferMemoryBarrier *bufferMemoryBarriers,
                    uint32_t imageMemoryBarrierCount,
                    const VkImageMemoryBarrier *imageMemoryBarriers);

    void resetQueryPool(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
    void beginQuery(VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags);
    void endQuery(VkQueryPool queryPool, uint32_t query);
    void writeTimestamp(VkPipelineStageFlagBits pipelineStage,
                        VkQueryPool queryPool,
                        uint32_t query);

    // No-op for compatibility
    VkResult end() { return VK_SUCCESS; }

    // Parse the cmds in this cmd buffer into given primary cmd buffer for execution
    void executeCommands(VkCommandBuffer cmdBuffer);

    // Initialize the SecondaryCommandBuffer by setting the allocator it will use
    void initialize(angle::PoolAllocator *allocator) { mAllocator = allocator; }
    // This will cause the SecondaryCommandBuffer to become invalid by clearing its allocator
    void releaseHandle() { mAllocator = nullptr; }
    // The SecondaryCommandBuffer is valid if it's been initialized
    bool valid() { return mAllocator != nullptr; }

  private:
    ANGLE_INLINE CommandHeader *commonInit(CommandID cmdID, size_t allocSize)
    {
        ASSERT(mAllocator != nullptr);
        CommandHeader *header = static_cast<CommandHeader *>(mAllocator->fastAllocate(allocSize));
        // Update cmd ID in header
        header->id   = cmdID;
        header->next = nullptr;
        // Update prev cmd's "next" ptr and mLast ptr
        if (mLast)
        {
            mLast->next = header;
        }
        else  // This is first cmd of Cmd Buffer so update mHead
        {
            ASSERT(mHead == nullptr);
            mHead = header;
        }
        // Update mLast ptr
        mLast = header;
        return header;
    }
    // Allocate and initialize memory for given commandID & variable param size
    //  returning a pointer to the start of the commands parameter data and updating
    //  mPtrCmdData to just past the fixed parameter data.
    template <class StructType>
    ANGLE_INLINE StructType *initCommand(CommandID cmdID, size_t variableSize)
    {
        size_t paramSize      = sizeof(StructType);
        CommandHeader *header = commonInit(cmdID, sizeof(CommandHeader) + paramSize + variableSize);

        uint8_t *fixedParamPtr = reinterpret_cast<uint8_t *>(header) + sizeof(CommandHeader);
        mPtrCmdData            = fixedParamPtr + sizeof(StructType);
        return reinterpret_cast<StructType *>(fixedParamPtr);
    }
    // Initialize a command that doesn't have variable-sized ptr data
    template <class StructType>
    ANGLE_INLINE StructType *initCommand(CommandID cmdID)
    {
        CommandHeader *header = commonInit(cmdID, sizeof(StructType) + sizeof(CommandHeader));
        return reinterpret_cast<StructType *>(reinterpret_cast<uint8_t *>(header) +
                                              sizeof(CommandHeader));
    }
    // Return a ptr to the parameter type
    template <class StructType>
    StructType *getParamPtr(CommandHeader *header)
    {
        return reinterpret_cast<StructType *>(reinterpret_cast<char *>(header) +
                                              sizeof(CommandHeader));
    }
    // Copy sizeInBytes data from paramData to mPtrCmdData and assign *writePtr
    //  to mPtrCmdData. Then increment mPtrCmdData by sizeInBytes.
    // Precondition: initCommand() must have already been called on the given cmd
    template <class PtrType>
    void storePointerParameter(const PtrType *paramData,
                               const PtrType **writePtr,
                               size_t sizeInBytes)
    {
        if (0 == sizeInBytes)
            return;
        *writePtr = reinterpret_cast<const PtrType *>(mPtrCmdData);
        memcpy(mPtrCmdData, paramData, sizeInBytes);
        mPtrCmdData += sizeInBytes;
    }

    // Pointer to start of cmd buffer
    CommandHeader *mHead;
    // Last command inserted in cmd buffer
    CommandHeader *mLast;
    // Allocator used by this class. If non-null then the class is valid.
    angle::PoolAllocator *mAllocator;
    // Ptr to write variable ptr data section of cmd into.
    //  This is set to just past fixed parameter data when initCommand() is called
    uint8_t *mPtrCmdData;
};

}  // namespace vk
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_SECONDARYCOMMANDBUFFERVK_H_
