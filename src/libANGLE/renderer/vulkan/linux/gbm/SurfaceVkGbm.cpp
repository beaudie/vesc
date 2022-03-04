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

namespace rx
{

SurfaceImage::SurfaceImage() = default;

SurfaceImage::SurfaceImage(gbm_bo *bo) : mGbmBO(bo) {}

SurfaceImage::SurfaceImage(SurfaceImage &&other)
    : mGbmBO(other.mGbmBO), mAcquired(other.mAcquired), mLocked(other.mLocked)
{
    other.mGbmBO = nullptr;
}

SurfaceImage::~SurfaceImage()
{
    if (mGbmBO != nullptr)
    {
        gbm_bo_destroy(mGbmBO);
    }
}

SurfaceImage &SurfaceImage::operator=(SurfaceImage &&other)
{
    std::swap(mGbmBO, other.mGbmBO);
    std::swap(mAcquired, other.mAcquired);
    std::swap(mLocked, other.mLocked);
    return *this;
}

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

SurfaceVkGbm::~SurfaceVkGbm()
{
    if (mGbmSurface != nullptr)
    {
        gbm_surface_set_overrides(mGbmSurface, nullptr, gbm_surface_overrides{});
    }
}

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
        impl::SwapchainImage &image = mSwapchainImages[mCurrentSwapchainImageIndex];
        mColorRenderTarget.updateSwapchainImage(&image.image, &image.imageViews, nullptr, nullptr);
    }

    return SurfaceVk::getAttachmentRenderTarget(context, binding, imageIndex, samples, rtOut);
}

angle::Result SurfaceVkGbm::initializeImpl(DisplayVk *displayVk)
{
    RendererVk *renderer = displayVk->getRenderer();

    mColorImageMSViews.init(renderer);
    mDepthStencilImageViews.init(renderer);

    renderer->reloadVolkIfNeeded();
    mSwapchainPresentMode = vk::PresentMode::GbmANGLE;
    ANGLE_TRY(createSwapChain(displayVk));
    return angle::Result::Continue;
}

angle::Result SurfaceVkGbm::createImages(gbm_device *gbmDevice,
                                         uint32_t width,
                                         uint32_t height,
                                         uint32_t format,
                                         const uint64_t *modifiers,
                                         size_t count,
                                         uint32_t usage)
{
    if (mState.hasProtectedContent())
    {
        usage |= GBM_BO_USE_PROTECTED;
    }

    angle::Result result = angle::Result::Continue;
    for (auto &image : mSurfaceImages)
    {
        gbm_bo *bo = gbm_bo_create_with_modifiers2(gbmDevice, width, height, format, modifiers,
                                                   static_cast<unsigned int>(count), usage);
        if (bo == nullptr)
        {
            result = angle::Result::Stop;
            break;
        }
        image = SurfaceImage(bo);
    }

    if (result != angle::Result::Continue)
    {
        destroyImages();
    }
    return result;
}

void SurfaceVkGbm::destroyImages()
{
    for (auto &image : mSurfaceImages)
    {
        image = SurfaceImage();
    }
}

gbm_bo *SurfaceVkGbm::lockFrontImage()
{
    SurfaceImage &image = mSurfaceImages[mCurrentFrontImage];
    ASSERT(!image.mAcquired);
    ASSERT(!image.mLocked);
    image.mLocked = true;
    return image.mGbmBO;
}

void SurfaceVkGbm::releaseImage(gbm_bo *bo)
{
    for (SurfaceImage &image : mSurfaceImages)
    {
        if (image.mGbmBO == bo)
        {
            ASSERT(!image.mAcquired);
            ASSERT(image.mLocked);
            image.mLocked = false;
            return;
        }
    }

    UNREACHABLE();
}

bool SurfaceVkGbm::hasFreeImages()
{
    for (SurfaceImage &image : mSurfaceImages)
    {
        if (!image.mAcquired && !image.mLocked)
        {
            return true;
        }
    }
    return false;
}

