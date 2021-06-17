//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vulkan_library.h:
//    Helper functions for the loading Vulkan libraries.
//

#include <memory>

#ifndef LIBANGLE_COMMON_VULKAN_VULKAN_LIBRARY_H_
#    define LIBANGLE_COMMON_VULKAN_VULKAN_LIBRARY_H_

namespace angle
{
class Library;

namespace vk
{
std::unique_ptr<Library> OpenLibVulkan();
}
}  // namespace angle

#endif  // LIBANGLE_COMMON_VULKAN_VULKAN_LIBRARY_H_
