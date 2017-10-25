//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// renderervk_utils:
//    Helper functions for the Vulkan Renderer.
//

#ifndef LIBANGLE_RENDERER_VULKAN_RENDERERVK_UTILS_H_
#define LIBANGLE_RENDERER_VULKAN_RENDERERVK_UTILS_H_

#include <limits>

#include <vulkan/vulkan.h>

#include "common/Optional.h"
#include "common/debug.h"
#include "libANGLE/Error.h"
#include "libANGLE/renderer/renderer_utils.h"

namespace gl
{
struct Box;
struct Extents;
struct RasterizerState;
struct Rectangle;
}

namespace rx
{
class ContextVk;

const char *VulkanResultString(VkResult result);
bool HasStandardValidationLayer(const std::vector<VkLayerProperties> &layerProps);

extern const char *g_VkStdValidationLayerName;
extern const char *g_VkLoaderLayersPathEnv;

enum class TextureDimension
{
    TEX_2D,
    TEX_CUBE,
    TEX_3D,
    TEX_2D_ARRAY,
};

enum DeleteSchedule
{
    NOW,
    LATER,
};

namespace vk
{
class Buffer;
class DeviceMemory;
class Framebuffer;
class Image;
class Pipeline;
class PipelineLayout;
class RenderPass;

template <typename ResourceT>
class Pointer;

class MemoryProperties final : angle::NonCopyable
{
  public:
    MemoryProperties();

    void init(VkPhysicalDevice physicalDevice);
    uint32_t findCompatibleMemoryIndex(uint32_t bitMask, uint32_t propertyFlags) const;

  private:
    VkPhysicalDeviceMemoryProperties mMemoryProperties;
};

class Error final
{
  public:
    Error(VkResult result);
    Error(VkResult result, const char *file, unsigned int line);
    ~Error();

    Error(const Error &other);
    Error &operator=(const Error &other);

    gl::Error toGL(GLenum glErrorCode) const;
    egl::Error toEGL(EGLint eglErrorCode) const;

    operator gl::Error() const;
    operator egl::Error() const;

    template <typename T>
    operator gl::ErrorOrResult<T>() const
    {
        return operator gl::Error();
    }

    bool isError() const;

    std::string toString() const;

  private:
    VkResult mResult;
    const char *mFile;
    unsigned int mLine;
};

template <typename ResultT>
using ErrorOrResult = angle::ErrorOrResultBase<Error, ResultT, VkResult, VK_SUCCESS>;

// Avoid conflicting with X headers which define "Success".
inline Error NoError()
{
    return Error(VK_SUCCESS);
}

// A vk::Resource is a reference-counted object that can be used in a graphics, compute or copy
// queue. They are reference counted to keep a fairly simple implementation for lifetime
// management. The destroy function is virtual so it can be called via release().
class Resource : angle::NonCopyable
{
  public:
    // Releasing can trigger object deallocation, hence it needs a device context.
    void release(VkDevice device);
    void addRef();

  protected:
    Resource();
    virtual ~Resource();
    virtual void destroy(VkDevice device) = 0;

  private:
    uint32_t mRefCount;
};

template <typename HandleT>
class TypedResource : public Resource
{
  public:
    HandleT getHandle() const { return mHandle; }
    bool valid() const { return (mHandle != VK_NULL_HANDLE); }

    const HandleT *ptr() const { return &mHandle; }

  protected:
    TypedResource() : mHandle(VK_NULL_HANDLE) {}
    TypedResource(HandleT handle) : mHandle(handle) {}
    ~TypedResource() override { ASSERT(!valid()); }

    HandleT mHandle;
};

// Use this smart pointer class to keep references to vk::Resource.
template <typename ResourceT>
class Pointer final : angle::NonCopyable
{
  public:
    Pointer() : mResource(nullptr) {}
    Pointer(Resource *resource) : mResource(resource) { resource->addRef(); }
    ~Pointer() { ASSERT(!mResource); }

