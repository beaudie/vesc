//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DeviceWGL.h: WGL implementation of egl::Device

#ifndef LIBANGLE_RENDERER_GL_WGL_DEVICEWGL_H_
#define LIBANGLE_RENDERER_GL_WGL_DEVICEWGL_H_

#include "libANGLE/renderer/gl/DeviceGL.h"

#include <GL/wglext.h>

namespace rx
{

class DeviceWGL : public DeviceGL
{
  public:
    DeviceWGL();
    ~DeviceWGL() override;

    egl::Error queryAttribute(EGLint attribute, EGLAttrib *value) override;

  private:
    DISALLOW_COPY_AND_ASSIGN(DeviceWGL);
};

}

#endif // LIBANGLE_RENDERER_GL_WGL_DEVICEWGL_H_
