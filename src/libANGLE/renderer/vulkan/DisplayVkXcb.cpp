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

#include "libANGLE/renderer/vulkan/SurfaceVk.h"

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
        // TODO(fjhenigman): return an error
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

SurfaceImpl *DisplayVkXcb::createWindowSurface(const egl::SurfaceState &state,
                                               EGLNativeWindowType window,
                                               const egl::AttributeMap &attribs)
{
    EGLint width  = attribs.getAsInt(EGL_WIDTH, 0);
    EGLint height = attribs.getAsInt(EGL_HEIGHT, 0);

    return new WindowSurfaceVk(state, window, width, height, mXcbConnection);
}

}  // namespace rx
