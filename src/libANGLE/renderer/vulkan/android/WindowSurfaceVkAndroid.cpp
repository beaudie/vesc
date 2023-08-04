//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkAndroid.cpp:
//    Implements the class methods for WindowSurfaceVkAndroid.
//

#include "libANGLE/renderer/vulkan/android/WindowSurfaceVkAndroid.h"

#include <android/native_window.h>

#include "libANGLE/Display.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/renderer/vulkan/android/DisplayVkAndroid.h"

namespace rx
{

WindowSurfaceVkAndroid::WindowSurfaceVkAndroid(const egl::SurfaceState &surfaceState,
                                               EGLNativeWindowType window)
    : WindowSurfaceVk(surfaceState, window)
{}

angle::Result WindowSurfaceVkAndroid::createSurfaceVk(vk::Context *context)
{
    DisplayVkAndroid *display = GetImplAs<DisplayVkAndroid>(context->getRenderer()->getDisplay());

    // See if this ANativeWindow already has a surface first.
    VkSurfaceKHR *surface = display->onSurfaceCreate(mNativeWindowType);

    // If not, create one.
    if (*surface == VK_NULL_HANDLE)
    {
        VkAndroidSurfaceCreateInfoKHR createInfo = {};

        createInfo.sType  = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
        createInfo.flags  = 0;
        createInfo.window = mNativeWindowType;
        ANGLE_VK_TRY(context, vkCreateAndroidSurfaceKHR(context->getRenderer()->getInstance(),
                                                        &createInfo, nullptr, surface));
    }

    mSurface = *surface;
    return angle::Result::Continue;
}

void WindowSurfaceVkAndroid::destroySurfaceVk(vk::Context *context)
{
    RendererVk *renderer      = context->getRenderer();
    DisplayVkAndroid *display = GetImplAs<DisplayVkAndroid>(renderer->getDisplay());

    // See if there is another ref to this surface.  If so, just remove a ref and do nothing.
    if (!display->onSurfaceDestroy(mNativeWindowType))
    {
        mSurface = VK_NULL_HANDLE;
        return;
    }

    // Destroy the surface without holding the EGL lock.  This works around a specific deadlock
    // in Android.  On this platform:
    //
    // - For EGL applications, parts of surface creation and destruction are handled by the
    //   platform, and parts of it are done by the native EGL driver.  Namely, on surface
    //   destruction, native_window_api_disconnect is called outside the EGL driver.
    // - For Vulkan applications, vkDestroySurfaceKHR takes full responsibility for destroying
    //   the surface, including calling native_window_api_disconnect.
    //
    // Unfortunately, native_window_api_disconnect may use EGL sync objects and can lead to
    // calling into the EGL driver.  For ANGLE, this is particularly problematic because it is
    // simultaneously a Vulkan application and the EGL driver, causing `vkDestroySurfaceKHR` to
    // call back into ANGLE and attempt to reacquire the EGL lock.
    //
    // Since there are no users of the surface when calling vkDestroySurfaceKHR, it is safe for
    // ANGLE to destroy it without holding the EGL lock, effectively simulating the situation
    // for EGL applications, where native_window_api_disconnect is called after the EGL driver
    // has returned.
    if (mSurface != VK_NULL_HANDLE)
    {
        egl::Display::GetCurrentThreadUnlockedTailCall()->add(
            [surface = mSurface, instance = renderer->getInstance()]() {
                ANGLE_TRACE_EVENT0("gpu.angle", "WindowSurfaceVk::destroy:vkDestroySurfaceKHR");
                vkDestroySurfaceKHR(instance, surface, nullptr);
            });
        mSurface = VK_NULL_HANDLE;
    }
}

angle::Result WindowSurfaceVkAndroid::getCurrentWindowSize(vk::Context *context,
                                                           gl::Extents *extentsOut)
{
    RendererVk *renderer                   = context->getRenderer();
    const VkPhysicalDevice &physicalDevice = renderer->getPhysicalDevice();
    VkSurfaceCapabilitiesKHR surfaceCaps;
    ANGLE_VK_TRY(context,
                 vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, mSurface, &surfaceCaps));
    *extentsOut = gl::Extents(surfaceCaps.currentExtent.width, surfaceCaps.currentExtent.height, 1);
    return angle::Result::Continue;
}

}  // namespace rx
