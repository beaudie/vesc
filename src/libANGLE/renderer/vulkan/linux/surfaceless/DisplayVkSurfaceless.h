//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkSurfaceless.h:
//    Defines the class interface for DisplayVkSurfaceless, implementing surfaceless DisplayVk for
//    Linux.
//

#ifndef LIBANGLE_RENDERER_VULKAN_SURFACELESS_DISPLAYVKSURFACELESS_H_
#define LIBANGLE_RENDERER_VULKAN_SURFACELESS_DISPLAYVKSURFACELESS_H_

#include "libANGLE/renderer/vulkan/linux/DisplayVkLinux.h"

namespace rx
{

class DisplayVkSurfaceless : public DisplayVkLinux
{
  public:
    DisplayVkSurfaceless(const egl::DisplayState &state);

    bool isValidNativeWindow(EGLNativeWindowType window) const override;

    SurfaceImpl *createWindowSurfaceVk(const egl::SurfaceState &state,
                                       EGLNativeWindowType window) override;

    egl::ConfigSet generateConfigs() override;

    void checkConfigSupport(egl::Config *config) override;

    const char *getWSIExtension() const override;

    bool isUsingSwapchain() const override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_SURFACELESS_DISPLAYVKSURFACELESS_H_
