//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SurfaceVkGbm.h:
//    Defines the class interface for SurfaceVkGbm, implementing FramebufferSurfaceVk.
//

#ifndef LIBANGLE_RENDERER_VULKAN_GBM_GBMSURFACEVK_H_
#define LIBANGLE_RENDERER_VULKAN_GBM_GBMSURFACEVK_H_

#include "libANGLE/renderer/vulkan/SurfaceVk.h"

struct gbm_device;
struct gbm_surface;
struct gbm_bo;

namespace rx
{
constexpr size_t kMaxMemoryPlanes        = 4;
static constexpr size_t kSwapHistorySize = 2;

struct GbmImage final : angle::NonCopyable
{
    GbmImage();
    GbmImage(GbmImage &&other);
    ~GbmImage();

    gbm_bo *bo;
    vk::ImageHelper image;
    vk::ImageViewHelper imageViews;
    vk::Framebuffer framebuffer;
    vk::Framebuffer fetchFramebuffer;
    vk::Framebuffer framebufferResolveMS;
};

class SurfaceVkGbm : public FramebufferSurfaceVk
{
  public:
    SurfaceVkGbm(const egl::SurfaceState &surfaceState,
                 EGLNativeWindowType window,
                 gbm_device *gbmDevice);
    ~SurfaceVkGbm() override;

    egl::Error initialize(const egl::Display *display) override;
    void destroy(const egl::Display *display) override;

    angle::Result getAttachmentRenderTarget(const gl::Context *context,
                                            GLenum binding,
                                            const gl::ImageIndex &imageIndex,
                                            GLsizei samples,
                                            FramebufferAttachmentRenderTarget **rtOut) override;
    FramebufferImpl *createDefaultFramebuffer(const gl::Context *context,
                                              const gl::FramebufferState &state) override;
    egl::Error swap(const gl::Context *context) override;
    egl::Error postSubBuffer(const gl::Context *context,
                             EGLint x,
                             EGLint y,
                             EGLint width,
                             EGLint height) override;
    egl::Error querySurfacePointerANGLE(EGLint attribute, void **value) override;
    egl::Error bindTexImage(const gl::Context *context,
                            gl::Texture *texture,
                            EGLint buffer) override;
    egl::Error releaseTexImage(const gl::Context *context, EGLint buffer) override;
    egl::Error getSyncValues(EGLuint64KHR *ust, EGLuint64KHR *msc, EGLuint64KHR *sbc) override;
    egl::Error getMscRate(EGLint *numerator, EGLint *denominator) override;
    void setSwapInterval(EGLint interval) override;

    // width and height can change with client window resizing
    EGLint getWidth() const override;
    EGLint getHeight() const override;

    EGLint isPostSubBufferSupported() const override;
    EGLint getSwapBehavior() const override;

    angle::Result initializeContents(const gl::Context *context,
                                     const gl::ImageIndex &imageIndex) override;

    angle::Result getCurrentFramebuffer(ContextVk *context,
                                        FramebufferFetchMode fetchMode,
                                        const vk::RenderPass &compatibleRenderPass,
                                        const SwapchainResolveMode swapchainResolveMode,
                                        vk::Framebuffer **framebufferOut) override;

    egl::Error lockSurface(const egl::Display *display,
                           EGLint usageHint,
                           bool preservePixels,
                           uint8_t **bufferPtrOut,
                           EGLint *bufferPitchOut) override;
    egl::Error unlockSurface(const egl::Display *display, bool preservePixels) override;
    EGLint origin() const override;

  private:
    angle::Result initializeImpl(DisplayVk *displayVk);
    vk::Framebuffer &chooseFramebuffer(const SwapchainResolveMode swapchainResolveMode);
    angle::Result initializeMS(ContextVk *context);
    angle::Result swapImpl(const gl::Context *context);
    angle::Result present(ContextVk *contextVk);

    bool acquireNextImage();

    bool isMultiSampled() const;

    gbm_device *mGbmDevice;
    gbm_surface *mGbmSurface;

    uint32_t mWidth;
    uint32_t mHeight;

    std::vector<GbmImage> mGbmImages;
    size_t mCurrentGbmImageIndex;

    // Depth/stencil image. Possibly multisampled.
    vk::ImageHelper mDepthStencilImage;
    vk::ImageViewHelper mDepthStencilImageViews;
    angle::ObserverBinding mDepthStencilImageBinding;

    // Multisample color image, view and framebuffer, if multisampling enabled.
    vk::ImageHelper mColorImageMS;
    vk::ImageViewHelper mColorImageMSViews;
    angle::ObserverBinding mColorImageMSBinding;
    vk::Framebuffer mFramebufferMS;

    // A circular buffer that stores the serial of the submission fence of the context on every
    // swap. The CPU is throttled by waiting for the 2nd previous serial to finish.
    angle::CircularBuffer<Serial, impl::kSwapHistorySize> mSwapHistory;

    // GL_EXT_shader_framebuffer_fetch
    FramebufferFetchMode mFramebufferFetchMode;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_GBM_WINDOWSURFACEVKGBM_H_
