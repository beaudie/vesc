//
// Copyright (c) 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DeviceCGL.h: CGL implementation of egl::Device

#ifndef LIBANGLE_RENDERER_GL_CGL_DEVICECGL_H_
#define LIBANGLE_RENDERER_GL_CGL_DEVICECGL_H_

#include "libANGLE/Device.h"
#include "libANGLE/renderer/DeviceImpl.h"

namespace rx
{
class DeviceCGL : public DeviceImpl
{
  public:
    DeviceCGL();
    ~DeviceCGL() override;

    egl::Error initialize() override;
    egl::Error getAttribute(DisplayImpl *owningDisplay, EGLint attribute, void **outValue) override;
    EGLint getType() override;
    void generateExtensions(egl::DeviceExtensions *outExtensions) const override;

  private:
    bool mIsInitialized = false;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_CGL_DEVICECGL_H_
