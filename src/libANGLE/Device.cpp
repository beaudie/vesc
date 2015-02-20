//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Device.cpp: Implements the egl::Device class, representing the abstract
// device. Implements EGLDevice.

#include "libANGLE/Device.h"

#include <platform/Platform.h>
#include <EGL/eglext.h>

#include "common/debug.h"
#include "common/platform.h"

namespace egl
{

Device::Device(Display *display)
    : mDisplay(display)
{

}

Device::~Device()
{

}

}