    Pointer(Pointer &&other) : mResource(other.mResource) { other.mResource = nullptr; }

    Pointer &operator=(Pointer &&other)
    {
        std::swap(mResource, other.mResource);
        return *this;
    }

    void bind(VkDevice device, ResourceT *resource)
    {
        reset(device);
        mResource = resource;
        mResource->addRef();
    }

    void reset(VkDevice device)
    {
        if (mResource)
        {
            mResource->release(device);
        }
        mResource = nullptr;
    }

    ResourceT *release(VkDevice device)
    {
        ResourceT *retVal = mResource;
        reset(device);
        return retVal;
    }

    operator ResourceT *() { return mResource; }

    ResourceT *operator->() const { return mResource; }

    ResourceT &operator*()
    {
        ASSERT(mResource);
        return *mResource;
    }

    const ResourceT &operator*() const
    {
        ASSERT(mResource);
        return *mResource;
    }

    ResourceT *get() const { return mResource; }

    template <typename... ArgsT>
    Error init(ArgsT &&... args)
    {
        ASSERT(!mResource);
        mResource = new ResourceT;
        return mResource->init(std::forward<ArgsT>(args)...);
    }

    bool valid() const { return mResource && mResource->valid(); }

  private:
    ResourceT *mResource;
};

class CommandPool final : public TypedResource<VkCommandPool>
{
  public:
    CommandPool();

    void destroy(VkDevice device) override;

    Error init(VkDevice device, const VkCommandPoolCreateInfo &createInfo);
};

// Helper class that wraps a Vulkan command buffer.
class CommandBuffer final : public TypedResource<VkCommandBuffer>
{
  public:
    CommandBuffer();

    bool started() const { return mStarted; }

    void destroy(VkDevice device) override;

    void setCommandPool(VkDevice device, CommandPool *commandPool);
    Error begin(VkDevice device);
    Error end();
    Error reset(VkDevice device);

    void singleImageBarrier(VkPipelineStageFlags srcStageMask,
                            VkPipelineStageFlags dstStageMask,
                            VkDependencyFlags dependencyFlags,
                            const VkImageMemoryBarrier &imageMemoryBarrier);

    void clearSingleColorImage(const vk::Image &image, const VkClearColorValue &color);

    void copySingleImage(const vk::Image &srcImage,
                         const vk::Image &destImage,
                         const gl::Box &copyRegion,
                         VkImageAspectFlags aspectMask);

    void copyImage(const vk::Image &srcImage,
                   const vk::Image &dstImage,
                   uint32_t regionCount,
                   const VkImageCopy *regions);

    void beginRenderPass(const RenderPass &renderPass,
                         const Framebuffer &framebuffer,
                         const gl::Rectangle &renderArea,
                         const std::vector<VkClearValue> &clearValues);
    void endRenderPass();

    void draw(uint32_t vertexCount,
              uint32_t instanceCount,
              uint32_t firstVertex,
              uint32_t firstInstance);

    void drawIndexed(uint32_t indexCount,
                     uint32_t instanceCount,
                     uint32_t firstIndex,
                     int32_t vertexOffset,
                     uint32_t firstInstance);

    void bindPipeline(VkPipelineBindPoint pipelineBindPoint, const vk::Pipeline &pipeline);
    void bindVertexBuffers(uint32_t firstBinding,
                           uint32_t bindingCount,
                           const VkBuffer *buffers,
                           const VkDeviceSize *offsets);
    void bindIndexBuffer(const vk::Buffer &buffer, VkDeviceSize offset, VkIndexType indexType);
    void bindDescriptorSets(VkPipelineBindPoint bindPoint,
                            const vk::PipelineLayout &layout,
                            uint32_t firstSet,
                            uint32_t descriptorSetCount,
                            const VkDescriptorSet *descriptorSets,
                            uint32_t dynamicOffsetCount,
                            const uint32_t *dynamicOffsets);

