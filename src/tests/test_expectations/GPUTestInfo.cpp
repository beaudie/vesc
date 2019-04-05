
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "GPUTestInfo.h"

#include "common/platform.h"
#include "common/string_utils.h"
#include "gpu_info_util/SystemInfo.h"

#if defined(ANGLE_PLATFORM_APPLE)
#    include "GPUTestInfo_mac.h"
#endif

#include <iostream>

namespace angle
{

bool OperatingSystemVersionNumbers(int32_t *major_version, int32_t *minor_version)
{
#if defined(ANGLE_PLATFORM_WINDOWS)
    OSVERSIONINFOEX version_info = {sizeof version_info};
    ::GetVersionEx(reinterpret_cast<OSVERSIONINFO *>(&version_info));
    *major_version = version_info.dwMajorVersion;
    *minor_version = version_info.dwMinorVersion;
    return true;

#elif defined(ANGLE_PLATFORM_APPLE)
    GetOperatingSystemVersionNumbers(&major_version, &minor_version);
    return true;

#else
    return false;
#endif
}

bool IsWinVersion(const int32_t major_version)
{
    if (IsWin())
    {
        int32_t current_major_version = 0;
        int32_t current_minor_version = 0;
        if (OperatingSystemVersionNumbers(&current_major_version, &current_minor_version))
        {
            if (current_major_version == major_version)
            {
                return true;
            }
        }
    }
    return false;
}

bool IsWinVersion(const int32_t major_version, const int32_t minor_version)
{
    if (IsWin())
    {
        int32_t current_major_version = 0;
        int32_t current_minor_version = 0;
        if (OperatingSystemVersionNumbers(&current_major_version, &current_minor_version))
        {
            if (current_major_version == major_version && current_minor_version == minor_version)
            {
                return true;
            }
        }
    }
    return false;
}

bool IsWinXP()
{
    if (IsWinVersion(5))
    {
        return true;
    }
    return false;
}

bool IsWinVista()
{
    if (IsWinVersion(6, 0))
    {
        return true;
    }
    return false;
}

bool IsWin7()
{
    if (IsWinVersion(6, 1))
    {
        return true;
    }
    return false;
}

bool IsWin8()
{
    if (IsWinVersion(6, 2) || IsWinVersion(6, 3))
    {
        return true;
    }
    return false;
}

bool IsWin10()
{
    if (IsWinVersion(10))
    {
        return true;
    }
    return false;
}

bool IsWin()
{
#if defined(ANGLE_PLATFORM_WINDOWS)
    return true;
#else
    return false;
#endif
}

bool IsMacVersion(const int32_t major_version, const int32_t minor_version)
{
    if (IsMac())
    {
        int32_t current_major_version = 0;
        int32_t current_minor_version = 0;
        if (OperatingSystemVersionNumbers(&current_major_version, &current_minor_version))
        {
            if (current_major_version == major_version && current_minor_version == minor_version)
            {
                return true;
            }
        }
    }
    return false;
}

bool IsMacLeopard()
{
    if (IsMacVersion(10, 5))
    {
        return true;
    }
    return false;
}

bool IsMacSnowLeopard()
{
    if (IsMacVersion(10, 6))
    {
        return true;
    }
    return false;
}

bool IsMacLion()
{
    if (IsMacVersion(10, 7))
    {
        return true;
    }
    return false;
}

bool IsMacMountainLion()
{
    if (IsMacVersion(10, 8))
    {
        return true;
    }
    return false;
}

bool IsMacMavericks()
{
    if (IsMacVersion(10, 9))
    {
        return true;
    }
    return false;
}

bool IsMacYosemite()
{
    if (IsMacVersion(10, 10))
    {
        return true;
    }
    return false;
}

bool IsMacElCapitan()
{
    if (IsMacVersion(10, 11))
    {
        return true;
    }
    return false;
}

bool IsMacSierra()
{
    if (IsMacVersion(10, 12))
    {
        return true;
    }
    return false;
}

bool IsMacHighSierra()
{
    if (IsMacVersion(10, 13))
    {
        return true;
    }
    return false;
}

bool IsMacMojave()
{
    if (IsMacVersion(10, 14))
    {
        return true;
    }
    return false;
}

bool IsMac()
{
#if defined(ANGLE_PLATFORM_APPLE)
    return true;
#else
    return false;
#endif
}

bool IsLinux()
{
#if defined(ANGLE_PLATFORM_LINUX)
    return true;
#else
    return false;
#endif
}

bool IsAndroid()
{
#if defined(ANGLE_PLATFORM_ANDROID)
    return true;
#else
    return false;
#endif
}

SystemInfo *GetGPUTestSystemInfo()
{
    static SystemInfo *sSystemInfo = nullptr;
    if (sSystemInfo == nullptr)
    {
        sSystemInfo = new SystemInfo;
        if (!GetSystemInfo(sSystemInfo))
        {
            std::cout << "Error populating SystemInfo for dEQP tests.";
        }
    }
    return sSystemInfo;
}

GPUDeviceInfo GetActiveGPU()
{
    SystemInfo *systemInfo = GetGPUTestSystemInfo();
    uint32_t index         = 0;
    if (systemInfo->activeGPUIndex != -1)
    {
        index = systemInfo->activeGPUIndex;
    }
    else if (systemInfo->primaryGPUIndex != -1)
    {
        index = systemInfo->primaryGPUIndex;
    }
    if (index < systemInfo->gpus.size())
    {
        return systemInfo->gpus[index];
    }
    GPUDeviceInfo emptyDevice;
    return emptyDevice;
}

VendorID GetActiveGPUVendorID()
{
    GPUDeviceInfo activeGPU = GetActiveGPU();
    return activeGPU.vendorId;
}

DeviceID GetActiveGPUDeviceID()
{
    GPUDeviceInfo activeGPU = GetActiveGPU();
    return activeGPU.deviceId;
}

bool IsNVidia()
{
    return IsNVIDIA(GetActiveGPUVendorID());
}

bool IsAMD()
{
    return IsAMD(GetActiveGPUVendorID());
}

bool IsIntel()
{
    return IsIntel(GetActiveGPUVendorID());
}

bool IsVMWare()
{
    return IsVMWare(GetActiveGPUVendorID());
}

bool IsDebug()
{
#if !defined(NDEBUG)
    return true;
#else
    return false;
#endif
}

bool IsRelease()
{
    return !IsDebug();
}

bool IsAndroidDevice(const std::string &deviceName)
{
    if (!IsAndroid())
    {
        return false;
    }
    SystemInfo *systemInfo = GetGPUTestSystemInfo();
    if (systemInfo->machineModelName == deviceName)
    {
        return true;
    }
    return false;
}

bool IsNexus5X()
{
    return IsAndroidDevice("Nexus 5X");
}

bool IsPixel2()
{
    return IsAndroidDevice("Pixel 2");
}

bool IsNVidiaQuadroP400()
{
    if (!IsNVidia())
    {
        return false;
    }
    return IsDeviceIdGPU("0x1CB3");
}

bool IsDeviceIdGPU(const std::string &gpu_device_id)
{
    uint32_t device_id = 0;
    if (!HexStringToUInt(gpu_device_id, &device_id) || device_id == 0)
    {
        // PushErrorMessage(kErrorMessage[kErrorEntryWithGpuDeviceIdConflicts], line_number);
        return false;
    }
    return (device_id == GetActiveGPUDeviceID());
}

static API api = kAPIUnknown;

bool LoadCurrentConfig(API a)
{
    api = a;
    return true;
}

bool IsD3D9()
{
    return (api == kAPID3D9);
}

bool IsD3D11()
{
    return (api == kAPID3D11);
}

bool IsGLDesktop()
{
    return (api == kAPIGLDesktop);
}

bool IsGLES()
{
    return (api == kAPIGLES);
}

bool IsVulkan()
{
    return (api == kAPIVulkan);
}

}  // namespace angle
