//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkDisplay.cpp:
//    Implements the class methods for WindowSurfaceVkDisplay.
//

#include "libANGLE/renderer/vulkan/display/WindowSurfaceVkDisplay.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"

namespace rx
{

WindowSurfaceVkDisplay::WindowSurfaceVkDisplay(const egl::SurfaceState &surfaceState,
                                               EGLNativeWindowType window)
    : WindowSurfaceVk(surfaceState, window)
{}

WindowSurfaceVkDisplay::~WindowSurfaceVkDisplay() {}

static VkResult getPrimaryDisplayModeProperty(rx::RendererVk *renderer,
                                              VkDisplayModePropertiesKHR *property)
{
    VkPhysicalDevice phy_dev = renderer->getPhysicalDevice();
    VkResult ret;

    // Query if there is a valid display
    uint32_t count = 1;
    if (vkGetPhysicalDeviceDisplayPropertiesKHR(phy_dev, &count, nullptr) != VK_SUCCESS ||
        count == 0)
    {
        ERR() << "No valid display device, please connect a display!";
    }

    // Get display properties
    VkDisplayPropertiesKHR prop = {};
    count                       = 1;
    ret                         = vkGetPhysicalDeviceDisplayPropertiesKHR(phy_dev, &count, &prop);
    if (VK_SUCCESS == ret)
    {
        // we should have a valid display now
        ASSERT(prop.display != nullptr);

        ret = vkGetDisplayModePropertiesKHR(phy_dev, prop.display, &count, nullptr);
        if (VK_SUCCESS == ret)
        {
            ASSERT(count != 0);

            std::vector<VkDisplayModePropertiesKHR> mode_props(count);
            ret = vkGetDisplayModePropertiesKHR(phy_dev, prop.display, &count, mode_props.data());
            if (VK_SUCCESS == ret)
            {
                *property = mode_props[0];
            }
        }
    }

    return ret;
}

angle::Result WindowSurfaceVkDisplay::createSurfaceVk(vk::Context *context, gl::Extents *extentsOut)
{
    RendererVk *renderer = context->getRenderer();
    ASSERT(renderer != nullptr);
    VkInstance instance = renderer->getInstance();

    VkDisplayModePropertiesKHR mode_prop = {};
    ANGLE_VK_TRY(context, getPrimaryDisplayModeProperty(renderer, &mode_prop));
    display_window *dpy_window = reinterpret_cast<display_window *>(mNativeWindowType);

    VkDisplaySurfaceCreateInfoKHR info = {};
    info.sType                         = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR;
    info.flags                         = 0;
    info.displayMode                   = mode_prop.displayMode;
    info.planeIndex                    = 0;
    info.planeStackIndex               = 0;
    info.transform                     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    info.globalAlpha                   = 1.0f;
    info.alphaMode                     = VK_DISPLAY_PLANE_ALPHA_GLOBAL_BIT_KHR;
    info.imageExtent.width             = dpy_window->width;
    info.imageExtent.height            = dpy_window->height;

    ANGLE_VK_TRY(context, vkCreateDisplayPlaneSurfaceKHR(instance, &info, nullptr, &mSurface));

    return getCurrentWindowSize(context, extentsOut);
}

angle::Result WindowSurfaceVkDisplay::getCurrentWindowSize(vk::Context *context,
                                                           gl::Extents *extentsOut)
{
    RendererVk *renderer                   = context->getRenderer();
    const VkPhysicalDevice &physicalDevice = renderer->getPhysicalDevice();

    ANGLE_VK_TRY(context, vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, mSurface,
                                                                    &mSurfaceCaps));

    *extentsOut =
        gl::Extents(mSurfaceCaps.currentExtent.width, mSurfaceCaps.currentExtent.height, 1);
    return angle::Result::Continue;
}

}  // namespace rx
