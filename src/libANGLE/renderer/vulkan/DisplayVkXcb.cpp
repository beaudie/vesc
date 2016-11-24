//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkXcb.cpp:
//    Implements the class methods for DisplayVkXcb.
//

#include "libANGLE/renderer/vulkan/DisplayVkXcb.h"

#include <xcb/xcb.h>

#include "libANGLE/renderer/vulkan/WindowSurfaceVkXcb.h"

namespace rx
{

DisplayVkXcb::DisplayVkXcb() : mXcbConnection(nullptr)
{
}

egl::Error DisplayVkXcb::initialize(egl::Display *display)
{
    mXcbConnection = xcb_connect(nullptr, nullptr);
    if (mXcbConnection == nullptr)
    {
        return egl::Error(EGL_NOT_INITIALIZED);
    }
    return DisplayVk::initialize(display);
}

void DisplayVkXcb::terminate()
{
    ASSERT(mXcbConnection != nullptr);
    xcb_disconnect(mXcbConnection);
    mXcbConnection = nullptr;
    DisplayVk::terminate();
}

bool DisplayVkXcb::isValidNativeWindow(EGLNativeWindowType window) const
{
    // TODO(fjhenigman): something like DisplayGLX::isValidNativeWindow
    return true;
}

SurfaceImpl *DisplayVkXcb::createWindowSurfaceVk(const egl::SurfaceState &state,
                                                 EGLNativeWindowType window,
                                                 EGLint width,
                                                 EGLint height)
{
    return new WindowSurfaceVkXcb(state, window, width, height, mXcbConnection);
}

const char *DisplayVkXcb::getWSIName() const
{
    return VK_KHR_XCB_SURFACE_EXTENSION_NAME;
}

}  // namespace rx
