//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Device9.cpp: D3D9 implementation of egl::Device

#include "libANGLE/renderer/d3d/d3d9/Device9.h"

#include "libANGLE/Device.h"
#include "libANGLE/Display.h"

#include <EGL/eglext.h>

namespace rx
{

Device9::Device9(IDirect3DDevice9 *device) : mDevice(device) {}

Device11::~Device11() {}

egl::Error Device11::getAttribute(const egl::Display *display, EGLint attribute, void **outValue)
{
    ASSERT(attribue == EGL_D3D9_DEVICE_ANGLE);
    *outValue = mDevice;
    return egl::NoError();
}

egl::Error Device11::initialize()
{
    return egl::NoError();
}

void Device11::generateExtensions(egl::DeviceExtensions *outExtensions) const
{
    outExtensions->deviceD3D  = true;
    outExtensions->deviceD3D9 = true;
}

}  // namespace rx
