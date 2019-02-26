//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vk_utils:
//    Helper functions for the Vulkan Renderer.
//

#ifndef LIBANGLE_RENDERER_VULKAN_VK_UTILS_H_
#define LIBANGLE_RENDERER_VULKAN_VK_UTILS_H_

#include <limits>

#include "common/FixedVector.h"
#include "common/Optional.h"
#include "common/PackedEnums.h"
#include "common/debug.h"
#include "common/PoolAlloc.h"
#include "libANGLE/Error.h"
#include "libANGLE/Observer.h"
#include "libANGLE/renderer/vulkan/vk_wrapper.h"

#define ANGLE_GL_OBJECTS_X(PROC) \
    PROC(Buffer)                 \
    PROC(Context)                \
    PROC(Framebuffer)            \
    PROC(Program)                \
    PROC(Texture)                \
    PROC(VertexArray)

#define ANGLE_PRE_DECLARE_OBJECT(OBJ) class OBJ;

namespace egl
{
class Display;
class Image;
}

namespace gl
{
struct Box;
struct Extents;
struct RasterizerState;
struct Rectangle;
class State;
struct SwizzleState;
struct VertexAttribute;
class VertexBinding;

ANGLE_GL_OBJECTS_X(ANGLE_PRE_DECLARE_OBJECT)
}  // namespace gl

#define ANGLE_PRE_DECLARE_VK_OBJECT(OBJ) class OBJ##Vk;

namespace rx
{
class CommandGraphResource;
class DisplayVk;
class ImageVk;
class RenderTargetVk;
class RendererVk;
class RenderPassCache;
}  // namespace rx

namespace angle
{
egl::Error ToEGL(Result result, rx::DisplayVk *displayVk, EGLint errorCode);
}  // namespace angle

namespace rx
{
ANGLE_GL_OBJECTS_X(ANGLE_PRE_DECLARE_VK_OBJECT)

const char *VulkanResultString(VkResult result);

constexpr size_t kMaxVulkanLayers = 20;
using VulkanLayerVector           = angle::FixedVector<const char *, kMaxVulkanLayers>;

// Verify that validation layers are available.
bool GetAvailableValidationLayers(const std::vector<VkLayerProperties> &layerProps,
                                  bool mustHaveLayers,
                                  VulkanLayerVector *enabledLayerNames);

extern const char *g_VkLoaderLayersPathEnv;
extern const char *g_VkICDPathEnv;

enum class TextureDimension
{
    TEX_2D,
    TEX_CUBE,
    TEX_3D,
    TEX_2D_ARRAY,
};

namespace vk
{
struct Format;

// Abstracts error handling. Implemented by both ContextVk for GL and DisplayVk for EGL errors.
class Context : angle::NonCopyable
{
  public:
    Context(RendererVk *renderer);
    virtual ~Context();

    virtual void handleError(VkResult result,
                             const char *file,
                             const char *function,
                             unsigned int line) = 0;
    VkDevice getDevice() const;
    RendererVk *getRenderer() const { return mRenderer; }

  protected:
    RendererVk *const mRenderer;
};

VkImageAspectFlags GetDepthStencilAspectFlags(const angle::Format &format);
VkImageAspectFlags GetFormatAspectFlags(const angle::Format &format);
VkImageAspectFlags GetDepthStencilAspectFlagsForCopy(bool copyDepth, bool copyStencil);

template <typename T>
struct ImplTypeHelper;

// clang-format off
#define ANGLE_IMPL_TYPE_HELPER_GL(OBJ) \
template<>                             \
struct ImplTypeHelper<gl::OBJ>         \
{                                      \
    using ImplType = OBJ##Vk;          \
};
// clang-format on

ANGLE_GL_OBJECTS_X(ANGLE_IMPL_TYPE_HELPER_GL)

template <>
struct ImplTypeHelper<egl::Display>
{
    using ImplType = DisplayVk;
};

template <>
struct ImplTypeHelper<egl::Image>
{
    using ImplType = ImageVk;
};

template <typename T>
using GetImplType = typename ImplTypeHelper<T>::ImplType;

template <typename T>
GetImplType<T> *GetImpl(const T *glObject)
{
    return GetImplAs<GetImplType<T>>(glObject);
}

class GarbageObject final
{
  public:
    template <typename ObjectT>
    GarbageObject(Serial serial, const ObjectT &object)
        : mSerial(serial),
          mHandleType(HandleTypeHelper<ObjectT>::kHandleType),
          mHandle(reinterpret_cast<VkDevice>(object.getHandle()))
    {}

