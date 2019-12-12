// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <vulkan/vulkan.h>

// THIS FILE SHOULD BE DELETED IF VK_EXT_iosurface_device_memory IS EVER ADDED TO THE VULKAN HEADERS
#ifdef VK_EXT_iosurface_device_memory
#    error \
        "VK_EXT_iosurface_device_memory is already defined in the Vulkan headers, you can delete this file"
#endif

static constexpr VkStructureType VK_STRUCTURE_TYPE_IOSURFACE_MEMORY_INFO =
    static_cast<VkStructureType>(1000074003);

#define VK_KHR_iosurface_memory 1
#define VK_KHR_IOSURFACE_MEMORY_SPEC_VERSION 1
#define VK_KHR_IOSURFACE_MEMORY_EXTENSION_NAME "VK_ANGLE_iosurface_memory"
typedef struct VkIOSurfaceMemoryInfo
{
    VkStructureType sType;
    const void *pNext;
    void *pIOSurfaceRef;
    int size;
    int plane;

} VkIOSurfaceMemoryInfo;
