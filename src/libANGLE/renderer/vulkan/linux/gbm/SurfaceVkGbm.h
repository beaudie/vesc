//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SurfaceVkGbm.h:
//    Defines the class interface for SurfaceVkGbm, implementing WindowSurfaceVk.
//

#ifndef LIBANGLE_RENDERER_VULKAN_GBM_WINDOWSURFACEVKGBM_H_
#define LIBANGLE_RENDERER_VULKAN_GBM_WINDOWSURFACEVKGBM_H_

#include "libANGLE/renderer/vulkan/SurfaceVk.h"

#include <array>

#include <gbm.h>

struct gbm_surface;

namespace rx
{

struct SurfaceImage : angle::NonCopyable
{
    SurfaceImage();
    SurfaceImage(SurfaceImage &&);
    SurfaceImage(struct gbm_bo *bo);
    ~SurfaceImage();

    SurfaceImage &operator=(SurfaceImage &&other);

    gbm_bo *mGbmBO = nullptr;
    bool mAcquired = false;
    bool mLocked   = false;
};

class SurfaceVkGbm : public WindowSurfaceVk
{
  public:
    SurfaceVkGbm(const egl::SurfaceState &surfaceState, EGLNativeWindowType window);
    ~SurfaceVkGbm() override;

    angle::Result getAttachmentRenderTarget(const gl::Context *context,
                                            GLenum binding,
                                            const gl::ImageIndex &imageIndex,
                                            GLsizei samples,
                                            FramebufferAttachmentRenderTarget **rtOut) override;

    void setSwapInterval(EGLint interval) override;

    egl::Error lockSurface(const egl::Display *display,
                           EGLint usageHint,
                           bool preservePixels,
                           uint8_t **bufferPtrOut,
                           EGLint *bufferPitchOut) override;
    egl::Error unlockSurface(const egl::Display *display, bool preservePixels) override;

    const vk::Semaphore *getAndResetAcquireImageSemaphore() override;

    angle::Result createImages(struct gbm_device *gbmDevice,
                               uint32_t width,
                               uint32_t height,
                               uint32_t format,
                               const uint64_t *modifiers,
                               size_t count,
                               uint32_t usage);
    void destroyImages();
    gbm_bo *lockFrontImage();
    void releaseImage(gbm_bo *bo);
    bool hasFreeImages();

  private:
    angle::Result initializeImpl(DisplayVk *displayVk) override;
    angle::Result createSwapChain(vk::Context *context);
    angle::Result createSwapChainImpl(vk::Context *context);

    angle::Result swapImpl(const gl::Context *context,
                           const EGLint *rects,
                           EGLint n_rects,
                           const void *pNextChain) override;

    angle::Result presentImpl(ContextVk *contextVk,
                              vk::OutsideRenderPassCommandBuffer *commandBuffer,
                              Serial *swapSerial,
                              const EGLint *rects,
                              EGLint n_rects,
                              const void *pNextChain,
                              bool *presentOutOfDate) override;

    angle::Result acquireNextImage(uint32_t *outImageIndex);

    std::array<SurfaceImage, 4> mSurfaceImages;
    size_t mCurrentFrontImage;

    gbm_surface *mGbmSurface;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_GBM_WINDOWSURFACEVKGBM_H_
