//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vk_wrapper:
//    Wrapper classes around Vulkan objects. In an ideal world we could generate this
//    from vk.xml. Or reuse the generator in the vkhpp tool. For now this is manually
//    generated and we must add missing functions and objects as we need them.

#ifndef LIBANGLE_RENDERER_VULKAN_VK_WRAPPER_H_
#define LIBANGLE_RENDERER_VULKAN_VK_WRAPPER_H_

#include "common/vulkan/vk_headers.h"
#include "libANGLE/renderer/renderer_utils.h"
#include "libANGLE/renderer/vulkan/vk_mem_alloc_wrapper.h"
#include "libANGLE/trace.h"

namespace rx
{
enum class DescriptorSetIndex : uint32_t;

namespace vk
{
// Helper macros that apply to all the wrapped object types.
// Unimplemented handle types:
// Instance
// PhysicalDevice
// Device
// Queue
// DescriptorSet

#define ANGLE_HANDLE_TYPES_X(FUNC) \
    FUNC(Allocation)               \
    FUNC(Allocator)                \
    FUNC(Buffer)                   \
    FUNC(BufferView)               \
    FUNC(CommandPool)              \
    FUNC(DescriptorPool)           \
    FUNC(DescriptorSetLayout)      \
    FUNC(DeviceMemory)             \
    FUNC(Event)                    \
    FUNC(Fence)                    \
    FUNC(Framebuffer)              \
    FUNC(Image)                    \
    FUNC(ImageView)                \
    FUNC(Pipeline)                 \
    FUNC(PipelineCache)            \
    FUNC(PipelineLayout)           \
    FUNC(QueryPool)                \
    FUNC(RenderPass)               \
    FUNC(Sampler)                  \
    FUNC(SamplerYcbcrConversion)   \
    FUNC(Semaphore)                \
    FUNC(ShaderModule)

#define ANGLE_COMMA_SEP_FUNC(TYPE) TYPE,

enum class HandleType
{
    Invalid,
    CommandBuffer,
    ANGLE_HANDLE_TYPES_X(ANGLE_COMMA_SEP_FUNC) EnumCount
};

#undef ANGLE_COMMA_SEP_FUNC

#define ANGLE_PRE_DECLARE_CLASS_FUNC(TYPE) class TYPE;
ANGLE_HANDLE_TYPES_X(ANGLE_PRE_DECLARE_CLASS_FUNC)
namespace priv
{
template <bool IsSecondary>
class CommandBuffer;
}  // namespace priv
#undef ANGLE_PRE_DECLARE_CLASS_FUNC

// Returns the HandleType of a Vk Handle.
template <typename T>
struct HandleTypeHelper;

#define ANGLE_HANDLE_TYPE_HELPER_FUNC(TYPE)                         \
    template <>                                                     \
    struct HandleTypeHelper<TYPE>                                   \
    {                                                               \
        constexpr static HandleType kHandleType = HandleType::TYPE; \
    };

ANGLE_HANDLE_TYPES_X(ANGLE_HANDLE_TYPE_HELPER_FUNC)
template <bool IsSecondary>
struct HandleTypeHelper<priv::CommandBuffer<IsSecondary>>
{
    constexpr static HandleType kHandleType = HandleType::CommandBuffer;
};

#undef ANGLE_HANDLE_TYPE_HELPER_FUNC

// Base class for all wrapped vulkan objects. Implements several common helper routines.
template <typename DerivedT, typename HandleT>
class WrappedObject : angle::NonCopyable
{
  public:
    HandleT getHandle() const { return mHandle; }
    void setHandle(HandleT handle) { mHandle = handle; }
    bool valid() const { return (mHandle != VK_NULL_HANDLE); }

    const HandleT *ptr() const { return &mHandle; }

    HandleT release()
    {
        HandleT handle = mHandle;
        mHandle        = VK_NULL_HANDLE;
        return handle;
    }

  protected:
    WrappedObject() : mHandle(VK_NULL_HANDLE) {}
    ~WrappedObject() { ASSERT(!valid()); }

    WrappedObject(WrappedObject &&other) : mHandle(other.mHandle)
    {
        other.mHandle = VK_NULL_HANDLE;
    }

    // Only works to initialize empty objects, since we don't have the device handle.
    WrappedObject &operator=(WrappedObject &&other)
    {
        ASSERT(!valid());
        std::swap(mHandle, other.mHandle);
        return *this;
    }

    HandleT mHandle;
};

class CommandPool final : public WrappedObject<CommandPool, VkCommandPool>
{
  public:
    CommandPool() = default;

    void destroy(VkDevice device);
    VkResult reset(VkDevice device, VkCommandPoolResetFlags flags);
    void freeCommandBuffers(VkDevice device,
                            uint32_t commandBufferCount,
                            const VkCommandBuffer *commandBuffers);

    VkResult init(VkDevice device, const VkCommandPoolCreateInfo &createInfo);
};

class Pipeline final : public WrappedObject<Pipeline, VkPipeline>
{
  public:
    Pipeline() = default;
    void destroy(VkDevice device);

    VkResult initGraphics(VkDevice device,
                          const VkGraphicsPipelineCreateInfo &createInfo,
                          const PipelineCache &pipelineCacheVk);
    VkResult initCompute(VkDevice device,
                         const VkComputePipelineCreateInfo &createInfo,
                         const PipelineCache &pipelineCacheVk);
};

namespace priv
{

// Helper class that wraps a Vulkan command buffer.
template <bool IsSecondary>
class CommandBuffer : public WrappedObject<CommandBuffer<IsSecondary>, VkCommandBuffer>
{
  public:
    CommandBuffer() = default;

    VkCommandBuffer releaseHandle();

    // This is used for normal pool allocated command buffers. It reset the handle.
    void destroy(VkDevice device);

    // This is used in conjunction with VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT.
    void destroy(VkDevice device, const CommandPool &commandPool);

    VkResult init(VkDevice device, const VkCommandBufferAllocateInfo &createInfo);

    using WrappedObject<CommandBuffer<IsSecondary>, VkCommandBuffer>::operator=;

    static bool SupportsQueries(const VkPhysicalDeviceFeatures &features)
    {
        return (features.inheritedQueries == VK_TRUE);
    }

    // Vulkan command buffers are executed as secondary command buffers within a primary command
    // buffer.
    static constexpr bool ExecutesInline() { return false; }

    VkResult begin(const VkCommandBufferBeginInfo &info);

    void beginQuery(const QueryPool &queryPool, uint32_t query, VkQueryControlFlags flags);

    void beginRenderPass(const VkRenderPassBeginInfo &beginInfo, VkSubpassContents subpassContents);

    void bindDescriptorSets(const PipelineLayout &layout,
                            VkPipelineBindPoint pipelineBindPoint,
                            DescriptorSetIndex firstSet,
                            uint32_t descriptorSetCount,
                            const VkDescriptorSet *descriptorSets,
                            uint32_t dynamicOffsetCount,
                            const uint32_t *dynamicOffsets);
    void bindGraphicsPipeline(const Pipeline &pipeline);
    void bindComputePipeline(const Pipeline &pipeline);
    void bindPipeline(VkPipelineBindPoint pipelineBindPoint, const Pipeline &pipeline);

    void bindIndexBuffer(const Buffer &buffer, VkDeviceSize offset, VkIndexType indexType);
    void bindVertexBuffers(uint32_t firstBinding,
                           uint32_t bindingCount,
                           const VkBuffer *buffers,
                           const VkDeviceSize *offsets);

    void blitImage(const Image &srcImage,
                   VkImageLayout srcImageLayout,
                   const Image &dstImage,
                   VkImageLayout dstImageLayout,
                   uint32_t regionCount,
                   const VkImageBlit *regions,
                   VkFilter filter);

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

    void clearAttachments(uint32_t attachmentCount,
                          const VkClearAttachment *attachments,
                          uint32_t rectCount,
                          const VkClearRect *rects);

    void copyBuffer(const Buffer &srcBuffer,
                    const Buffer &destBuffer,
                    uint32_t regionCount,
                    const VkBufferCopy *regions);