    GarbageObject();
    GarbageObject(const GarbageObject &other);
    GarbageObject &operator=(const GarbageObject &other);

    bool destroyIfComplete(VkDevice device, Serial completedSerial);
    void destroy(VkDevice device);

  private:
    // TODO(jmadill): Since many objects will have the same serial, it might be more efficient to
    // store the serial outside of the garbage object itself. We could index ranges of garbage
    // objects in the Renderer, using a circular buffer.
    Serial mSerial;
    HandleType mHandleType;
    VkDevice mHandle;
};

class MemoryProperties final : angle::NonCopyable
{
  public:
    MemoryProperties();

    void init(VkPhysicalDevice physicalDevice);
    angle::Result findCompatibleMemoryIndex(Context *context,
                                            const VkMemoryRequirements &memoryRequirements,
                                            VkMemoryPropertyFlags requestedMemoryPropertyFlags,
                                            VkMemoryPropertyFlags *memoryPropertyFlagsOut,
                                            uint32_t *indexOut) const;
    void destroy();

  private:
    VkPhysicalDeviceMemoryProperties mMemoryProperties;
};

// Similar to StagingImage, for Buffers.
class StagingBuffer final : angle::NonCopyable
{
  public:
    StagingBuffer();
    void destroy(VkDevice device);

    angle::Result init(Context *context, VkDeviceSize size, StagingUsage usage);

    Buffer &getBuffer() { return mBuffer; }
    const Buffer &getBuffer() const { return mBuffer; }
    DeviceMemory &getDeviceMemory() { return mDeviceMemory; }
    const DeviceMemory &getDeviceMemory() const { return mDeviceMemory; }
    size_t getSize() const { return mSize; }

    void dumpResources(Serial serial, std::vector<GarbageObject> *garbageQueue);

  private:
    Buffer mBuffer;
    DeviceMemory mDeviceMemory;
    size_t mSize;
};

template <typename ObjT>
class ObjectAndSerial final : angle::NonCopyable
{
  public:
    ObjectAndSerial() {}

    ObjectAndSerial(ObjT &&object, Serial serial) : mObject(std::move(object)), mSerial(serial) {}

    ObjectAndSerial(ObjectAndSerial &&other)
        : mObject(std::move(other.mObject)), mSerial(std::move(other.mSerial))
    {}
    ObjectAndSerial &operator=(ObjectAndSerial &&other)
    {
        mObject = std::move(other.mObject);
        mSerial = std::move(other.mSerial);
        return *this;
    }

    Serial getSerial() const { return mSerial; }
    void updateSerial(Serial newSerial) { mSerial = newSerial; }

    const ObjT &get() const { return mObject; }
    ObjT &get() { return mObject; }

    bool valid() const { return mObject.valid(); }

    void destroy(VkDevice device)
    {
        mObject.destroy(device);
        mSerial = Serial();
    }

  private:
    ObjT mObject;
    Serial mSerial;
};

angle::Result AllocateBufferMemory(vk::Context *context,
                                   VkMemoryPropertyFlags requestedMemoryPropertyFlags,
                                   VkMemoryPropertyFlags *memoryPropertyFlagsOut,
                                   Buffer *buffer,
                                   DeviceMemory *deviceMemoryOut);

angle::Result AllocateImageMemory(vk::Context *context,
                                  VkMemoryPropertyFlags memoryPropertyFlags,
                                  Image *image,
                                  DeviceMemory *deviceMemoryOut);

using ShaderAndSerial = ObjectAndSerial<ShaderModule>;

angle::Result InitShaderAndSerial(Context *context,
                                  ShaderAndSerial *shaderAndSerial,
                                  const uint32_t *shaderCode,
                                  size_t shaderCodeSize);

enum class RecordingMode
{
    Start,
    Append,
};

// Helper class to handle RAII patterns for initialization. Requires that T have a destroy method
// that takes a VkDevice and returns void.
template <typename T>
class Scoped final : angle::NonCopyable
{
  public:
    Scoped(VkDevice device) : mDevice(device) {}
    ~Scoped() { mVar.destroy(mDevice); }

