//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SystemInfo_android.cpp: implementation of the Android-specific parts of SystemInfo.h

#include "gpu_info_util/SystemInfo_internal.h"

#include <cstring>
#include <fstream>

#include "common/angleutils.h"
#include "common/debug.h"

namespace angle
{
struct GetSystemInfoCleanup
{
    GetSystemInfoCleanup();
    ~GetSystemInfoCleanup()
    {
        if (instance != VK_NULL_HANDLE)
        {
            PFN_vkDestroyInstance pfnDestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(
                dlsym(cleanup.libvulkan, "vkDestroyInstance"));
            if (pfnDestroyInstance)
            {
                pfnDestroyInstance(instance, nullptr);
            }
        }
        if (libvulkan)
            dlclose(libvulkan);
    }

    void *libvulkan     = nullptr;
    VkInstance instance = VK_NULL_HANDLE;
};

bool GetSystemInfo(SystemInfo *info)
{
    GetSystemInfoCleanup cleanup;

    // This implementation builds on top of the Vulkan API, but cannot
    // assume the existence of the Vulkan library.  ANGLE can be
    // installed on versions of Android as old as Ice Cream Sandwich.
    // Therefore, we need to use dlopen()/dlsym() in order to see if
    // Vulkan is installed on the system, and if so, to use it:

    // Find the system's Vulkan library and open it:
    cleanup.libvulkan = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    if (!cleanup.libvulkan)
    {
        // If Vulkan doesn't exist, bail-out early:
        return false;
    }

    // Determine the available Vulkan instance version:
    uint32_t instanceVersion = VK_API_VERSION_1_0;
#if defined(VK_VERSION_1_1)
    PFN_vkEnumerateInstanceVersion pfnEnumerateInstanceVersion =
        reinterpret_cast<PFN_vkCreateInstance>(
            dlsym(cleanup.libvulkan, "vkEnumerateInstanceVersion"));
    if (!pfnEnumerateInstanceVersion || pfnEnumerateInstanceVersion(&instanceVersion) != VK_SUCCESS)
    {
        instanceVersion = VK_API_VERSION_1_0;
    }
#endif  // VK_VERSION_1_1

    // Create a Vulkan instance:
    VkApplicationInfo appInfo;
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext              = nullptr;
    appInfo.pApplicationName   = "";
    appInfo.applicationVersion = 1;
    appInfo.pEngineName        = "";
    appInfo.engineVersion      = 1;
    appInfo.apiVersion         = instanceVersion;

    VkInstanceCreateInfo createInstanceInfo;
    createInstanceInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInstanceInfo.pNext                   = nullptr;
    createInstanceInfo.flags                   = 0;
    createInstanceInfo.pApplicationInfo        = &appInfo;
    createInstanceInfo.enabledLayerCount       = 0;
    createInstanceInfo.ppEnabledLayerNames     = nullptr;
    createInstanceInfo.enabledExtensionCount   = 0;
    createInstanceInfo.ppEnabledExtensionNames = nullptr;

    PFN_vkCreateInstance pfnCreateInstance =
        reinterpret_cast<PFN_vkCreateInstance>(dlsym(cleanup.libvulkan, "vkCreateInstance"));
    if (!pfnCreateInstance ||
        pfnCreateInstance(&createInstanceInfo, nullptr, &instance) != VK_SUCCESS)
    {
        return false;
    }
    PFN_vkEnumeratePhysicalDevices pfnEnumeratePhysicalDevices =
        reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(
            dlsym(cleanup.libvulkan, "vkEnumeratePhysicalDevices"));
    PFN_vkGetPhysicalDeviceProperties pfnGetPhysicalDeviceProperties =
        reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(
            dlsym(cleanup.libvulkan, "vkGetPhysicalDeviceProperties"));
    uint32_t physicalDeviceCount       = 0;
    VkPhysicalDevice *pPhysicalDevices = nullptr;

    if (!pfnEnumeratePhysicalDevices ||
        pfnEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr) != VK_SUCCESS)
    {
        return false;
    }
    std::vector<VkPhysicalDevice *> physicalDevices;
    physicalDevices.resize(physicalDeviceCount);
    info->gpus.resize(physicalDeviceCount);
    if (pfnEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()) !=
        VK_SUCCESS)
    {
        return false;
    }

    for (int i = 0; i < physicalDeviceCount; i++)
    {
        VkPhysicalDeviceProperties properties;
        pfnGetPhysicalDeviceProperties(pPhysicalDevices[i], &properties);
        // Fill in data for a given physical device (a.k.a. gpu):
        info->gpus[i].vendorId = properties.vendorID;
        info->gpus[i].deviceId = properties.deviceID;
        // Need to parse/re-format properties.driverVersion.
        //
        // TODO(ianelliott): Determine the formatting used for each
        // vendor.
        constexpr std::size_t driverVersionSize = 20;
        char driverVersion[driverVersionSize];
        switch (properties.vendorID)
        {
            case kVendorID_AMD:
                info->gpus[i].driverVendor = "Advanced Micro Devices, Inc";
                snprintf(driverVersion, driverVersionSize, "%d", properties.driverVersion);
                info->gpus[i].driverVersion = driverVersion;
                break;
            case kVendorID_ARM:
                info->gpus[i].driverVendor = "Arm Holdings";
                snprintf(driverVersion, driverVersionSize, "%d", properties.driverVersion);
                info->gpus[i].driverVersion = driverVersion;
                break;
            case kVendorID_ImgTec:
                info->gpus[i].driverVendor = "Imagination Technologies Limited";
                snprintf(driverVersion, driverVersionSize, "%d", properties.driverVersion);
                info->gpus[i].driverVersion = driverVersion;
                break;
            case kVendorID_Intel:
                info->gpus[i].driverVendor = "Intel Corporation";
                snprintf(driverVersion, driverVersionSize, "%d", properties.driverVersion);
                info->gpus[i].driverVersion = driverVersion;
                break;
            case kVendorID_Nvidia:
                info->gpus[i].driverVendor = "NVIDIA Corporation";
                snprintf(driverVersion, driverVersionSize, "%d", properties.driverVersion);
                info->gpus[i].driverVersion = driverVersion;
                break;
            case kVendorID_Qualcomm:
                info->gpus[i].driverVendor = "Qualcomm Technologies, Inc";
                if (properties.driverVersion & 0x80000000)
                {
                    snprintf(driverVersion, driverVersionSize, "%d.%d.%d",
                             properties.driverVersion >> 22,
                             (properties.driverVersion >> 12) & 0X3FF,
                             properties.driverVersion & 0xFFF);
                }
                else
                {
                    snprintf(driverVersion, driverVersionSize, "0x%x", properties.driverVersion);
                }
                info->gpus[i].driverVersion = driverVersion;
                break;
            case kVendorID_Vivante:
                info->gpus[i].driverVendor = "Vivante";
                snprintf(driverVersion, driverVersionSize, "%d", properties.driverVersion);
                info->gpus[i].driverVersion = driverVersion;
                break;
            case kVendorID_VeriSilicon:
                info->gpus[i].driverVendor = "VeriSilicon";
                snprintf(driverVersion, driverVersionSize, "%d", properties.driverVersion);
                info->gpus[i].driverVersion = driverVersion;
                break;
            case kVendorID_Kazan:
                info->gpus[i].driverVendor = "Kazan Software";
                snprintf(driverVersion, driverVersionSize, "%d", properties.driverVersion);
                info->gpus[i].driverVersion = driverVersion;
                break;
            default:
                return false;
        }
        info->gpus[i].driverDate = "";
    }

    return true;
}

}  // namespace angle
