//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkDisplay.h:
//    Defines the class interface for DisplayVkDisplay, implementing
//    DisplayVk for Linux via VK_KHR_display.
//

#ifndef LIBANGLE_RENDERER_VULKAN_XCB_DISPLAYVKDISPLAY_H_
#define LIBANGLE_RENDERER_VULKAN_XCB_DISPLAYVKDISPLAY_H_

#include "libANGLE/renderer/vulkan/DisplayVk.h"

namespace rx
{

class DisplayVkDisplay : public DisplayVk
{
  public:
    DisplayVkDisplay(const egl::DisplayState &state);
    void terminate() override;

    bool isValidNativeWindow(EGLNativeWindowType window) const override;

    SurfaceImpl *createWindowSurfaceVk(const egl::SurfaceState &state,
                                       EGLNativeWindowType window) override;

    egl::ConfigSet generateConfigs() override;
    void checkConfigSupport(egl::Config *config) override;

    const char *getWSIExtension() const override;

  private:
    std::vector<VkSurfaceFormatKHR> mSurfaceFormats;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_XCB_DISPLAYVKDISPLAY_H_
