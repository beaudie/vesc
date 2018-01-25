//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vk_utils:
//    Helper functions for the Vulkan Caps.
//

#ifndef LIBANGLE_RENDERER_VULKAN_VK_CAPS_UTILS_H_
#define LIBANGLE_RENDERER_VULKAN_VK_CAPS_UTILS_H_

#include <limits>

#include <vulkan/vulkan.h>
#include "libANGLE/Caps.h"

namespace rx
{

namespace vk
{

void GenerateCaps(gl::Caps *outCaps,
    gl::TextureCapsMap * /*outTextureCaps*/,
    gl::Extensions *outExtensions,
    gl::Limitations * /* outLimitations */,
    const VkPhysicalDeviceProperties *physicalDeviceProperties);
}
}

#endif