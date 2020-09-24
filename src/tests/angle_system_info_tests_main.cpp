//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Simple unit test suite that prints out system info.

#include "gpu_info_util/SystemInfo.h"

int main(int argc, char **argv)
{
    angle::SystemInfo info;

#if defined(ANGLE_ENABLE_VULKAN)
    angle::GetSystemInfoVulkan(&info);
#else
    angle::GetSystemInfo(&info);
#endif  // defined(ANGLE_ENABLE_VULKAN)

    if (info.gpus.empty())
    {
        printf("{}");
        return 1;
    }

    const angle::GPUDeviceInfo &gpu = info.gpus[0];

    printf("Vendor: %s\n", gpu.driverVendor.c_str());
    return 0;
}
