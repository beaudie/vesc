//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SurfaceVk.cpp:
//    Implements the class methods for SurfaceVk.
//

#include "libANGLE/renderer/vulkan/SurfaceVk.h"

#include "common/debug.h"
#include "libANGLE/renderer/vulkan/FramebufferVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/renderer/vulkan/formatutilsvk.h"

namespace rx
{

namespace
{

const vk::Format &GetVkFormatFromConfig(const egl::Config &config)
{
    // TODO(jmadill): Properly handle format interpretation.
    return vk::Format::Get(GL_BGRA8_EXT);
}

}  // namespace

OffscreenSurfaceVk::OffscreenSurfaceVk(const egl::SurfaceState &surfaceState,
                                       RendererVk *renderer,
                                       EGLint width,
                                       EGLint height)
    : SurfaceImpl(surfaceState), mWidth(width), mHeight(height)
{
}

OffscreenSurfaceVk::~OffscreenSurfaceVk()
{
}

egl::Error OffscreenSurfaceVk::initialize()
{
    return egl::Error(EGL_SUCCESS);
}

FramebufferImpl *OffscreenSurfaceVk::createDefaultFramebuffer(const gl::FramebufferState &state)
{
    // Use a user FBO for an offscreen RT.
    return FramebufferVk::CreateUserFBO(state);
}

egl::Error OffscreenSurfaceVk::swap()
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error OffscreenSurfaceVk::postSubBuffer(EGLint /*x*/,
                                             EGLint /*y*/,
                                             EGLint /*width*/,
                                             EGLint /*height*/)
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error OffscreenSurfaceVk::querySurfacePointerANGLE(EGLint /*attribute*/, void ** /*value*/)
{
    UNREACHABLE();
    return egl::Error(EGL_BAD_CURRENT_SURFACE);
}

egl::Error OffscreenSurfaceVk::bindTexImage(gl::Texture * /*texture*/, EGLint /*buffer*/)
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error OffscreenSurfaceVk::releaseTexImage(EGLint /*buffer*/)
{
    return egl::Error(EGL_SUCCESS);
}

void OffscreenSurfaceVk::setSwapInterval(EGLint /*interval*/)
{
}

EGLint OffscreenSurfaceVk::getWidth() const
{
    return mWidth;
}

EGLint OffscreenSurfaceVk::getHeight() const
{
    return mHeight;
}

EGLint OffscreenSurfaceVk::isPostSubBufferSupported() const
{
    return EGL_FALSE;
}

EGLint OffscreenSurfaceVk::getSwapBehavior() const
{
    return EGL_BUFFER_PRESERVED;
}

gl::Error OffscreenSurfaceVk::getAttachmentRenderTarget(
    const gl::FramebufferAttachment::Target & /*target*/,
    FramebufferAttachmentRenderTarget ** /*rtOut*/)
{
    UNREACHABLE();
    return gl::Error(GL_INVALID_OPERATION);
}

WindowSurfaceVk::WindowSurfaceVk(const egl::SurfaceState &surfaceState,
                                 RendererVk *renderer,
                                 const egl::Config &config,
                                 EGLNativeWindowType window,
                                 EGLint width,
                                 EGLint height,
                                 xcb_connection_t *conn)
    : SurfaceImpl(surfaceState),
      mRenderer(renderer),
      mConfig(config),
      mNativeWindowType(window),
      mSurface(VK_NULL_HANDLE),
      mSwapchain(VK_NULL_HANDLE),
      mCurrentSwapchainImageIndex(0),
      mRenderTarget(GetVkFormatFromConfig(mConfig),
                    gl::Extents(static_cast<int>(width), static_cast<int>(height), 1)),
      mXcbConnection(conn)
{
}

WindowSurfaceVk::~WindowSurfaceVk()
{
    mSwapchainImages.clear();

    if (mSwapchain)
    {
        vkDestroySwapchainKHR(mRenderer->getDevice(), mSwapchain, nullptr);
        mSwapchain = VK_NULL_HANDLE;
    }

    if (mSurface)
    {
        vkDestroySurfaceKHR(mRenderer->getInstance(), mSurface, nullptr);
        mSurface = VK_NULL_HANDLE;
    }
}

egl::Error WindowSurfaceVk::initialize()
{
    return initializeImpl().toEGL(EGL_BAD_SURFACE);
}

vk::Error WindowSurfaceVk::initializeImpl()
{
#if defined(ANGLE_PLATFORM_WINDOWS)
    VkWin32SurfaceCreateInfoKHR createInfo;

    createInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext     = nullptr;
    createInfo.flags     = 0;
    createInfo.hinstance = GetModuleHandle(nullptr);
    createInfo.hwnd      = mNativeWindowType;
    ANGLE_VK_TRY(
        vkCreateWin32SurfaceKHR(mRenderer->getInstance(), &createInfo, nullptr, &mSurface));
#else
    VkXcbSurfaceCreateInfoKHR createInfo;

    createInfo.sType     = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext     = nullptr;
    createInfo.flags     = 0;
    createInfo.connection = mXcbConnection;
    createInfo.window     = mNativeWindowType;
    ANGLE_VK_TRY(
        vkCreateXcbSurfaceKHR(mRenderer->getInstance(), &createInfo, nullptr, &mSurface));
#endif

    uint32_t presentQueue = 0;
    ANGLE_TRY_RESULT(mRenderer->selectPresentQueueForSurface(mSurface), presentQueue);

    const auto &physicalDevice = mRenderer->getPhysicalDevice();

    VkSurfaceCapabilitiesKHR surfaceCaps;
    ANGLE_VK_TRY(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, mSurface, &surfaceCaps));

