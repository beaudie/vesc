//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayVk.cpp: Implements the class methods for DisplayVk.

#include "libANGLE/renderer/vulkan/DisplayVk.h"

#include "common/debug.h"

namespace rx
{

DisplayVk::DisplayVk() : DisplayImpl()
{
}

DisplayVk::~DisplayVk()
{
}

egl::Error DisplayVk::initialize(egl::Display *display)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_BAD_ACCESS);
}

void DisplayVk::terminate()
{
    UNIMPLEMENTED();
}

SurfaceImpl *DisplayVk::createWindowSurface(const egl::Config *configuration,
                                            EGLNativeWindowType window,
                                            const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return static_cast<SurfaceImpl *>(0);
}

SurfaceImpl *DisplayVk::createPbufferSurface(const egl::Config *configuration,
                                             const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return static_cast<SurfaceImpl *>(0);
}

SurfaceImpl *DisplayVk::createPbufferFromClientBuffer(const egl::Config *configuration,
                                                      EGLClientBuffer shareHandle,
                                                      const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return static_cast<SurfaceImpl *>(0);
}

SurfaceImpl *DisplayVk::createPixmapSurface(const egl::Config *configuration,
                                            NativePixmapType nativePixmap,
                                            const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return static_cast<SurfaceImpl *>(0);
}

ImageImpl *DisplayVk::createImage(EGLenum target,
                                  egl::ImageSibling *buffer,
                                  const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return static_cast<ImageImpl *>(0);
}

gl::Context *DisplayVk::createContext(const egl::Config *config,
                                      const gl::Context *shareContext,
                                      const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return static_cast<gl::Context *>(0);
}

egl::Error DisplayVk::makeCurrent(egl::Surface *drawSurface,
                                  egl::Surface *readSurface,
                                  gl::Context *context)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_BAD_ACCESS);
}

egl::ConfigSet DisplayVk::generateConfigs() const
{
    UNIMPLEMENTED();
    return egl::ConfigSet();
}

bool DisplayVk::isDeviceLost() const
{
    UNIMPLEMENTED();
    return bool();
}

bool DisplayVk::testDeviceLost()
{
    UNIMPLEMENTED();
    return bool();
}

egl::Error DisplayVk::restoreLostDevice()
{
    UNIMPLEMENTED();
    return egl::Error(EGL_BAD_ACCESS);
}

bool DisplayVk::isValidNativeWindow(EGLNativeWindowType window) const
{
    UNIMPLEMENTED();
    return bool();
}

std::string DisplayVk::getVendorString() const
{
    UNIMPLEMENTED();
    return std::string();
}

egl::Error DisplayVk::getDevice(DeviceImpl **device)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_BAD_ACCESS);
}

egl::Error DisplayVk::waitClient() const
{
    UNIMPLEMENTED();
    return egl::Error(EGL_BAD_ACCESS);
}

egl::Error DisplayVk::waitNative(EGLint engine,
                                 egl::Surface *drawSurface,
                                 egl::Surface *readSurface) const
{
    UNIMPLEMENTED();
    return egl::Error(EGL_BAD_ACCESS);
}

void DisplayVk::generateExtensions(egl::DisplayExtensions *outExtensions) const
{
    UNIMPLEMENTED();
}

void DisplayVk::generateCaps(egl::Caps *outCaps) const
{
    UNIMPLEMENTED();
}

}  // namespace rx
