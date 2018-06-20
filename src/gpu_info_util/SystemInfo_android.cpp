//
// Copyright (c) 2013-2018 The ANGLE Project Authors. All rights reserved.
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
bool GetSystemInfo(SystemInfo *info)
{
    // This implementation builds on top of the Vulkan API, but cannot
    // assume the existence of the Vulkan library.  ANGLE can be
    // installed on versions of Android as old as Ice Cream Sandwhich.
    // Therefore, we need to use dlopen()/dlsym() in order to see if
    // Vulkan is installed on the system, and if so, to use it:

    // Find the system's Vulkan library and open it:
    void *libvulkan = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    if (!libvulkan)
    {
        // If Vulkan doesn't exist, bail-out early:
        return false;
    }

    // Determine the available Vulkan instance version:
    uint32_t instanceVersion;
#ifdef VK_VERSION_1_1
    PFN_vkEnumerateInstanceVersion pfnEnumerateInstanceVersion =
        reinterpret_cast<PFN_vkCreateInstance>(dlsym(libvulkan, "vkEnumerateInstanceVersion"));
    if (!pfnEnumerateInstanceVersion || pfnEnumerateInstanceVersion(&instanceVersion) != VK_SUCCESS)
    {
        instanceVersion = VK_API_VERSION_1_0;
    }
#else   // VK_VERSION_1_1
    instanceVersion = VK_API_VERSION_1_0;
#endif  // VK_VERSION_1_1

    // Create a Vulkan instance:
    VkApplicationInfo appInfo;
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext              = NULL;
    appInfo.pApplicationName   = "";
    appInfo.applicationVersion = 1;
    appInfo.pEngineName        = "";
    appInfo.engineVersion      = 1;
    appInfo.apiVersion         = instanceVersion;

    VkInstanceCreateInfo createInstanceInfo;
    createInstanceInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInstanceInfo.pNext                   = NULL;
    createInstanceInfo.flags                   = 0;
    createInstanceInfo.pApplicationInfo        = &appInfo;
    createInstanceInfo.enabledLayerCount       = 0;
    createInstanceInfo.ppEnabledLayerNames     = NULL;
    createInstanceInfo.enabledExtensionCount   = 0;
    createInstanceInfo.ppEnabledExtensionNames = NULL;

    VkInstance instance;
    PFN_vkCreateInstance pfnCreateInstance =
        reinterpret_cast<PFN_vkCreateInstance>(dlsym(libvulkan, "vkCreateInstance"));
    if (pfnCreateInstance(&createInstanceInfo, NULL, &instance) == VK_SUCCESS)
    {
        PFN_vkEnumeratePhysicalDevices pfnEnumeratePhysicalDevices =
            reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(
                dlsym(libvulkan, "vkEnumeratePhysicalDevices"));
        PFN_vkGetPhysicalDeviceProperties pfnGetPhysicalDeviceProperties =
            reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(
                dlsym(libvulkan, "vkGetPhysicalDeviceProperties"));
        uint32_t physicalDeviceCount       = 0;
        VkPhysicalDevice *pPhysicalDevices = NULL;

        if (pfnEnumeratePhysicalDevices(instance, &physicalDeviceCount, NULL) != VK_SUCCESS)
        {
            return false;
        }
        pPhysicalDevices =
            (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);
        info->gpus.resize(physicalDeviceCount);
        if (!pPhysicalDevices || (pfnEnumeratePhysicalDevices(instance, &physicalDeviceCount,
                                                              pPhysicalDevices) != VK_SUCCESS))
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
            // Need to parse/re-format properties.driverVersion.  Is
            // the formatting different for each vendor?  How to determine?
            char driverVersion[100];
            switch (properties.vendorID)
            {
                case kVendorID_AMD:
                    info->gpus[i].driverVendor = "Advanced Micro Devices, Inc";
                    sprintf(driverVersion, "%d", properties.driverVersion);
                    info->gpus[i].driverVersion = driverVersion;
                    break;
                case kVendorID_ARM:
                    info->gpus[i].driverVendor = "Arm Holdings";
                    sprintf(driverVersion, "%d", properties.driverVersion);
                    info->gpus[i].driverVersion = driverVersion;
                    break;
                case kVendorID_ImgTec:
                    info->gpus[i].driverVendor = "Imagination Technologies Limited";
                    sprintf(driverVersion, "%d", properties.driverVersion);
                    info->gpus[i].driverVersion = driverVersion;
                    break;
                case kVendorID_Intel:
                    info->gpus[i].driverVendor = "Intel Corporation";
                    sprintf(driverVersion, "%d", properties.driverVersion);
                    info->gpus[i].driverVersion = driverVersion;
                    break;
                case kVendorID_Nvidia:
                    info->gpus[i].driverVendor = "NVIDIA Corporation";
                    sprintf(driverVersion, "%d", properties.driverVersion);
                    info->gpus[i].driverVersion = driverVersion;
                    break;
                case kVendorID_Qualcomm:
                    info->gpus[i].driverVendor = "Qualcomm Technologies, Inc";
                    if (properties.driverVersion & 0x80000000)
                    {
                        sprintf(driverVersion, "%d.%d.%d", properties.driverVersion >> 22,
                                (properties.driverVersion >> 12) & 0X3FF,
                                properties.driverVersion & 0xFFF);
                    }
                    else
                    {
                        sprintf(driverVersion, "0x%x", properties.driverVersion);
                    }
                    info->gpus[i].driverVersion = driverVersion;
                    break;
                case kVendorID_Vivante:
                    info->gpus[i].driverVendor = "Vivante";
                    sprintf(driverVersion, "%d", properties.driverVersion);
                    info->gpus[i].driverVersion = driverVersion;
                    break;
                case kVendorID_VeriSilicon:
                    info->gpus[i].driverVendor = "VeriSilicon";
                    sprintf(driverVersion, "%d", properties.driverVersion);
                    info->gpus[i].driverVersion = driverVersion;
                    break;
                case kVendorID_Kazan:
                    info->gpus[i].driverVendor = "Kazan Software";
                    sprintf(driverVersion, "%d", properties.driverVersion);
                    info->gpus[i].driverVersion = driverVersion;
                    break;
                default:
                    return false;
            }
            info->gpus[i].driverDate = "Unknown Date";
        }

        PFN_vkDestroyInstance pfnDestroyInstance =
            reinterpret_cast<PFN_vkDestroyInstance>(dlsym(libvulkan, "vkDestroyInstance"));
        pfnDestroyInstance(instance, NULL);

        return true;
    }
    return false;
}

}  // namespace angle
