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
struct Rectangle;
}

namespace rx
{
const char *VulkanResultString(VkResult result);
bool HasStandardValidationLayer(const std::vector<VkLayerProperties> &layerProps);

extern const char *g_VkStdValidationLayerName;

namespace vk
{
class Framebuffer;

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

// Helper class that wraps a Vulkan command buffer.
class CommandBuffer final : angle::NonCopyable
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

    VkCommandBuffer getHandle() const { return mHandle; }

  private:
    VkDevice mDevice;
    VkCommandPool mCommandPool;
    VkCommandBuffer mHandle;
};

class Semaphore final : angle::NonCopyable
{
  public:
    Semaphore();
    Semaphore(VkDevice device);
    Semaphore(Semaphore &&other);
    ~Semaphore();
    Semaphore &operator=(Semaphore &&other);

    Error init();

    VkSemaphore getHandle() const { return mHandle; }

  private:
    VkDevice mDevice;
    VkSemaphore mHandle;
};

class Framebuffer final : angle::NonCopyable
{
  public:
    Framebuffer();
    Framebuffer(VkDevice device);
    Framebuffer(Framebuffer &&other);
    ~Framebuffer();
    Framebuffer &operator=(Framebuffer &&other);

    Error init(const VkFramebufferCreateInfo &createInfo);

    VkFramebuffer getHandle() const { return mHandle; }

  private:
    VkDevice mDevice;
    VkFramebuffer mHandle;
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
