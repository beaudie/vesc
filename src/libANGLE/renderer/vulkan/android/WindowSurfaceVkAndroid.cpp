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

#include "libANGLE/renderer/vulkan/RendererVk.h"

#if SVDT_ENABLE_VULKAN_ANDROID_SWAPCHAIN_ACQUIRE_TIMEOUT_WA
// Taken from nativebase/nativebase.h
// https://android.googlesource.com/platform/frameworks/native/+/master/libs/nativebase/include/nativebase/nativebase.h
typedef struct android_native_base_t
{
    /* a magic value defined by the actual EGL native type */
    int magic;
    /* the sizeof() of the actual EGL native type */
    int version;
    void* reserved[4];
    /* reference-counting interface */
    void (*incRef)(struct android_native_base_t* base);
    void (*decRef)(struct android_native_base_t* base);
} android_native_base_t;

// Taken from vndk/window.h
// https://android.googlesource.com/platform/frameworks/native/+/master/libs/nativewindow/include/vndk/window.h
enum ANativeWindowQuery
{
    /* The minimum number of buffers that must remain un-dequeued after a buffer
     * has been queued.  This value applies only if set_buffer_count was used to
     * override the number of buffers and if a buffer has since been queued.
     * Users of the set_buffer_count ANativeWindow method should query this
     * value before calling set_buffer_count.  If it is necessary to have N
     * buffers simultaneously dequeued as part of the steady-state operation,
     * and this query returns M then N+M buffers should be requested via
     * native_window_set_buffer_count.
     *
     * Note that this value does NOT apply until a single buffer has been
     * queued.  In particular this means that it is possible to:
     *
     * 1. Query M = min undequeued buffers
     * 2. Set the buffer count to N + M
     * 3. Dequeue all N + M buffers
     * 4. Cancel M buffers
     * 5. Queue, dequeue, queue, dequeue, ad infinitum
     */
    ANATIVEWINDOW_QUERY_MIN_UNDEQUEUED_BUFFERS = 3,
};

// Taken from system/window.h
// https://android.googlesource.com/platform/frameworks/native/+/master/libs/nativewindow/include/system/window.h
struct ANativeWindow
{
    struct android_native_base_t common;
    /* flags describing some attributes of this surface or its updater */
    const uint32_t flags;
    /* min swap interval supported by this updated */
    const int   minSwapInterval;
    /* max swap interval supported by this updated */
    const int   maxSwapInterval;
    /* horizontal and vertical resolution in DPI */
    const float xdpi;
    const float ydpi;
    /* Some storage reserved for the OEM's driver. */
    intptr_t    oem[4];

    int     (*setSwapInterval)(struct ANativeWindow* window,
                int interval);
    int     (*dequeueBuffer_DEPRECATED)(struct ANativeWindow* window,
                struct ANativeWindowBuffer** buffer);
    int     (*lockBuffer_DEPRECATED)(struct ANativeWindow* window,
                struct ANativeWindowBuffer* buffer);
    int     (*queueBuffer_DEPRECATED)(struct ANativeWindow* window,
                struct ANativeWindowBuffer* buffer);

    /*
     * hook used to retrieve information about the native window.
     *
     * Returns 0 on success or -errno on error.
     */
    int     (*query)(const struct ANativeWindow* window,
                int what, int* value);

    /*
     * hook used to perform various operations on the surface.
     * (*perform)() is a generic mechanism to add functionality to
     * ANativeWindow while keeping backward binary compatibility.
     *
     * DO NOT CALL THIS HOOK DIRECTLY.  Instead, use the helper functions
     * defined below.
     *
     * (*perform)() returns -ENOENT if the 'what' parameter is not supported
     * by the surface's implementation.
     *
     * See above for a list of valid operations, such as
     * NATIVE_WINDOW_SET_USAGE or NATIVE_WINDOW_CONNECT
     */
    int     (*perform)(struct ANativeWindow* window,
                int operation, ... );

    /* Other members were removed. */
};

/* attributes queriable with query() */
enum
{
    /* see ANativeWindowQuery in vndk/window.h */
    NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS = ANATIVEWINDOW_QUERY_MIN_UNDEQUEUED_BUFFERS,
};

/* Valid operations for the (*perform)() hook.
 *
 * Values marked as 'deprecated' are supported, but have been superceded by
 * other functionality.
 *
 * Values marked as 'private' should be considered private to the framework.
 * HAL implementation code with access to an ANativeWindow should not use these,
 * as it may not interact properly with the framework's use of the
 * ANativeWindow.
 */
enum
{
    NATIVE_WINDOW_SET_DEQUEUE_TIMEOUT = 37,    /* private */
};
// END: system/window.h

namespace
{
int32_t query(ANativeWindow* window, int what)
{
    int value;
    int res = window->query(window, what, &value);
    return res < 0 ? res : value;
}
}  // namespace

int32_t ANativeWindow_getMinUndequeuedBuffers(ANativeWindow* window)
{
    return query(window, NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS);
}

int32_t ANativeWindow_setDequeueTimeout(ANativeWindow* window, int64_t timeout)
{
    return window->perform(window, NATIVE_WINDOW_SET_DEQUEUE_TIMEOUT, timeout);
}
#endif  // SVDT_ENABLE_VULKAN_ANDROID_SWAPCHAIN_ACQUIRE_TIMEOUT_WA

namespace rx
{

WindowSurfaceVkAndroid::WindowSurfaceVkAndroid(const egl::SurfaceState &surfaceState,
                                               EGLNativeWindowType window)
    : WindowSurfaceVk(surfaceState, window)
{}

angle::Result WindowSurfaceVkAndroid::createSurfaceVk(vk::Context *context, gl::Extents *extentsOut)
{
    VkAndroidSurfaceCreateInfoKHR createInfo = {};

    createInfo.sType  = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    createInfo.flags  = 0;
    createInfo.window = mNativeWindowType;
    ANGLE_VK_TRY(context, vkCreateAndroidSurfaceKHR(context->getRenderer()->getInstance(),
                                                    &createInfo, nullptr, &mSurface));

    return getCurrentWindowSize(context, extentsOut);
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
