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

#include <vulkan/vulkan.h>

#include "libANGLE/Error.h"

namespace gl
{
struct Box;
struct Extents;
struct Rectangle;
}

namespace rx
{
const char *VulkanResultString(VkResult result);
bool HasStandardValidationLayer(const std::vector<VkLayerProperties> &layerProps);

extern const char *g_VkStdValidationLayerName;

enum class TextureDimension
{
    TEX_2D,
    TEX_CUBE,
    TEX_3D,
    TEX_2D_ARRAY,
};

namespace vk
{
class Framebuffer;
class RenderPass;

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

    bool isError() const;

  private:
    std::string getExtendedMessage() const;

    VkResult mResult;
    const char *mFile;
    unsigned int mLine;
};

template <typename ResultT>
using ErrorOrResult = angle::ErrorOrResultBase<Error, ResultT, VkResult, VK_SUCCESS>;

// Avoid conflicting with X headers which define "Success".
Error VkSuccess();

template <typename HandleT>
class WrappedObject : angle::NonCopyable
{
  public:
    WrappedObject() : mDevice(VK_NULL_HANDLE), mHandle(VK_NULL_HANDLE) {}
    WrappedObject(VkDevice device) : mDevice(device), mHandle(VK_NULL_HANDLE) {}
    WrappedObject(WrappedObject &&other) : mDevice(other.mDevice), mHandle(other.mHandle)
    {
        other.mDevice = VK_NULL_HANDLE;
        other.mHandle = VK_NULL_HANDLE;
    }
    virtual ~WrappedObject() {}

    HandleT getHandle() const { return mHandle; }

  protected:
    VkDevice mDevice;
    HandleT mHandle;
};

// Helper class that wraps a Vulkan command buffer.
class CommandBuffer final : public WrappedObject<VkCommandBuffer>
{
  public:
    CommandBuffer(VkDevice device, VkCommandPool commandPool);
    ~CommandBuffer();

    Error begin();
    Error end();
    Error reset();
    void changeImageLayout(VkImage image,
                           VkImageAspectFlags aspectMask,
                           VkImageLayout oldLayout,
                           VkImageLayout newLayout);

    void beginRenderPass(const RenderPass &renderPass,
                         const Framebuffer &framebuffer,
                         const gl::Rectangle &renderArea,
                         const std::vector<VkClearValue> &clearValues);
    void endRenderPass();

    void imageBarrier(VkPipelineStageFlags srcStageMask,
                      VkPipelineStageFlags destStageMask,
                      const VkImageMemoryBarrier &barrier);

    void clearSingleColorImage(VkImage image,
                               VkImageLayout imageLayout,
                               const VkClearColorValue &color);

    void copySingleImage(VkImage srcImage,
                         VkImageLayout srcImageLayout,
                         VkImage destImage,
                         VkImageLayout destImageLayout,
                         const gl::Box &copyRegion,
                         VkImageAspectFlags aspectMask);

  private:
    VkCommandPool mCommandPool;
};

class Semaphore final : public WrappedObject<VkSemaphore>
{
  public:
    Semaphore();
    Semaphore(VkDevice device);
    Semaphore(Semaphore &&other);
    ~Semaphore();
    Semaphore &operator=(Semaphore &&other);

    Error init();
};

class Framebuffer final : public WrappedObject<VkFramebuffer>
{
  public:
    Framebuffer();
    Framebuffer(VkDevice device);
    Framebuffer(Framebuffer &&other);
    ~Framebuffer();
    Framebuffer &operator=(Framebuffer &&other);

    Error init(const VkFramebufferCreateInfo &createInfo);
};

class DeviceMemory final : public WrappedObject<VkDeviceMemory>
{
  public:
    DeviceMemory();
    DeviceMemory(VkDevice device);
    DeviceMemory(DeviceMemory &&other);
    ~DeviceMemory();
    DeviceMemory &operator=(DeviceMemory &&other);

    Error allocate(const VkMemoryAllocateInfo &allocInfo);
    Error map(VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, uint8_t **mapPointer);
    void unmap();
};

class RenderPass final : public WrappedObject<VkRenderPass>
{
  public:
    RenderPass();
    RenderPass(VkDevice device);
    RenderPass(RenderPass &&other);
    ~RenderPass();
    RenderPass &operator=(RenderPass &&other);

    Error init(const VkRenderPassCreateInfo &createInfo);
};

class StagingImage final : angle::NonCopyable
{
  public:
    StagingImage();
    StagingImage(StagingImage &&other);
    ~StagingImage();
    StagingImage &operator=(StagingImage &&other);

    vk::Error init(VkDevice device,
                   uint32_t queueFamilyIndex,
                   uint32_t hostVisibleMemoryIndex,
                   TextureDimension dimension,
                   VkFormat format,
                   const gl::Extents &extent);

    VkImage getHandle() const { return mHandle; }
    DeviceMemory &getMemory() { return mMemory; }
    const DeviceMemory &getMemory() const { return mMemory; }
    VkDeviceSize getSize() const { return mSize; }

  private:
    VkDevice mDevice;
    VkImage mHandle;
    DeviceMemory mMemory;
    VkDeviceSize mSize;
};

}  // namespace vk

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
#define ANGLE_VK_UNREACHABLE() rx::vk::Error(VK_ERROR_VALIDATION_FAILED_EXT, __FILE__, __LINE__)

#endif  // LIBANGLE_RENDERER_VULKAN_RENDERERVK_UTILS_H_
