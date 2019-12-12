//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// IOSurfaceSurfaceVkMac.h:
//    Subclasses SurfaceVk for the Mac platform to implement PBuffers using an IOSurface
//

#ifndef LIBANGLE_RENDERER_VULKAN_MAC_IOSURFACESURFACEVKMAC_H_
#define LIBANGLE_RENDERER_VULKAN_MAC_IOSURFACESURFACEVKMAC_H_

#include "libANGLE/renderer/vulkan/SurfaceVk.h"

struct __IOSurface;
typedef __IOSurface *IOSurfaceRef;

namespace egl
{
class AttributeMap;
}  // namespace egl

namespace rx
{

class DisplayCGL;
class FunctionsGL;
class StateManagerGL;

class IOSurfaceSurfaceVkMac : public SurfaceVk
{
  public:
    IOSurfaceSurfaceVkMac(const egl::SurfaceState &state,
                          EGLClientBuffer buffer,
                          const egl::AttributeMap &attribs);
    ~IOSurfaceSurfaceVkMac() override;

    egl::Error initialize(const egl::Display *display) override;
    egl::Error makeCurrent(const gl::Context *context) override;
    egl::Error unMakeCurrent(const gl::Context *context) override;

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
    void setSwapInterval(EGLint interval) override;

    EGLint getWidth() const override;
    EGLint getHeight() const override;

    EGLint isPostSubBufferSupported() const override;
    EGLint getSwapBehavior() const override;

    static bool validateAttributes(EGLClientBuffer buffer, const egl::AttributeMap &attribs);
    FramebufferImpl *createDefaultFramebuffer(const gl::Context *context,
                                              const gl::FramebufferState &state) override;

    bool hasEmulatedAlphaChannel() const override;

  private:
    angle::Result initializeAlphaChannel(const gl::Context *context, GLuint texture);

    IOSurfaceRef mIOSurface;
    int mWidth;
    int mHeight;
    int mPlane;
    int mFormatIndex;

    bool mAlphaInitialized;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_MAC_IOSURFACESURFACEVKMAC_H_
