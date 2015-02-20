//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DeviceGL.h: Defines the class interface for DeviceGL.

#ifndef LIBANGLE_RENDERER_GL_DEVICEGL_H_
#define LIBANGLE_RENDERER_GL_DEVICEGL_H_

#include "libANGLE/Device.h"

namespace rx
{

class DeviceGL : public egl::Device
{
  public:
    DeviceGL(egl::Display* display);
    ~DeviceGL() override;

    egl::Error getDevice(EGLAttrib *value) override;

  private:
    DISALLOW_COPY_AND_ASSIGN(DeviceGL);
};

}

#endif // LIBANGLE_RENDERER_GL_DEVICEGL_H_