    // Adjust width and height to the swapchain if necessary.
    uint32_t width  = surfaceCaps.currentExtent.width;
    uint32_t height = surfaceCaps.currentExtent.height;

    // TODO(jmadill): Support devices which don't support copy. We use this for ReadPixels.
    ANGLE_VK_CHECK((surfaceCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) != 0,
                   VK_ERROR_INITIALIZATION_FAILED);

    if (surfaceCaps.currentExtent.width == 0xFFFFFFFFu)
    {
        ASSERT(surfaceCaps.currentExtent.height == 0xFFFFFFFFu);

#if defined(ANGLE_PLATFORM_WINDOWS)
        RECT rect;
        ANGLE_VK_CHECK(GetClientRect(mNativeWindowType, &rect) == TRUE,
                       VK_ERROR_INITIALIZATION_FAILED);
#endif
        if (mRenderTarget.getExtents().width == 0)
        {
#if defined(ANGLE_PLATFORM_WINDOWS)
            width = static_cast<uint32_t>(rect.right - rect.left);
#else
            width = 640;
#endif
        }
        if (mRenderTarget.getExtents().height == 0)
        {
#if defined(ANGLE_PLATFORM_WINDOWS)
            height = static_cast<uint32_t>(rect.bottom - rect.top);
#else
            height = 480;
#endif
        }
    }

    mRenderTarget.updateExtents(gl::Extents(static_cast<int>(width), static_cast<int>(height), 1));

