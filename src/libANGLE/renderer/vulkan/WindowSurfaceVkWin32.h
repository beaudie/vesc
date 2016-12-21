//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SurfaceVkWin32.h:
//    Defines the class interface for WindowSurfaceVkWin32, implementing SurfaceVk
//

#ifndef LIBANGLE_RENDERER_VULKAN_SURFACEVKWIN32_H_
#define LIBANGLE_RENDERER_VULKAN_SURFACEVKWIN32_H_

#include "libANGLE/renderer/SurfaceVk.h"

namespace rx
{

class WindowSurfaceVkWin32 : public WindowSurfaceVk
{
  public:
    WindowSurfaceVk(const egl::SurfaceState &surfaceState,
                    EGLNativeWindowType window,
                    EGLint width,
                    EGLint height);

  private:
    vk::Error createSurfaceVk(uint32_t *width, uint32_t *height) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_SURFACEVKWIN32_H_
