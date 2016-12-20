//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkXcb.h:
//    Defines the class interface for DisplayVkXcb, implementing DisplayVk for X via XCB.
//

#ifndef LIBANGLE_RENDERER_VULKAN_DISPLAYVKXCB_H_
#define LIBANGLE_RENDERER_VULKAN_DISPLAYVKXCB_H_

#include "libANGLE/renderer/vulkan/DisplayVk.h"

struct xcb_connection_t;

namespace rx
{
class DisplayVkXcb : public DisplayVk
{
  public:
    DisplayVkXcb();

    egl::Error initialize(egl::Display *display) override;
    void terminate() override;

    bool isValidNativeWindow(EGLNativeWindowType window) const override;

    SurfaceImpl *createWindowSurface(const egl::SurfaceState &state,
                                     EGLNativeWindowType window,
                                     const egl::AttributeMap &attribs) override;

  private:
    xcb_connection_t *mXcbConnection;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_DISPLAYVKXCB_H_