    uint32_t presentModeCount = 0;
    ANGLE_VK_TRY(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, mSurface,
                                                           &presentModeCount, nullptr));
    ASSERT(presentModeCount > 0);

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    ANGLE_VK_TRY(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, mSurface,
                                                           &presentModeCount, presentModes.data()));

    // Use FIFO mode if available, since it throttles you to the display rate. Mailbox can lead
    // to rendering frames which are never seen by the user, wasting power.
    VkPresentModeKHR swapchainPresentMode = presentModes[0];
    for (auto presentMode : presentModes)
    {
        if (presentMode == VK_PRESENT_MODE_FIFO_KHR)
        {
            swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
            break;
        }

        // Fallback to immediate mode if FIFO is unavailable.
        if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
        {
            swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        }
    }

    // Determine number of swapchain images. Aim for one more than the minimum.
    uint32_t minImageCount = surfaceCaps.minImageCount + 1;
    if (surfaceCaps.maxImageCount > 0 && minImageCount > surfaceCaps.maxImageCount)
    {
        minImageCount = surfaceCaps.maxImageCount;
    }

    // Default to identity transform.
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    if ((surfaceCaps.supportedTransforms & preTransform) == 0)
    {
        preTransform = surfaceCaps.currentTransform;
    }

    uint32_t surfaceFormatCount = 0;
    ANGLE_VK_TRY(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, mSurface, &surfaceFormatCount,
                                                      nullptr));

    std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
    ANGLE_VK_TRY(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, mSurface, &surfaceFormatCount,
                                                      surfaceFormats.data()));

    auto nativeFormat = mRenderTarget.getFormat().native;

    if (surfaceFormatCount == 1u && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
    {
        // This is fine.
    }
    else
    {
        bool foundFormat = false;
        for (const auto &surfaceFormat : surfaceFormats)
        {
            if (surfaceFormat.format == nativeFormat)
            {
                foundFormat = true;
                break;
            }
        }

        ANGLE_VK_CHECK(foundFormat, VK_ERROR_INITIALIZATION_FAILED);
    }

    VkSwapchainCreateInfoKHR swapchainInfo;
    swapchainInfo.sType              = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.pNext              = nullptr;
    swapchainInfo.flags              = 0;
    swapchainInfo.surface            = mSurface;
    swapchainInfo.minImageCount      = minImageCount;
    swapchainInfo.imageFormat        = nativeFormat;
    swapchainInfo.imageColorSpace    = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    swapchainInfo.imageExtent.width  = width;
    swapchainInfo.imageExtent.height = height;
    swapchainInfo.imageArrayLayers   = 1;
    swapchainInfo.imageUsage =
        (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    swapchainInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.queueFamilyIndexCount = 0;
    swapchainInfo.pQueueFamilyIndices   = nullptr;
    swapchainInfo.preTransform          = preTransform;
    swapchainInfo.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode           = swapchainPresentMode;
    swapchainInfo.clipped               = VK_TRUE;
    swapchainInfo.oldSwapchain          = VK_NULL_HANDLE;

    const auto &device = mRenderer->getDevice();
    ANGLE_VK_TRY(vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &mSwapchain));

    // Intialize the swapchain image views.
    uint32_t imageCount = 0;
    ANGLE_VK_TRY(vkGetSwapchainImagesKHR(device, mSwapchain, &imageCount, nullptr));

    std::vector<VkImage> swapchainImages(imageCount);
    ANGLE_VK_TRY(vkGetSwapchainImagesKHR(device, mSwapchain, &imageCount, swapchainImages.data()));

    // CommandBuffer is a singleton in the Renderer.
    vk::CommandBuffer *commandBuffer = mRenderer->getCommandBuffer();
    ANGLE_TRY(commandBuffer->begin());

    VkClearColorValue transparentBlack;
    transparentBlack.float32[0] = 0.0f;
    transparentBlack.float32[1] = 0.0f;
    transparentBlack.float32[2] = 0.0f;
    transparentBlack.float32[3] = 0.0f;

    for (auto swapchainImage : swapchainImages)
    {
        VkImageViewCreateInfo imageViewInfo;
        imageViewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.pNext                           = nullptr;
        imageViewInfo.flags                           = 0;
        imageViewInfo.image                           = swapchainImage;
        imageViewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format                          = nativeFormat;
        imageViewInfo.components.r                    = VK_COMPONENT_SWIZZLE_R;
        imageViewInfo.components.g                    = VK_COMPONENT_SWIZZLE_G;
        imageViewInfo.components.b                    = VK_COMPONENT_SWIZZLE_B;
        imageViewInfo.components.a                    = VK_COMPONENT_SWIZZLE_A;
        imageViewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewInfo.subresourceRange.baseMipLevel   = 0;
        imageViewInfo.subresourceRange.levelCount     = 1;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        imageViewInfo.subresourceRange.layerCount     = 1;

        vk::Image image(swapchainImage);
        vk::ImageView imageView(device);
        ANGLE_TRY(imageView.init(imageViewInfo));

        // Set transfer dest layout, and clear the image to black.
        image.changeLayoutTop(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              commandBuffer);
        commandBuffer->clearSingleColorImage(image, transparentBlack);

        mSwapchainImages.push_back(std::move(image));
        mSwapchainImageViews.push_back(std::move(imageView));
    }

    ANGLE_TRY(commandBuffer->end());
    ANGLE_TRY(mRenderer->submitAndFinishCommandBuffer(*commandBuffer, 2000ull));

    // Start by getting the next available swapchain image.
    ANGLE_TRY(nextSwapchainImage());

    return vk::NoError();
}

FramebufferImpl *WindowSurfaceVk::createDefaultFramebuffer(const gl::FramebufferState &state)
{
    return FramebufferVk::CreateDefaultFBO(state, this);
}

egl::Error WindowSurfaceVk::swap()
{
    return swapImpl().toEGL(EGL_BAD_ALLOC);
}

