//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkMvk.h:
//    Defines the class interface for DisplayVkMvk, implementing DisplayVk for Apple
//    platforms using the VK_MVK_{ios/macos}_surface extensions.

#ifndef LIBANGLE_RENDERER_VULKAN_MVK_DISPLAYVKMVK_H_
#define LIBANGLE_RENDERER_VULKAN_MVK_DISPLAYVKMVK_H_

#include "libANGLE/renderer/vulkan/DisplayVk.h"

namespace rx
{

class DisplayVkMvk : public DisplayVk
{
  public:
    DisplayVkMvk(const egl::DisplayState &state);

    egl::Error initialize(egl::Display *display) override;
    void terminate() override;

    bool isValidNativeWindow(EGLNativeWindowType window) const override;

    SurfaceImpl *createWindowSurfaceVk(const egl::SurfaceState &state,
                                       EGLNativeWindowType window,
                                       EGLint width,
                                       EGLint height) override;

    egl::ConfigSet generateConfigs() override;
    bool checkConfigSupport(egl::Config *config) override;

    const char *getWSIName() const override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_MVK_DISPLAYVKMVK_H_
