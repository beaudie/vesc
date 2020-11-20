//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DeviceImpl.cpp: Implementation methods of egl::Device

#include "libANGLE/renderer/DeviceImpl.h"

namespace rx
{

DeviceImpl::DeviceImpl() {}

DeviceImpl::~DeviceImpl() {}

// MockDevice implementation.
MockDevice::MockDevice() = default;

egl::Error MockDevice::initialize()
{
    return egl::NoError();
}

egl::Error MockDevice::getAttribute(const egl::Display *display, EGLint attribute, void **outValue)
{
    UNREACHABLE();
    return egl::EglBadAttribute();
}

EGLint MockDevice::getType()
{
    UNREACHABLE();
    return EGL_NONE;
}

void MockDevice::generateExtensions(egl::DeviceExtensions *outExtensions) const
{
    *outExtensions = egl::DeviceExtensions();
}
}  // namespace rx
