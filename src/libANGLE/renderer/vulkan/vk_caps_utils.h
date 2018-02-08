//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vk_utils:
//    Helper functions for the Vulkan Caps.
//

#ifndef LIBANGLE_RENDERER_VULKAN_VK_CAPS_UTILS_H_
#define LIBANGLE_RENDERER_VULKAN_VK_CAPS_UTILS_H_

#include <vulkan/vulkan.h>
#include "libANGLE/Caps.h"

namespace gl
{
struct Limitations;
struct Extensions;
class TextureCapsMap;
struct Caps;
}

namespace rx
{

namespace vk
{
class FormatTable;

// This will return a pointer to a VkFormatProperties with the feature flags supported
// if the texture is a mandatory texture described in section 31.3.3. Required Format Support
// of the Vulkan spec. If the vkFormat isn't mandatory, it will return a nullptr.
VkFormatProperties *GetMandatoryFormatSupport(VkFormat vkFormat);

void GenerateCaps(const VkPhysicalDeviceProperties &physicalDeviceProperties,
                  const gl::TextureCapsMap &textureCaps,
                  gl::Caps *outCaps,
                  gl::Extensions *outExtensions,
                  gl::Limitations * /* outLimitations */);

gl::TextureCaps GenerateTextureFormatCaps(const VkFormatProperties &formatProperties);

}  // namespace vk
}  // namespace rx

#endif