    const T &get() const { return mVar; }
    T &get() { return mVar; }

    T &&release() { return std::move(mVar); }

  private:
    VkDevice mDevice;
    T mVar;
};

// This is a very simple RefCount class that has no autoreleasing. Used in the descriptor set and
// pipeline layout caches.
template <typename T>
class RefCounted : angle::NonCopyable
{
  public:
    RefCounted() : mRefCount(0) {}
    explicit RefCounted(T &&newObject) : mRefCount(0), mObject(std::move(newObject)) {}
    ~RefCounted() { ASSERT(mRefCount == 0 && !mObject.valid()); }

    RefCounted(RefCounted &&copy) : mRefCount(copy.mRefCount), mObject(std::move(copy.mObject))
    {
        copy.mRefCount = 0;
    }

    RefCounted &operator=(RefCounted &&rhs)
    {
        std::swap(mRefCount, rhs.mRefCount);
        mObject = std::move(rhs.mObject);
        return *this;
    }

    void addRef()
    {
        ASSERT(mRefCount != std::numeric_limits<uint32_t>::max());
        mRefCount++;
    }

    void releaseRef()
    {
        ASSERT(isReferenced());
        mRefCount--;
    }

    bool isReferenced() const { return mRefCount != 0; }

    T &get() { return mObject; }
    const T &get() const { return mObject; }

  private:
    uint32_t mRefCount;
    T mObject;
};

template <typename T>
class BindingPointer final : angle::NonCopyable
{
  public:
    BindingPointer() : mRefCounted(nullptr) {}

    ~BindingPointer() { reset(); }

    void set(RefCounted<T> *refCounted)
    {
        if (mRefCounted)
        {
            mRefCounted->releaseRef();
        }

        mRefCounted = refCounted;

        if (mRefCounted)
        {
            mRefCounted->addRef();
        }
    }

    void reset() { set(nullptr); }

    T &get() { return mRefCounted->get(); }
    const T &get() const { return mRefCounted->get(); }

    bool valid() const { return mRefCounted != nullptr; }