vk::Error WindowSurfaceVk::swapImpl()
{
    vk::CommandBuffer *currentCB = mRenderer->getCommandBuffer();

    auto *image = &mSwapchainImages[mCurrentSwapchainImageIndex];

    currentCB->begin();
    image->changeLayoutWithStages(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                  VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                  VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, currentCB);
    currentCB->end();

    ANGLE_TRY(
        mRenderer->submitAndFinishCommandBuffer(*currentCB, mPresentCompleteSemaphore, 1000u));

    VkPresentInfoKHR presentInfo;
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext              = nullptr;
    presentInfo.waitSemaphoreCount = 0;
    presentInfo.pWaitSemaphores    = nullptr;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &mSwapchain;
    presentInfo.pImageIndices      = &mCurrentSwapchainImageIndex;
    presentInfo.pResults           = nullptr;

    ANGLE_VK_TRY(vkQueuePresentKHR(mRenderer->getQueue(), &presentInfo));

    // Get the next available swapchain iamge.
    ANGLE_TRY(nextSwapchainImage());

    return vk::NoError();
}

vk::Error WindowSurfaceVk::nextSwapchainImage()
{
    VkDevice device = mRenderer->getDevice();

    vk::Semaphore presentComplete(device);
    ANGLE_TRY(presentComplete.init());

    ANGLE_VK_TRY(vkAcquireNextImageKHR(device, mSwapchain, std::numeric_limits<uint64_t>::max(),
                                       presentComplete.getHandle(), nullptr,
                                       &mCurrentSwapchainImageIndex));

    // TODO(jmadill): Figure out ownership here.
    mPresentCompleteSemaphore = std::move(presentComplete);
    mRenderTarget.updateImage(&mSwapchainImages[mCurrentSwapchainImageIndex],
                              &mSwapchainImageViews[mCurrentSwapchainImageIndex]);

    return vk::NoError();
}

egl::Error WindowSurfaceVk::postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height)
{
    // TODO(jmadill)
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceVk::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    UNREACHABLE();
    return egl::Error(EGL_BAD_CURRENT_SURFACE);
}

egl::Error WindowSurfaceVk::bindTexImage(gl::Texture *texture, EGLint buffer)
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceVk::releaseTexImage(EGLint buffer)
{
    return egl::Error(EGL_SUCCESS);
}

void WindowSurfaceVk::setSwapInterval(EGLint interval)
{
}

EGLint WindowSurfaceVk::getWidth() const
{
    return mRenderTarget.getExtents().width;
}

EGLint WindowSurfaceVk::getHeight() const
{
    return mRenderTarget.getExtents().height;
}

EGLint WindowSurfaceVk::isPostSubBufferSupported() const
{
    // TODO(jmadill)
    return EGL_FALSE;
}

EGLint WindowSurfaceVk::getSwapBehavior() const
{
    // TODO(jmadill)
    return EGL_BUFFER_DESTROYED;
}

gl::Error WindowSurfaceVk::getAttachmentRenderTarget(
    const gl::FramebufferAttachment::Target & /*target*/,
    FramebufferAttachmentRenderTarget **rtOut)
{
    *rtOut = &mRenderTarget;
    return gl::NoError();
}

gl::ErrorOrResult<vk::Framebuffer *> WindowSurfaceVk::getCurrentFramebuffer(
    VkDevice device,
    const vk::RenderPass &compatibleRenderPass)
{
    if (!mSwapchainFramebuffers.empty())
    {
        // Validation layers should detect if the render pass is really compatible.
        return &mSwapchainFramebuffers[mCurrentSwapchainImageIndex];
    }

    VkFramebufferCreateInfo framebufferInfo;

    // TODO(jmadill): Depth/Stencil attachments.
    framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.pNext           = nullptr;
    framebufferInfo.flags           = 0;
    framebufferInfo.renderPass      = compatibleRenderPass.getHandle();
    framebufferInfo.attachmentCount = 1u;
    framebufferInfo.pAttachments    = nullptr;
    framebufferInfo.width           = static_cast<uint32_t>(mRenderTarget.getExtents().width);
    framebufferInfo.height          = static_cast<uint32_t>(mRenderTarget.getExtents().height);
    framebufferInfo.layers          = 1;

    for (const auto &imageView : mSwapchainImageViews)
    {
        VkImageView imageViewHandle  = imageView.getHandle();
        framebufferInfo.pAttachments = &imageViewHandle;

        vk::Framebuffer framebuffer(device);
        ANGLE_TRY(framebuffer.init(framebufferInfo));

        mSwapchainFramebuffers.push_back(std::move(framebuffer));
    }

    // We should only initialize framebuffers on the first swap.
    ASSERT(mCurrentSwapchainImageIndex == 0u);
    return &mSwapchainFramebuffers[mCurrentSwapchainImageIndex];
}

void WindowSurfaceVk::onBeginRenderPass()
{
    // FIXME
    mSwapchainImages[mCurrentSwapchainImageIndex].updateLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}

}  // namespace rx
