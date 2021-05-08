//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLDeviceImpl.h: Defines the abstract rx::CLDeviceImpl class.

#ifndef LIBANGLE_RENDERER_CLDEVICEIMPL_H_
#define LIBANGLE_RENDERER_CLDEVICEIMPL_H_

#include "libANGLE/renderer/CLtypes.h"

#include <string>

namespace rx
{

class CLDeviceImpl : angle::NonCopyable
{
  public:
    struct Info
    {
        Info();
        ~Info();

        Info(const Info &) = delete;
        Info &operator=(const Info &) = delete;

        Info(Info &&);
        Info &operator=(Info &&);

        bool isValid() const;

        std::vector<size_t> mMaxWorkItemSizes;
        NameVersionArray mILsWithVersion;
        NameVersionArray mBuiltInKernelsWithVersion;
        NameVersionArray mOpenCL_C_AllVersions;
        NameVersionArray mOpenCL_C_Features;
        std::string mExtensions;
        NameVersionArray mExtensionsWithVersion;
        std::vector<cl_device_partition_property> mPartitionProperties;
        std::vector<cl_device_partition_property> mPartitionType;

        bool mIsSupportedILsWithVersion            = false;
        bool mIsSupportedBuiltInKernelsWithVersion = false;
        bool mIsSupportedOpenCL_C_AllVersions      = false;
        bool mIsSupportedOpenCL_C_Features         = false;
        bool mIsSupportedExtensionsWithVersion     = false;
    };

    using Ptr      = std::unique_ptr<CLDeviceImpl>;
    using ImplList = std::list<std::pair<Ptr, Info>>;

    CLDeviceImpl()          = default;
    virtual ~CLDeviceImpl() = default;

    virtual cl_int getInfoUInt(cl::DeviceInfo name, cl_uint *value) const             = 0;
    virtual cl_int getInfoULong(cl::DeviceInfo name, cl_ulong *value) const           = 0;
    virtual cl_int getInfoSizeT(cl::DeviceInfo name, size_t *value) const             = 0;
    virtual cl_int getInfoStringLength(cl::DeviceInfo name, size_t *value) const      = 0;
    virtual cl_int getInfoString(cl::DeviceInfo name, size_t size, char *value) const = 0;

    virtual cl_int createSubDevices(const cl_device_partition_property *properties,
                                    cl_uint numDevices,
                                    ImplList &deviceImplList,
                                    cl_uint *numDevicesRet) = 0;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_CLDEVICEIMPL_H_
