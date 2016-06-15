//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// renderervk_utils:
//    Helper functions for the Vulkan Renderer.
//

#include "renderervk_utils.h"

#include "common/debug.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"

namespace rx
{

namespace
{
GLenum DefaultGLErrorCode(VkResult result)
{
    switch (result)
    {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        case VK_ERROR_TOO_MANY_OBJECTS:
            return GL_OUT_OF_MEMORY;
        default:
            return GL_INVALID_OPERATION;
    }
}

EGLint DefaultEGLErrorCode(VkResult result)
{
    switch (result)
    {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        case VK_ERROR_TOO_MANY_OBJECTS:
            return EGL_BAD_ALLOC;
        case VK_ERROR_INITIALIZATION_FAILED:
            return EGL_NOT_INITIALIZED;
        case VK_ERROR_SURFACE_LOST_KHR:
        case VK_ERROR_DEVICE_LOST:
            return EGL_CONTEXT_LOST;
        default:
            return EGL_BAD_ACCESS;
    }
}
}  // anonymous namespace

// Mirrors std_validation_str in loader.h
// TODO(jmadill): Possibly wrap the loader into a safe source file. Can't be included trivially.
const char *g_VkStdValidationLayerName = "VK_LAYER_LUNARG_standard_validation";

const char *VulkanResultString(VkResult result)
{
    switch (result)
    {
        case VK_SUCCESS:
            return "Command successfully completed.";
        case VK_NOT_READY:
            return "A fence or query has not yet completed.";
        case VK_TIMEOUT:
            return "A wait operation has not completed in the specified time.";
        case VK_EVENT_SET:
            return "An event is signaled.";
        case VK_EVENT_RESET:
            return "An event is unsignaled.";
        case VK_INCOMPLETE:
            return "A return array was too small for the result.";
        case VK_SUBOPTIMAL_KHR:
            return "A swapchain no longer matches the surface properties exactly, but can still be "
                   "used to present to the surface successfully.";
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "A host memory allocation has failed.";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "A device memory allocation has failed.";
        case VK_ERROR_INITIALIZATION_FAILED:
            return "Initialization of an object could not be completed for implementation-specific "
                   "reasons.";
        case VK_ERROR_DEVICE_LOST:
            return "The logical or physical device has been lost.";
        case VK_ERROR_MEMORY_MAP_FAILED:
            return "Mapping of a memory object has failed.";
        case VK_ERROR_LAYER_NOT_PRESENT:
            return "A requested layer is not present or could not be loaded.";
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            return "A requested extension is not supported.";
        case VK_ERROR_FEATURE_NOT_PRESENT:
            return "A requested feature is not supported.";
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            return "The requested version of Vulkan is not supported by the driver or is otherwise "
                   "incompatible for implementation-specific reasons.";
        case VK_ERROR_TOO_MANY_OBJECTS:
            return "Too many objects of the type have already been created.";
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            return "A requested format is not supported on this device.";
        case VK_ERROR_SURFACE_LOST_KHR:
            return "A surface is no longer available.";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            return "The requested window is already connected to a VkSurfaceKHR, or to some other "
                   "non-Vulkan API.";
        case VK_ERROR_OUT_OF_DATE_KHR:
            return "A surface has changed in such a way that it is no longer compatible with the "
                   "swapchain.";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            return "The display used by a swapchain does not use the same presentable image "
                   "layout, or is incompatible in a way that prevents sharing an image.";
        case VK_ERROR_VALIDATION_FAILED_EXT:
            return "The validation layers detected invalid API usage.";
        default:
            return "Unknown vulkan error code.";
    }
}

bool HasStandardValidationLayer(const std::vector<VkLayerProperties> &layerProps)
{
    // Compile the layer names into a set.
    std::set<std::string> layerNames;
    for (const auto &layerProp : layerProps)
    {
        layerNames.insert(layerProp.layerName);
    }

    return layerNames.count(g_VkStdValidationLayerName) > 0;
}

namespace vk
{

Error::Error(VkResult result) : mResult(result), mFile(nullptr), mLine(0)
{
    ASSERT(result == VK_SUCCESS);
}

Error::Error(VkResult result, const char *file, unsigned int line)
    : mResult(result), mFile(file), mLine(line)
{
}

Error::~Error()
{
}

Error::Error(const Error &other) = default;
Error &Error::operator=(const Error &other) = default;

gl::Error Error::toGL(GLenum glErrorCode) const
{
    if (!isError())
    {
        return gl::NoError();
    }

    // TODO(jmadill): Set extended error code to 'vulkan internal error'.
    const std::string &message = getExtendedMessage();
    return gl::Error(glErrorCode, message.c_str());
}

egl::Error Error::toEGL(EGLint eglErrorCode) const
{
    if (!isError())
    {
        return egl::Error(EGL_SUCCESS);
    }

    // TODO(jmadill): Set extended error code to 'vulkan internal error'.
    const std::string &message = getExtendedMessage();
    return egl::Error(eglErrorCode, message.c_str());
}

std::string Error::getExtendedMessage() const
{
    std::stringstream errorStream;
    errorStream << "Internal Vulkan error: " << VulkanResultString(mResult) << ", in " << mFile
                << ", line " << mLine << ".";
    return errorStream.str();
}

Error::operator gl::Error() const
{
    return toGL(DefaultGLErrorCode(mResult));
}

Error::operator egl::Error() const
{
    return toEGL(DefaultEGLErrorCode(mResult));
}

bool Error::isError() const
{
    return (mResult != VK_SUCCESS);
}

Error VkSuccess()
{
    return Error(VK_SUCCESS);
}

// CommandBuffer implementation.
CommandBuffer::CommandBuffer(VkDevice device, VkCommandPool commandPool)
    : mDevice(device), mCommandPool(commandPool), mHandle(VK_NULL_HANDLE)
{
}

Error CommandBuffer::begin()
{
    if (mHandle == VK_NULL_HANDLE)
    {
        VkCommandBufferAllocateInfo commandBufferInfo = {};
        commandBufferInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferInfo.pNext              = nullptr;
        commandBufferInfo.commandPool        = mCommandPool;
        commandBufferInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferInfo.commandBufferCount = 1;

        ANGLE_VK_TRY(vkAllocateCommandBuffers(mDevice, &commandBufferInfo, &mHandle));
    }
    else
    {
        reset();
    }

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext                    = nullptr;
    // TODO(jmadill): Use other flags?
    beginInfo.flags            = 0;
    beginInfo.pInheritanceInfo = nullptr;

    ANGLE_VK_TRY(vkBeginCommandBuffer(mHandle, &beginInfo));

    return VkSuccess();
}

Error CommandBuffer::end()
{
    ASSERT(mHandle != VK_NULL_HANDLE);
    ANGLE_VK_TRY(vkEndCommandBuffer(mHandle));
    return VkSuccess();
}

Error CommandBuffer::reset()
{
    ASSERT(mHandle != VK_NULL_HANDLE);
    ANGLE_VK_TRY(vkResetCommandBuffer(mHandle, 0));
    return VkSuccess();
}

void CommandBuffer::changeImageLayout(VkImage image,
                                      VkImageAspectFlags aspectMask,
                                      VkImageLayout oldLayout,
                                      VkImageLayout newLayout)
{
    ASSERT(mHandle != VK_NULL_HANDLE);
    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType                = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.pNext                = nullptr;
    imageMemoryBarrier.srcAccessMask        = 0;
    imageMemoryBarrier.dstAccessMask        = 0;
    imageMemoryBarrier.oldLayout            = oldLayout;
    imageMemoryBarrier.newLayout            = newLayout;
    imageMemoryBarrier.srcQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.image                = image;

    // TODO(jmadill): Is this needed for mipped/layer images?
    imageMemoryBarrier.subresourceRange.aspectMask     = aspectMask;
    imageMemoryBarrier.subresourceRange.baseMipLevel   = 0;
    imageMemoryBarrier.subresourceRange.levelCount     = 1;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.layerCount     = 1;

    if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        imageMemoryBarrier.srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        imageMemoryBarrier.srcAccessMask |= VK_ACCESS_TRANSFER_WRITE_BIT;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
    {
        imageMemoryBarrier.srcAccessMask |= VK_ACCESS_HOST_WRITE_BIT;
    }

    if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        imageMemoryBarrier.dstAccessMask |= VK_ACCESS_TRANSFER_WRITE_BIT;
    }

