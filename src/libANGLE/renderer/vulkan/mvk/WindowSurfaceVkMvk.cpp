//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkXcb.h:
//    Defines the class interface for WindowSurfaceVkMvk.
//    This implements WindowSurfaceVk for Apple platforms.
//

#include "libANGLE/renderer/vulkan/mvk/WindowSurfaceVkMvk.h"

namespace rx
{

WindowSurfaceVkMvk::WindowSurfaceVkMvk(const egl::SurfaceState &surfaceState,
                                       EGLNativeWindowType window,
                                       EGLint width,
                                       EGLint height)
    : WindowSurfaceVk(surfaceState, window, width, height)
{}

angle::Result WindowSurfaceVkMvk::createSurfaceVk(vk::Context *context, gl::Extents *extentsOut)
{
    return angle::Result::Continue;
}

}  // namespace rx
