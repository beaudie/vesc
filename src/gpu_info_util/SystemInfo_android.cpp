//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SystemInfo_android.cpp: implementation of the Android-specific parts of SystemInfo.h

#include <dlfcn.h>
#include <vulkan/vulkan.h>
#include "gpu_info_util/SystemInfo_internal.h"

#include <cstring>
#include <fstream>

#include "common/angleutils.h"
#include "common/debug.h"

namespace angle
{
struct VulkanLibrary
{
    VulkanLibrary() {}
    ~VulkanLibrary()
    {
        if (instance != VK_NULL_HANDLE)
        {
            PFN_vkDestroyInstance pfnDestroyInstance =
                reinterpret_cast<PFN_vkDestroyInstance>(dlsym(libvulkan, "vkDestroyInstance"));
            if (pfnDestroyInstance)
            {
                pfnDestroyInstance(instance, nullptr);
            }
        }
        if (libvulkan)
            dlclose(libvulkan);
    }
    VkInstance GetVulkanInstance()
    {
        // Find the system's Vulkan library and open it:
        libvulkan = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
        if (!libvulkan)
        {
            // If Vulkan doesn't exist, bail-out early:
            return instance;
        }

        // Determine the available Vulkan instance version:
        uint32_t instanceVersion = VK_API_VERSION_1_0;
#if defined(VK_VERSION_1_1)
        PFN_vkEnumerateInstanceVersion pfnEnumerateInstanceVersion =
            reinterpret_cast<PFN_vkCreateInstance>(dlsym(libvulkan, "vkEnumerateInstanceVersion"));
        if (!pfnEnumerateInstanceVersion ||
            pfnEnumerateInstanceVersion(&instanceVersion) != VK_SUCCESS)
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
            reinterpret_cast<PFN_vkCreateInstance>(dlsym(libvulkan, "vkCreateInstance"));
        if (pfnCreateInstance)
        {
            pfnCreateInstance(&createInstanceInfo, nullptr, &instance);
        }

        return instance;
    }

    void *libvulkan     = nullptr;
    VkInstance instance = VK_NULL_HANDLE;
};

bool GetSystemInfo(SystemInfo *info)
{
    // This implementation builds on top of the Vulkan API, but cannot assume the existence of the
    // Vulkan library.  ANGLE can be installed on versions of Android as old as Ice Cream Sandwich.
    // Therefore, we need to use dlopen()/dlsym() in order to see if Vulkan is installed on the
    // system, and if so, to use it:
    VulkanLibrary vkLibrary;
    VkInstance instance = vkLibrary.GetVulkanInstance();
    if (instance == VK_NULL_HANDLE)
    {
        // If Vulkan doesn't exist, bail-out early:
        return false;
    }

    // Enumerate the Vulkan physical devices, which are ANGLE gpus:
    PFN_vkEnumeratePhysicalDevices pfnEnumeratePhysicalDevices =
        reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(
            dlsym(vkLibrary.libvulkan, "vkEnumeratePhysicalDevices"));
    PFN_vkGetPhysicalDeviceProperties pfnGetPhysicalDeviceProperties =
        reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(
            dlsym(vkLibrary.libvulkan, "vkGetPhysicalDeviceProperties"));
    uint32_t physicalDeviceCount       = 0;
    VkPhysicalDevice *pPhysicalDevices = nullptr;
    if (!pfnEnumeratePhysicalDevices ||
        pfnEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr) != VK_SUCCESS)
    {
        return false;
    }
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    info->gpus.resize(physicalDeviceCount);
    if (pfnEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()) !=
        VK_SUCCESS)
    {
        return false;
    }

    for (uint32_t i = 0; i < physicalDeviceCount; i++)
    {
        VkPhysicalDeviceProperties properties;
        pfnGetPhysicalDeviceProperties(pPhysicalDevices[i], &properties);
        // Fill in data for a given physical device (a.k.a. gpu):
        GPUDeviceInfo &gpu = info->gpus[i];
        gpu.vendorId       = properties.vendorID;
        gpu.deviceId       = properties.deviceID;
        // Need to parse/re-format properties.driverVersion.
        //
        // TODO(ianelliott): Determine the formatting used for each vendor
        // (http://anglebug.com/2677)
        constexpr std::size_t kDriverVersionSize = 20;
        char driverVersion[kDriverVersionSize];
        snprintf(driverVersion, kDriverVersionSize, "0x%x", properties.driverVersion);
        switch (properties.vendorID)
        {
            case kVendorID_AMD:
                gpu.driverVendor = "Advanced Micro Devices, Inc";
                break;
            case kVendorID_ARM:
                gpu.driverVendor = "Arm Holdings";
                snprintf(driverVersion, kDriverVersionSize, "%d", properties.driverVersion);
                break;
            case kVendorID_ImgTec:
                gpu.driverVendor = "Imagination Technologies Limited";
                snprintf(driverVersion, kDriverVersionSize, "%d", properties.driverVersion);
                break;
            case kVendorID_Intel:
                gpu.driverVendor = "Intel Corporation";
                snprintf(driverVersion, kDriverVersionSize, "%d", properties.driverVersion);
                break;
            case kVendorID_Nvidia:
                gpu.driverVendor = "NVIDIA Corporation";
                snprintf(driverVersion, kDriverVersionSize, "%d.%d.%d.%d",
                         properties.driverVersion >> 22, (properties.driverVersion >> 14) & 0XFF,
                         (properties.driverVersion >> 6) & 0XFF, properties.driverVersion & 0x3F);
                break;
            case kVendorID_Qualcomm:
                gpu.driverVendor = "Qualcomm Technologies, Inc";
                if (properties.driverVersion & 0x80000000)
                {
                    snprintf(driverVersion, kDriverVersionSize, "%d.%d.%d",
                             properties.driverVersion >> 22,
                             (properties.driverVersion >> 12) & 0X3FF,
                             properties.driverVersion & 0xFFF);
                }
                break;
            case kVendorID_Vivante:
                gpu.driverVendor = "Vivante";
                break;
            case kVendorID_VeriSilicon:
                gpu.driverVendor = "VeriSilicon";
                break;
            case kVendorID_Kazan:
                gpu.driverVendor = "Kazan Software";
                break;
            default:
                return false;
        }
        gpu.driverVersion = driverVersion;
        gpu.driverDate    = "";
    }

    return true;
}

}  // namespace angle
