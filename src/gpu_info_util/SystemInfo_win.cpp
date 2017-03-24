//
// Copyright (c) 2013-2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SystemInfo_linux.cpp: implementation of the Linux-specific parts of SystemInfo.h

#include "gpu_info_util/SystemInfo_internal.h"

// Windows.h needs to be included first
#include <windows.h>
#include <cfgmgr32.h>
#include <setupapi.h>

namespace angle
{

namespace
{

std::string GetRegistryStringValue(HKEY key, const char *valueName)
{
    char value[255];
    DWORD valueSize = sizeof(value);
    if (RegQueryValueExA(key, valueName, nullptr, nullptr, reinterpret_cast<LPBYTE>(value),
                         &valueSize) == ERROR_SUCCESS)
    {
        return value;
    }
    return "";
}

// Gathers information about the devices from the registry. The reason why we aren't using
// a dedicated API such as DXGI is that we need information like the driver vendor and date.
// DXGI doesn't provide a way to know the device registry key from an IDXGIAdapter.
bool GetDevicesFromRegistry(std::vector<GPUDeviceInfo> *devices)
{
    // Display adapter class GUID from
    // https://msdn.microsoft.com/en-us/library/windows/hardware/ff553426%28v=vs.85%29.aspx
    GUID displayClass = {
        0x4d36e968, 0xe325, 0x11ce, {0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18}};

    HDEVINFO deviceInfo = SetupDiGetClassDevsW(&displayClass, nullptr, nullptr, DIGCF_PRESENT);

    if (deviceInfo == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    // This iterates over the devices of the "Display adapter" class
    DWORD deviceIndex = 0;
    SP_DEVINFO_DATA deviceData;
    deviceData.cbSize = sizeof(deviceData);
    while (SetupDiEnumDeviceInfo(deviceInfo, deviceIndex++, &deviceData))
    {
        // The device and vendor IDs can be gathered directly, but information about the driver
        // requires some registry digging
        char fullDeviceID[MAX_DEVICE_ID_LEN];
        if (CM_Get_Device_ID(deviceData.DevInst, fullDeviceID, MAX_DEVICE_ID_LEN, 0) != CR_SUCCESS)
        {
            continue;
        }

        GPUDeviceInfo device;

        if (!CMDeviceIDToDeviceAndVendorID(fullDeviceID, &device.vendorId, &device.deviceId))
        {
            continue;
        }

        // The driver key will end with something like {<displayClass>}/<4 digit number>.
        WCHAR value[255];
        if (!SetupDiGetDeviceRegistryPropertyW(deviceInfo, &deviceData, SPDRP_DRIVER, nullptr,
                                               reinterpret_cast<PBYTE>(value), sizeof(value),
                                               nullptr))
        {
            continue;
        }

        std::wstring driverKey = L"System\\CurrentControlSet\\Control\\Class\\";
        driverKey += value;

        HKEY key;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, driverKey.c_str(), 0, KEY_QUERY_VALUE, &key) !=
            ERROR_SUCCESS)
        {
            continue;
        }

        device.driverVersion = GetRegistryStringValue(key, "DriverVersion");
        device.driverDate    = GetRegistryStringValue(key, "DriverDate");
        device.driverVendor  = GetRegistryStringValue(key, "ProviderName");

        RegCloseKey(key);

        devices->push_back(device);
    }

    return true;
}

}  // anonymous namespace

bool GetSystemInfo(SystemInfo *info)
{
    if (!GetDevicesFromRegistry(&info->gpus))
    {
        return false;
    }

    if (info->gpus.size() == 0)
    {
        return false;
    }

    FindPrimaryGPU(info);

    // nvd3d9wrap.dll is loaded into all processes when Optimus is enabled.
    HMODULE nvd3d9wrap = GetModuleHandleW(L"nvd3d9wrap.dll");
    info->isOptimus    = nvd3d9wrap != nullptr;

    return true;
}

}  // namespace angle