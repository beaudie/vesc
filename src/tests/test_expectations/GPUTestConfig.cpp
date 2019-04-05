
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "GPUTestConfig.h"

#include "common/angleutils.h"
#include "common/debug.h"
#include "common/platform.h"
#include "common/string_utils.h"
#include "gpu_info_util/SystemInfo.h"

#if defined(ANGLE_PLATFORM_APPLE)
#    include "GPUTestConfig_mac.h"
#endif

#include <iostream>

namespace angle
{

bool OperatingSystemVersionNumbers(int32_t *major_version, int32_t *minor_version);

bool IsWinVersion(const int32_t major_version);
bool IsWinVersion(const int32_t major_version, const int32_t minor_version);
bool IsWinXP();
bool IsWinVista();
bool IsWin7();
bool IsWin8();
bool IsWin10();
bool IsWin();

bool IsMacVersion(const int32_t major_version, const int32_t minor_version);
bool IsMacLeopard();
bool IsMacSnowLeopard();
bool IsMacLion();
bool IsMacMountainLion();
bool IsMacMavericks();
bool IsMacYosemite();
bool IsMacElCapitan();
bool IsMacSierra();
bool IsMacHighSierra();
bool IsMacMojave();
bool IsMac();

bool IsLinux();
bool IsAndroid();

SystemInfo *GetGPUTestSystemInfo();
GPUDeviceInfo GetActiveGPU();
VendorID GetActiveGPUVendorID();
DeviceID GetActiveGPUDeviceID();

bool IsNVIDIA();
bool IsAMD();
bool IsIntel();
bool IsVMWare();

bool IsRelease();
bool IsDebug();

bool IsAndroidDevice(const std::string &deviceName);
bool IsNexus5X();
bool IsPixel2();

bool IsNVIDIAQuadroP400();

bool IsDeviceIdGPU(const std::string &gpu_device_id);

bool IsD3D9();
bool IsD3D11();
bool IsGLDesktop();
bool IsGLES();
bool IsVulkan();

// Generic function call to get the OS version information from any platform
// defined below. This function will also cache the OS version info in static
// variables.
bool OperatingSystemVersionNumbers(int32_t *major_version, int32_t *minor_version)
{
    static int32_t saved_major_version = -1;
    static int32_t saved_minor_version = -1;
    bool ret                           = false;
    if (saved_major_version == -1 || saved_minor_version == -1)
    {
#if defined(ANGLE_PLATFORM_WINDOWS)
        OSVERSIONINFOEX version_info = {sizeof version_info};
        ::GetVersionEx(reinterpret_cast<OSVERSIONINFO *>(&version_info));
        saved_major_version = version_info.dwMajorVersion;
        *major_version      = saved_major_version;
        saved_minor_version = version_info.dwMinorVersion;
        *minor_version      = saved_minor_version;
        ret                 = true;

#elif defined(ANGLE_PLATFORM_APPLE)
        GetOperatingSystemVersionNumbers(&saved_major_version, &saved_minor_version);
        *major_version = saved_major_version;
        *minor_version = saved_minor_version;
        ret            = true;

#else
        ret = false;
#endif
    }
    else
    {
        ret = true;
    }
    *major_version = saved_major_version;
    *minor_version = saved_minor_version;
    return ret;
}

// Check if the OS is a specific major version of windows.
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

// Check if the OS is a specific major and minor version of windows.
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

// Check if the OS is Windows XP
bool IsWinXP()
{
    if (IsWinVersion(5))
    {
        return true;
    }
    return false;
}

// Check if the OS is Windows Vista
bool IsWinVista()
{
    if (IsWinVersion(6, 0))
    {
        return true;
    }
    return false;
}

// Check if the OS is Windows 7
bool IsWin7()
{
    if (IsWinVersion(6, 1))
    {
        return true;
    }
    return false;
}

// Check if the OS is Windows 8
bool IsWin8()
{
    if (IsWinVersion(6, 2) || IsWinVersion(6, 3))
    {
        return true;
    }
    return false;
}

// Check if the OS is Windows 10
bool IsWin10()
{
    if (IsWinVersion(10))
    {
        return true;
    }
    return false;
}

// Check if the OS is any version of Windows
bool IsWin()
{
#if defined(ANGLE_PLATFORM_WINDOWS)
    return true;
#else
    return false;
#endif
}

// Check if the OS is a specific major and minor version of OSX
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

// Check if the OS is OSX Leopard
bool IsMacLeopard()
{
    if (IsMacVersion(10, 5))
    {
        return true;
    }
    return false;
}

// Check if the OS is OSX Snow Leopard
bool IsMacSnowLeopard()
{
    if (IsMacVersion(10, 6))
    {
        return true;
    }
    return false;
}

// Check if the OS is OSX Lion
bool IsMacLion()
{
    if (IsMacVersion(10, 7))
    {
        return true;
    }
    return false;
}

// Check if the OS is OSX Mountain Lion
bool IsMacMountainLion()
{
    if (IsMacVersion(10, 8))
    {
        return true;
    }
    return false;
}

// Check if the OS is OSX Mavericks
bool IsMacMavericks()
{
    if (IsMacVersion(10, 9))
    {
        return true;
    }
    return false;
}

// Check if the OS is OSX Yosemite
bool IsMacYosemite()
{
    if (IsMacVersion(10, 10))
    {
        return true;
    }
    return false;
}

// Check if the OS is OSX El Capitan
bool IsMacElCapitan()
{
    if (IsMacVersion(10, 11))
    {
        return true;
    }
    return false;
}

// Check if the OS is OSX Sierra
bool IsMacSierra()
{
    if (IsMacVersion(10, 12))
    {
        return true;
    }
    return false;
}

// Check if the OS is OSX High Sierra
bool IsMacHighSierra()
{
    if (IsMacVersion(10, 13))
    {
        return true;
    }
    return false;
}

// Check if the OS is OSX Mojave
bool IsMacMojave()
{
    if (IsMacVersion(10, 14))
    {
        return true;
    }
    return false;
}

// Check if the OS is any version of OSX
bool IsMac()
{
#if defined(ANGLE_PLATFORM_APPLE)
    return true;
#else
    return false;
#endif
}

// Check if the OS is any version of Linux
bool IsLinux()
{
#if defined(ANGLE_PLATFORM_LINUX)
    return true;
#else
    return false;
#endif
}

// Check if the OS is any version of Android
bool IsAndroid()
{
#if defined(ANGLE_PLATFORM_ANDROID)
    return true;
#else
    return false;
#endif
}

// Generic function call to populate the SystemInfo struct. This function will
// also cache the SystemInfo struct for future calls.
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

// Get the active GPUDeviceInfo from the SystemInfo struct.
GPUDeviceInfo GetActiveGPU()
{
    SystemInfo *systemInfo = GetGPUTestSystemInfo();
    uint32_t index         = 0;
    // See if the activeGPUIndex was set first
    if (systemInfo->activeGPUIndex != -1)
    {
        index = systemInfo->activeGPUIndex;
    }
    // Else fallback to the primaryGPUIndex
    else if (systemInfo->primaryGPUIndex != -1)
    {
        index = systemInfo->primaryGPUIndex;
    }
    ASSERT(index < systemInfo->gpus.size());
    return systemInfo->gpus[index];
}

// Get the vendor ID of the active GPU from the SystemInfo struct.
VendorID GetActiveGPUVendorID()
{
    GPUDeviceInfo activeGPU = GetActiveGPU();
    return activeGPU.vendorId;
}

// Get the device ID of the active GPU from the SystemInfo struct.
DeviceID GetActiveGPUDeviceID()
{
    GPUDeviceInfo activeGPU = GetActiveGPU();
    return activeGPU.deviceId;
}

// Check whether the active GPU is NVIDIA.
bool IsNVIDIA()
{
    return IsNVIDIA(GetActiveGPUVendorID());
}

// Check whether the active GPU is AMD.
bool IsAMD()
{
    return IsAMD(GetActiveGPUVendorID());
}

// Check whether the active GPU is Intel.
bool IsIntel()
{
    return IsIntel(GetActiveGPUVendorID());
}

// Check whether the active GPU is VMWare.
bool IsVMWare()
{
    return IsVMWare(GetActiveGPUVendorID());
}

// Check whether this is a debug build.
bool IsDebug()
{
#if !defined(NDEBUG)
    return true;
#else
    return false;
#endif
}

// Check whether this is a release build.
bool IsRelease()
{
    return !IsDebug();
}

// Check whether the system is a specific Android device based on the name.
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

// Check whether the system is a Nexus 5X device.
bool IsNexus5X()
{
    return IsAndroidDevice("Nexus 5X");
}

// Check whether the system is a Pixel 2 device.
bool IsPixel2()
{
    return IsAndroidDevice("Pixel 2");
}

// Check whether the active GPU is a NVIDIA Quadro P400
bool IsNVIDIAQuadroP400()
{
    if (!IsNVIDIA())
    {
        return false;
    }
    return IsDeviceIdGPU("0x1CB3");
}

// Check whether the active GPU is a specific device based on the string device ID.
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

// Check whether the backend API has been set to D3D9 in the constructor
bool IsD3D9(const GPUTestConfig::API &api)
{
    return (api == GPUTestConfig::kAPID3D9);
}

// Check whether the backend API has been set to D3D11 in the constructor
bool IsD3D11(const GPUTestConfig::API &api)
{
    return (api == GPUTestConfig::kAPID3D11);
}

// Check whether the backend API has been set to OpenGL in the constructor
bool IsGLDesktop(const GPUTestConfig::API &api)
{
    return (api == GPUTestConfig::kAPIGLDesktop);
}

// Check whether the backend API has been set to OpenGLES in the constructor
bool IsGLES(const GPUTestConfig::API &api)
{
    return (api == GPUTestConfig::kAPIGLES);
}

// Check whether the backend API has been set to Vulkan in the constructor
bool IsVulkan(const GPUTestConfig::API &api)
{
    return (api == GPUTestConfig::kAPIVulkan);
}

// Load all conditions in the constructor since this data will not change during a test set.
GPUTestConfig::GPUTestConfig()
{
    conditions_[kConditionNone]            = false;
    conditions_[kConditionWinXP]           = IsWinXP();
    conditions_[kConditionWinVista]        = IsWinVista();
    conditions_[kConditionWin7]            = IsWin7();
    conditions_[kConditionWin8]            = IsWin8();
    conditions_[kConditionWin10]           = IsWin10();
    conditions_[kConditionWin]             = IsWin();
    conditions_[kConditionMacLeopard]      = IsMacLeopard();
    conditions_[kConditionMacSnowLeopard]  = IsMacSnowLeopard();
    conditions_[kConditionMacLion]         = IsMacLion();
    conditions_[kConditionMacMountainLion] = IsMacMountainLion();
    conditions_[kConditionMacMavericks]    = IsMacMavericks();
    conditions_[kConditionMacYosemite]     = IsMacYosemite();
    conditions_[kConditionMacElCapitan]    = IsMacElCapitan();
    conditions_[kConditionMacSierra]       = IsMacSierra();
    conditions_[kConditionMacHighSierra]   = IsMacHighSierra();
    conditions_[kConditionMacMojave]       = IsMacMojave();
    conditions_[kConditionMac]             = IsMac();
    conditions_[kConditionLinux]           = IsLinux();
    conditions_[kConditionAndroid]         = IsAndroid();
    conditions_[kConditionNVIDIA]          = IsNVIDIA();
    conditions_[kConditionAMD]             = IsAMD();
    conditions_[kConditionIntel]           = IsIntel();
    conditions_[kConditionVMWare]          = IsVMWare();
    conditions_[kConditionRelease]         = IsRelease();
    conditions_[kConditionDebug]           = IsDebug();
    // If no API provided, pass these conditions by default
    conditions_[kConditionD3D9]      = true;
    conditions_[kConditionD3D11]     = true;
    conditions_[kConditionGLDesktop] = true;
    conditions_[kConditionGLES]      = true;
    conditions_[kConditionVulkan]    = true;

    conditions_[kConditionNexus5X]          = IsNexus5X();
    conditions_[kConditionPixel2]           = IsPixel2();
    conditions_[kConditionNVIDIAQuadroP400] = IsNVIDIAQuadroP400();
}

// If the constructor is passed an API, load those conditions as well
GPUTestConfig::GPUTestConfig(const API &api) : GPUTestConfig()
{
    conditions_[kConditionD3D9]      = IsD3D9(api);
    conditions_[kConditionD3D11]     = IsD3D11(api);
    conditions_[kConditionGLDesktop] = IsGLDesktop(api);
    conditions_[kConditionGLES]      = IsGLES(api);
    conditions_[kConditionVulkan]    = IsVulkan(api);
}

// Return a const reference to the list of all pre-calculated conditions.
const std::array<bool, GPUTestConfig::kNumberOfConditions> &GPUTestConfig::GetConditions() const
{
    return conditions_;
}

}  // namespace angle
