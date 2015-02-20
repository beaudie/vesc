//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DeviceD3D.cpp: D3D implementation of egl::Device

#include "libANGLE/renderer/d3d/DeviceD3D.h"

#include "libANGLE/Device.h"
#include "libANGLE/Display.h"

#include <EGL/eglext.h>

#if defined (ANGLE_ENABLE_D3D9)
#   include "libANGLE/renderer/d3d/d3d9/Renderer9.h"
#endif // ANGLE_ENABLE_D3D9

#if defined (ANGLE_ENABLE_D3D11)
#   include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#endif // ANGLE_ENABLE_D3D11

#if defined (ANGLE_TEST_CONFIG)
#   define ANGLE_DEFAULT_D3D11 1
#endif

#if !defined(ANGLE_DEFAULT_D3D11)
// Enables use of the Direct3D 11 API for a default display, when available
#   define ANGLE_DEFAULT_D3D11 0
#endif

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
