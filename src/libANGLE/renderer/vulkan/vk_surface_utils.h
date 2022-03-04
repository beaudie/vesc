//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vk_surface_utils.h:
//    Contains the structures and classes for the Vulkan Surface.
//

#ifndef LIBANGLE_RENDERER_VULKAN_VK_SURFACE_UTILS_H_
#define LIBANGLE_RENDERER_VULKAN_VK_SURFACE_UTILS_H_

#include "libANGLE/renderer/vulkan/vk_utils.h"

namespace rx
{
namespace vk
{

enum class PresentMode
{
    ImmediateKHR               = VK_PRESENT_MODE_IMMEDIATE_KHR,
    MailboxKHR                 = VK_PRESENT_MODE_MAILBOX_KHR,
    FifoKHR                    = VK_PRESENT_MODE_FIFO_KHR,
    FifoRelaxedKHR             = VK_PRESENT_MODE_FIFO_RELAXED_KHR,
    SharedDemandRefreshKHR     = VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR,
    SharedContinuousRefreshKHR = VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR,

    GbmANGLE = 6,

    InvalidEnum,
    EnumCount = InvalidEnum,
};

VkPresentModeKHR ConvertPresentModeToVkPresentMode(PresentMode presentMode);
PresentMode ConvertVkPresentModeToPresentMode(VkPresentModeKHR vkPresentMode);

}  // namespace vk

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_VK_SURFACE_UTILS_H_
