//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Simple unit test suite that prints out system info.

#include "gpu_info_util/SystemInfo.h"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

namespace js = rapidjson;

int main(int argc, char **argv)
{
    angle::SystemInfo info;

    if (argc > 1 && strcmp(argv[1], "--vulkan") == 0)
    {
#if defined(ANGLE_ENABLE_VULKAN)
        angle::GetSystemInfoVulkan(&info);
#else
        printf("Vulkan not supported.\n");
        return 1;
#endif  // defined(ANGLE_ENABLE_VULKAN)
    }
    else
    {
        angle::GetSystemInfo(&info);
    }

    if (info.gpus.empty())
    {
        printf("{}");
        return 1;
    }

    js::Document doc;
    doc.SetObject();

    js::Document::AllocatorType &allocator = doc.GetAllocator();

    doc.AddMember("activeGPUIndex", info.activeGPUIndex, allocator);
    doc.AddMember("isOptimus", info.isOptimus, allocator);
    doc.AddMember("isMacSwitchable", info.isMacSwitchable, allocator);

    js::Value machineManufacturer;
    machineManufacturer.SetString(info.machineManufacturer.c_str(), allocator);
    doc.AddMember("machineManufacturer", machineManufacturer, allocator);

    js::Value machineModelVersion;
    machineModelVersion.SetString(info.machineModelVersion.c_str(), allocator);
    doc.AddMember("machineModelVersion", machineModelVersion, allocator);

    js::Value gpus;
    gpus.SetArray();

    for (const angle::GPUDeviceInfo &gpu : info.gpus)
    {
        js::Value obj;
        obj.SetObject();

        obj.AddMember("vendorId", gpu.vendorId, allocator);
        obj.AddMember("deviceId", gpu.deviceId, allocator);

        js::Value driverVendor;
        driverVendor.SetString(gpu.driverVendor.c_str(), allocator);
        obj.AddMember("driverVendor", driverVendor, allocator);

        js::Value driverVersion;
        driverVersion.SetString(gpu.driverVersion.c_str(), allocator);
        obj.AddMember("driverVersion", driverVersion, allocator);

        js::Value driverDate;
        driverDate.SetString(gpu.driverDate.c_str(), allocator);
        obj.AddMember("driverDate", driverDate, allocator);

        js::Value versionInfo;
        versionInfo.SetObject();
        versionInfo.AddMember("major", gpu.detailedDriverVersion.major, allocator);
        versionInfo.AddMember("minor", gpu.detailedDriverVersion.minor, allocator);
        versionInfo.AddMember("subMinor", gpu.detailedDriverVersion.subMinor, allocator);
        versionInfo.AddMember("patch", gpu.detailedDriverVersion.patch, allocator);
        obj.AddMember("detailedDriverVersion", versionInfo, allocator);

        gpus.PushBack(obj, allocator);
    }

    doc.AddMember("gpus", gpus, allocator);

    js::StringBuffer buffer;
    js::PrettyWriter<js::StringBuffer> writer(buffer);
    doc.Accept(writer);

    const char *output = buffer.GetString();
    printf("%s", output);

    return 0;
}
