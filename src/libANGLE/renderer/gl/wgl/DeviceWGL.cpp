//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DeviceWGL.h: WGL implementation of egl::Device

#include "libANGLE/renderer/gl/wgl/DeviceWGL.h"

#include "common/debug.h"

#include <EGL/eglext.h>

namespace rx
{

DeviceWGL::DeviceWGL(egl::Display* display)
    : DeviceGL(display)
{
}

DeviceWGL::~DeviceWGL()
{
}

egl::Error DeviceWGL::getDevice(EGLAttrib *value)
{
    return DeviceGL::getDevice(value);
}

}
