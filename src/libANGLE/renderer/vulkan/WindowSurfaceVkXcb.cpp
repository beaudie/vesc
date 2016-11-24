//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SurfaceVk.cpp:
//    Implements the class methods for SurfaceVk.
//

#include "libANGLE/renderer/vulkan/WindowSurfaceVkXcb.h"

namespace rx
{

WindowSurfaceVkXcb::WindowSurfaceVkXcb(const egl::SurfaceState &surfaceState,
                                       EGLNativeWindowType window,
                                       EGLint width,
                                       EGLint height,
                                       xcb_connection_t *conn)
    : WindowSurfaceVk(surfaceState, window, width, height), mXcbConnection(conn)
{
}

vk::Error WindowSurfaceVkXcb::createSurfaceVk(uint32_t *width, uint32_t *height)
{
    VkXcbSurfaceCreateInfoKHR createInfo;

    createInfo.sType      = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext      = nullptr;
    createInfo.flags      = 0;
    createInfo.connection = mXcbConnection;
    createInfo.window     = mNativeWindowType;
    ANGLE_VK_TRY(vkCreateXcbSurfaceKHR(mInstance, &createInfo, nullptr, &mSurface));

    // TODO(fjhenigman): get window size
    *width  = 640;
    *height = 480;

    return vk::NoError();
}

}  // namespace rx