    void copyBufferToImage(VkBuffer srcBuffer,
                           const Image &dstImage,
                           VkImageLayout dstImageLayout,
                           uint32_t regionCount,
                           const VkBufferImageCopy *regions);
    void copyImageToBuffer(const Image &srcImage,
                           VkImageLayout srcImageLayout,
                           VkBuffer dstBuffer,
                           uint32_t regionCount,
                           const VkBufferImageCopy *regions);
    void copyImage(const Image &srcImage,
                   VkImageLayout srcImageLayout,
                   const Image &dstImage,
                   VkImageLayout dstImageLayout,
                   uint32_t regionCount,
                   const VkImageCopy *regions);

    void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
    void dispatchIndirect(const Buffer &buffer, VkDeviceSize offset);

    void draw(uint32_t vertexCount,
              uint32_t instanceCount,
              uint32_t firstVertex,
              uint32_t firstInstance);
    void draw(uint32_t vertexCount, uint32_t firstVertex);
    void drawInstanced(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex);
    void drawInstancedBaseInstance(uint32_t vertexCount,
                                   uint32_t instanceCount,
                                   uint32_t firstVertex,
                                   uint32_t firstInstance);
    void drawIndexed(uint32_t indexCount,
                     uint32_t instanceCount,
                     uint32_t firstIndex,
                     int32_t vertexOffset,
                     uint32_t firstInstance);
    void drawIndexed(uint32_t indexCount);
    void drawIndexedBaseVertex(uint32_t indexCount, uint32_t vertexOffset);
    void drawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount);
    void drawIndexedInstancedBaseVertex(uint32_t indexCount,
                                        uint32_t instanceCount,
                                        uint32_t vertexOffset);
    void drawIndexedInstancedBaseVertexBaseInstance(uint32_t indexCount,
                                                    uint32_t instanceCount,
                                                    uint32_t firstIndex,
                                                    int32_t vertexOffset,
                                                    uint32_t firstInstance);
    void drawIndexedIndirect(const Buffer &buffer,
                             VkDeviceSize offset,
                             uint32_t drawCount,
                             uint32_t stride);
    void drawIndirect(const Buffer &buffer,
                      VkDeviceSize offset,
                      uint32_t drawCount,
                      uint32_t stride);

    VkResult end();
    void endQuery(const QueryPool &queryPool, uint32_t query);
    void endRenderPass();
    void executeCommands(uint32_t commandBufferCount, const CommandBuffer<true> *commandBuffers);

    void getMemoryUsageStats(size_t *usedMemoryOut, size_t *allocatedMemoryOut) const;

    void executionBarrier(VkPipelineStageFlags stageMask);

    void fillBuffer(const Buffer &dstBuffer,
                    VkDeviceSize dstOffset,
                    VkDeviceSize size,
                    uint32_t data);

    void bufferBarrier(VkPipelineStageFlags srcStageMask,
                       VkPipelineStageFlags dstStageMask,
                       const VkBufferMemoryBarrier *bufferMemoryBarrier);

    void imageBarrier(VkPipelineStageFlags srcStageMask,
                      VkPipelineStageFlags dstStageMask,
                      const VkImageMemoryBarrier &imageMemoryBarrier);

    void memoryBarrier(VkPipelineStageFlags srcStageMask,
                       VkPipelineStageFlags dstStageMask,
                       const VkMemoryBarrier *memoryBarrier);

    void nextSubpass(VkSubpassContents subpassContents);

    void pipelineBarrier(VkPipelineStageFlags srcStageMask,
                         VkPipelineStageFlags dstStageMask,
                         VkDependencyFlags dependencyFlags,
                         uint32_t memoryBarrierCount,
                         const VkMemoryBarrier *memoryBarriers,
                         uint32_t bufferMemoryBarrierCount,
                         const VkBufferMemoryBarrier *bufferMemoryBarriers,
                         uint32_t imageMemoryBarrierCount,
                         const VkImageMemoryBarrier *imageMemoryBarriers);

    void pushConstants(const PipelineLayout &layout,
                       VkShaderStageFlags flag,
                       uint32_t offset,
                       uint32_t size,
                       const void *data);

    void setEvent(VkEvent event, VkPipelineStageFlags stageMask);
    void setViewport(uint32_t firstViewport, uint32_t viewportCount, const VkViewport *viewports);
    void setScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D *scissors);
    VkResult reset();
    void resetEvent(VkEvent event, VkPipelineStageFlags stageMask);
    void resetQueryPool(const QueryPool &queryPool, uint32_t firstQuery, uint32_t queryCount);
    void resolveImage(const Image &srcImage,
                      VkImageLayout srcImageLayout,
                      const Image &dstImage,
                      VkImageLayout dstImageLayout,
                      uint32_t regionCount,
                      const VkImageResolve *regions);
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

    void writeTimestamp(VkPipelineStageFlagBits pipelineStage,
                        const QueryPool &queryPool,
                        uint32_t query);

    // VK_EXT_transform_feedback
    void beginTransformFeedback(uint32_t firstCounterBuffer,
                                uint32_t counterBufferCount,
                                const VkBuffer *counterBuffers,
                                const VkDeviceSize *counterBufferOffsets);
    void endTransformFeedback(uint32_t firstCounterBuffer,
                              uint32_t counterBufferCount,
                              const VkBuffer *counterBuffers,
                              const VkDeviceSize *counterBufferOffsets);
    void bindTransformFeedbackBuffers(uint32_t firstBinding,
                                      uint32_t bindingCount,
                                      const VkBuffer *buffers,
                                      const VkDeviceSize *offsets,
                                      const VkDeviceSize *sizes);

    // VK_EXT_debug_utils
    void beginDebugUtilsLabelEXT(const VkDebugUtilsLabelEXT &labelInfo);
    void endDebugUtilsLabelEXT();
    void insertDebugUtilsLabelEXT(const VkDebugUtilsLabelEXT &labelInfo);

    // Only used when the command buffer is used as a Vulkan secondary command buffer.
    void open() const {}
    void close() const {}
    bool empty() const { return mSize == 0; }
    uint32_t getCommandSize() const { return mSize; }
    std::string dumpCommands(const char *separator) const { return ""; }

  private:
    // Only used when the command buffer is used as a Vulkan secondary command buffer.
    void onRecordCommand()
    {
        if (IsSecondary)
        {
            ++mSize;
        }
    }
    uint32_t mSize;
};
}  // namespace priv

class Image final : public WrappedObject<Image, VkImage>
{
  public:
    Image() = default;

    // Use this method if the lifetime of the image is not controlled by ANGLE. (SwapChain)
    void setHandle(VkImage handle);

    // Called on shutdown when the helper class *doesn't* own the handle to the image resource.
    void reset();

    // Called on shutdown when the helper class *does* own the handle to the image resource.
    void destroy(VkDevice device);

    VkResult init(VkDevice device, const VkImageCreateInfo &createInfo);

    void getMemoryRequirements(VkDevice device, VkMemoryRequirements *requirementsOut) const;
    VkResult bindMemory(VkDevice device, const DeviceMemory &deviceMemory);
    VkResult bindMemory2(VkDevice device, const VkBindImageMemoryInfoKHR &bindInfo);

    void getSubresourceLayout(VkDevice device,
                              VkImageAspectFlagBits aspectMask,
                              uint32_t mipLevel,
                              uint32_t arrayLayer,
                              VkSubresourceLayout *outSubresourceLayout) const;
};

class ImageView final : public WrappedObject<ImageView, VkImageView>
{
  public:
    ImageView() = default;
    void destroy(VkDevice device);

    VkResult init(VkDevice device, const VkImageViewCreateInfo &createInfo);
};

class Semaphore final : public WrappedObject<Semaphore, VkSemaphore>
{
  public:
    Semaphore() = default;
    void destroy(VkDevice device);

    VkResult init(VkDevice device);
    VkResult init(VkDevice device, const VkSemaphoreCreateInfo &createInfo);
    VkResult importFd(VkDevice device, const VkImportSemaphoreFdInfoKHR &importFdInfo) const;
};