    if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
    {
        imageMemoryBarrier.dstAccessMask |= VK_ACCESS_TRANSFER_READ_BIT;
    }

    if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        imageMemoryBarrier.srcAccessMask |=
            (VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT);
        imageMemoryBarrier.dstAccessMask |= VK_ACCESS_SHADER_READ_BIT;
    }

    if (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        imageMemoryBarrier.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        imageMemoryBarrier.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }

    imageBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                 imageMemoryBarrier);
}

void CommandBuffer::clearSingleColorImage(VkImage image,
                                          VkImageLayout imageLayout,
                                          const VkClearColorValue &color)
{
    ASSERT(mHandle != VK_NULL_HANDLE);

    VkImageSubresourceRange range;
    range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel   = 0;
    range.levelCount     = 1;
    range.baseArrayLayer = 0;
    range.layerCount     = 1;

    vkCmdClearColorImage(mHandle, image, imageLayout, &color, 1, &range);
}

void CommandBuffer::copySingleImage(VkImage srcImage,
                                    VkImageLayout srcImageLayout,
                                    VkImage destImage,
                                    VkImageLayout destImageLayout,
                                    const gl::Box &copyRegion,
                                    VkImageAspectFlags aspectMask)
{
    ASSERT(mHandle != VK_NULL_HANDLE);

    VkImageCopy region;
    region.srcSubresource.aspectMask     = aspectMask;
    region.srcSubresource.mipLevel       = 0;
    region.srcSubresource.baseArrayLayer = 0;
    region.srcSubresource.layerCount     = 1;
    region.srcOffset.x                   = copyRegion.x;
    region.srcOffset.y                   = copyRegion.y;
    region.srcOffset.z                   = copyRegion.z;
    region.dstSubresource.aspectMask     = aspectMask;
    region.dstSubresource.mipLevel       = 0;
    region.dstSubresource.baseArrayLayer = 0;
    region.dstSubresource.layerCount     = 1;
    region.dstOffset.x                   = copyRegion.x;
    region.dstOffset.y                   = copyRegion.y;
    region.dstOffset.z                   = copyRegion.z;
    region.extent.width                  = copyRegion.width;
    region.extent.height                 = copyRegion.height;
    region.extent.depth                  = copyRegion.depth;

    vkCmdCopyImage(mHandle, srcImage, srcImageLayout, destImage, destImageLayout, 1, &region);
}

