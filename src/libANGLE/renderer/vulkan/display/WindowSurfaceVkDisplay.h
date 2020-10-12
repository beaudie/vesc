//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkDisplay.h:
//    Defines the class interface for WindowSurfaceVkDisplay, implementing WindowSurfaceVk.
//

#ifndef LIBANGLE_RENDERER_VULKAN_XCB_WINDOWSURFACEVKDISPLAY_H_
#define LIBANGLE_RENDERER_VULKAN_XCB_WINDOWSURFACEVKDISPLAY_H_

#include "libANGLE/renderer/vulkan/SurfaceVk.h"

namespace rx
{

// TODO: here is to align the structure with the util/display/DisplayWindow.h
// could update this part with including common header file
typedef struct
{
    unsigned short width;
    unsigned short height;
} display_window;
class WindowSurfaceVkDisplay : public WindowSurfaceVk
{
  public:
    WindowSurfaceVkDisplay(const egl::SurfaceState &surfaceState, EGLNativeWindowType window);
    ~WindowSurfaceVkDisplay() final;

  private:
    angle::Result createSurfaceVk(vk::Context *context, gl::Extents *extentsOut) override;
    angle::Result getCurrentWindowSize(vk::Context *context, gl::Extents *extentsOut) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_XCB_WINDOWSURFACEVKDISPLAY_H_
