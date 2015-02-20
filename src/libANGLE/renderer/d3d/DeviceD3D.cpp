//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DeviceD3D.cpp: D3D implementation of egl::Device

#include "libANGLE/renderer/d3d/DeviceD3D.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"

#include "libANGLE/Device.h"
#include "libANGLE/Display.h"

#include <EGL/eglext.h>

namespace rx
{

DeviceD3D::DeviceD3D(egl::Display *display, rx::RendererD3D* renderer)
    : Device(display),
      mRenderer(renderer)
{
}

egl::Error DeviceD3D::getDevice(EGLAttrib *value)
{
    *value = reinterpret_cast<EGLAttrib>(mRenderer->getD3DDevice());
    if (*value == 0)
    {
        return egl::Error(EGL_BAD_DEVICE_ANGLE);
    }
    return egl::Error(EGL_SUCCESS);
}

}
