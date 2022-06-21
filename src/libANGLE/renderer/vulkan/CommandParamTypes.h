//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CommandParamTypes.h:
//    Includes command ID and parameter types used in command buffers and allocator managers.
//

#ifndef LIBANGLE_RENDERER_VULKAN_ALLOCATORMANAGERUTILS_H_
#define LIBANGLE_RENDERER_VULKAN_ALLOCATORMANAGERUTILS_H_

#include "common/vulkan/vk_headers.h"
#include "libANGLE/renderer/vulkan/vk_command_buffer_utils.h"
#include "libANGLE/renderer/vulkan/vk_wrapper.h"

namespace rx
{
namespace vk
{

namespace priv
{

// NOTE: Please keep command-related enums, structs, functions
//  and other code dealing with commands in alphabetical order
//  This simplifies searching and updating commands.
enum class CommandID : uint16_t
{
    // Invalid cmd used to mark end of sequence of commands
    Invalid = 0,
    BeginDebugUtilsLabel,
    BeginQuery,
    BeginTransformFeedback,
    BindComputePipeline,
    BindDescriptorSets,
    BindGraphicsPipeline,
    BindIndexBuffer,
    BindTransformFeedbackBuffers,
    BindVertexBuffers,
    BindVertexBuffers2,
    BlitImage,
    BufferBarrier,
    ClearAttachments,
    ClearColorImage,
    ClearDepthStencilImage,
    CopyBuffer,
    CopyBufferToImage,
    CopyImage,
    CopyImageToBuffer,
    Dispatch,
    DispatchIndirect,
    Draw,
    DrawIndexed,
    DrawIndexedBaseVertex,
    DrawIndexedIndirect,
    DrawIndexedInstanced,
    DrawIndexedInstancedBaseVertex,
    DrawIndexedInstancedBaseVertexBaseInstance,
    DrawIndirect,
    DrawInstanced,
    DrawInstancedBaseInstance,
    EndDebugUtilsLabel,
    EndQuery,
    EndTransformFeedback,
    FillBuffer,
    ImageBarrier,
    InsertDebugUtilsLabel,
    MemoryBarrier,
    NextSubpass,
    PipelineBarrier,
    PushConstants,
    ResetEvent,
    ResetQueryPool,
    ResolveImage,
    SetBlendConstants,
    SetCullMode,
    SetDepthBias,
    SetDepthBiasEnable,
    SetDepthCompareOp,
    SetDepthTestEnable,
    SetDepthWriteEnable,
    SetEvent,
    SetFragmentShadingRate,
    SetFrontFace,
    SetLineWidth,
    SetLogicOp,
    SetPrimitiveRestartEnable,
    SetRasterizerDiscardEnable,
    SetScissor,
    SetStencilCompareMask,
    SetStencilOp,
    SetStencilReference,
    SetStencilTestEnable,
    SetStencilWriteMask,
    SetViewport,
    WaitEvents,
    WriteTimestamp,
};

#define VERIFY_4_BYTE_ALIGNMENT(StructName) \
    static_assert((sizeof(StructName) % 4) == 0, "Check StructName alignment");

// Structs to encapsulate parameters for different commands
// This makes it easy to know the size of params & to copy params
// TODO: Could optimize the size of some of these structs through bit-packing
//  and customizing sizing based on limited parameter sets used by ANGLE
struct BeginQueryParams
{
    VkQueryPool queryPool;
    uint32_t query;
    VkQueryControlFlags flags;
};
VERIFY_4_BYTE_ALIGNMENT(BeginQueryParams)

struct BeginTransformFeedbackParams
{
    uint32_t bufferCount;
};
VERIFY_4_BYTE_ALIGNMENT(BeginTransformFeedbackParams)

struct BindDescriptorSetParams
{
    VkPipelineLayout layout;
    VkPipelineBindPoint pipelineBindPoint;
    uint32_t firstSet;
    uint32_t descriptorSetCount;
    uint32_t dynamicOffsetCount;
};
VERIFY_4_BYTE_ALIGNMENT(BindDescriptorSetParams)

struct BindIndexBufferParams
{
    VkBuffer buffer;
    VkDeviceSize offset;
    VkIndexType indexType;
};
VERIFY_4_BYTE_ALIGNMENT(BindIndexBufferParams)

struct BindPipelineParams
{
    VkPipeline pipeline;
};
VERIFY_4_BYTE_ALIGNMENT(BindPipelineParams)

struct BindTransformFeedbackBuffersParams
{
    // ANGLE always has firstBinding of 0 so not storing that currently
    uint32_t bindingCount;
};
VERIFY_4_BYTE_ALIGNMENT(BindTransformFeedbackBuffersParams)

using BindVertexBuffersParams  = BindTransformFeedbackBuffersParams;
using BindVertexBuffers2Params = BindVertexBuffersParams;

struct BlitImageParams
{
    VkImage srcImage;
    VkImage dstImage;
    VkFilter filter;
    VkImageBlit region;
};
VERIFY_4_BYTE_ALIGNMENT(BlitImageParams)

struct BufferBarrierParams
{
    VkPipelineStageFlags srcStageMask;
    VkPipelineStageFlags dstStageMask;
    VkBufferMemoryBarrier bufferMemoryBarrier;
};
VERIFY_4_BYTE_ALIGNMENT(BufferBarrierParams)

struct ClearAttachmentsParams
{
    uint32_t attachmentCount;
    VkClearRect rect;
};
VERIFY_4_BYTE_ALIGNMENT(ClearAttachmentsParams)

struct ClearColorImageParams
{
    VkImage image;
    VkImageLayout imageLayout;
    VkClearColorValue color;
    VkImageSubresourceRange range;
};
VERIFY_4_BYTE_ALIGNMENT(ClearColorImageParams)

struct ClearDepthStencilImageParams
{
    VkImage image;
    VkImageLayout imageLayout;
    VkClearDepthStencilValue depthStencil;
    VkImageSubresourceRange range;
};
VERIFY_4_BYTE_ALIGNMENT(ClearDepthStencilImageParams)

struct CopyBufferParams
{
    VkBuffer srcBuffer;
    VkBuffer destBuffer;
    uint32_t regionCount;
};
VERIFY_4_BYTE_ALIGNMENT(CopyBufferParams)

struct CopyBufferToImageParams
{
    VkBuffer srcBuffer;
    VkImage dstImage;
    VkImageLayout dstImageLayout;
    VkBufferImageCopy region;
};
VERIFY_4_BYTE_ALIGNMENT(CopyBufferToImageParams)

struct CopyImageParams
{
    VkImage srcImage;
    VkImageLayout srcImageLayout;
    VkImage dstImage;
    VkImageLayout dstImageLayout;
    VkImageCopy region;
};
VERIFY_4_BYTE_ALIGNMENT(CopyImageParams)

struct CopyImageToBufferParams
{
    VkImage srcImage;
    VkImageLayout srcImageLayout;
    VkBuffer dstBuffer;
    VkBufferImageCopy region;
};
VERIFY_4_BYTE_ALIGNMENT(CopyImageToBufferParams)

// This is a common struct used by both begin & insert DebugUtilsLabelEXT() functions
struct DebugUtilsLabelParams
{
    float color[4];
};
VERIFY_4_BYTE_ALIGNMENT(DebugUtilsLabelParams)

struct DispatchParams
{
    uint32_t groupCountX;
    uint32_t groupCountY;
    uint32_t groupCountZ;
};
VERIFY_4_BYTE_ALIGNMENT(DispatchParams)

struct DispatchIndirectParams
{
    VkBuffer buffer;
    VkDeviceSize offset;
};
VERIFY_4_BYTE_ALIGNMENT(DispatchIndirectParams)

struct DrawParams
{
    uint32_t vertexCount;
    uint32_t firstVertex;
};
VERIFY_4_BYTE_ALIGNMENT(DrawParams)

struct DrawIndexedParams
{
    uint32_t indexCount;
};
VERIFY_4_BYTE_ALIGNMENT(DrawIndexedParams)

struct DrawIndexedBaseVertexParams
{
    uint32_t indexCount;
    uint32_t vertexOffset;
};
VERIFY_4_BYTE_ALIGNMENT(DrawIndexedBaseVertexParams)

struct DrawIndexedIndirectParams
{
    VkBuffer buffer;
    VkDeviceSize offset;
    uint32_t drawCount;
    uint32_t stride;
};
VERIFY_4_BYTE_ALIGNMENT(DrawIndexedIndirectParams)

struct DrawIndexedInstancedParams
{
    uint32_t indexCount;
    uint32_t instanceCount;
};
VERIFY_4_BYTE_ALIGNMENT(DrawIndexedInstancedParams)

struct DrawIndexedInstancedBaseVertexParams
{
    uint32_t indexCount;
    uint32_t instanceCount;
    uint32_t vertexOffset;
};
VERIFY_4_BYTE_ALIGNMENT(DrawIndexedInstancedBaseVertexParams)

struct DrawIndexedInstancedBaseVertexBaseInstanceParams
{
    uint32_t indexCount;
    uint32_t instanceCount;
    uint32_t firstIndex;
    int32_t vertexOffset;
    uint32_t firstInstance;
};
VERIFY_4_BYTE_ALIGNMENT(DrawIndexedInstancedBaseVertexBaseInstanceParams)

struct DrawIndirectParams
{
    VkBuffer buffer;
    VkDeviceSize offset;
    uint32_t drawCount;
    uint32_t stride;
};
VERIFY_4_BYTE_ALIGNMENT(DrawIndirectParams)

struct DrawInstancedParams
{
    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
};
VERIFY_4_BYTE_ALIGNMENT(DrawInstancedParams)

struct DrawInstancedBaseInstanceParams
{
    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
    uint32_t firstInstance;
};
VERIFY_4_BYTE_ALIGNMENT(DrawInstancedBaseInstanceParams)

// A special struct used with commands that don't have params
struct EmptyParams
{};

struct EndQueryParams
{
    VkQueryPool queryPool;
    uint32_t query;
};
VERIFY_4_BYTE_ALIGNMENT(EndQueryParams)

struct EndTransformFeedbackParams
{
    uint32_t bufferCount;
};
VERIFY_4_BYTE_ALIGNMENT(EndTransformFeedbackParams)

struct FillBufferParams
{
    VkBuffer dstBuffer;
    VkDeviceSize dstOffset;
    VkDeviceSize size;
    uint32_t data;
};
VERIFY_4_BYTE_ALIGNMENT(FillBufferParams)

struct ImageBarrierParams
{
    VkPipelineStageFlags srcStageMask;
    VkPipelineStageFlags dstStageMask;
    VkImageMemoryBarrier imageMemoryBarrier;
};
VERIFY_4_BYTE_ALIGNMENT(ImageBarrierParams)

struct MemoryBarrierParams
{
    VkPipelineStageFlags srcStageMask;
    VkPipelineStageFlags dstStageMask;
    VkMemoryBarrier memoryBarrier;
};
VERIFY_4_BYTE_ALIGNMENT(MemoryBarrierParams)

struct NextSubpassParams
{
    VkSubpassContents subpassContents;
};
VERIFY_4_BYTE_ALIGNMENT(NextSubpassParams)

struct PipelineBarrierParams
{
    VkPipelineStageFlags srcStageMask;
    VkPipelineStageFlags dstStageMask;
    VkDependencyFlags dependencyFlags;
    uint32_t memoryBarrierCount;
    uint32_t bufferMemoryBarrierCount;
    uint32_t imageMemoryBarrierCount;
};
VERIFY_4_BYTE_ALIGNMENT(PipelineBarrierParams)

struct PushConstantsParams
{
    VkPipelineLayout layout;
    VkShaderStageFlags flag;
    uint32_t offset;
    uint32_t size;
};
VERIFY_4_BYTE_ALIGNMENT(PushConstantsParams)

struct ResetEventParams
{
    VkEvent event;
    VkPipelineStageFlags stageMask;
};
VERIFY_4_BYTE_ALIGNMENT(ResetEventParams)

struct ResetQueryPoolParams
{
    VkQueryPool queryPool;
    uint32_t firstQuery;
    uint32_t queryCount;
};
VERIFY_4_BYTE_ALIGNMENT(ResetQueryPoolParams)

struct ResolveImageParams
{
    VkImage srcImage;
    VkImage dstImage;
    VkImageResolve region;
};
VERIFY_4_BYTE_ALIGNMENT(ResolveImageParams)

struct SetBlendConstantsParams
{
    float blendConstants[4];
};
VERIFY_4_BYTE_ALIGNMENT(SetBlendConstantsParams)

struct SetCullModeParams
{
    VkCullModeFlags cullMode;
};
VERIFY_4_BYTE_ALIGNMENT(SetCullModeParams)

struct SetDepthBiasParams
{
    float depthBiasConstantFactor;
    float depthBiasClamp;
    float depthBiasSlopeFactor;
};
VERIFY_4_BYTE_ALIGNMENT(SetDepthBiasParams)

struct SetDepthBiasEnableParams
{
    VkBool32 depthBiasEnable;
};
VERIFY_4_BYTE_ALIGNMENT(SetDepthBiasEnableParams)

struct SetDepthCompareOpParams
{
    VkCompareOp depthCompareOp;
};
VERIFY_4_BYTE_ALIGNMENT(SetDepthCompareOpParams)

struct SetDepthTestEnableParams
{
    VkBool32 depthTestEnable;
};
VERIFY_4_BYTE_ALIGNMENT(SetDepthTestEnableParams)

struct SetDepthWriteEnableParams
{
    VkBool32 depthWriteEnable;
};
VERIFY_4_BYTE_ALIGNMENT(SetDepthWriteEnableParams)

struct SetEventParams
{
    VkEvent event;
    VkPipelineStageFlags stageMask;
};
VERIFY_4_BYTE_ALIGNMENT(SetEventParams)

struct SetFragmentShadingRateParams
{
    uint16_t fragmentWidth;
    uint16_t fragmentHeight;
};
VERIFY_4_BYTE_ALIGNMENT(SetFragmentShadingRateParams)

struct SetFrontFaceParams
{
    VkFrontFace frontFace;
};
VERIFY_4_BYTE_ALIGNMENT(SetFrontFaceParams)

struct SetLineWidthParams
{
    float lineWidth;
};
VERIFY_4_BYTE_ALIGNMENT(SetLineWidthParams)

struct SetLogicOpParams
{
    VkLogicOp logicOp;
};
VERIFY_4_BYTE_ALIGNMENT(SetLogicOpParams)

struct SetPrimitiveRestartEnableParams
{
    VkBool32 primitiveRestartEnable;
};
VERIFY_4_BYTE_ALIGNMENT(SetPrimitiveRestartEnableParams)

struct SetRasterizerDiscardEnableParams
{
    VkBool32 rasterizerDiscardEnable;
};
VERIFY_4_BYTE_ALIGNMENT(SetRasterizerDiscardEnableParams)

struct SetScissorParams
{
    VkRect2D scissor;
};
VERIFY_4_BYTE_ALIGNMENT(SetScissorParams)

struct SetStencilCompareMaskParams
{
    uint16_t compareFrontMask;
    uint16_t compareBackMask;
};
VERIFY_4_BYTE_ALIGNMENT(SetStencilCompareMaskParams)

struct SetStencilOpParams
{
    uint32_t faceMask : 4;
    uint32_t failOp : 3;
    uint32_t passOp : 3;
    uint32_t depthFailOp : 3;
    uint32_t compareOp : 3;
};
VERIFY_4_BYTE_ALIGNMENT(SetStencilOpParams)

struct SetStencilReferenceParams
{
    uint16_t frontReference;
    uint16_t backReference;
};
VERIFY_4_BYTE_ALIGNMENT(SetStencilReferenceParams)

struct SetStencilTestEnableParams
{
    VkBool32 stencilTestEnable;
};
VERIFY_4_BYTE_ALIGNMENT(SetStencilTestEnableParams)

struct SetStencilWriteMaskParams
{
    uint16_t writeFrontMask;
    uint16_t writeBackMask;
};
VERIFY_4_BYTE_ALIGNMENT(SetStencilWriteMaskParams)

struct SetViewportParams
{
    VkViewport viewport;
};
VERIFY_4_BYTE_ALIGNMENT(SetViewportParams)

struct WaitEventsParams
{
    uint32_t eventCount;
    VkPipelineStageFlags srcStageMask;
    VkPipelineStageFlags dstStageMask;
    uint32_t memoryBarrierCount;
    uint32_t bufferMemoryBarrierCount;
    uint32_t imageMemoryBarrierCount;
};
VERIFY_4_BYTE_ALIGNMENT(WaitEventsParams)

struct WriteTimestampParams
{
    VkPipelineStageFlagBits pipelineStage;
    VkQueryPool queryPool;
    uint32_t query;
};
VERIFY_4_BYTE_ALIGNMENT(WriteTimestampParams)

template <typename DestT, typename T>
ANGLE_INLINE DestT *Offset(T *ptr, size_t bytes)
{
    return reinterpret_cast<DestT *>((reinterpret_cast<uint8_t *>(ptr) + bytes));
}

template <typename DestT, typename T>
ANGLE_INLINE const DestT *Offset(const T *ptr, size_t bytes)
{
    return reinterpret_cast<const DestT *>((reinterpret_cast<const uint8_t *>(ptr) + bytes));
}

// Header for every cmd in custom cmd buffer
struct CommandHeader
{
    CommandID id;
    uint16_t size;
};
static_assert(sizeof(CommandHeader) == 4, "Check CommandHeader size");

ANGLE_INLINE const CommandHeader *NextCommand(const CommandHeader *command)
{
    return reinterpret_cast<const CommandHeader *>(reinterpret_cast<const uint8_t *>(command) +
                                                   command->size);
}

}  // namespace priv
}  // namespace vk
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_ALLOCATORMANAGERUTILS_H_
