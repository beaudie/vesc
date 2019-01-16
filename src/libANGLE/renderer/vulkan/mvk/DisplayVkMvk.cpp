//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkMvk.h:
//    Defines the class interface for DisplayVkMvk, implementing DisplayVk for Apple
//    platforms using the VK_MVK_{ios/macos}_surface extensions.

#include "libANGLE/renderer/vulkan/mvk/DisplayVkMvk.h"

namespace rx
{
DisplayVkMvk::DisplayVkMvk(const egl::DisplayState &state) : DisplayVk(state) {}

egl::Error DisplayVkMvk::initialize(egl::Display *display)
{
    return egl::NoError();
}

void DisplayVkMvk::terminate() {}

bool DisplayVkMvk::isValidNativeWindow(EGLNativeWindowType window) const
{
    return false;
}

SurfaceImpl *DisplayVkMvk::createWindowSurfaceVk(const egl::SurfaceState &state,
                                                 EGLNativeWindowType window,
                                                 EGLint width,
                                                 EGLint height)
{
    return nullptr;
}

egl::ConfigSet DisplayVkMvk::generateConfigs()
{
    return egl::ConfigSet();
}

bool DisplayVkMvk::checkConfigSupport(egl::Config *config)
{
    return false;
}

const char *DisplayVkMvk::getWSIName() const
{
    // Would need to be updated for iOS surfaces.
    return VK_MVK_MACOS_SURFACE_EXTENSION_NAME;
}
}  // namespace rx