    void addResource(Resource *resource);

  private:
    bool mStarted;
    Pointer<CommandPool> mCommandPool;
    std::vector<Pointer<Resource>> mResources;
};

class Image final : public TypedResource<VkImage>
{
  public:
    // Use this constructor if the lifetime of the image is not controlled by ANGLE. (SwapChain)
    Image();
    explicit Image(VkImage image);

    // Called on shutdown when the helper class *doesn't* own the handle to the image resource.
    void reset();

    // Called on shutdown when the helper class *does* own the handle to the image resource.
    void destroy(VkDevice device) override;

    Error init(VkDevice device, const VkImageCreateInfo &createInfo);

    void changeLayoutTop(VkImageAspectFlags aspectMask,
                         VkImageLayout newLayout,
                         CommandBuffer *commandBuffer);

    void changeLayoutWithStages(VkImageAspectFlags aspectMask,
                                VkImageLayout newLayout,
                                VkPipelineStageFlags srcStageMask,
                                VkPipelineStageFlags dstStageMask,
                                CommandBuffer *commandBuffer);

    void getMemoryRequirements(VkDevice device, VkMemoryRequirements *requirementsOut) const;
    Error bindMemory(VkDevice device, const vk::DeviceMemory &deviceMemory);

    VkImageLayout getCurrentLayout() const { return mCurrentLayout; }
    void updateLayout(VkImageLayout layout) { mCurrentLayout = layout; }

  private:
    VkImageLayout mCurrentLayout;
};

class ImageView final : public TypedResource<VkImageView>
{
  public:
    ImageView();

    Error init(VkDevice device, const VkImageViewCreateInfo &createInfo);

  protected:
    void destroy(VkDevice device) override;
};

class Semaphore final : public TypedResource<VkSemaphore>
{
  public:
    Semaphore();

    Error init(VkDevice device);

  protected:
    void destroy(VkDevice device) override;
};

class Framebuffer final : public TypedResource<VkFramebuffer>
{
  public:
    Framebuffer();

    Error init(VkDevice device, const VkFramebufferCreateInfo &createInfo);

  protected:
    void destroy(VkDevice device) override;
};

class DeviceMemory final : public TypedResource<VkDeviceMemory>
{
  public:
    DeviceMemory();

    Error init(VkDevice device, const VkMemoryAllocateInfo &allocInfo);
    Error map(VkDevice device,
              VkDeviceSize offset,
              VkDeviceSize size,
              VkMemoryMapFlags flags,
              uint8_t **mapPointer);
    void unmap(VkDevice device);

  protected:
    void destroy(VkDevice device) override;
};

class RenderPass final : public TypedResource<VkRenderPass>
{
  public:
    RenderPass();

    Error init(VkDevice device, const VkRenderPassCreateInfo &createInfo);

  protected:
    void destroy(VkDevice device) override;
};

enum class StagingUsage
{
    Read,
    Write,
    Both,
};

class StagingImage final : public Resource
{
  public:
    StagingImage();

    vk::Error init(VkDevice device,
                   uint32_t queueFamilyIndex,
                   const MemoryProperties &memoryProperties,
                   TextureDimension dimension,
                   VkFormat format,
                   const gl::Extents &extent,
                   StagingUsage usage);

    Image &getImage() { return mImage; }
    const Image &getImage() const { return mImage; }
    DeviceMemory &getDeviceMemory() { return mDeviceMemory; }
    const DeviceMemory &getDeviceMemory() const { return mDeviceMemory; }
    VkDeviceSize getSize() const { return mSize; }

  protected:
    void destroy(VkDevice device) override;

  private:
    Image mImage;
    DeviceMemory mDeviceMemory;
    VkDeviceSize mSize;
};

class Buffer final : public TypedResource<VkBuffer>
{
  public:
    Buffer();