  private:
    RefCounted<T> *mRefCounted;
};

enum class CommandType
{
    // State update cmds
    CMD_TYPE_BIND_DESCRIPTOR_SETS      = 0,
    CMD_TYPE_BIND_INDEX_BUFFER         = 1,
    CMD_TYPE_BIND_PIPELINE             = 2,
    CMD_TYPE_BIND_VERTEX_BUFFERS       = 3,
    CMD_TYPE_BLIT_IMAGE                = 4,
    CMD_TYPE_COPY_BUFFER               = 5,
    CMD_TYPE_COPY_BUFFER_TO_IMAGE      = 6,
    CMD_TYPE_COPY_IMAGE                = 7,
    CMD_TYPE_COPY_IMAGE_TO_BUFFER      = 8,
    CMD_TYPE_CLEAR_ATTACHMENTS         = 9,
    CMD_TYPE_CLEAR_COLOR_IMAGE         = 10,
    CMD_TYPE_CLEAR_DEPTH_STENCIL_IMAGE = 11,
    CMD_TYPE_UPDATE_BUFFER             = 12,
    CMD_TYPE_PUSH_CONSTANTS            = 13,
    CMD_TYPE_SET_VIEWPORT              = 14,
    CMD_TYPE_SET_SCISSORS              = 15,
    // Draw/dispatch cmds
    CMD_TYPE_DRAW                      = 16,
    CMD_TYPE_DRAW_INDEXED              = 17,
    CMD_TYPE_DISPATCH                  = 18,
    // Sync & Query cmds
    CMD_TYPE_PIPELINE_BARRIER          = 19,
    CMD_TYPE_RESET_EVENT               = 20,
    CMD_TYPE_SET_EVENT                 = 21,
    CMD_TYPE_WAIT_EVENTS               = 22,
    CMD_TYPE_RESET_QUERY_POOL          = 23,
    CMD_TYPE_BEGIN_QUERY               = 24,
    CMD_TYPE_END_QUERY                 = 25,
    CMD_TYPE_WRITE_TIMESTAMP           = 26,
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
    const VkDescriptorSet* descriptorSets;
    uint32_t dynamicOffsetCount;
    const uint32_t* dynamicOffsets;
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
    CommandType type; // type of cmd
    size_t paramSize; // size of cmd params
    bool copyPtrData; // are any params pointers that need to have data copied?
};

static constexpr CommandInfo CommandInfos[1] = {
    {
        CommandType::CMD_TYPE_BIND_DESCRIPTOR_SETS,
        sizeof(BindDescriptorSetParams),
        true
    },
};
#if 0
    //BIND_INDEX_BUFFER         = 1,
    sizeof(BindIndexBufferParams),
    //BIND_PIPELINE             = 2,
    sizeof(BindPipelineParams),
    //BIND_VERTEX_BUFFERS       = 3,
    sizeof(BindVertexBuffersParams),
    //BLIT_IMAGE                = 4,
    sizeof(BlitImageParams),
    //COPY_BUFFER               = 5,
    sizeof(CopyBufferParams),
    //COPY_BUFFER_TO_IMAGE      = 6,
    sizeof(CopyBufferToImageParams),
    //COPY_IMAGE                = 7,
    sizeof(CopyImageParams),
    //COPY_IMAGE_TO_BUFFER      = 8,
    sizeof(CopyImageToBufferParams),
    //CLEAR_ATTACHMENTS         = 9,
    sizeof(ClearAttachmentsParams),
    //CLEAR_COLOR_IMAGE         = 10,
    sizeof(ClearColorImageParams),
    //CLEAR_DEPTH_STENCIL_IMAGE = 11,
    sizeof(ClearDepthStencilImageParams),
    //UPDATE_BUFFER             = 12,
    sizeof(UpdateBufferParams),
    //PUSH_CONSTANTS            = 13,
    sizeof(PushConstantsParams),
    //SET_VIEWPORT              = 14,
    sizeof(SetViewportParams),
    //SET_SCISSORS              = 15,
    sizeof(SetScissorParams),
    //DRAW                      = 16,
    sizeof(DrawParams),
    //DRAW_INDEXED              = 17,
    sizeof(DrawIndexedParams),
    //DISPATCH                  = 18,
    sizeof(DispatchParams),
    //PIPELINE_BARRIER          = 19,
    sizeof(PipelineBarrierParams),
    //RESET_EVENT               = 20,
    sizeof(ResetEventParams),
    //SET_EVENT                 = 21,
    sizeof(SetEventParams),
    //WAIT_EVENTS               = 22,
    sizeof(WaitEventsParams),
    //RESET_QUERY_POOL          = 23,
    sizeof(ResetQueryPoolParams),
    //BEGIN_QUERY               = 24,
    sizeof(BeginQueryParams),
    //END_QUERY                 = 25,
    sizeof(EndQueryParams),
    //WRITE_TIMESTAMP           = 26,
    sizeof(WriteTimestampParams),
};
#endif
// Header for every cmd in custom cmd buffer
struct CommandHeader
{
    CommandType type : 8;
    CommandHeader *next;
};

class CustomCommandBuffer final : angle::NonCopyable
{
  public:
    CustomCommandBuffer() : mHead(nullptr), mLast(nullptr) {}
    ~CustomCommandBuffer() {}

    // Add commands
    void bindDescriptorSets(VkPipelineBindPoint bindPoint,
                            const PipelineLayout &layout,
                            uint32_t firstSet,
                            uint32_t descriptorSetCount,
                            const VkDescriptorSet *descriptorSets,
                            uint32_t dynamicOffsetCount,
                            const uint32_t *dynamicOffsets);

    void bindIndexBuffer(const VkBuffer &buffer, VkDeviceSize offset, VkIndexType indexType);

    void bindPipeline(VkPipelineBindPoint pipelineBindPoint, const Pipeline &pipeline);

    void bindVertexBuffers(uint32_t firstBinding,
                           uint32_t bindingCount,
                           const VkBuffer *buffers,
                           const VkDeviceSize *offsets);

    void blitImage(const Image &srcImage,
                   VkImageLayout srcImageLayout,
                   const Image &dstImage,
                   VkImageLayout dstImageLayout,
                   uint32_t regionCount,
                   VkImageBlit *pRegions,
                   VkFilter filter);

