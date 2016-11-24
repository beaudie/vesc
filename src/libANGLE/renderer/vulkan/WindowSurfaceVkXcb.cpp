//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkXcb.cpp:
//    Implements the class methods for WindowSurfaceVk.
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

vk::ErrorOrResult<gl::Extents> WindowSurfaceVkXcb::createSurfaceVk()
{
    VkXcbSurfaceCreateInfoKHR createInfo;

    createInfo.sType      = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext      = nullptr;
    createInfo.flags      = 0;
    createInfo.connection = mXcbConnection;
    createInfo.window     = mNativeWindowType;
    ANGLE_VK_TRY(vkCreateXcbSurfaceKHR(mInstance, &createInfo, nullptr, &mSurface));

    // TODO(fjhenigman): get window size
    return gl::Extents(640, 480, 0);
}

}  // namespace rx