    Error init(VkDevice device, const VkBufferCreateInfo &createInfo);
    Error bindMemory(VkDevice device, const DeviceMemory &deviceMemory);

  protected:
    void destroy(VkDevice device) override;
};

class ShaderModule final : public TypedResource<VkShaderModule>
{
  public:
    ShaderModule();

    Error init(VkDevice device, const VkShaderModuleCreateInfo &createInfo);

  protected:
    void destroy(VkDevice device) override;
};

class Pipeline final : public TypedResource<VkPipeline>
{
  public:
    Pipeline();

    Error initGraphics(VkDevice device, const VkGraphicsPipelineCreateInfo &createInfo);

  protected:
    void destroy(VkDevice device) override;
};

class PipelineLayout final : public TypedResource<VkPipelineLayout>
{
  public:
    PipelineLayout();
    Error init(VkDevice device, const VkPipelineLayoutCreateInfo &createInfo);

  protected:
    void destroy(VkDevice device) override;
};

class DescriptorSetLayout final : public TypedResource<VkDescriptorSetLayout>
{
  public:
    DescriptorSetLayout();

    Error init(VkDevice device, const VkDescriptorSetLayoutCreateInfo &createInfo);

  protected:
    void destroy(VkDevice device) override;
};

class DescriptorPool final : public TypedResource<VkDescriptorPool>
{
  public:
    DescriptorPool();

    Error init(VkDevice device, const VkDescriptorPoolCreateInfo &createInfo);

    Error allocateDescriptorSets(VkDevice device,
                                 const VkDescriptorSetAllocateInfo &allocInfo,
                                 VkDescriptorSet *descriptorSetsOut);

  protected:
    void destroy(VkDevice device) override;
};

class Sampler final : public TypedResource<VkSampler>
{
  public:
    Sampler();
    Error init(VkDevice device, const VkSamplerCreateInfo &createInfo);

  protected:
    void destroy(VkDevice device) override;
};

class Fence final : public TypedResource<VkFence>
{
  public:
    Fence();

    Error init(VkDevice device, const VkFenceCreateInfo &createInfo);
    VkResult getStatus(VkDevice device) const;

  protected:
    void destroy(VkDevice device) override;
};

Optional<uint32_t> FindMemoryType(const VkPhysicalDeviceMemoryProperties &memoryProps,
                                  const VkMemoryRequirements &requirements,
                                  uint32_t propertyFlagMask);

gl::Error AllocateBufferMemory(ContextVk *contextVk,
                               size_t size,
                               vk::Buffer *buffer,
                               vk::DeviceMemory *deviceMemoryOut,
                               size_t *requiredSizeOut);

struct BufferAndMemory final : private angle::NonCopyable
{
    vk::Buffer buffer;
    vk::DeviceMemory memory;
};

}  // namespace vk

namespace gl_vk
{
VkPrimitiveTopology GetPrimitiveTopology(GLenum mode);
VkCullModeFlags GetCullMode(const gl::RasterizerState &rasterState);
VkFrontFace GetFrontFace(GLenum frontFace);
}  // namespace gl_vk

}  // namespace rx

#define ANGLE_VK_TRY(command)                                          \
    {                                                                  \
        auto ANGLE_LOCAL_VAR = command;                                \
        if (ANGLE_LOCAL_VAR != VK_SUCCESS)                             \
        {                                                              \
            return rx::vk::Error(ANGLE_LOCAL_VAR, __FILE__, __LINE__); \
        }                                                              \
    }                                                                  \
    ANGLE_EMPTY_STATEMENT

#define ANGLE_VK_CHECK(test, error) ANGLE_VK_TRY(test ? VK_SUCCESS : error)

std::ostream &operator<<(std::ostream &stream, const rx::vk::Error &error);

#endif  // LIBANGLE_RENDERER_VULKAN_RENDERERVK_UTILS_H_
