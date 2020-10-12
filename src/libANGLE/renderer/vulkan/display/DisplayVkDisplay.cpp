//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkDisplay.cpp:
//    Implements the class methods for DisplayVkDisplay.
//

#include "DisplayVkDisplay.h"
#include "WindowSurfaceVkDisplay.h"

#include "libANGLE/Display.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/renderer/vulkan/vk_caps_utils.h"

namespace rx
{

DisplayVkDisplay::DisplayVkDisplay(const egl::DisplayState &state) : DisplayVk(state) {}

void DisplayVkDisplay::terminate()
{
    DisplayVk::terminate();
}

bool DisplayVkDisplay::isValidNativeWindow(EGLNativeWindowType window) const
{
    return true;
}

SurfaceImpl *DisplayVkDisplay::createWindowSurfaceVk(const egl::SurfaceState &state,
                                                     EGLNativeWindowType window)
{
    return new WindowSurfaceVkDisplay(state, window);
}

egl::ConfigSet DisplayVkDisplay::generateConfigs()
{
    constexpr GLenum kColorFormats[] = {GL_RGBA8, GL_BGRA8_EXT, GL_RGB565, GL_RGB8};

    return egl_vk::GenerateConfigs(kColorFormats, egl_vk::kConfigDepthStencilFormats, this);
}

// TODO: Add config check
void DisplayVkDisplay::checkConfigSupport(egl::Config *config) {}

const char *DisplayVkDisplay::getWSIExtension() const
{
    return VK_KHR_DISPLAY_EXTENSION_NAME;
}

bool IsVulkanDisplayDisplayAvailable()
{
    return true;
}

// TODO: Discuss the 'display' if appropriate
DisplayImpl *CreateVulkanDisplayDisplay(const egl::DisplayState &state)
{
    return new DisplayVkDisplay(state);
}

}  // namespace rx
