//
// Copyright 2021-2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkWayland.cpp:
//    Implements the class methods for WindowSurfaceVkWayland.
//

#include "libANGLE/renderer/vulkan/linux/wayland/WindowSurfaceVkWayland.h"

#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"

#include <wayland-egl-backend.h>

namespace rx
{

void WindowSurfaceVkWayland::ResizeCallback(wl_egl_window *eglWindow, void *payload)
{
    WindowSurfaceVkWayland *windowSurface = reinterpret_cast<WindowSurfaceVkWayland *>(payload);

    if (windowSurface->mExtents.width != eglWindow->width ||
        windowSurface->mExtents.height != eglWindow->height)
    {
        windowSurface->mExtents.width  = eglWindow->width;
        windowSurface->mExtents.height = eglWindow->height;

        // Trigger swapchain resize
        windowSurface->mResized = true;
    }
}

WindowSurfaceVkWayland::WindowSurfaceVkWayland(const egl::SurfaceState &surfaceState,
                                               EGLNativeWindowType window,
                                               wl_display *display)
    : WindowSurfaceVk(surfaceState, window), mWaylandDisplay(display)
{
    wl_egl_window *eglWindow   = reinterpret_cast<wl_egl_window *>(window);
    eglWindow->resize_callback = WindowSurfaceVkWayland::ResizeCallback;
    eglWindow->driver_private  = this;

    mExtents = gl::Extents(eglWindow->width, eglWindow->height, 1);
}

angle::Result WindowSurfaceVkWayland::createSurfaceVk(vk::Context *context, gl::Extents *extentsOut)
{
    ANGLE_VK_CHECK(context,
                   vkGetPhysicalDeviceWaylandPresentationSupportKHR(
                       context->getRenderer()->getPhysicalDevice(), 0, mWaylandDisplay),
                   VK_ERROR_INITIALIZATION_FAILED);

    wl_egl_window *eglWindow = reinterpret_cast<wl_egl_window *>(mNativeWindowType);

    VkWaylandSurfaceCreateInfoKHR createInfo = {};

    createInfo.sType   = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.flags   = 0;
    createInfo.display = mWaylandDisplay;
    createInfo.surface = eglWindow->surface;
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

egl::Error WindowSurfaceVkWayland::getUserWidth(const egl::Display *display, EGLint *value) const
{
    *value = getWidth();
    return egl::NoError();
}

egl::Error WindowSurfaceVkWayland::getUserHeight(const egl::Display *display, EGLint *value) const
{
    *value = getHeight();
    return egl::NoError();
}

egl::Error WindowSurfaceVkWayland::maybeResize(const gl::Context *context)
{
    if (!mResized)
    {
        return egl::NoError();
    }

    mResized             = false;
    ContextVk *contextVk = vk::GetImpl(context);
    angle::Result result = recreateSwapchain(contextVk, mExtents);
    DisplayVk *displayVk = vk::GetImpl(context->getDisplay());
    return angle::ToEGL(result, displayVk, EGL_BAD_SURFACE);
}

egl::Error WindowSurfaceVkWayland::swap(const gl::Context *context)
{
    egl::Error ret = WindowSurfaceVk::swap(context);
    if (ret.isError())
    {
        return ret;
    }

    return maybeResize(context);
}

egl::Error WindowSurfaceVkWayland::swapWithDamage(const gl::Context *context,
                                                  const EGLint *rects,
                                                  EGLint n_rects)
{
    egl::Error ret = WindowSurfaceVk::swapWithDamage(context, rects, n_rects);
    if (ret.isError())
    {
        return ret;
    }

    return maybeResize(context);
}

}  // namespace rx
