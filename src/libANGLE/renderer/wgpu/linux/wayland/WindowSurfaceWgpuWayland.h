//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceWgpuWayland.h:
//    Defines the class interface for WindowSurfaceWgpuWayland, implementing WindowSurfaceWgpu.
//

#ifndef LIBANGLE_RENDERER_WGPU_LINUX_WAYLAND_WINDOWSURFACEWGPUWAYLAND_H_
#define LIBANGLE_RENDERER_WGPU_LINUX_WAYLAND_WINDOWSURFACEWGPUWAYLAND_H_

#include "libANGLE/renderer/wgpu/SurfaceWgpu.h"

namespace rx
{
class WindowSurfaceWgpuWayland : public WindowSurfaceWgpu
{
  public:
    WindowSurfaceWgpuWayland(const egl::SurfaceState &surfaceState, EGLNativeWindowType window);

  private:
    angle::Result createWgpuSurface(const egl::Display *display,
                                    wgpu::Surface *outSurface) override;
    angle::Result getCurrentWindowSize(const egl::Display *display, gl::Extents *outSize) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_LINUX_WAYLAND_WINDOWSURFACEWGPUWAYLAND_H_
