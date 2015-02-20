//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DeviceGL.h: GL implementation of egl::Device

#include "libANGLE/renderer/gl/DeviceGL.h"

#include <EGL/eglext.h>

namespace rx
{

DeviceGL::DeviceGL(egl::Display* display)
    : Device(display)
{
}

DeviceGL::~DeviceGL()
{
}

egl::Error DeviceGL::getDevice(EGLAttrib *value)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_BAD_DEVICE_ANGLE);
}

}