class Framebuffer final : public WrappedObject<Framebuffer, VkFramebuffer>
{
  public:
    Framebuffer() = default;
    void destroy(VkDevice device);

    // Use this method only in necessary cases. (RenderPass)
    void setHandle(VkFramebuffer handle);

    VkResult init(VkDevice device, const VkFramebufferCreateInfo &createInfo);
};

class DeviceMemory final : public WrappedObject<DeviceMemory, VkDeviceMemory>
{
  public:
    DeviceMemory() = default;
    void destroy(VkDevice device);

    VkResult allocate(VkDevice device, const VkMemoryAllocateInfo &allocInfo);
    VkResult map(VkDevice device,
                 VkDeviceSize offset,
                 VkDeviceSize size,
                 VkMemoryMapFlags flags,
                 uint8_t **mapPointer) const;
    void unmap(VkDevice device) const;
    void flush(VkDevice device, VkMappedMemoryRange &memRange);
    void invalidate(VkDevice device, VkMappedMemoryRange &memRange);
};

class Allocator : public WrappedObject<Allocator, VmaAllocator>
{
  public:
    Allocator() = default;
    void destroy();

    VkResult init(VkPhysicalDevice physicalDevice,
                  VkDevice device,
                  VkInstance instance,
                  uint32_t apiVersion,
                  VkDeviceSize preferredLargeHeapBlockSize);

    // Initializes the buffer handle and memory allocation.
    VkResult createBuffer(const VkBufferCreateInfo &bufferCreateInfo,
                          VkMemoryPropertyFlags requiredFlags,
                          VkMemoryPropertyFlags preferredFlags,
                          bool persistentlyMappedBuffers,
                          uint32_t *memoryTypeIndexOut,
                          Buffer *bufferOut,
                          Allocation *allocationOut) const;

    void getMemoryTypeProperties(uint32_t memoryTypeIndex, VkMemoryPropertyFlags *flagsOut) const;
    VkResult findMemoryTypeIndexForBufferInfo(const VkBufferCreateInfo &bufferCreateInfo,
                                              VkMemoryPropertyFlags requiredFlags,
                                              VkMemoryPropertyFlags preferredFlags,
                                              bool persistentlyMappedBuffers,
                                              uint32_t *memoryTypeIndexOut) const;

    void buildStatsString(char **statsString, VkBool32 detailedMap);
    void freeStatsString(char *statsString);
};

class Allocation final : public WrappedObject<Allocation, VmaAllocation>
{
  public:
    Allocation() = default;
    void destroy(const Allocator &allocator);

    VkResult map(const Allocator &allocator, uint8_t **mapPointer) const;
    void unmap(const Allocator &allocator) const;
    void flush(const Allocator &allocator, VkDeviceSize offset, VkDeviceSize size);
    void invalidate(const Allocator &allocator, VkDeviceSize offset, VkDeviceSize size);

  private:
    friend class Allocator;
};

class RenderPass final : public WrappedObject<RenderPass, VkRenderPass>
{
  public:
    RenderPass() = default;
    void destroy(VkDevice device);

    VkResult init(VkDevice device, const VkRenderPassCreateInfo &createInfo);
    VkResult init2(VkDevice device, const VkRenderPassCreateInfo2 &createInfo);
};

enum class StagingUsage
{
    Read,
    Write,
    Both,
};

class Buffer final : public WrappedObject<Buffer, VkBuffer>
{
  public:
    Buffer() = default;
    void destroy(VkDevice device);

    VkResult init(VkDevice device, const VkBufferCreateInfo &createInfo);
    VkResult bindMemory(VkDevice device, const DeviceMemory &deviceMemory);
    void getMemoryRequirements(VkDevice device, VkMemoryRequirements *memoryRequirementsOut);

  private:
    friend class Allocator;
};

class BufferView final : public WrappedObject<BufferView, VkBufferView>
{
  public:
    BufferView() = default;
    void destroy(VkDevice device);

    VkResult init(VkDevice device, const VkBufferViewCreateInfo &createInfo);
};

class ShaderModule final : public WrappedObject<ShaderModule, VkShaderModule>
{
  public:
    ShaderModule() = default;
    void destroy(VkDevice device);

    VkResult init(VkDevice device, const VkShaderModuleCreateInfo &createInfo);
};

class PipelineLayout final : public WrappedObject<PipelineLayout, VkPipelineLayout>
{
  public:
    PipelineLayout() = default;
    void destroy(VkDevice device);

    VkResult init(VkDevice device, const VkPipelineLayoutCreateInfo &createInfo);
};

class PipelineCache final : public WrappedObject<PipelineCache, VkPipelineCache>
{
  public:
    PipelineCache() = default;
    void destroy(VkDevice device);

    VkResult init(VkDevice device, const VkPipelineCacheCreateInfo &createInfo);
    VkResult getCacheData(VkDevice device, size_t *cacheSize, void *cacheData);
    VkResult merge(VkDevice device,
                   VkPipelineCache dstCache,
                   uint32_t srcCacheCount,
                   const VkPipelineCache *srcCaches);
};

class DescriptorSetLayout final : public WrappedObject<DescriptorSetLayout, VkDescriptorSetLayout>
{
  public:
    DescriptorSetLayout() = default;
    void destroy(VkDevice device);

    VkResult init(VkDevice device, const VkDescriptorSetLayoutCreateInfo &createInfo);
};

class DescriptorPool final : public WrappedObject<DescriptorPool, VkDescriptorPool>
{
  public:
    DescriptorPool() = default;
    void destroy(VkDevice device);

    VkResult init(VkDevice device, const VkDescriptorPoolCreateInfo &createInfo);

    VkResult allocateDescriptorSets(VkDevice device,
                                    const VkDescriptorSetAllocateInfo &allocInfo,
                                    VkDescriptorSet *descriptorSetsOut);
    VkResult freeDescriptorSets(VkDevice device,
                                uint32_t descriptorSetCount,
                                const VkDescriptorSet *descriptorSets);
};

class Sampler final : public WrappedObject<Sampler, VkSampler>
{
  public:
    Sampler() = default;
    void destroy(VkDevice device);
    VkResult init(VkDevice device, const VkSamplerCreateInfo &createInfo);
};

class SamplerYcbcrConversion final
    : public WrappedObject<SamplerYcbcrConversion, VkSamplerYcbcrConversion>
{
  public:
    SamplerYcbcrConversion() = default;
    void destroy(VkDevice device);
    VkResult init(VkDevice device, const VkSamplerYcbcrConversionCreateInfo &createInfo);
};

class Event final : public WrappedObject<Event, VkEvent>
{
  public:
    Event() = default;
    void destroy(VkDevice device);
    using WrappedObject::operator=;

    VkResult init(VkDevice device, const VkEventCreateInfo &createInfo);
    VkResult getStatus(VkDevice device) const;
    VkResult set(VkDevice device) const;
    VkResult reset(VkDevice device) const;
};

class Fence final : public WrappedObject<Fence, VkFence>
{
  public:
    Fence() = default;
    void destroy(VkDevice device);
    using WrappedObject::operator=;

    VkResult init(VkDevice device, const VkFenceCreateInfo &createInfo);
    VkResult reset(VkDevice device);
    VkResult getStatus(VkDevice device) const;
    VkResult wait(VkDevice device, uint64_t timeout) const;
    VkResult importFd(VkDevice device, const VkImportFenceFdInfoKHR &importFenceFdInfo) const;
    VkResult exportFd(VkDevice device, const VkFenceGetFdInfoKHR &fenceGetFdInfo, int *outFd) const;
};

class QueryPool final : public WrappedObject<QueryPool, VkQueryPool>
{
  public:
    QueryPool() = default;
    void destroy(VkDevice device);

    VkResult init(VkDevice device, const VkQueryPoolCreateInfo &createInfo);
    VkResult getResults(VkDevice device,
                        uint32_t firstQuery,
                        uint32_t queryCount,
                        size_t dataSize,
                        void *data,
                        VkDeviceSize stride,
                        VkQueryResultFlags flags) const;
};

