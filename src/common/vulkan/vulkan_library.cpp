//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vulkan_library.cpp:
//    Helper functions for the loading Vulkan libraries.
//

#include "common/vulkan/vulkan_library.h"

#include "common/system_utils.h"

namespace angle
{
namespace vk
{
std::unique_ptr<Library> OpenLibVulkan()
{
    constexpr const char *kLibVulkanNames[] = {
#if defined(ANGLE_PLATFORM_WINDOWS)
        "vulkan-1.dll",
#elif defined(ANGLE_PLATFORM_APPLE)
        "libvulkan.dylib",
        "libvulkan.1.dylib",
        "libMoltenVK.dylib"
#else
        "libvulkan.so.1",
        "libvulkan.so",
#endif
    };

    // Prefer local vulkan loaders if they exist
    constexpr SearchType kSearchTypes[] = {
        SearchType::ModuleDir,
        SearchType::SystemDir,
        SearchType::Default,
    };

    for (angle::SearchType searchType : kSearchTypes)
    {
        for (const char *libraryName : kLibVulkanNames)
        {
            std::unique_ptr<Library> library(
                OpenSharedLibraryWithExtension(libraryName, searchType));
            if (library)
            {
                return library;
            }
        }
    }

    return nullptr;
}
}  // namespace vk
}  // namespace angle