namespace gbm
{
int surface_overrides_init(void *user_surface,
                           gbm_device *gbmDevice,
                           uint32_t width,
                           uint32_t height,
                           uint32_t format,
                           const uint64_t *modifiers,
                           size_t count,
                           uint32_t usage)
{
    SurfaceVkGbm *surfaceVkGbm = static_cast<SurfaceVkGbm *>(user_surface);
    if (surfaceVkGbm->createImages(gbmDevice, width, height, format, modifiers, count, usage) !=
        angle::Result::Continue)
    {
        return 0;
    }
    return 1;
}

gbm_bo *surface_lock_front_buffer(void *user_surface)
{
    SurfaceVkGbm *surfaceVkGbm = static_cast<SurfaceVkGbm *>(user_surface);
    return surfaceVkGbm->lockFrontImage();
}

void surface_release_buffer(void *user_surface, gbm_bo *bo)
{
    SurfaceVkGbm *surfaceVkGbm = static_cast<SurfaceVkGbm *>(user_surface);
    surfaceVkGbm->releaseImage(bo);
}

int surface_has_free_buffers(void *user_surface)
{
    SurfaceVkGbm *surfaceVkGbm = static_cast<SurfaceVkGbm *>(user_surface);
    return surfaceVkGbm->hasFreeImages() ? 1 : 0;
}
}  // namespace gbm

angle::Result SurfaceVkGbm::createSwapChain(vk::Context *context)
{
    RendererVk *renderer = context->getRenderer();

    if (!renderer->getFeatures().supportsExternalMemoryDmaBufAndModifiers.enabled)
    {
        return angle::Result::Stop;
    }

    if (!gbm_surface_set_overrides(mGbmSurface, this,
                                   gbm_surface_overrides{
                                       .surface_overrides_init    = gbm::surface_overrides_init,
                                       .surface_lock_front_buffer = gbm::surface_lock_front_buffer,
                                       .surface_release_buffer    = gbm::surface_release_buffer,
                                       .surface_has_free_buffers  = gbm::surface_has_free_buffers,
                                   }))
    {
        ERR() << "Failed to set GBM surface user backend";
        return angle::Result::Stop;
    }

    if (createSwapChainImpl(context) != angle::Result::Continue)
    {
        gbm_surface_set_overrides(mGbmSurface, nullptr, gbm_surface_overrides{});
        for (auto &image : mSurfaceImages)
        {
            image = SurfaceImage();
        }
        return angle::Result::Stop;
    }

    return angle::Result::Continue;
}

