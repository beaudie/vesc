//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// LiteCommandBuffer:
//    Lightweight, CPU-Side command buffers used to hold command state until
//    it has to be submitted to GPU.
//

#ifndef LIBANGLE_RENDERER_VULKAN_LITECOMMANDBUFFERVK_H_
#define LIBANGLE_RENDERER_VULKAN_LITECOMMANDBUFFERVK_H_

#include <vulkan/vulkan.h>

#include "common/PoolAlloc.h"

namespace rx
{

namespace vk
{

enum class CommandID
{
    // State update cmds
    CMD_ID_BIND_DESCRIPTOR_SETS      = 0,
    CMD_ID_BIND_INDEX_BUFFER         = 1,
    CMD_ID_BIND_PIPELINE             = 2,
    CMD_ID_BIND_VERTEX_BUFFERS       = 3,
    CMD_ID_BLIT_IMAGE                = 4,
    CMD_ID_COPY_BUFFER               = 5,
    CMD_ID_COPY_BUFFER_TO_IMAGE      = 6,
    CMD_ID_COPY_IMAGE                = 7,
    CMD_ID_COPY_IMAGE_TO_BUFFER      = 8,
    CMD_ID_CLEAR_ATTACHMENTS         = 9,
    CMD_ID_CLEAR_COLOR_IMAGE         = 10,
    CMD_ID_CLEAR_DEPTH_STENCIL_IMAGE = 11,
    CMD_ID_UPDATE_BUFFER             = 12,
    CMD_ID_PUSH_CONSTANTS            = 13,
    CMD_ID_SET_VIEWPORT              = 14,
    CMD_ID_SET_SCISSORS              = 15,
    // Draw/dispatch cmds
    CMD_ID_DRAW         = 16,
    CMD_ID_DRAW_INDEXED = 17,
    CMD_ID_DISPATCH     = 18,
    // Sync & Query cmds
    CMD_ID_PIPELINE_BARRIER = 19,
    CMD_ID_RESET_EVENT      = 20,
    CMD_ID_SET_EVENT        = 21,
    CMD_ID_WAIT_EVENTS      = 22,
    CMD_ID_RESET_QUERY_POOL = 23,
    CMD_ID_BEGIN_QUERY      = 24,
    CMD_ID_END_QUERY        = 25,
    CMD_ID_WRITE_TIMESTAMP  = 26,
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
    VkPipelineBindPoint pipelineBindPoint;
    VkPipeline pipeline;
};

struct BindVertexBuffersParams
{
    uint32_t firstBinding;
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
    VkImageBlit *pRegions;
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
    uint32_t instanceCount;
    uint32_t firstVertex;
    uint32_t firstInstance;
};

struct DrawIndexedParams
{
    uint32_t indexCount;
    uint32_t instanceCount;
    uint32_t firstIndex;
    int32_t vertexOffset;
    uint32_t firstInstance;
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

// Information about each
struct CommandInfo
{
    CommandID type;    // type of cmd
    size_t paramSize;  // size of cmd params
    bool copyPtrData;  // are any params pointers that need to have data copied?
};

static constexpr size_t CommandParamSizes[] = {
    // BIND_DESCRIPTOR_SETS      = 0,
    sizeof(BindDescriptorSetParams),
    // BIND_INDEX_BUFFER         = 1,
    sizeof(BindIndexBufferParams),
    // BIND_PIPELINE             = 2,
    sizeof(BindPipelineParams),
    // BIND_VERTEX_BUFFERS       = 3,
    sizeof(BindVertexBuffersParams),
    // BLIT_IMAGE                = 4,
    sizeof(BlitImageParams),
    // COPY_BUFFER               = 5,
    sizeof(CopyBufferParams),
    // COPY_BUFFER_TO_IMAGE      = 6,
    sizeof(CopyBufferToImageParams),
    // COPY_IMAGE                = 7,
    sizeof(CopyImageParams),
    // COPY_IMAGE_TO_BUFFER      = 8,
    sizeof(CopyImageToBufferParams),
    // CLEAR_ATTACHMENTS         = 9,
    sizeof(ClearAttachmentsParams),
    // CLEAR_COLOR_IMAGE         = 10,
    sizeof(ClearColorImageParams),
    // CLEAR_DEPTH_STENCIL_IMAGE = 11,
    sizeof(ClearDepthStencilImageParams),
    // UPDATE_BUFFER             = 12,
    sizeof(UpdateBufferParams),
    // PUSH_CONSTANTS            = 13,
    sizeof(PushConstantsParams),
    // SET_VIEWPORT              = 14,
    sizeof(SetViewportParams),
    // SET_SCISSORS              = 15,
    sizeof(SetScissorParams),
    // DRAW                      = 16,
    sizeof(DrawParams),
    // DRAW_INDEXED              = 17,
    sizeof(DrawIndexedParams),
    // DISPATCH                  = 18,
    sizeof(DispatchParams),
    // PIPELINE_BARRIER          = 19,
    sizeof(PipelineBarrierParams),
    // RESET_EVENT               = 20,
    sizeof(ResetEventParams),
    // SET_EVENT                 = 21,
    sizeof(SetEventParams),
    // WAIT_EVENTS               = 22,
    sizeof(WaitEventsParams),
    // RESET_QUERY_POOL          = 23,
    sizeof(ResetQueryPoolParams),
    // BEGIN_QUERY               = 24,
    sizeof(BeginQueryParams),
    // END_QUERY                 = 25,
    sizeof(EndQueryParams),
    // WRITE_TIMESTAMP           = 26,
    sizeof(WriteTimestampParams),
};

// Header for every cmd in custom cmd buffer
struct CommandHeader
{
    CommandID id : 8;
    CommandHeader *next;
};

class LiteCommandBuffer final : angle::NonCopyable
{
  public:
    LiteCommandBuffer() : mHead(nullptr), mLast(nullptr) {}
    ~LiteCommandBuffer() {}

