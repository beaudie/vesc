//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkMacOS.mm:
//    Implements methods from DisplayVkMacOS
//

#include "libANGLE/renderer/vulkan/macOS/DisplayVkMacOS.h"

#include <vulkan/vulkan.h>

#include "libANGLE/renderer/vulkan/macOS/WindowSurfaceVkMacOS.h"
#include "libANGLE/renderer/vulkan/vk_caps_utils.h"

#import <Cocoa/Cocoa.h>

namespace rx
{

DisplayVkMacOS::DisplayVkMacOS(const egl::DisplayState &state) : DisplayVk(state) {}

bool DisplayVkMacOS::isValidNativeWindow(EGLNativeWindowType window) const
{
    NSObject *layer = reinterpret_cast<NSObject *>(window);
    return [layer isKindOfClass:[CALayer class]];
}

SurfaceImpl *DisplayVkMacOS::createWindowSurfaceVk(const egl::SurfaceState &state,
                                                   EGLNativeWindowType window,
                                                   EGLint width,
                                                   EGLint height)
{
    ASSERT(isValidNativeWindow(window));
    return new WindowSurfaceVkMacOS(state, window, width, height);
}

egl::ConfigSet DisplayVkMacOS::generateConfigs()
{
    constexpr GLenum kColorFormats[] = {GL_BGRA8_EXT, GL_BGRX8_ANGLEX};
    return egl_vk::GenerateConfigs(kColorFormats, egl_vk::kConfigDepthStencilFormats, this);
}

bool DisplayVkMacOS::checkConfigSupport(egl::Config *config)
{
    // TODO(geofflang): Test for native support and modify the config accordingly.
    // anglebug.com/2692
    return true;
}

const char *DisplayVkMacOS::getWSIExtension() const
{
    return VK_MVK_MACOS_SURFACE_EXTENSION_NAME;
}

}  // namespace rx
