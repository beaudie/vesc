//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkSurfaceless.cpp:
//    Implements the class methods for DisplayVkSurfaceless.
//

#include "libANGLE/renderer/vulkan/linux/surfaceless/DisplayVkSurfaceless.h"

#include "libANGLE/renderer/vulkan/vk_caps_utils.h"

namespace rx
{

DisplayVkSurfaceless::DisplayVkSurfaceless(const egl::DisplayState &state) : DisplayVkLinux(state)
{}

bool DisplayVkSurfaceless::isValidNativeWindow(EGLNativeWindowType window) const
{
    return false;
}

SurfaceImpl *DisplayVkSurfaceless::createWindowSurfaceVk(const egl::SurfaceState &state,
                                                         EGLNativeWindowType window)
{
    UNIMPLEMENTED();
    return nullptr;
}

egl::ConfigSet DisplayVkSurfaceless::generateConfigs()
{
    const std::array<GLenum, 1> kColorFormats = {GL_BGRA8_EXT};

    std::vector<GLenum> depthStencilFormats(
        egl_vk::kConfigDepthStencilFormats,
        egl_vk::kConfigDepthStencilFormats + ArraySize(egl_vk::kConfigDepthStencilFormats));

    if (getCaps().stencil8)
    {
        depthStencilFormats.push_back(GL_STENCIL_INDEX8);
    }

    egl::ConfigSet cfgSet =
        egl_vk::GenerateConfigs(kColorFormats.data(), kColorFormats.size(),
                                depthStencilFormats.data(), depthStencilFormats.size(), this);

    return cfgSet;
}

void DisplayVkSurfaceless::checkConfigSupport(egl::Config *config)
{
    // No window support.
    config->surfaceType &= ~EGL_WINDOW_BIT;
}

const char *DisplayVkSurfaceless::getWSIExtension() const
{
    return nullptr;
}

bool DisplayVkSurfaceless::isUsingSwapchain() const
{
    return false;
}

bool IsVulkanSurfacelessDisplayAvailable()
{
    return true;
}

DisplayImpl *CreateVulkanSurfacelessDisplay(const egl::DisplayState &state)
{
    return new DisplayVkSurfaceless(state);
}

}  // namespace rx