    void copyBuffer(const VkBuffer &srcBuffer,
                    const VkBuffer &destBuffer,
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

    void updateBuffer(const vk::Buffer &buffer,
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
     //size_t commandSize();
};

}  // namespace vk

// List of function pointers for used extensions.
// VK_EXT_debug_utils
extern PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
extern PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;

// VK_EXT_debug_report
extern PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
extern PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;

// VK_KHR_get_physical_device_properties2
extern PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR;

// Lazily load entry points for each extension as necessary.
void InitDebugUtilsEXTFunctions(VkInstance instance);
void InitDebugReportEXTFunctions(VkInstance instance);
void InitGetPhysicalDeviceProperties2KHRFunctions(VkInstance instance);

#if defined(ANGLE_PLATFORM_FUCHSIA)
// VK_FUCHSIA_imagepipe_surface
extern PFN_vkCreateImagePipeSurfaceFUCHSIA vkCreateImagePipeSurfaceFUCHSIA;
void InitImagePipeSurfaceFUCHSIAFunctions(VkInstance instance);
#endif

namespace gl_vk
{
VkRect2D GetRect(const gl::Rectangle &source);
VkFilter GetFilter(const GLenum filter);
VkSamplerMipmapMode GetSamplerMipmapMode(const GLenum filter);
VkSamplerAddressMode GetSamplerAddressMode(const GLenum wrap);
VkPrimitiveTopology GetPrimitiveTopology(gl::PrimitiveMode mode);
VkCullModeFlags GetCullMode(const gl::RasterizerState &rasterState);
VkFrontFace GetFrontFace(GLenum frontFace, bool invertCullFace);
VkSampleCountFlagBits GetSamples(GLint sampleCount);
VkComponentSwizzle GetSwizzle(const GLenum swizzle);

constexpr angle::PackedEnumMap<gl::DrawElementsType, VkIndexType> kIndexTypeMap = {
    {gl::DrawElementsType::UnsignedByte, VK_INDEX_TYPE_UINT16},
    {gl::DrawElementsType::UnsignedShort, VK_INDEX_TYPE_UINT16},
    {gl::DrawElementsType::UnsignedInt, VK_INDEX_TYPE_UINT32},
};

void GetOffset(const gl::Offset &glOffset, VkOffset3D *vkOffset);
void GetExtent(const gl::Extents &glExtent, VkExtent3D *vkExtent);
VkImageType GetImageType(gl::TextureType textureType);
VkImageViewType GetImageViewType(gl::TextureType textureType);
VkColorComponentFlags GetColorComponentFlags(bool red, bool green, bool blue, bool alpha);

void GetViewport(const gl::Rectangle &viewport,
                 float nearPlane,
                 float farPlane,
                 bool invertViewport,
                 GLint renderAreaHeight,
                 VkViewport *viewportOut);
void GetScissor(const gl::State &glState,
                bool invertViewport,
                const gl::Rectangle &renderArea,
                VkRect2D *scissorOut);
}  // namespace gl_vk

}  // namespace rx

#define ANGLE_VK_TRY(context, command)                                                 \
    do                                                                                 \
    {                                                                                  \
        auto ANGLE_LOCAL_VAR = command;                                                \
        if (ANGLE_UNLIKELY(ANGLE_LOCAL_VAR != VK_SUCCESS))                             \
        {                                                                              \
            context->handleError(ANGLE_LOCAL_VAR, __FILE__, ANGLE_FUNCTION, __LINE__); \
            return angle::Result::Stop;                                                \
        }                                                                              \
    } while (0)

#define ANGLE_VK_CHECK(context, test, error) ANGLE_VK_TRY(context, test ? VK_SUCCESS : error)

#define ANGLE_VK_CHECK_MATH(context, result) \
    ANGLE_VK_CHECK(context, result, VK_ERROR_VALIDATION_FAILED_EXT)

#define ANGLE_VK_CHECK_ALLOC(context, result) \
    ANGLE_VK_CHECK(context, result, VK_ERROR_OUT_OF_HOST_MEMORY)

#define ANGLE_VK_UNREACHABLE(context) \
    UNREACHABLE();                    \
    ANGLE_VK_CHECK(context, false, VK_ERROR_FEATURE_NOT_PRESENT)

#endif  // LIBANGLE_RENDERER_VULKAN_VK_UTILS_H_