angle::Result SurfaceVkGbm::createSwapChainImpl(vk::Context *context)
{
    RendererVk *renderer = context->getRenderer();

    gbm_bo *firstBO      = mSurfaceImages[0].mGbmBO;
    VkExtent3D vkExtents = {gbm_bo_get_width(firstBO), gbm_bo_get_height(firstBO), 1};

    VkImageUsageFlags usage = GetSwapchainImageUsageFlags(renderer->getFeatures());

    mSwapchainImages.resize(mSurfaceImages.size());

    for (size_t boIndex = 0; boIndex < mSurfaceImages.size(); ++boIndex)
    {
        gbm_bo *bo = mSurfaceImages[boIndex].mGbmBO;

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
        GLenum internalFormat      = angle::DrmFourCCFormatToGLInternalFormat(drmFormat, &isYUV);
        const vk::Format &vkFormat = renderer->getFormat(internalFormat);
        angle::FormatID actualFormatID = vkFormat.getActualRenderableImageFormatID();

        impl::SwapchainImage &image = mSwapchainImages[boIndex];

        ANGLE_TRY(image.image.initExternal(
            context, gl::TextureType::_2D, extents, vkFormat.getIntendedFormatID(), actualFormatID,
            1, usage, imageCreateFlags, vk::ImageLayout::ExternalPreInitialized,
            &externalMemoryInfo, gl::LevelIndex(0), 1, 1, mState.isRobustResourceInitEnabled(),
            mState.hasProtectedContent()));

        image.imageViews.init(renderer);

        VkImage imageHandle = image.image.getImage().getHandle();

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
        ANGLE_TRY(image.image.initExternalMemory(
            context, renderer->getMemoryProperties(), reqs0, planeCount, allocInfos,
            renderer->getQueueFamilyIndex(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
    }

    GLenum renderTargetFormat = mState.config->renderTargetFormat;
    const vk::Format &format  = renderer->getFormat(renderTargetFormat);
    ANGLE_TRY(initColor(context, vkExtents, format));

    if (renderTargetFormat != GL_NONE)
    {
        angle::Result acquired = acquireNextImage(&mCurrentSwapchainImageIndex);
        ASSERT(acquired == angle::Result::Continue);
    }

    if (!isMultiSampled())
    {
        // Initialize the color render target with the color image.
        mColorRenderTarget.updateSwapchainImage(
            &mSwapchainImages[mCurrentSwapchainImageIndex].image,
            &mSwapchainImages[mCurrentSwapchainImageIndex].imageViews, nullptr, nullptr);
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
    ASSERT(!mNeedToAcquireNextSwapchainImage);

    impl::SwapchainImage &image = mSwapchainImages[mCurrentSwapchainImageIndex];
    image.image.recordReadBarrier(contextVk, VK_IMAGE_ASPECT_COLOR_BIT,
                                  vk::ImageLayout::ColorAttachment, commandBuffer);

    // We are not going to present with Vulkan, therefore there is no need for a present semaphore.
    //
    // GBM itself doesn't synchronise with anything. All it does is give the caller a handle to the
    // current front buffer, with the user performing presentation itself (e.g. the GBM user obtains
    // the current front buffer from GBM and then generates a KMS request including the handle to
    // that buffer).
    //
    // Users such as Weston use EGL_KHR_fence_sync and EGL_ANDROID_native_fence_sync
    // to mark the bottom-of-pipe synchronisation point, transform that into a dma-fence, and
    // synchronise against it in the kernel. Users who do not do this need to run on a system
    // implementing the old implicit synchronisation workflow, or see incoherent output. There is no
    // third option as GBM does not participate in the actual presentation.
    ANGLE_TRY(
        contextVk->flushAndGetSerial(nullptr, swapSerial, RenderPassClosureReason::EGLSwapBuffers));

    mCurrentFrontImage = mCurrentSwapchainImageIndex;
    ASSERT(mSurfaceImages[mCurrentFrontImage].mAcquired);
    mSurfaceImages[mCurrentFrontImage].mAcquired = false;

    // Set FrameNumber for the presented image.
    mSwapchainImages[mCurrentSwapchainImageIndex].mFrameNumber = mFrameCount++;

    contextVk->resetPerFramePerfCounters();

    return angle::Result::Continue;
}

angle::Result SurfaceVkGbm::acquireNextImage(uint32_t *outIndex)
{
    for (uint32_t i = 0; i < mSurfaceImages.size(); i++)
    {
        SurfaceImage &image = mSurfaceImages[i];
        if (!image.mAcquired && !image.mLocked)
        {
            image.mAcquired = true;
            *outIndex       = i;
            return angle::Result::Continue;
        }
    }

    ERR() << "Failed to acquire a back buffer";
    *outIndex = 0;
    return angle::Result::Stop;
}

angle::Result SurfaceVkGbm::swapImpl(const gl::Context *context,
                                     const EGLint *rects,
                                     EGLint n_rects,
                                     const void *pNextChain)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "SurfaceVkGbm::swapImpl");

    // We need to acquire new back buffer before presenting previous one
    // otherwise we would end up acquiring again the buffer we just presented.
    uint32_t gbmImageIndex;
    ANGLE_TRY(acquireNextImage(&gbmImageIndex));

    // Make sure render target is updated with this GBM image if not multisampling.
    if (!isMultiSampled())
    {
        impl::SwapchainImage &image = mSwapchainImages[mCurrentSwapchainImageIndex];
        mColorRenderTarget.updateSwapchainImage(&image.image, &image.imageViews, nullptr, nullptr);
    }

    // Present previously acquired back-buffer
    ContextVk *contextVk = vk::GetImpl(context);
    ANGLE_TRY(present(contextVk, rects, n_rects, pNextChain, nullptr));

    RendererVk *renderer = contextVk->getRenderer();
    DisplayVk *displayVk = vk::GetImpl(context->getDisplay());
    ANGLE_TRY(renderer->syncPipelineCacheVk(displayVk, context));

    // Finally update current image index
    mCurrentSwapchainImageIndex = gbmImageIndex;

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

const vk::Semaphore *SurfaceVkGbm::getAndResetAcquireImageSemaphore()
{
    return nullptr;
}

}  // namespace rx
