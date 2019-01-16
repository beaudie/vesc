//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkXcb.h:
//    Defines the class interface for WindowSurfaceVkMvk.
//    This implements WindowSurfaceVk for Apple platforms.
//

#ifndef LIBANGLE_RENDERER_VULKAN_XCB_WINDOWSURFACEVKMVK_H_
#define LIBANGLE_RENDERER_VULKAN_XCB_WINDOWSURFACEVKMVK_H_

#include "libANGLE/renderer/vulkan/SurfaceVk.h"

namespace rx
{

class WindowSurfaceVkMvk : public WindowSurfaceVk
{
  public:
    WindowSurfaceVkMvk(const egl::SurfaceState &surfaceState,
                       EGLNativeWindowType window,
                       EGLint width,
                       EGLint height);

  private:
    angle::Result createSurfaceVk(vk::Context *context, gl::Extents *extentsOut) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_XCB_WINDOWSURFACEVKMVK_H_
