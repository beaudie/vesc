//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkWayland.cpp:
//    Implements the class methods for WindowSurfaceVkWayland.
//

#include "libANGLE/renderer/vulkan/linux/wayland/WindowSurfaceVkWayland.h"

#include "libANGLE/renderer/vulkan/RendererVk.h"

#include <wayland-egl-backend.h>

namespace rx
{

void WindowSurfaceVkWayland::ResizeCallback(wl_egl_window *egl_window, void *payload)
{
    WindowSurfaceVkWayland *window_surface = reinterpret_cast<WindowSurfaceVkWayland *>(payload);

    window_surface->mExtents.width  = egl_window->width;
    window_surface->mExtents.height = egl_window->height;
}

WindowSurfaceVkWayland::WindowSurfaceVkWayland(const egl::SurfaceState &surfaceState,
                                               EGLNativeWindowType window,
                                               wl_display *display)
    : WindowSurfaceVk(surfaceState, window), mWaylandDisplay(display)
{
    wl_egl_window *egl_window   = reinterpret_cast<wl_egl_window *>(window);
    egl_window->resize_callback = WindowSurfaceVkWayland::ResizeCallback;
    egl_window->driver_private  = this;

    mExtents = gl::Extents(egl_window->width, egl_window->height, 1);
}

angle::Result WindowSurfaceVkWayland::createSurfaceVk(vk::Context *context, gl::Extents *extentsOut)
{
    ANGLE_VK_CHECK(context,
                   vkGetPhysicalDeviceWaylandPresentationSupportKHR(
                       context->getRenderer()->getPhysicalDevice(), 0, mWaylandDisplay),
                   VK_ERROR_INITIALIZATION_FAILED);

    wl_egl_window *egl_window = reinterpret_cast<wl_egl_window *>(mNativeWindowType);

    VkWaylandSurfaceCreateInfoKHR createInfo = {};

    createInfo.sType   = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.flags   = 0;
    createInfo.display = mWaylandDisplay;
    createInfo.surface = egl_window->surface;
    ANGLE_VK_TRY(context, vkCreateWaylandSurfaceKHR(context->getRenderer()->getInstance(),
                                                    &createInfo, nullptr, &mSurface));

    return getCurrentWindowSize(context, extentsOut);
}

angle::Result WindowSurfaceVkWayland::getCurrentWindowSize(vk::Context *context,
                                                           gl::Extents *extentsOut)
{
    *extentsOut = mExtents;
    return angle::Result::Continue;
}

}  // namespace rx
