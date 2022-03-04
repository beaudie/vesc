//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SurfaceVkGbm.cpp:
//    Implements the class methods for SurfaceVkGbm.
//

#include "libANGLE/renderer/vulkan/linux/gbm/SurfaceVkGbm.h"

#include "common/debug.h"
#include "common/linux/dma_buf_utils.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/Overlay.h"
#include "libANGLE/Surface.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/driver_utils.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/FramebufferVk.h"
#include "libANGLE/renderer/vulkan/OverlayVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/renderer/vulkan/vk_format_utils.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"
#include "libANGLE/trace.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <drm.h>

#include <gbm.h>

namespace rx
{

constexpr size_t kMaxMemoryPlanes = 4;

constexpr VkImageAspectFlagBits kMemoryPlaneAspects[kMaxMemoryPlanes] = {
    VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT,
    VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT,
    VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT,
    VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT,
};

SurfaceVkGbm::SurfaceVkGbm(const egl::SurfaceState &surfaceState, EGLNativeWindowType window)
    : WindowSurfaceVk(surfaceState, window), mGbmSurface(reinterpret_cast<gbm_surface *>(window))
{}

SurfaceVkGbm::~SurfaceVkGbm() = default;

angle::Result SurfaceVkGbm::getAttachmentRenderTarget(const gl::Context *context,
                                                      GLenum binding,
                                                      const gl::ImageIndex &imageIndex,
                                                      GLsizei samples,
                                                      FramebufferAttachmentRenderTarget **rtOut)
{
    ContextVk *contextVk = vk::GetImpl(context);
    ANGLE_VK_TRACE_EVENT_AND_MARKER(contextVk, "First Swap Image Use");

    if (mState.config->renderTargetFormat != GL_NONE && !isMultiSampled())
    {
        // Update RenderTarget pointers to this swapchain image if not multisampling
        impl::SwapchainImage &gbmImage = mSwapchainImages[mCurrentSwapchainImageIndex];
        mColorRenderTarget.updateSwapchainImage(&gbmImage.image, &gbmImage.imageViews, nullptr,
                                                nullptr);
    }

    return SurfaceVk::getAttachmentRenderTarget(context, binding, imageIndex, samples, rtOut);
}

angle::Result SurfaceVkGbm::initializeImpl(DisplayVk *displayVk)
{
    RendererVk *renderer = displayVk->getRenderer();
    renderer->reloadVolkIfNeeded();
    ANGLE_TRY(createSwapChain(displayVk));
    return angle::Result::Continue;
}

angle::Result SurfaceVkGbm::createSwapChain(vk::Context *context)
{
    RendererVk *renderer = context->getRenderer();

    size_t boCount = gbm_surface_get_buffer_count(mGbmSurface);
    if (boCount == 0)
    {
        ERR() << "Could not get BOs out of a GBM surface. Too old GBM?";
        return angle::Result::Stop;
    }

    std::vector<gbm_bo *> bos(boCount);
    gbm_surface_get_buffers(mGbmSurface, bos.data());

    gbm_bo *firstBO      = bos[0];
    VkExtent3D vkExtents = {gbm_bo_get_width(firstBO), gbm_bo_get_height(firstBO), 1};

    // We need transfer src for reading back from the backbuffer.
    VkImageUsageFlags usage = kSurfaceVkColorImageUsageFlags;

    // If shaders may be fetching from this, we need this image to be an input
    if (renderer->getFeatures().supportsShaderFramebufferFetch.enabled ||
        renderer->getFeatures().supportsShaderFramebufferFetchNonCoherent.enabled)
    {
        usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }

    mSwapchainImages.resize(boCount);

    for (size_t boIndex = 0; boIndex < boCount; ++boIndex)
    {
        gbm_bo *bo = bos[boIndex];

        uint32_t planeCount = gbm_bo_get_plane_count(bo);
        bool disjoint       = planeCount > 1;
        uint64_t modifier   = gbm_bo_get_modifier(bo);

        ASSERT(planeCount <= kMaxMemoryPlanes);

        VkSubresourceLayout planeLayouts[kMaxMemoryPlanes] = {};
        for (uint32_t planeIndex = 0; planeIndex < planeCount; planeIndex++)
        {
            planeLayouts[planeIndex].offset     = gbm_bo_get_offset(bo, planeIndex);
            planeLayouts[planeIndex].rowPitch   = gbm_bo_get_stride_for_plane(bo, planeIndex);
            planeLayouts[planeIndex].arrayPitch = 0;
            planeLayouts[planeIndex].depthPitch = 0;
            planeLayouts[planeIndex].size       = 0;
        }

        VkImageDrmFormatModifierExplicitCreateInfoEXT drmInfo = {};
        drmInfo.sType = VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT;
        drmInfo.drmFormatModifier           = modifier;
        drmInfo.drmFormatModifierPlaneCount = planeCount;
        drmInfo.pPlaneLayouts               = planeLayouts;

        VkExternalMemoryImageCreateInfo externalMemoryInfo = {};
        externalMemoryInfo.sType       = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
        externalMemoryInfo.pNext       = &drmInfo;
        externalMemoryInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;

        VkImageCreateFlags imageCreateFlags = 0;
        if (disjoint)
        {
            imageCreateFlags |= VK_IMAGE_CREATE_DISJOINT_BIT;
        }

        VkExtent3D extents = {gbm_bo_get_width(bo), gbm_bo_get_height(bo), 1};
        ASSERT(extents.width == vkExtents.width && extents.height == vkExtents.height);

        // Get format from BO and convert it to angle::FormatID
        uint32_t drmFormat = gbm_bo_get_format(bo);
        bool isYUV;
        GLenum glFormat        = angle::DrmFourCCFormatToGLInternalFormat(drmFormat, &isYUV);
        glFormat               = gl::GetNonLinearFormat(glFormat);
        angle::FormatID format = angle::Format::InternalFormatToID(glFormat);

        impl::SwapchainImage &gbmImage = mSwapchainImages[boIndex];

        ANGLE_TRY(gbmImage.image.initExternal(
            context, gl::TextureType::_2D, extents, format, format, 1, usage, imageCreateFlags,
            vk::ImageLayout::ExternalPreInitialized, &externalMemoryInfo, gl::LevelIndex(0), 1, 1,
            mState.isRobustResourceInitEnabled(), mState.hasProtectedContent()));

        gbmImage.imageViews.init(renderer);

        VkImage imageHandle = gbmImage.image.getImage().getHandle();

        VkMemoryRequirements reqs0                                          = {};
        const void *allocInfos[kMaxMemoryPlanes]                            = {};
        VkMemoryDedicatedAllocateInfo dedicatedAllocInfos[kMaxMemoryPlanes] = {};
        VkImportMemoryFdInfoKHR importMemoryFdInfos[kMaxMemoryPlanes]       = {};
        for (uint32_t planeIndex = 0; planeIndex < planeCount; planeIndex++)
        {
            int ifd = gbm_bo_get_fd_for_plane(bo, planeIndex);
            // Vulkan takes ownership of the FD, closed on vkFreeMemory.
            int fd = fcntl(ifd, F_DUPFD_CLOEXEC, 0);
            if (fd < 0)
            {
                ERR() << "Failed to duplicate fd for dma_buf import";
                return angle::Result::Stop;
            }

            // Compute requirements
            VkMemoryFdPropertiesKHR fdProps = {};
            fdProps.sType                   = VK_STRUCTURE_TYPE_MEMORY_FD_PROPERTIES_KHR;
            ANGLE_VK_TRY(context,
                         vkGetMemoryFdPropertiesKHR(context->getDevice(),
                                                    VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT,
                                                    fd, &fdProps));

            VkImagePlaneMemoryRequirementsInfo planeReqsInfo = {};
            planeReqsInfo.sType       = VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO;
            planeReqsInfo.planeAspect = kMemoryPlaneAspects[planeIndex];

            VkImageMemoryRequirementsInfo2 reqsInfo = {};
            reqsInfo.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
            reqsInfo.pNext = disjoint ? &planeReqsInfo : nullptr;
            reqsInfo.image = imageHandle;

            VkMemoryRequirements2 reqs = {};
            reqs.sType                 = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;

            vkGetImageMemoryRequirements2(context->getDevice(), &reqsInfo, &reqs);

            // https://github.com/nyorain/kms-vulkan/blob/e66655b05ebc6281c1ebffec12681240caf305be/vulkan.c#L1128
            reqs.memoryRequirements.size = std::max(VkDeviceSize(1), reqs.memoryRequirements.size);

            // Allocation has to satisfy both image's and fd's requirements
            reqs.memoryRequirements.memoryTypeBits &= fdProps.memoryTypeBits;

            // Ideally initExternalMemory would take requirements for each plane
            if (planeIndex > 0)
            {
                ASSERT(reqs.memoryRequirements.size == reqs0.size &&
                       reqs.memoryRequirements.alignment == reqs0.alignment &&
                       reqs.memoryRequirements.memoryTypeBits == reqs0.memoryTypeBits);
            }
            reqs0 = reqs.memoryRequirements;

            // Allocation

            importMemoryFdInfos[planeIndex].sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR;
            importMemoryFdInfos[planeIndex].fd    = fd;
            importMemoryFdInfos[planeIndex].handleType =
                VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;

            dedicatedAllocInfos[planeIndex].sType =
                VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
            dedicatedAllocInfos[planeIndex].pNext = &importMemoryFdInfos[planeIndex];
            dedicatedAllocInfos[planeIndex].image = imageHandle;

            allocInfos[planeIndex] = &dedicatedAllocInfos[planeIndex];
        }

        // We can not initialize image external memory with a FOREIGN queue, otherwise
        // image flush staged updates would fail the assert in vk_helpers.cpp:onImageWrite()
        ANGLE_TRY(gbmImage.image.initExternalMemory(
            context, renderer->getMemoryProperties(), reqs0, planeCount, allocInfos,
            renderer->getQueueFamilyIndex(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
    }

    GLenum renderTargetFormat = gl::GetNonLinearFormat(mState.config->renderTargetFormat);
    const vk::Format &format  = renderer->getFormat(renderTargetFormat);
    ANGLE_TRY(initColor(context, vkExtents, format));

    if (mState.config->renderTargetFormat != GL_NONE)
    {
        size_t gbmImageIndex;
        int acquired = gbm_surface_acquire_back_buffer(mGbmSurface, &gbmImageIndex);
        ASSERT(acquired);
        mCurrentSwapchainImageIndex = static_cast<uint32_t>(gbmImageIndex);
    }

    ANGLE_TRY(initDepthStencil(context, vkExtents));

    return angle::Result::Continue;
}

angle::Result SurfaceVkGbm::presentImpl(ContextVk *contextVk,
                                        vk::OutsideRenderPassCommandBuffer *commandBuffer,
                                        Serial *swapSerial,
                                        const EGLint *rects,
                                        EGLint n_rects,
                                        const void *pNextChain,
                                        bool *presentOutOfDate)
{
    impl::SwapchainImage &gbmImage = mSwapchainImages[mCurrentSwapchainImageIndex];
    gbmImage.image.recordReadBarrier(contextVk, VK_IMAGE_ASPECT_COLOR_BIT,
                                     vk::ImageLayout::ColorAttachment, commandBuffer);

    // We are not going to present with Vulkan, therefore there is no need for a present semaphore
    ANGLE_TRY(
        contextVk->flushAndGetSerial(nullptr, swapSerial, RenderPassClosureReason::EGLSwapBuffers));

    gbm_surface_present_buffer(mGbmSurface, mCurrentSwapchainImageIndex);

    return angle::Result::Continue;
}

angle::Result SurfaceVkGbm::swapImpl(const gl::Context *context,
                                     const EGLint *rects,
                                     EGLint n_rects,
                                     const void *pNextChain)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "SurfaceVkGbm::swapImpl");

    // We need to acquire new back buffer before presenting previous one
    // otherwise we would end up acquiring again the buffer we just presented.
    size_t gbmImageIndex;
    if (!gbm_surface_acquire_back_buffer(mGbmSurface, &gbmImageIndex))
    {
        ERR() << "Failed to acquire a back buffer";
        return angle::Result::Stop;
    }

    // Make sure render target is updated with this GBM image if not multisampling.
    if (!isMultiSampled())
    {
        impl::SwapchainImage &gbmImage = mSwapchainImages[mCurrentSwapchainImageIndex];
        mColorRenderTarget.updateSwapchainImage(&gbmImage.image, &gbmImage.imageViews, nullptr,
                                                nullptr);
    }

    // Present previously acquired back-buffer
    ContextVk *contextVk = vk::GetImpl(context);
    // Can not do present optimization as there is no Vulkan presentation involved here
    ANGLE_TRY(present(contextVk, false, rects, n_rects, pNextChain, nullptr));

    RendererVk *renderer = contextVk->getRenderer();
    DisplayVk *displayVk = vk::GetImpl(context->getDisplay());
    ANGLE_TRY(renderer->syncPipelineCacheVk(displayVk, context));

    // Finally update current image index
    mCurrentSwapchainImageIndex = static_cast<uint32_t>(gbmImageIndex);

    return angle::Result::Continue;
}

void SurfaceVkGbm::setSwapInterval(EGLint /*interval*/) {}

egl::Error SurfaceVkGbm::lockSurface(const egl::Display *display,
                                     EGLint usageHint,
                                     bool preservePixels,
                                     uint8_t **bufferPtrOut,
                                     EGLint *bufferPitchOut)
{
    return egl::NoError();
}

egl::Error SurfaceVkGbm::unlockSurface(const egl::Display *display, bool preservePixels)
{
    return egl::NoError();
}

angle::Result SurfaceVkGbm::createSurfaceVk(vk::Context *context, gl::Extents *extentsOut)
{
    return angle::Result::Continue;
}

angle::Result SurfaceVkGbm::getCurrentWindowSize(vk::Context *context, gl::Extents *extentsOut)
{
    return angle::Result::Continue;
}

}  // namespace rx
