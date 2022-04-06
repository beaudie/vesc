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
namespace
{
angle::SubjectIndex kAnySurfaceImageSubjectIndex = 0;

GLint GetSampleCount(const egl::Config *config)
{
    GLint samples = 1;
    if (config->sampleBuffers && config->samples > 1)
    {
        samples = config->samples;
    }
    return samples;
}
}  // namespace

GbmImage::GbmImage()  = default;
GbmImage::~GbmImage() = default;

GbmImage::GbmImage(GbmImage &&other)
    : image(std::move(other.image)),
      imageViews(std::move(other.imageViews)),
      framebuffer(std::move(other.framebuffer)),
      fetchFramebuffer(std::move(other.fetchFramebuffer)),
      framebufferResolveMS(std::move(other.framebufferResolveMS))
{}

SurfaceVkGbm::SurfaceVkGbm(const egl::SurfaceState &surfaceState,
                           EGLNativeWindowType window,
                           gbm_device *gbmDevice)
    : FramebufferSurfaceVk(surfaceState),
      mGbmDevice(gbmDevice),
      mGbmSurface(reinterpret_cast<gbm_surface *>(window)),
      mWidth(0),
      mHeight(0),
      mCurrentGbmImageIndex(0),
      mDepthStencilImageBinding(this, kAnySurfaceImageSubjectIndex),
      mColorImageMSBinding(this, kAnySurfaceImageSubjectIndex),
      mFramebufferFetchMode(FramebufferFetchMode::Disabled)
{
    (void)mGbmDevice;

    // Initialize the color render target with the multisampled targets. If not multisampled, the
    // render target will be updated to refer to a GBM image.
    mColorRenderTarget.init(&mColorImageMS, &mColorImageMSViews, nullptr, nullptr,
                            gl::LevelIndex(0), 0, 1, RenderTargetTransience::Default);
    mDepthStencilRenderTarget.init(&mDepthStencilImage, &mDepthStencilImageViews, nullptr, nullptr,
                                   gl::LevelIndex(0), 0, 1, RenderTargetTransience::Default);
    mDepthStencilImageBinding.bind(&mDepthStencilImage);
    mColorImageMSBinding.bind(&mColorImageMS);
}

SurfaceVkGbm::~SurfaceVkGbm() {}

egl::Error SurfaceVkGbm::initialize(const egl::Display *display)
{
    DisplayVk *displayVk = vk::GetImpl(display);
    angle::Result result = initializeImpl(displayVk);
    return angle::ToEGL(result, displayVk, EGL_BAD_SURFACE);
}

constexpr VkImageAspectFlagBits kMemoryPlaneAspects[kMaxMemoryPlanes] = {
    VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT,
    VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT,
    VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT,
    VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT,
};

