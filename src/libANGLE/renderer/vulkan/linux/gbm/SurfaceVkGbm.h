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

struct gbm_surface;

namespace rx
{

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

  private:
    angle::Result createSurfaceVk(vk::Context *context, gl::Extents *extentsOut) override;
    angle::Result getCurrentWindowSize(vk::Context *context, gl::Extents *extentsOut) override;

    angle::Result initializeImpl(DisplayVk *displayVk) override;
    angle::Result createSwapChain(vk::Context *context);

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

    gbm_surface *mGbmSurface;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_GBM_WINDOWSURFACEVKGBM_H_
