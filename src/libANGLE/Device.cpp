//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Device.cpp: Implements the egl::Device class, representing the abstract
// device. Implements EGLDevice.

#include "libANGLE/Device.h"

#include <iterator>

#include <platform/Platform.h>
#include <EGL/eglext.h>

#include "common/debug.h"
#include "common/platform.h"
#include "libANGLE/renderer/DeviceImpl.h"

#if defined(ANGLE_ENABLE_D3D11)
#include "libANGLE/renderer/D3D/DeviceD3D.h"
#endif

namespace egl
{

template <typename T>
static std::string GenerateExtensionsString(const T &extensions)
{
    std::vector<std::string> extensionsVector = extensions.getStrings();

    std::ostringstream stream;
    std::copy(extensionsVector.begin(), extensionsVector.end(), std::ostream_iterator<std::string>(stream, " "));
    return stream.str();
}

// Static factory method
egl::Error Device::CreateDevice(EGLAttrib devicePointer, EGLint deviceType, Device **outDevice)
{
#if defined(ANGLE_ENABLE_D3D11)
    if (deviceType == EGL_D3D11_DEVICE_ANGLE)
    {
        rx::DeviceD3D *deviceD3D = new rx::DeviceD3D();
        egl::Error error = deviceD3D->initialize(devicePointer, deviceType, EGL_TRUE);
        if (error.isError())
        {
            *outDevice = nullptr;
            return error;
        }

        *outDevice = new Device(deviceD3D);
        return egl::Error(EGL_SUCCESS);
    }
#endif

    // Note that creating an EGL device from inputted D3D9 parameters isn't currently supported
    *outDevice = nullptr;
    return egl::Error(EGL_BAD_ATTRIBUTE);
}

Device::Device(Display *owningDisplay, rx::DeviceImpl *impl)
    : mOwningDisplay(owningDisplay),
      mImplementation(impl)
{
    initDeviceExtensions();
}

Device::Device(rx::DeviceImpl *impl)
    : mOwningDisplay(nullptr),
      mImplementation(impl)
{
    initDeviceExtensions();
}

Device::~Device()
{
    if (mImplementation != nullptr)
    {
        SafeDelete(mImplementation);
        mImplementation = nullptr;
    }
}

Error Device::getDevice(EGLAttrib *value)
{
    return getImplementation()->getDevice(value);
}

EGLint Device::getType()
{
    return getImplementation()->getType();
}

void Device::initDeviceExtensions()
{
    mImplementation->generateExtensions(&mDeviceExtensions);
    mDeviceExtensionString = GenerateExtensionsString(mDeviceExtensions);
}

const DeviceExtensions &Device::getExtensions() const
{
    return mDeviceExtensions;
}

const std::string &Device::getExtensionString() const
{
    return mDeviceExtensionString;
}

}
