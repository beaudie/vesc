//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SystemInfo_libdrm.cpp: implementation of the libdrm-specific parts of SystemInfo.h

#include "gpu_info_util/SystemInfo_internal.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <xf86drm.h>
#include <string>
#include <vector>

#if !defined(GPU_INFO_USE_LIBDRM)
#    error SystemInfo_libdrm.cpp compiled without GPU_INFO_USE_LIBDRM
#endif

namespace angle
{

// Adds an entry per GPU found and fills the device, vendor and system device ID.
bool GetDevicesWithLibDRM(std::vector<GPUDeviceInfo> *devices)
{
    int numDevices = drmGetDevices2(DRM_DEVICE_GET_PCI_REVISION, nullptr, 0);
    if (numDevices <= 0)
        return false;

    std::vector<drmDevicePtr> drmDevices(numDevices);
    if (drmGetDevices2(DRM_DEVICE_GET_PCI_REVISION, drmDevices.data(), numDevices) != numDevices)
        return false;

    for (drmDevicePtr drmDevice : drmDevices)
    {
        if (!(drmDevice->available_nodes & (1 << DRM_NODE_RENDER)))
            continue;
        const char *render_node = drmDevice->nodes[DRM_NODE_RENDER];

        int fd = open(render_node, O_RDWR | O_CLOEXEC);
        if (fd < 0)
            continue;

        struct stat sb;
        if (fstat(fd, &sb) < 0)
            continue;

        drmVersionPtr version = drmGetVersion(fd);
        std::string driverName(version->name, version->name_len);
        drmFreeVersion(version);

        close(fd);

        // skip the virtual GEM kernel driver
        if (driverName == "vgem")
            continue;

        GPUDeviceInfo info;
        info.systemDeviceId = sb.st_rdev;
        if (drmDevice->bustype == DRM_BUS_PCI)
        {
            info.vendorId   = drmDevice->deviceinfo.pci->vendor_id;
            info.deviceId   = drmDevice->deviceinfo.pci->device_id;
            info.revisionId = drmDevice->deviceinfo.pci->revision_id;
        }

        devices->push_back(info);
    }

    drmFreeDevices(drmDevices.data(), numDevices);

    return !devices->empty();
}

}  // namespace angle
