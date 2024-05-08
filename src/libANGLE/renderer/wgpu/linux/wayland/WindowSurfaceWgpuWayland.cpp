//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceWgpuWayland.cpp:
//    Defines the class interface for WindowSurfaceWgpuWayland, implementing WindowSurfaceWgpu.
//

#include "libANGLE/renderer/wgpu/linux/wayland/WindowSurfaceWgpuWayland.h"

#include "libANGLE/Display.h"
#include "libANGLE/renderer/wgpu/DisplayWgpu.h"
#include "libANGLE/renderer/wgpu/wgpu_utils.h"

namespace rx
{

WindowSurfaceWgpuWayland::WindowSurfaceWgpuWayland(const egl::SurfaceState &surfaceState,
                                                   EGLNativeWindowType window)
    : WindowSurfaceWgpu(surfaceState, window)
{}

angle::Result WindowSurfaceWgpuWayland::createWgpuSurface(const egl::Display *display,
                                                          wgpu::Surface *outSurface)
{
    UNIMPLEMENTED();
    return angle::Result::Continue;
}

angle::Result WindowSurfaceWgpuWayland::getCurrentWindowSize(const egl::Display *display,
                                                             gl::Extents *outSize)
{
    UNIMPLEMENTED();
    return angle::Result::Continue;
}

WindowSurfaceWgpu *CreateWgpuWindowSurface(const egl::SurfaceState &surfaceState,
                                           EGLNativeWindowType window)
{
    return new WindowSurfaceWgpuWayland(surfaceState, window);
}
}  // namespace rx