CommandBuffer::~CommandBuffer()
{
    if (mHandle)
    {
        vkFreeCommandBuffers(mDevice, mCommandPool, 1, &mHandle);
        mHandle = VK_NULL_HANDLE;
    }
}

void CommandBuffer::beginRenderPass(const RenderPass &renderPass,
                                    const Framebuffer &framebuffer,
                                    const gl::Rectangle &renderArea,
                                    const std::vector<VkClearValue> &clearValues)
{
    ASSERT(!clearValues.empty());
    ASSERT(mHandle != VK_NULL_HANDLE);

    VkRenderPassBeginInfo beginInfo;
    beginInfo.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.pNext                    = nullptr;
    beginInfo.renderPass               = renderPass.getHandle();
    beginInfo.framebuffer              = framebuffer.getHandle();
    beginInfo.renderArea.offset.x      = static_cast<uint32_t>(renderArea.x);
    beginInfo.renderArea.offset.y      = static_cast<uint32_t>(renderArea.y);
    beginInfo.renderArea.extent.width  = static_cast<uint32_t>(renderArea.width);
    beginInfo.renderArea.extent.height = static_cast<uint32_t>(renderArea.height);
    beginInfo.clearValueCount          = static_cast<uint32_t>(clearValues.size());
    beginInfo.pClearValues             = clearValues.data();

    vkCmdBeginRenderPass(mHandle, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::endRenderPass()
{
    ASSERT(mHandle != VK_NULL_HANDLE);
    vkCmdEndRenderPass(mHandle);
}

void CommandBuffer::imageBarrier(VkPipelineStageFlags srcStageMask,
                                 VkPipelineStageFlags destStageMask,
                                 const VkImageMemoryBarrier &barrier)
{
    ASSERT(mHandle != VK_NULL_HANDLE);
    vkCmdPipelineBarrier(mHandle, srcStageMask, destStageMask, 0, 0, nullptr, 0, nullptr, 1,
                         &barrier);
}

// Semaphore implementation.
Semaphore::Semaphore() : mDevice(VK_NULL_HANDLE), mHandle(VK_NULL_HANDLE)
{
}

Semaphore::Semaphore(VkDevice device) : mDevice(device), mHandle(VK_NULL_HANDLE)
{
}

Semaphore::Semaphore(Semaphore &&other) : mDevice(other.mDevice), mHandle(other.mHandle)
{
    other.mDevice = VK_NULL_HANDLE;
    other.mHandle = VK_NULL_HANDLE;
}

Semaphore::~Semaphore()
{
    if (mHandle != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(mDevice, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

Semaphore &Semaphore::operator=(Semaphore &&other)
{
    std::swap(mDevice, other.mDevice);
    std::swap(mHandle, other.mHandle);
    return *this;
}

Error Semaphore::init()
{
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext                 = nullptr;
    semaphoreInfo.flags                 = 0;

    ANGLE_VK_TRY(vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mHandle));

    return VkSuccess();
}

// RenderPass implementation.
RenderPass::RenderPass() : mDevice(VK_NULL_HANDLE), mHandle(VK_NULL_HANDLE)
{
}

RenderPass::RenderPass(VkDevice device) : mDevice(device), mHandle(VK_NULL_HANDLE)
{
}

RenderPass::RenderPass(RenderPass &&other) : mDevice(other.mDevice), mHandle(other.mHandle)
{
    other.mDevice = VK_NULL_HANDLE;
    other.mHandle = VK_NULL_HANDLE;
}

RenderPass::~RenderPass()
{
    if (mHandle != VK_NULL_HANDLE)
    {
        ASSERT(mDevice != VK_NULL_HANDLE);
        vkDestroyRenderPass(mDevice, mHandle, nullptr);
    }
}

RenderPass &RenderPass::operator=(RenderPass &&other)
{
    std::swap(mDevice, other.mDevice);
    std::swap(mHandle, other.mHandle);
    return *this;
}

Error RenderPass::init(const VkRenderPassCreateInfo &createInfo)
{
    ASSERT(mDevice != VK_NULL_HANDLE && mHandle == VK_NULL_HANDLE);
    ANGLE_VK_TRY(vkCreateRenderPass(mDevice, &createInfo, nullptr, &mHandle));
    return VkSuccess();
}

Framebuffer::Framebuffer() : mDevice(VK_NULL_HANDLE), mHandle(VK_NULL_HANDLE)
{
}

Framebuffer::Framebuffer(VkDevice device) : mDevice(device), mHandle(VK_NULL_HANDLE)
{
}

Framebuffer::Framebuffer(Framebuffer &&other) : mDevice(other.mDevice), mHandle(other.mHandle)
{
    other.mDevice = VK_NULL_HANDLE;
    other.mHandle = VK_NULL_HANDLE;
}

Framebuffer::~Framebuffer()
{
    if (mHandle != VK_NULL_HANDLE)
    {
        ASSERT(mDevice != VK_NULL_HANDLE);
        vkDestroyFramebuffer(mDevice, mHandle, nullptr);
    }
}

Framebuffer &Framebuffer::operator=(Framebuffer &&other)
{
    std::swap(mDevice, other.mDevice);
    std::swap(mHandle, other.mHandle);
    return *this;
}

Error Framebuffer::init(const VkFramebufferCreateInfo &createInfo)
{
    ASSERT(mDevice != VK_NULL_HANDLE && mHandle == VK_NULL_HANDLE);
    ANGLE_VK_TRY(vkCreateFramebuffer(mDevice, &createInfo, nullptr, &mHandle));
    return VkSuccess();
}

}  // namespace vk

}  // namespace rx