    // Add commands
    void bindDescriptorSets(VkPipelineBindPoint bindPoint,
                            VkPipelineLayout layout,
                            uint32_t firstSet,
                            uint32_t descriptorSetCount,
                            const VkDescriptorSet *descriptorSets,
                            uint32_t dynamicOffsetCount,
                            const uint32_t *dynamicOffsets);

    void bindIndexBuffer(const VkBuffer &buffer, VkDeviceSize offset, VkIndexType indexType);

    void bindPipeline(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);

    void bindVertexBuffers(uint32_t firstBinding,
                           uint32_t bindingCount,
                           const VkBuffer *buffers,
                           const VkDeviceSize *offsets);

    void blitImage(VkImage srcImage,
                   VkImageLayout srcImageLayout,
                   VkImage dstImage,
                   VkImageLayout dstImageLayout,
                   uint32_t regionCount,
                   VkImageBlit *pRegions,
                   VkFilter filter);

    void copyBuffer(const VkBuffer &srcBuffer,
                    const VkBuffer &destBuffer,
                    uint32_t regionCount,
                    const VkBufferCopy *regions);

    void copyBufferToImage(VkBuffer srcBuffer,
                           VkImage dstImage,
                           VkImageLayout dstImageLayout,
                           uint32_t regionCount,
                           const VkBufferImageCopy *regions);

    void copyImage(VkImage srcImage,
                   VkImageLayout srcImageLayout,
                   VkImage dstImage,
                   VkImageLayout dstImageLayout,
                   uint32_t regionCount,
                   const VkImageCopy *regions);

    void copyImageToBuffer(VkImage srcImage,
                           VkImageLayout srcImageLayout,
                           VkBuffer dstBuffer,
                           uint32_t regionCount,
                           const VkBufferImageCopy *regions);

    void clearAttachments(uint32_t attachmentCount,
                          const VkClearAttachment *attachments,
                          uint32_t rectCount,
                          const VkClearRect *rects);

    void clearColorImage(VkImage image,
                         VkImageLayout imageLayout,
                         const VkClearColorValue &color,
                         uint32_t rangeCount,
                         const VkImageSubresourceRange *ranges);

    void clearDepthStencilImage(VkImage image,
                                VkImageLayout imageLayout,
                                const VkClearDepthStencilValue &depthStencil,
                                uint32_t rangeCount,
                                const VkImageSubresourceRange *ranges);

    void updateBuffer(VkBuffer buffer,
                      VkDeviceSize dstOffset,
                      VkDeviceSize dataSize,
                      const void *data);

    void pushConstants(VkPipelineLayout layout,
                       VkShaderStageFlags flag,
                       uint32_t offset,
                       uint32_t size,
                       const void *data);

    void setViewport(uint32_t firstViewport, uint32_t viewportCount, const VkViewport *viewports);
    void setScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D *scissors);

    void draw(uint32_t vertexCount,
              uint32_t instanceCount,
              uint32_t firstVertex,
              uint32_t firstInstance);

    void drawIndexed(uint32_t indexCount,
                     uint32_t instanceCount,
                     uint32_t firstIndex,
                     int32_t vertexOffset,
                     uint32_t firstInstance);

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

    // Parse the cmds in this cmd buffer into given primary cmd buffer
    void parse(VkCommandBuffer cmdBuffer);

  private:
    // Pointer to start of cmd buffer
    CommandHeader *mHead;
    // Last command inserted in cmd buffer
    CommandHeader *mLast;
    angle::PoolAllocator mAllocator;

    // Allocate and initialize memory for given commandID & variable param size
    //  returning a pointer to the start of the commands parameter data
    void *initCommand(CommandID cmdID, size_t variableSize);
    // Return a ptr to the parameter type
    template <class structType>
    structType *getParamPtr(CommandHeader *header)
    {
        return reinterpret_cast<structType *>(reinterpret_cast<char *>(header) +
                                              sizeof(CommandHeader));
    }
    // For the given command ID and ptr to that commands parameter data, return a pointer
    //  to that command's variable size data, which is just past the parameter data.
    void *getCmdVariableDataPtr(CommandID cmdID, void *paramPtr);
};

}  // namespace vk
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_LITECOMMANDBUFFERVK_H_