angle::Result SurfaceVkGbm::initializeImpl(DisplayVk *displayVk)
{
    RendererVk *renderer = displayVk->getRenderer();

    mColorImageMSViews.init(renderer);
    mDepthStencilImageViews.init(renderer);

    renderer->reloadVolkIfNeeded();

    size_t boCount = gbm_surface_get_buffer_count(mGbmSurface);
    if (boCount == 0)
    {
        ERR() << "Could not get BOs out of a GBM surface. Too old GBM?";
        return angle::Result::Stop;
    }

    std::vector<gbm_bo *> bos(boCount);
    gbm_surface_get_buffers(mGbmSurface, bos.data());

    gbm_bo *firstBO = bos[0];

    mWidth  = gbm_bo_get_width(firstBO);
    mHeight = gbm_bo_get_height(firstBO);

    // We need transfer src for reading back from the backbuffer.
    VkImageUsageFlags usage = kSurfaceVkColorImageUsageFlags;

    // If shaders may be fetching from this, we need this image to be an input
    if (renderer->getFeatures().supportsShaderFramebufferFetch.enabled ||
        renderer->getFeatures().supportsShaderFramebufferFetchNonCoherent.enabled)
    {
        usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }

    mGbmImages.resize(boCount);

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
        ASSERT(extents.width == mWidth && extents.height == mHeight);

        // Get format from BO and convert it to angle::FormatID
        uint32_t drmFormat = gbm_bo_get_format(bo);
        bool isYUV;
        GLenum glFormat        = angle::DrmFourCCFormatToGLInternalFormat(drmFormat, &isYUV);
        glFormat               = gl::GetNonLinearFormat(glFormat);
        angle::FormatID format = angle::Format::InternalFormatToID(glFormat);

        GbmImage &gbmImage = mGbmImages[boIndex];
        gbmImage.bo        = bo;

        ANGLE_TRY(gbmImage.image.initExternal(
            displayVk, gl::TextureType::_2D, extents, format, format, 1, usage, imageCreateFlags,
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
            ANGLE_VK_TRY(displayVk,
                         vkGetMemoryFdPropertiesKHR(displayVk->getDevice(),
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

            vkGetImageMemoryRequirements2(displayVk->getDevice(), &reqsInfo, &reqs);

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
            displayVk, renderer->getMemoryProperties(), reqs0, planeCount, allocInfos,
            renderer->getQueueFamilyIndex(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
    }

    if (mState.config->renderTargetFormat != GL_NONE)
    {
        int acquired = gbm_surface_acquire_back_buffer(mGbmSurface, &mCurrentGbmImageIndex);
        ASSERT(acquired);
    }

    GLint samples = 1;

    // Initialize depth/stencil if requested.
    if (mState.config->depthStencilFormat != GL_NONE)
    {
        const vk::Format &dsFormat = renderer->getFormat(mState.config->depthStencilFormat);

        const VkImageUsageFlags dsUsage = kSurfaceVkDepthStencilImageUsageFlags;

        VkExtent3D extents = {mWidth, mHeight, 1};

        ANGLE_TRY(mDepthStencilImage.init(
            displayVk, gl::TextureType::_2D, extents, dsFormat, samples, dsUsage, gl::LevelIndex(0),
            1, 1, mState.isRobustResourceInitEnabled(), mState.hasProtectedContent()));
        ANGLE_TRY(mDepthStencilImage.initMemory(displayVk, mState.hasProtectedContent(),
                                                renderer->getMemoryProperties(),
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

        mDepthStencilRenderTarget.init(&mDepthStencilImage, &mDepthStencilImageViews, nullptr,
                                       nullptr, gl::LevelIndex(0), 0, 1,
                                       RenderTargetTransience::Default);

        // We will need to pass depth/stencil image views to the RenderTargetVk in the future.
    }

    return angle::Result::Continue;
}

void SurfaceVkGbm::destroy(const egl::Display *display)
{
    DisplayVk *displayVk = vk::GetImpl(display);
    RendererVk *renderer = displayVk->getRenderer();
    VkDevice device      = displayVk->getDevice();

    mDepthStencilImage.destroy(renderer);
    mDepthStencilImageViews.destroy(device);
    mColorImageMS.destroy(renderer);
    mColorImageMSViews.destroy(device);
    mFramebufferMS.destroy(device);

    for (GbmImage &gbmImage : mGbmImages)
    {
        gbmImage.image.destroy(renderer);
        gbmImage.imageViews.destroy(device);
        gbmImage.framebuffer.destroy(device);
        if (gbmImage.fetchFramebuffer.valid())
        {
            gbmImage.fetchFramebuffer.destroy(device);
        }
        if (gbmImage.framebufferResolveMS.valid())
        {
            gbmImage.framebufferResolveMS.destroy(device);
        }
    }

    mGbmImages.clear();
}

angle::Result SurfaceVkGbm::getAttachmentRenderTarget(const gl::Context *context,
                                                      GLenum binding,
                                                      const gl::ImageIndex &imageIndex,
                                                      GLsizei samples,
                                                      FramebufferAttachmentRenderTarget **rtOut)
{
    ContextVk *contextVk = vk::GetImpl(context);
    ANGLE_VK_TRACE_EVENT_AND_MARKER(contextVk, "First Swap Image Use");
    ANGLE_TRY(initializeMS(contextVk));

    if (mState.config->renderTargetFormat != GL_NONE && !isMultiSampled())
    {
        // Update RenderTarget pointers to this swapchain image if not multisampling
        GbmImage &gbmImage = mGbmImages[mCurrentGbmImageIndex];
        mColorRenderTarget.updateSwapchainImage(&gbmImage.image, &gbmImage.imageViews, nullptr,
                                                nullptr);
    }

    return SurfaceVk::getAttachmentRenderTarget(context, binding, imageIndex, samples, rtOut);
}

FramebufferImpl *SurfaceVkGbm::createDefaultFramebuffer(const gl::Context *context,
                                                        const gl::FramebufferState &state)
{
    RendererVk *renderer = vk::GetImpl(context)->getRenderer();
    return FramebufferVk::CreateDefaultFBO(renderer, state, this);
}

angle::Result SurfaceVkGbm::initializeMS(ContextVk *context)
{
    if (mColorImageMS.valid())
    {
        return angle::Result::Continue;
    }

    // If multisampling is enabled, create a multisampled image which gets resolved just prior to
    // present.
    GLint samples = GetSampleCount(mState.config);
    ANGLE_VK_CHECK(context, samples > 0, VK_ERROR_INITIALIZATION_FAILED);
    if (samples == 1)
    {
        return angle::Result::Continue;
    }

    RendererVk *renderer = context->getRenderer();

    GLenum renderTargetFormat = gl::GetNonLinearFormat(mState.config->renderTargetFormat);
    const vk::Format &format  = renderer->getFormat(renderTargetFormat);

    VkExtent3D vkExtents = {mWidth, mHeight, 1};

    bool robustInit = mState.isRobustResourceInitEnabled();

    const VkImageUsageFlags usage = kSurfaceVkColorImageUsageFlags;

    // Create a multisampled image that will be rendered to, and then resolved to a GBM image.
    ANGLE_TRY(mColorImageMS.initMSAASwapchain(context, gl::TextureType::_2D, vkExtents, false,
                                              format, samples, usage, gl::LevelIndex(0), 1, 1,
                                              robustInit, mState.hasProtectedContent()));
    ANGLE_TRY(mColorImageMS.initMemory(context, mState.hasProtectedContent(),
                                       renderer->getMemoryProperties(),
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

    // Initialize the color render target with the multisampled targets.  If not multisampled,
    // the render target will be updated to refer to a swapchain image on every acquire.
    mColorRenderTarget.init(&mColorImageMS, &mColorImageMSViews, nullptr, nullptr,
                            gl::LevelIndex(0), 0, 1, RenderTargetTransience::Default);

    return angle::Result::Continue;
}

egl::Error SurfaceVkGbm::swap(const gl::Context *context)
{
    DisplayVk *displayVk = vk::GetImpl(context->getDisplay());
    angle::Result result = swapImpl(context);
    return angle::ToEGL(result, displayVk, EGL_BAD_SURFACE);
}

angle::Result SurfaceVkGbm::present(ContextVk *contextVk)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "SurfaceVkGbm::present");
    RendererVk *renderer = contextVk->getRenderer();

    // Throttle the submissions to avoid getting too far ahead of the GPU.
    Serial *swapSerial = &mSwapHistory.front();
    mSwapHistory.next();

    {
        ANGLE_TRACE_EVENT0("gpu.angle", "SurfaceVkGbm::present: Throttle CPU");
        ANGLE_TRY(renderer->finishToSerial(contextVk, *swapSerial));
    }

    GbmImage &image = mGbmImages[mCurrentGbmImageIndex];

    // Make sure deferred clears are applied, if any.
    ANGLE_TRY(
        image.image.flushStagedUpdates(contextVk, gl::LevelIndex(0), gl::LevelIndex(1), 0, 1, {}));

    vk::Framebuffer &currentFramebuffer = chooseFramebuffer(SwapchainResolveMode::Disabled);

    // We can only do present related optimization if this is the last renderpass that touches the
    // swapchain image. MSAA resolve and overlay will insert another renderpass which disqualifies
    // the optimization.
    bool imageResolved = false;
    if (currentFramebuffer.valid())
    {
        ANGLE_TRY(contextVk->optimizeRenderPassForPresent(currentFramebuffer.getHandle(),
                                                          &image.imageViews, &image.image,
                                                          &mColorImageMS, &imageResolved));
    }

    // Because the color attachment defers layout changes until endRenderPass time, we must call
    // finalize the layout transition in the renderpass before we insert layout change to
    // ImageLayout::ColorAttachment below.
    contextVk->finalizeImageLayout(&image.image);
    contextVk->finalizeImageLayout(&mColorImageMS);

    vk::OutsideRenderPassCommandBuffer *commandBuffer;
    ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer({}, &commandBuffer));

    if (isMultiSampled())
    {
        // Transition the multisampled image to TRANSFER_SRC for resolve.
        vk::CommandBufferAccess access;
        access.onImageTransferRead(VK_IMAGE_ASPECT_COLOR_BIT, &mColorImageMS);
        access.onImageTransferWrite(gl::LevelIndex(0), 1, 0, 1, VK_IMAGE_ASPECT_COLOR_BIT,
                                    &image.image);

        ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &commandBuffer));

        VkImageResolve resolveRegion                = {};
        resolveRegion.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        resolveRegion.srcSubresource.mipLevel       = 0;
        resolveRegion.srcSubresource.baseArrayLayer = 0;
        resolveRegion.srcSubresource.layerCount     = 1;
        resolveRegion.srcOffset                     = {};
        resolveRegion.dstSubresource                = resolveRegion.srcSubresource;
        resolveRegion.dstOffset                     = {};
        resolveRegion.extent                        = image.image.getRotatedExtents();

        mColorImageMS.resolve(&image.image, resolveRegion, commandBuffer);
    }

    image.image.recordReadBarrier(contextVk, VK_IMAGE_ASPECT_COLOR_BIT,
                                  vk::ImageLayout::ColorAttachment, commandBuffer);

    // We are not going to present with Vulkan, therefore there is no need for a present semaphore
    ANGLE_TRY(
        contextVk->flushAndGetSerial(nullptr, swapSerial, RenderPassClosureReason::EGLSwapBuffers));

    gbm_surface_present_buffer(mGbmSurface, mCurrentGbmImageIndex);

    return angle::Result::Continue;
}

angle::Result SurfaceVkGbm::swapImpl(const gl::Context *context)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "SurfaceVkGbm::swapImpl");

    // We need to acquire new back buffer before presenting previous one
    // otherwise we would end up acquiring again the buffer we just presented.
    size_t imageIndex;
    if (!gbm_surface_acquire_back_buffer(mGbmSurface, &imageIndex))
    {
        ERR() << "Failed to acquire a back buffer";
        return angle::Result::Stop;
    }

    // Make sure render target is up to date
    GbmImage &gbmImage = mGbmImages[mCurrentGbmImageIndex];
    // Update RenderTarget pointers to this swapchain image if not multisampling.
    if (!isMultiSampled())
    {
        mColorRenderTarget.updateSwapchainImage(&gbmImage.image, &gbmImage.imageViews, nullptr,
                                                nullptr);
    }

    // Present previously acquired back-buffer
    ContextVk *contextVk = vk::GetImpl(context);
    ANGLE_TRY(initializeMS(contextVk));

    ANGLE_TRY(present(contextVk));

    RendererVk *renderer = contextVk->getRenderer();
    DisplayVk *displayVk = vk::GetImpl(context->getDisplay());
    ANGLE_TRY(renderer->syncPipelineCacheVk(displayVk, context));

    // Finally update current image index
    mCurrentGbmImageIndex = imageIndex;

    return angle::Result::Continue;
}

egl::Error SurfaceVkGbm::postSubBuffer(const gl::Context * /*context*/,
                                       EGLint /*x*/,
                                       EGLint /*y*/,
                                       EGLint /*width*/,
                                       EGLint /*height*/)
{
    return egl::NoError();
}

egl::Error SurfaceVkGbm::querySurfacePointerANGLE(EGLint /*attribute*/, void ** /*value*/)
{
    UNREACHABLE();
    return egl::EglBadCurrentSurface();
}

egl::Error SurfaceVkGbm::bindTexImage(const gl::Context * /*context*/,
                                      gl::Texture * /*texture*/,
                                      EGLint /*buffer*/)
{
    return egl::NoError();
}

egl::Error SurfaceVkGbm::releaseTexImage(const gl::Context * /*context*/, EGLint /*buffer*/)
{
    return egl::NoError();
}

egl::Error SurfaceVkGbm::getSyncValues(EGLuint64KHR * /*ust*/,
                                       EGLuint64KHR * /*msc*/,
                                       EGLuint64KHR * /*sbc*/)
{
    UNIMPLEMENTED();
    return egl::EglBadAccess();
}

egl::Error SurfaceVkGbm::getMscRate(EGLint * /*numerator*/, EGLint * /*denominator*/)
{
    UNIMPLEMENTED();
    return egl::EglBadAccess();
}

void SurfaceVkGbm::setSwapInterval(EGLint /*interval*/) {}

EGLint SurfaceVkGbm::getWidth() const
{
    return static_cast<EGLint>(mWidth);
}

EGLint SurfaceVkGbm::getHeight() const
{
    return static_cast<EGLint>(mHeight);
}

EGLint SurfaceVkGbm::isPostSubBufferSupported() const
{
    return EGL_FALSE;
}

EGLint SurfaceVkGbm::getSwapBehavior() const
{
    return EGL_BUFFER_DESTROYED;
}

vk::Framebuffer &SurfaceVkGbm::chooseFramebuffer(const SwapchainResolveMode swapchainResolveMode)
{
    if (isMultiSampled())
    {
        return swapchainResolveMode == SwapchainResolveMode::Enabled
                   ? mGbmImages[mCurrentGbmImageIndex].framebufferResolveMS
                   : mFramebufferMS;
    }

    return mFramebufferFetchMode == FramebufferFetchMode::Enabled
               ? mGbmImages[mCurrentGbmImageIndex].fetchFramebuffer
               : mGbmImages[mCurrentGbmImageIndex].framebuffer;
}

angle::Result SurfaceVkGbm::getCurrentFramebuffer(ContextVk *contextVk,
                                                  FramebufferFetchMode fetchMode,
                                                  const vk::RenderPass &compatibleRenderPass,
                                                  const SwapchainResolveMode swapchainResolveMode,
                                                  vk::Framebuffer **framebufferOut)
{
    // Track the new fetch mode
    mFramebufferFetchMode = fetchMode;

    vk::Framebuffer &currentFramebuffer = chooseFramebuffer(swapchainResolveMode);

    if (currentFramebuffer.valid())
    {
        // Validation layers should detect if the render pass is really compatible.
        *framebufferOut = &currentFramebuffer;
        return angle::Result::Continue;
    }

    VkFramebufferCreateInfo framebufferInfo = {};
    uint32_t attachmentCount                = mDepthStencilImage.valid() ? 2u : 1u;

    const gl::Extents rotatedExtents      = mColorRenderTarget.getRotatedExtents();
    std::array<VkImageView, 2> imageViews = {};

    if (mDepthStencilImage.valid())
    {
        const vk::ImageView *imageView = nullptr;
        ANGLE_TRY(mDepthStencilRenderTarget.getImageView(contextVk, &imageView));
        imageViews[1] = imageView->getHandle();
    }

    framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.flags           = 0;
    framebufferInfo.renderPass      = compatibleRenderPass.getHandle();
    framebufferInfo.attachmentCount = attachmentCount;
    framebufferInfo.pAttachments    = imageViews.data();
    framebufferInfo.width           = static_cast<uint32_t>(rotatedExtents.width);
    framebufferInfo.height          = static_cast<uint32_t>(rotatedExtents.height);
    framebufferInfo.layers          = 1;

    if (isMultiSampled())
    {
        const vk::ImageView *imageView = nullptr;
        ANGLE_TRY(mColorRenderTarget.getImageView(contextVk, &imageView));
        imageViews[0] = imageView->getHandle();

        if (swapchainResolveMode == SwapchainResolveMode::Enabled)
        {
            framebufferInfo.attachmentCount = attachmentCount + 1;

            for (GbmImage &gbmImage : mGbmImages)
            {
                ANGLE_TRY(gbmImage.imageViews.getLevelLayerDrawImageView(
                    contextVk, gbmImage.image, vk::LevelIndex(0), 0,
                    gl::SrgbWriteControlMode::Default, &imageView));
                imageViews[attachmentCount] = imageView->getHandle();
                ANGLE_VK_TRY(contextVk, gbmImage.framebufferResolveMS.init(contextVk->getDevice(),
                                                                           framebufferInfo));
            }
        }
        else
        {

            // If multisampled, there is only a single color image and framebuffer.
            ANGLE_VK_TRY(contextVk, mFramebufferMS.init(contextVk->getDevice(), framebufferInfo));
        }
    }
    else
    {
        for (GbmImage &gbmImage : mGbmImages)
        {
            const vk::ImageView *imageView = nullptr;
            ANGLE_TRY(gbmImage.imageViews.getLevelLayerDrawImageView(
                contextVk, gbmImage.image, vk::LevelIndex(0), 0, gl::SrgbWriteControlMode::Default,
                &imageView));

            imageViews[0] = imageView->getHandle();

            if (fetchMode == FramebufferFetchMode::Enabled)
            {
                ANGLE_VK_TRY(contextVk, gbmImage.fetchFramebuffer.init(contextVk->getDevice(),
                                                                       framebufferInfo));
            }
            else
            {
                ANGLE_VK_TRY(contextVk,
                             gbmImage.framebuffer.init(contextVk->getDevice(), framebufferInfo));
            }
        }
    }

    ASSERT(currentFramebuffer.valid());
    *framebufferOut = &currentFramebuffer;
    return angle::Result::Continue;
}

angle::Result SurfaceVkGbm::initializeContents(const gl::Context *context,
                                               const gl::ImageIndex &imageIndex)
{
    ContextVk *contextVk = vk::GetImpl(context);

    ANGLE_TRY(initializeMS(contextVk));

    ASSERT(mGbmImages.size() > 0);
    ASSERT(mCurrentGbmImageIndex < mGbmImages.size());

    vk::ImageHelper *image =
        isMultiSampled() ? &mColorImageMS : &mGbmImages[mCurrentGbmImageIndex].image;
    image->stageRobustResourceClear(imageIndex);
    ANGLE_TRY(image->flushAllStagedUpdates(contextVk));

    if (mDepthStencilImage.valid())
    {
        mDepthStencilImage.stageRobustResourceClear(gl::ImageIndex::Make2D(0));
        ANGLE_TRY(mDepthStencilImage.flushAllStagedUpdates(contextVk));
    }

    return angle::Result::Continue;
}

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

EGLint SurfaceVkGbm::origin() const
{
    return EGL_UPPER_LEFT_KHR;
}

bool SurfaceVkGbm::isMultiSampled() const
{
    return mColorImageMS.valid();
}

}  // namespace rx