// CommandPool implementation.
ANGLE_INLINE void CommandPool::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyCommandPool(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

ANGLE_INLINE VkResult CommandPool::reset(VkDevice device, VkCommandPoolResetFlags flags)
{
    ASSERT(valid());
    return vkResetCommandPool(device, mHandle, flags);
}

ANGLE_INLINE void CommandPool::freeCommandBuffers(VkDevice device,
                                                  uint32_t commandBufferCount,
                                                  const VkCommandBuffer *commandBuffers)
{
    ASSERT(valid());
    vkFreeCommandBuffers(device, mHandle, commandBufferCount, commandBuffers);
}

ANGLE_INLINE VkResult CommandPool::init(VkDevice device, const VkCommandPoolCreateInfo &createInfo)
{
    ASSERT(!valid());
    return vkCreateCommandPool(device, &createInfo, nullptr, &mHandle);
}

namespace priv
{

// CommandBuffer implementation.
template <bool IsSecondary>
ANGLE_INLINE VkCommandBuffer CommandBuffer<IsSecondary>::releaseHandle()
{
    VkCommandBuffer handle = this->mHandle;
    this->mHandle          = nullptr;
    mSize                  = 0;
    return handle;
}

template <bool IsSecondary>
ANGLE_INLINE VkResult
CommandBuffer<IsSecondary>::init(VkDevice device, const VkCommandBufferAllocateInfo &createInfo)
{
    ASSERT(!this->valid());
    mSize = 0;
    return vkAllocateCommandBuffers(device, &createInfo, &this->mHandle);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::blitImage(const Image &srcImage,
                                                        VkImageLayout srcImageLayout,
                                                        const Image &dstImage,
                                                        VkImageLayout dstImageLayout,
                                                        uint32_t regionCount,
                                                        const VkImageBlit *regions,
                                                        VkFilter filter)
{
    ASSERT(this->valid() && srcImage.valid() && dstImage.valid());
    ASSERT(regionCount == 1);
    vkCmdBlitImage(this->mHandle, srcImage.getHandle(), srcImageLayout, dstImage.getHandle(),
                   dstImageLayout, 1, regions, filter);
}

template <bool IsSecondary>
ANGLE_INLINE VkResult CommandBuffer<IsSecondary>::begin(const VkCommandBufferBeginInfo &info)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandBuffer::begin");

    ASSERT(this->valid());
    ASSERT(mSize == 0);
    return vkBeginCommandBuffer(this->mHandle, &info);
}

template <bool IsSecondary>
ANGLE_INLINE VkResult CommandBuffer<IsSecondary>::end()
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandBuffer::end");
    ASSERT(this->valid());
    return vkEndCommandBuffer(this->mHandle);
}

template <bool IsSecondary>
ANGLE_INLINE VkResult CommandBuffer<IsSecondary>::reset()
{
    ANGLE_TRACE_EVENT0("gpu.angle", "CommandBuffer::reset");

    ASSERT(this->valid());
    mSize = 0;
    return vkResetCommandBuffer(this->mHandle, 0);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::memoryBarrier(VkPipelineStageFlags srcStageMask,
                                                            VkPipelineStageFlags dstStageMask,
                                                            const VkMemoryBarrier *memoryBarrier)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdPipelineBarrier(this->mHandle, srcStageMask, dstStageMask, 0, 1, memoryBarrier, 0, nullptr,
                         0, nullptr);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::nextSubpass(VkSubpassContents subpassContents)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdNextSubpass(this->mHandle, subpassContents);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::pipelineBarrier(
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags,
    uint32_t memoryBarrierCount,
    const VkMemoryBarrier *memoryBarriers,
    uint32_t bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier *bufferMemoryBarriers,
    uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier *imageMemoryBarriers)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdPipelineBarrier(this->mHandle, srcStageMask, dstStageMask, dependencyFlags,
                         memoryBarrierCount, memoryBarriers, bufferMemoryBarrierCount,
                         bufferMemoryBarriers, imageMemoryBarrierCount, imageMemoryBarriers);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::executionBarrier(VkPipelineStageFlags stageMask)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdPipelineBarrier(this->mHandle, stageMask, stageMask, 0, 0, nullptr, 0, nullptr, 0,
                         nullptr);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::bufferBarrier(
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    const VkBufferMemoryBarrier *bufferMemoryBarrier)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdPipelineBarrier(this->mHandle, srcStageMask, dstStageMask, 0, 0, nullptr, 1,
                         bufferMemoryBarrier, 0, nullptr);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::imageBarrier(
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    const VkImageMemoryBarrier &imageMemoryBarrier)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdPipelineBarrier(this->mHandle, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1,
                         &imageMemoryBarrier);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::destroy(VkDevice device)
{
    releaseHandle();
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::destroy(VkDevice device,
                                                      const vk::CommandPool &commandPool)
{
    if (this->valid())
    {
        ASSERT(commandPool.valid());
        vkFreeCommandBuffers(device, commandPool.getHandle(), 1, &this->mHandle);
        this->mHandle = VK_NULL_HANDLE;
        mSize         = 0;
    }
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::copyBuffer(const Buffer &srcBuffer,
                                                         const Buffer &destBuffer,
                                                         uint32_t regionCount,
                                                         const VkBufferCopy *regions)
{
    onRecordCommand();
    ASSERT(this->valid() && srcBuffer.valid() && destBuffer.valid());
    vkCmdCopyBuffer(this->mHandle, srcBuffer.getHandle(), destBuffer.getHandle(), regionCount,
                    regions);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::copyBufferToImage(VkBuffer srcBuffer,
                                                                const Image &dstImage,
                                                                VkImageLayout dstImageLayout,
                                                                uint32_t regionCount,
                                                                const VkBufferImageCopy *regions)
{
    onRecordCommand();
    ASSERT(this->valid() && dstImage.valid());
    ASSERT(srcBuffer != VK_NULL_HANDLE);
    ASSERT(regionCount == 1);
    vkCmdCopyBufferToImage(this->mHandle, srcBuffer, dstImage.getHandle(), dstImageLayout, 1,
                           regions);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::copyImageToBuffer(const Image &srcImage,
                                                                VkImageLayout srcImageLayout,
                                                                VkBuffer dstBuffer,
                                                                uint32_t regionCount,
                                                                const VkBufferImageCopy *regions)
{
    onRecordCommand();
    ASSERT(this->valid() && srcImage.valid());
    ASSERT(dstBuffer != VK_NULL_HANDLE);
    ASSERT(regionCount == 1);
    vkCmdCopyImageToBuffer(this->mHandle, srcImage.getHandle(), srcImageLayout, dstBuffer, 1,
                           regions);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::clearColorImage(const Image &image,
                                                              VkImageLayout imageLayout,
                                                              const VkClearColorValue &color,
                                                              uint32_t rangeCount,
                                                              const VkImageSubresourceRange *ranges)
{
    onRecordCommand();
    ASSERT(this->valid());
    ASSERT(rangeCount == 1);
    vkCmdClearColorImage(this->mHandle, image.getHandle(), imageLayout, &color, 1, ranges);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::clearDepthStencilImage(
    const Image &image,
    VkImageLayout imageLayout,
    const VkClearDepthStencilValue &depthStencil,
    uint32_t rangeCount,
    const VkImageSubresourceRange *ranges)
{
    onRecordCommand();
    ASSERT(this->valid());
    ASSERT(rangeCount == 1);
    vkCmdClearDepthStencilImage(this->mHandle, image.getHandle(), imageLayout, &depthStencil, 1,
                                ranges);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::clearAttachments(uint32_t attachmentCount,
                                                               const VkClearAttachment *attachments,
                                                               uint32_t rectCount,
                                                               const VkClearRect *rects)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdClearAttachments(this->mHandle, attachmentCount, attachments, rectCount, rects);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::copyImage(const Image &srcImage,
                                                        VkImageLayout srcImageLayout,
                                                        const Image &dstImage,
                                                        VkImageLayout dstImageLayout,
                                                        uint32_t regionCount,
                                                        const VkImageCopy *regions)
{
    onRecordCommand();
    ASSERT(this->valid() && srcImage.valid() && dstImage.valid());
    ASSERT(regionCount == 1);
    vkCmdCopyImage(this->mHandle, srcImage.getHandle(), srcImageLayout, dstImage.getHandle(),
                   dstImageLayout, 1, regions);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::beginRenderPass(
    const VkRenderPassBeginInfo &beginInfo,
    VkSubpassContents subpassContents)
{
    ASSERT(this->valid());
    vkCmdBeginRenderPass(this->mHandle, &beginInfo, subpassContents);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::endRenderPass()
{
    ASSERT(this->mHandle != VK_NULL_HANDLE);
    vkCmdEndRenderPass(this->mHandle);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::bindIndexBuffer(const Buffer &buffer,
                                                              VkDeviceSize offset,
                                                              VkIndexType indexType)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdBindIndexBuffer(this->mHandle, buffer.getHandle(), offset, indexType);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::bindDescriptorSets(
    const PipelineLayout &layout,
    VkPipelineBindPoint pipelineBindPoint,
    DescriptorSetIndex firstSet,
    uint32_t descriptorSetCount,
    const VkDescriptorSet *descriptorSets,
    uint32_t dynamicOffsetCount,
    const uint32_t *dynamicOffsets)
{
    onRecordCommand();
    ASSERT(this->valid() && layout.valid());
    vkCmdBindDescriptorSets(this->mHandle, pipelineBindPoint, layout.getHandle(),
                            ToUnderlying(firstSet), descriptorSetCount, descriptorSets,
                            dynamicOffsetCount, dynamicOffsets);
}

template <>
ANGLE_INLINE void CommandBuffer<false>::executeCommands(uint32_t commandBufferCount,
                                                        const CommandBuffer<true> *commandBuffers)
{
    ASSERT(this->valid());
    vkCmdExecuteCommands(this->mHandle, commandBufferCount, commandBuffers[0].ptr());
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::getMemoryUsageStats(size_t *usedMemoryOut,
                                                                  size_t *allocatedMemoryOut) const
{
    // No data available.
    *usedMemoryOut      = 0;
    *allocatedMemoryOut = 1;
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::fillBuffer(const Buffer &dstBuffer,
                                                         VkDeviceSize dstOffset,
                                                         VkDeviceSize size,
                                                         uint32_t data)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdFillBuffer(this->mHandle, dstBuffer.getHandle(), dstOffset, size, data);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::pushConstants(const PipelineLayout &layout,
                                                            VkShaderStageFlags flag,
                                                            uint32_t offset,
                                                            uint32_t size,
                                                            const void *data)
{
    onRecordCommand();
    ASSERT(this->valid() && layout.valid());
    ASSERT(offset == 0);
    vkCmdPushConstants(this->mHandle, layout.getHandle(), flag, 0, size, data);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::setEvent(VkEvent event,
                                                       VkPipelineStageFlags stageMask)
{
    onRecordCommand();
    ASSERT(this->valid() && event != VK_NULL_HANDLE);
    vkCmdSetEvent(this->mHandle, event, stageMask);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::setViewport(uint32_t firstViewport,
                                                          uint32_t viewportCount,
                                                          const VkViewport *viewports)
{
    onRecordCommand();
    ASSERT(this->valid() && viewports != nullptr);
    vkCmdSetViewport(this->mHandle, firstViewport, viewportCount, viewports);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::setScissor(uint32_t firstScissor,
                                                         uint32_t scissorCount,
                                                         const VkRect2D *scissors)
{
    onRecordCommand();
    ASSERT(this->valid() && scissors != nullptr);
    vkCmdSetScissor(this->mHandle, firstScissor, scissorCount, scissors);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::resetEvent(VkEvent event,
                                                         VkPipelineStageFlags stageMask)
{
    onRecordCommand();
    ASSERT(this->valid() && event != VK_NULL_HANDLE);
    vkCmdResetEvent(this->mHandle, event, stageMask);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::waitEvents(
    uint32_t eventCount,
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
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdWaitEvents(this->mHandle, eventCount, events, srcStageMask, dstStageMask,
                    memoryBarrierCount, memoryBarriers, bufferMemoryBarrierCount,
                    bufferMemoryBarriers, imageMemoryBarrierCount, imageMemoryBarriers);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::resetQueryPool(const QueryPool &queryPool,
                                                             uint32_t firstQuery,
                                                             uint32_t queryCount)
{
    onRecordCommand();
    ASSERT(this->valid() && queryPool.valid());
    vkCmdResetQueryPool(this->mHandle, queryPool.getHandle(), firstQuery, queryCount);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::resolveImage(const Image &srcImage,
                                                           VkImageLayout srcImageLayout,
                                                           const Image &dstImage,
                                                           VkImageLayout dstImageLayout,
                                                           uint32_t regionCount,
                                                           const VkImageResolve *regions)
{
    onRecordCommand();
    ASSERT(this->valid() && srcImage.valid() && dstImage.valid());
    vkCmdResolveImage(this->mHandle, srcImage.getHandle(), srcImageLayout, dstImage.getHandle(),
                      dstImageLayout, regionCount, regions);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::beginQuery(const QueryPool &queryPool,
                                                         uint32_t query,
                                                         VkQueryControlFlags flags)
{
    onRecordCommand();
    ASSERT(this->valid() && queryPool.valid());
    vkCmdBeginQuery(this->mHandle, queryPool.getHandle(), query, flags);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::endQuery(const QueryPool &queryPool, uint32_t query)
{
    onRecordCommand();
    ASSERT(this->valid() && queryPool.valid());
    vkCmdEndQuery(this->mHandle, queryPool.getHandle(), query);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::writeTimestamp(VkPipelineStageFlagBits pipelineStage,
                                                             const QueryPool &queryPool,
                                                             uint32_t query)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdWriteTimestamp(this->mHandle, pipelineStage, queryPool.getHandle(), query);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::draw(uint32_t vertexCount,
                                                   uint32_t instanceCount,
                                                   uint32_t firstVertex,
                                                   uint32_t firstInstance)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdDraw(this->mHandle, vertexCount, instanceCount, firstVertex, firstInstance);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::draw(uint32_t vertexCount, uint32_t firstVertex)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdDraw(this->mHandle, vertexCount, 1, firstVertex, 0);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::drawInstanced(uint32_t vertexCount,
                                                            uint32_t instanceCount,
                                                            uint32_t firstVertex)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdDraw(this->mHandle, vertexCount, instanceCount, firstVertex, 0);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::drawInstancedBaseInstance(uint32_t vertexCount,
                                                                        uint32_t instanceCount,
                                                                        uint32_t firstVertex,
                                                                        uint32_t firstInstance)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdDraw(this->mHandle, vertexCount, instanceCount, firstVertex, firstInstance);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::drawIndexed(uint32_t indexCount,
                                                          uint32_t instanceCount,
                                                          uint32_t firstIndex,
                                                          int32_t vertexOffset,
                                                          uint32_t firstInstance)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdDrawIndexed(this->mHandle, indexCount, instanceCount, firstIndex, vertexOffset,
                     firstInstance);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::drawIndexed(uint32_t indexCount)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdDrawIndexed(this->mHandle, indexCount, 1, 0, 0, 0);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::drawIndexedBaseVertex(uint32_t indexCount,
                                                                    uint32_t vertexOffset)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdDrawIndexed(this->mHandle, indexCount, 1, 0, vertexOffset, 0);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::drawIndexedInstanced(uint32_t indexCount,
                                                                   uint32_t instanceCount)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdDrawIndexed(this->mHandle, indexCount, instanceCount, 0, 0, 0);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::drawIndexedInstancedBaseVertex(uint32_t indexCount,
                                                                             uint32_t instanceCount,
                                                                             uint32_t vertexOffset)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdDrawIndexed(this->mHandle, indexCount, instanceCount, 0, vertexOffset, 0);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::drawIndexedInstancedBaseVertexBaseInstance(
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t vertexOffset,
    uint32_t firstInstance)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdDrawIndexed(this->mHandle, indexCount, instanceCount, firstIndex, vertexOffset,
                     firstInstance);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::drawIndexedIndirect(const Buffer &buffer,
                                                                  VkDeviceSize offset,
                                                                  uint32_t drawCount,
                                                                  uint32_t stride)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdDrawIndexedIndirect(this->mHandle, buffer.getHandle(), offset, drawCount, stride);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::drawIndirect(const Buffer &buffer,
                                                           VkDeviceSize offset,
                                                           uint32_t drawCount,
                                                           uint32_t stride)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdDrawIndirect(this->mHandle, buffer.getHandle(), offset, drawCount, stride);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::dispatch(uint32_t groupCountX,
                                                       uint32_t groupCountY,
                                                       uint32_t groupCountZ)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdDispatch(this->mHandle, groupCountX, groupCountY, groupCountZ);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::dispatchIndirect(const Buffer &buffer,
                                                               VkDeviceSize offset)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdDispatchIndirect(this->mHandle, buffer.getHandle(), offset);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::bindPipeline(VkPipelineBindPoint pipelineBindPoint,
                                                           const Pipeline &pipeline)
{
    onRecordCommand();
    ASSERT(this->valid() && pipeline.valid());
    vkCmdBindPipeline(this->mHandle, pipelineBindPoint, pipeline.getHandle());
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::bindGraphicsPipeline(const Pipeline &pipeline)
{
    onRecordCommand();
    ASSERT(this->valid() && pipeline.valid());
    vkCmdBindPipeline(this->mHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getHandle());
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::bindComputePipeline(const Pipeline &pipeline)
{
    onRecordCommand();
    ASSERT(this->valid() && pipeline.valid());
    vkCmdBindPipeline(this->mHandle, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.getHandle());
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::bindVertexBuffers(uint32_t firstBinding,
                                                                uint32_t bindingCount,
                                                                const VkBuffer *buffers,
                                                                const VkDeviceSize *offsets)
{
    onRecordCommand();
    ASSERT(this->valid());
    vkCmdBindVertexBuffers(this->mHandle, firstBinding, bindingCount, buffers, offsets);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::beginTransformFeedback(
    uint32_t firstCounterBuffer,
    uint32_t counterBufferCount,
    const VkBuffer *counterBuffers,
    const VkDeviceSize *counterBufferOffsets)
{
    onRecordCommand();
    ASSERT(this->valid());
    ASSERT(vkCmdBeginTransformFeedbackEXT);
    vkCmdBeginTransformFeedbackEXT(this->mHandle, firstCounterBuffer, counterBufferCount,
                                   counterBuffers, counterBufferOffsets);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::endTransformFeedback(
    uint32_t firstCounterBuffer,
    uint32_t counterBufferCount,
    const VkBuffer *counterBuffers,
    const VkDeviceSize *counterBufferOffsets)
{
    onRecordCommand();
    ASSERT(this->valid());
    ASSERT(vkCmdEndTransformFeedbackEXT);
    vkCmdEndTransformFeedbackEXT(this->mHandle, firstCounterBuffer, counterBufferCount,
                                 counterBuffers, counterBufferOffsets);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::bindTransformFeedbackBuffers(
    uint32_t firstBinding,
    uint32_t bindingCount,
    const VkBuffer *buffers,
    const VkDeviceSize *offsets,
    const VkDeviceSize *sizes)
{
    onRecordCommand();
    ASSERT(this->valid());
    ASSERT(vkCmdBindTransformFeedbackBuffersEXT);
    vkCmdBindTransformFeedbackBuffersEXT(this->mHandle, firstBinding, bindingCount, buffers,
                                         offsets, sizes);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::beginDebugUtilsLabelEXT(
    const VkDebugUtilsLabelEXT &labelInfo)
{
    onRecordCommand();
    ASSERT(this->valid());
    {
#if !defined(ANGLE_SHARED_LIBVULKAN)
        // When the vulkan-loader is statically linked, we need to use the extension
        // functions defined in ANGLE's rx namespace. When it's dynamically linked
        // with volk, this will default to the function definitions with no namespace
        using rx::vkCmdBeginDebugUtilsLabelEXT;
#endif  // !defined(ANGLE_SHARED_LIBVULKAN)
        ASSERT(vkCmdBeginDebugUtilsLabelEXT);
        vkCmdBeginDebugUtilsLabelEXT(this->mHandle, &labelInfo);
    }
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::endDebugUtilsLabelEXT()
{
    onRecordCommand();
    ASSERT(this->valid());
    ASSERT(vkCmdEndDebugUtilsLabelEXT);
    vkCmdEndDebugUtilsLabelEXT(this->mHandle);
}

template <bool IsSecondary>
ANGLE_INLINE void CommandBuffer<IsSecondary>::insertDebugUtilsLabelEXT(
    const VkDebugUtilsLabelEXT &labelInfo)
{
    onRecordCommand();
    ASSERT(this->valid());
    ASSERT(vkCmdInsertDebugUtilsLabelEXT);
    vkCmdInsertDebugUtilsLabelEXT(this->mHandle, &labelInfo);
}
}  // namespace priv

// Image implementation.
ANGLE_INLINE void Image::setHandle(VkImage handle)
{
    mHandle = handle;
}

ANGLE_INLINE void Image::reset()
{
    mHandle = VK_NULL_HANDLE;
}

ANGLE_INLINE void Image::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyImage(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

ANGLE_INLINE VkResult Image::init(VkDevice device, const VkImageCreateInfo &createInfo)
{
    ASSERT(!valid());
    return vkCreateImage(device, &createInfo, nullptr, &mHandle);
}

ANGLE_INLINE void Image::getMemoryRequirements(VkDevice device,
                                               VkMemoryRequirements *requirementsOut) const
{
    ASSERT(valid());
    vkGetImageMemoryRequirements(device, mHandle, requirementsOut);
}

ANGLE_INLINE VkResult Image::bindMemory(VkDevice device, const vk::DeviceMemory &deviceMemory)
{
    ASSERT(valid() && deviceMemory.valid());
    return vkBindImageMemory(device, mHandle, deviceMemory.getHandle(), 0);
}

ANGLE_INLINE VkResult Image::bindMemory2(VkDevice device, const VkBindImageMemoryInfoKHR &bindInfo)
{
    ASSERT(valid());
    return vkBindImageMemory2KHR(device, 1, &bindInfo);
}

ANGLE_INLINE void Image::getSubresourceLayout(VkDevice device,
                                              VkImageAspectFlagBits aspectMask,
                                              uint32_t mipLevel,
                                              uint32_t arrayLayer,
                                              VkSubresourceLayout *outSubresourceLayout) const
{
    VkImageSubresource subresource = {};
    subresource.aspectMask         = aspectMask;
    subresource.mipLevel           = mipLevel;
    subresource.arrayLayer         = arrayLayer;

    vkGetImageSubresourceLayout(device, getHandle(), &subresource, outSubresourceLayout);
}

// ImageView implementation.
ANGLE_INLINE void ImageView::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyImageView(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

ANGLE_INLINE VkResult ImageView::init(VkDevice device, const VkImageViewCreateInfo &createInfo)
{
    return vkCreateImageView(device, &createInfo, nullptr, &mHandle);
}

// Semaphore implementation.
ANGLE_INLINE void Semaphore::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroySemaphore(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

ANGLE_INLINE VkResult Semaphore::init(VkDevice device)
{
    ASSERT(!valid());

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.flags                 = 0;

    return vkCreateSemaphore(device, &semaphoreInfo, nullptr, &mHandle);
}

ANGLE_INLINE VkResult Semaphore::init(VkDevice device, const VkSemaphoreCreateInfo &createInfo)
{
    ASSERT(valid());
    return vkCreateSemaphore(device, &createInfo, nullptr, &mHandle);
}

ANGLE_INLINE VkResult Semaphore::importFd(VkDevice device,
                                          const VkImportSemaphoreFdInfoKHR &importFdInfo) const
{
    ASSERT(valid());
    return vkImportSemaphoreFdKHR(device, &importFdInfo);
}

// Framebuffer implementation.
ANGLE_INLINE void Framebuffer::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyFramebuffer(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

ANGLE_INLINE VkResult Framebuffer::init(VkDevice device, const VkFramebufferCreateInfo &createInfo)
{
    ASSERT(!valid());
    return vkCreateFramebuffer(device, &createInfo, nullptr, &mHandle);
}

ANGLE_INLINE void Framebuffer::setHandle(VkFramebuffer handle)
{
    mHandle = handle;
}

// DeviceMemory implementation.
ANGLE_INLINE void DeviceMemory::destroy(VkDevice device)
{
    if (valid())
    {
        vkFreeMemory(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

ANGLE_INLINE VkResult DeviceMemory::allocate(VkDevice device, const VkMemoryAllocateInfo &allocInfo)
{
    ASSERT(!valid());
    return vkAllocateMemory(device, &allocInfo, nullptr, &mHandle);
}

ANGLE_INLINE VkResult DeviceMemory::map(VkDevice device,
                                        VkDeviceSize offset,
                                        VkDeviceSize size,
                                        VkMemoryMapFlags flags,
                                        uint8_t **mapPointer) const
{
    ANGLE_TRACE_EVENT0("gpu.angle", "DeviceMemory::map");
    ASSERT(valid());
    return vkMapMemory(device, mHandle, offset, size, flags, reinterpret_cast<void **>(mapPointer));
}

ANGLE_INLINE void DeviceMemory::unmap(VkDevice device) const
{
    ASSERT(valid());
    vkUnmapMemory(device, mHandle);
}

ANGLE_INLINE void DeviceMemory::flush(VkDevice device, VkMappedMemoryRange &memRange)
{
    vkFlushMappedMemoryRanges(device, 1, &memRange);
}

ANGLE_INLINE void DeviceMemory::invalidate(VkDevice device, VkMappedMemoryRange &memRange)
{
    vkInvalidateMappedMemoryRanges(device, 1, &memRange);
}

// Allocator implementation.
ANGLE_INLINE void Allocator::destroy()
{
    if (valid())
    {
        vma::DestroyAllocator(mHandle);
        mHandle = VK_NULL_HANDLE;
    }
}

ANGLE_INLINE VkResult Allocator::init(VkPhysicalDevice physicalDevice,
                                      VkDevice device,
                                      VkInstance instance,
                                      uint32_t apiVersion,
                                      VkDeviceSize preferredLargeHeapBlockSize)
{
    ASSERT(!valid());
    return vma::InitAllocator(physicalDevice, device, instance, apiVersion,
                              preferredLargeHeapBlockSize, &mHandle);
}

ANGLE_INLINE VkResult Allocator::createBuffer(const VkBufferCreateInfo &bufferCreateInfo,
                                              VkMemoryPropertyFlags requiredFlags,
                                              VkMemoryPropertyFlags preferredFlags,
                                              bool persistentlyMappedBuffers,
                                              uint32_t *memoryTypeIndexOut,
                                              Buffer *bufferOut,
                                              Allocation *allocationOut) const
{
    ASSERT(valid());
    ASSERT(bufferOut && !bufferOut->valid());
    ASSERT(allocationOut && !allocationOut->valid());
    return vma::CreateBuffer(mHandle, &bufferCreateInfo, requiredFlags, preferredFlags,
                             persistentlyMappedBuffers, memoryTypeIndexOut, &bufferOut->mHandle,
                             &allocationOut->mHandle);
}

ANGLE_INLINE void Allocator::getMemoryTypeProperties(uint32_t memoryTypeIndex,
                                                     VkMemoryPropertyFlags *flagsOut) const
{
    ASSERT(valid());
    vma::GetMemoryTypeProperties(mHandle, memoryTypeIndex, flagsOut);
}

ANGLE_INLINE VkResult
Allocator::findMemoryTypeIndexForBufferInfo(const VkBufferCreateInfo &bufferCreateInfo,
                                            VkMemoryPropertyFlags requiredFlags,
                                            VkMemoryPropertyFlags preferredFlags,
                                            bool persistentlyMappedBuffers,
                                            uint32_t *memoryTypeIndexOut) const
{
    ASSERT(valid());
    return vma::FindMemoryTypeIndexForBufferInfo(mHandle, &bufferCreateInfo, requiredFlags,
                                                 preferredFlags, persistentlyMappedBuffers,
                                                 memoryTypeIndexOut);
}

ANGLE_INLINE void Allocator::buildStatsString(char **statsString, VkBool32 detailedMap)
{
    ASSERT(valid());
    vma::BuildStatsString(mHandle, statsString, detailedMap);
}

ANGLE_INLINE void Allocator::freeStatsString(char *statsString)
{
    ASSERT(valid());
    vma::FreeStatsString(mHandle, statsString);
}

// Allocation implementation.
ANGLE_INLINE void Allocation::destroy(const Allocator &allocator)
{
    if (valid())
    {
        vma::FreeMemory(allocator.getHandle(), mHandle);
        mHandle = VK_NULL_HANDLE;
    }
}

ANGLE_INLINE VkResult Allocation::map(const Allocator &allocator, uint8_t **mapPointer) const
{
    ASSERT(valid());
    return vma::MapMemory(allocator.getHandle(), mHandle, (void **)mapPointer);
}

ANGLE_INLINE void Allocation::unmap(const Allocator &allocator) const
{
    ASSERT(valid());
    vma::UnmapMemory(allocator.getHandle(), mHandle);
}

ANGLE_INLINE void Allocation::flush(const Allocator &allocator,
                                    VkDeviceSize offset,
                                    VkDeviceSize size)
{
    ASSERT(valid());
    vma::FlushAllocation(allocator.getHandle(), mHandle, offset, size);
}

ANGLE_INLINE void Allocation::invalidate(const Allocator &allocator,
                                         VkDeviceSize offset,
                                         VkDeviceSize size)
{
    ASSERT(valid());
    vma::InvalidateAllocation(allocator.getHandle(), mHandle, offset, size);
}

// RenderPass implementation.
ANGLE_INLINE void RenderPass::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyRenderPass(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

ANGLE_INLINE VkResult RenderPass::init(VkDevice device, const VkRenderPassCreateInfo &createInfo)
{
    ASSERT(!valid());
    return vkCreateRenderPass(device, &createInfo, nullptr, &mHandle);
}

ANGLE_INLINE VkResult RenderPass::init2(VkDevice device, const VkRenderPassCreateInfo2 &createInfo)
{
    ASSERT(!valid());
    return vkCreateRenderPass2KHR(device, &createInfo, nullptr, &mHandle);
}

// Buffer implementation.
ANGLE_INLINE void Buffer::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyBuffer(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

ANGLE_INLINE VkResult Buffer::init(VkDevice device, const VkBufferCreateInfo &createInfo)
{
    ASSERT(!valid());
    return vkCreateBuffer(device, &createInfo, nullptr, &mHandle);
}

ANGLE_INLINE VkResult Buffer::bindMemory(VkDevice device, const DeviceMemory &deviceMemory)
{
    ASSERT(valid() && deviceMemory.valid());
    return vkBindBufferMemory(device, mHandle, deviceMemory.getHandle(), 0);
}

ANGLE_INLINE void Buffer::getMemoryRequirements(VkDevice device,
                                                VkMemoryRequirements *memoryRequirementsOut)
{
    ASSERT(valid());
    vkGetBufferMemoryRequirements(device, mHandle, memoryRequirementsOut);
}

// BufferView implementation.
ANGLE_INLINE void BufferView::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyBufferView(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

ANGLE_INLINE VkResult BufferView::init(VkDevice device, const VkBufferViewCreateInfo &createInfo)
{
    ASSERT(!valid());
    return vkCreateBufferView(device, &createInfo, nullptr, &mHandle);
}

// ShaderModule implementation.
ANGLE_INLINE void ShaderModule::destroy(VkDevice device)
{
    if (mHandle != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

ANGLE_INLINE VkResult ShaderModule::init(VkDevice device,
                                         const VkShaderModuleCreateInfo &createInfo)
{
    ASSERT(!valid());
    return vkCreateShaderModule(device, &createInfo, nullptr, &mHandle);
}

// PipelineLayout implementation.
ANGLE_INLINE void PipelineLayout::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyPipelineLayout(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

ANGLE_INLINE VkResult PipelineLayout::init(VkDevice device,
                                           const VkPipelineLayoutCreateInfo &createInfo)
{
    ASSERT(!valid());
    return vkCreatePipelineLayout(device, &createInfo, nullptr, &mHandle);
}

// PipelineCache implementation.
ANGLE_INLINE void PipelineCache::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyPipelineCache(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

ANGLE_INLINE VkResult PipelineCache::init(VkDevice device,
                                          const VkPipelineCacheCreateInfo &createInfo)
{
    ASSERT(!valid());
    // Note: if we are concerned with memory usage of this cache, we should give it custom
    // allocators.  Also, failure of this function is of little importance.
    return vkCreatePipelineCache(device, &createInfo, nullptr, &mHandle);
}

ANGLE_INLINE VkResult PipelineCache::merge(VkDevice device,
                                           VkPipelineCache dstCache,
                                           uint32_t srcCacheCount,
                                           const VkPipelineCache *srcCaches)
{
    ASSERT(valid());
    return vkMergePipelineCaches(device, dstCache, srcCacheCount, srcCaches);
}

ANGLE_INLINE VkResult PipelineCache::getCacheData(VkDevice device,
                                                  size_t *cacheSize,
                                                  void *cacheData)
{
    ASSERT(valid());

    // Note: vkGetPipelineCacheData can return VK_INCOMPLETE if cacheSize is smaller than actual
    // size. There are two usages of this function.  One is with *cacheSize == 0 to query the size
    // of the cache, and one is with an appropriate buffer to retrieve the cache contents.
    // VK_INCOMPLETE in the first case is an expected output.  In the second case, VK_INCOMPLETE is
    // also acceptable and the resulting buffer will contain valid value by spec.  Angle currently
    // ensures *cacheSize to be either 0 or of enough size, therefore VK_INCOMPLETE is not expected.
    return vkGetPipelineCacheData(device, mHandle, cacheSize, cacheData);
}

// Pipeline implementation.
ANGLE_INLINE void Pipeline::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyPipeline(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

ANGLE_INLINE VkResult Pipeline::initGraphics(VkDevice device,
                                             const VkGraphicsPipelineCreateInfo &createInfo,
                                             const PipelineCache &pipelineCacheVk)
{
    ASSERT(!valid());
    return vkCreateGraphicsPipelines(device, pipelineCacheVk.getHandle(), 1, &createInfo, nullptr,
                                     &mHandle);
}

ANGLE_INLINE VkResult Pipeline::initCompute(VkDevice device,
                                            const VkComputePipelineCreateInfo &createInfo,
                                            const PipelineCache &pipelineCacheVk)
{
    ASSERT(!valid());
    return vkCreateComputePipelines(device, pipelineCacheVk.getHandle(), 1, &createInfo, nullptr,
                                    &mHandle);
}

// DescriptorSetLayout implementation.
ANGLE_INLINE void DescriptorSetLayout::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyDescriptorSetLayout(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

ANGLE_INLINE VkResult DescriptorSetLayout::init(VkDevice device,
                                                const VkDescriptorSetLayoutCreateInfo &createInfo)
{
    ASSERT(!valid());
    return vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &mHandle);
}

// DescriptorPool implementation.
ANGLE_INLINE void DescriptorPool::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyDescriptorPool(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

ANGLE_INLINE VkResult DescriptorPool::init(VkDevice device,
                                           const VkDescriptorPoolCreateInfo &createInfo)
{
    ASSERT(!valid());
    return vkCreateDescriptorPool(device, &createInfo, nullptr, &mHandle);
}

ANGLE_INLINE VkResult
DescriptorPool::allocateDescriptorSets(VkDevice device,
                                       const VkDescriptorSetAllocateInfo &allocInfo,
                                       VkDescriptorSet *descriptorSetsOut)
{
    ASSERT(valid());
    return vkAllocateDescriptorSets(device, &allocInfo, descriptorSetsOut);
}

ANGLE_INLINE VkResult DescriptorPool::freeDescriptorSets(VkDevice device,
                                                         uint32_t descriptorSetCount,
                                                         const VkDescriptorSet *descriptorSets)
{
    ASSERT(valid());
    ASSERT(descriptorSetCount > 0);
    return vkFreeDescriptorSets(device, mHandle, descriptorSetCount, descriptorSets);
}

// Sampler implementation.
ANGLE_INLINE void Sampler::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroySampler(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

ANGLE_INLINE VkResult Sampler::init(VkDevice device, const VkSamplerCreateInfo &createInfo)
{
    ASSERT(!valid());
    return vkCreateSampler(device, &createInfo, nullptr, &mHandle);
}

// SamplerYuvConversion implementation.
ANGLE_INLINE void SamplerYcbcrConversion::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroySamplerYcbcrConversionKHR(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

ANGLE_INLINE VkResult
SamplerYcbcrConversion::init(VkDevice device, const VkSamplerYcbcrConversionCreateInfo &createInfo)
{
    ASSERT(!valid());
    return vkCreateSamplerYcbcrConversionKHR(device, &createInfo, nullptr, &mHandle);
}

// Event implementation.
ANGLE_INLINE void Event::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyEvent(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

ANGLE_INLINE VkResult Event::init(VkDevice device, const VkEventCreateInfo &createInfo)
{
    ASSERT(!valid());
    return vkCreateEvent(device, &createInfo, nullptr, &mHandle);
}

ANGLE_INLINE VkResult Event::getStatus(VkDevice device) const
{
    ASSERT(valid());
    return vkGetEventStatus(device, mHandle);
}

ANGLE_INLINE VkResult Event::set(VkDevice device) const
{
    ASSERT(valid());
    return vkSetEvent(device, mHandle);
}

ANGLE_INLINE VkResult Event::reset(VkDevice device) const
{
    ASSERT(valid());
    return vkResetEvent(device, mHandle);
}

// Fence implementation.
ANGLE_INLINE void Fence::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyFence(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

ANGLE_INLINE VkResult Fence::init(VkDevice device, const VkFenceCreateInfo &createInfo)
{
    ASSERT(!valid());
    return vkCreateFence(device, &createInfo, nullptr, &mHandle);
}

ANGLE_INLINE VkResult Fence::reset(VkDevice device)
{
    ASSERT(valid());
    return vkResetFences(device, 1, &mHandle);
}

ANGLE_INLINE VkResult Fence::getStatus(VkDevice device) const
{
    ASSERT(valid());
    return vkGetFenceStatus(device, mHandle);
}

ANGLE_INLINE VkResult Fence::wait(VkDevice device, uint64_t timeout) const
{
    ASSERT(valid());
    return vkWaitForFences(device, 1, &mHandle, true, timeout);
}

ANGLE_INLINE VkResult Fence::importFd(VkDevice device,
                                      const VkImportFenceFdInfoKHR &importFenceFdInfo) const
{
    ASSERT(valid());
    return vkImportFenceFdKHR(device, &importFenceFdInfo);
}

ANGLE_INLINE VkResult Fence::exportFd(VkDevice device,
                                      const VkFenceGetFdInfoKHR &fenceGetFdInfo,
                                      int *fdOut) const
{
    ASSERT(valid());
    return vkGetFenceFdKHR(device, &fenceGetFdInfo, fdOut);
}

// QueryPool implementation.
ANGLE_INLINE void QueryPool::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyQueryPool(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

ANGLE_INLINE VkResult QueryPool::init(VkDevice device, const VkQueryPoolCreateInfo &createInfo)
{
    ASSERT(!valid());
    return vkCreateQueryPool(device, &createInfo, nullptr, &mHandle);
}

ANGLE_INLINE VkResult QueryPool::getResults(VkDevice device,
                                            uint32_t firstQuery,
                                            uint32_t queryCount,
                                            size_t dataSize,
                                            void *data,
                                            VkDeviceSize stride,
                                            VkQueryResultFlags flags) const
{
    ASSERT(valid());
    return vkGetQueryPoolResults(device, mHandle, firstQuery, queryCount, dataSize, data, stride,
                                 flags);
}
}  // namespace vk
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_VK_WRAPPER_H_
