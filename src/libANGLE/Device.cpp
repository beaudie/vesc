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
#include "libANGLE/renderer/DeviceImpl.h"

#if defined(ANGLE_ENABLE_D3D9) || defined(ANGLE_ENABLE_D3D11)
#   include "libANGLE/renderer/d3d/DeviceD3D.h"
#endif

#if defined(ANGLE_ENABLE_OPENGL)
#   if defined(ANGLE_PLATFORM_WINDOWS)
#       include "libANGLE/renderer/gl/wgl/DeviceWGL.h"
#   else
#       error Unsupported OpenGL platform.
#   endif
#endif

namespace egl
{

Device::Device(rx::DeviceImpl *impl)
    : mImplementation(impl)
{
    ASSERT(mImplementation != nullptr);
}

Device::~Device()
{
    SafeDelete(mImplementation);
}

egl::Error Device::queryAttribute(EGLint attribute, EGLAttrib *value)
{
    return mImplementation->queryAttribute(attribute, value);
}

}
