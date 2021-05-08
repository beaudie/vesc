//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLPlatform.cpp: Implements the cl::Platform class.

#include "libANGLE/CLPlatform.h"

namespace cl
{

Platform::~Platform() = default;

void Platform::CreatePlatform(const cl_icd_dispatch &dispatch, rx::CLPlatformImpl::Ptr &&impl)
{
    rx::CLDeviceImpl::ImplList deviceImplList = impl->getDevices();
    if (!deviceImplList.empty())
    {
        GetList().emplace_back(new Platform(dispatch, std::move(impl), std::move(deviceImplList)));
    }
}

cl_int Platform::getDeviceIDs(cl_device_type deviceType,
                              cl_uint numEntries,
                              Device **devices,
                              cl_uint *numDevices) const
{
    cl_uint found = 0u;
    for (const Device::Ptr &device : mDevices)
    {
        cl_device_type type = 0u;
        if (device->getInfoULong(DeviceInfo::Type, &type) == CL_SUCCESS &&
            (type == CL_DEVICE_TYPE_CUSTOM ? deviceType == CL_DEVICE_TYPE_CUSTOM
                                           : (type & deviceType) != 0u))
        {
            if (devices != nullptr && found < numEntries)
            {
                devices[found] = device.get();
            }
            ++found;
        }
    }
    if (numDevices != nullptr)
    {
        *numDevices = found;
    }
    return found == 0u ? CL_DEVICE_NOT_FOUND : CL_SUCCESS;
}

Platform::Platform(const cl_icd_dispatch &dispatch,
                   rx::CLPlatformImpl::Ptr &&impl,
                   rx::CLDeviceImpl::ImplList &&deviceImplList)
    : _cl_platform_id(dispatch),
      mImpl(std::move(impl)),
      mDevices(Device::CreateDevices(*this, std::move(deviceImplList)))
{}

constexpr char Platform::kVendor[];
constexpr char Platform::kIcdSuffix[];

}  // namespace cl
