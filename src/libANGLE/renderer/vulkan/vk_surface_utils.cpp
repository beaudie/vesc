//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vk_surface_utils.cpp:
//    Contains the structures and classes for the Vulkan Surface.
//

#include "libANGLE/renderer/vulkan/vk_surface_utils.h"

namespace rx
{
namespace vk
{
namespace
{

static_assert(static_cast<uint32_t>(PresentMode::ImmediateKHR) == VK_PRESENT_MODE_IMMEDIATE_KHR,
              "PresentMode must be updated");
static_assert(static_cast<uint32_t>(PresentMode::MailboxKHR) == VK_PRESENT_MODE_MAILBOX_KHR,
              "PresentMode must be updated");
static_assert(static_cast<uint32_t>(PresentMode::FifoKHR) == VK_PRESENT_MODE_FIFO_KHR,
              "PresentMode must be updated");
static_assert(static_cast<uint32_t>(PresentMode::FifoRelaxedKHR) ==
                  VK_PRESENT_MODE_FIFO_RELAXED_KHR,
              "PresentMode must be updated");
static_assert(static_cast<uint32_t>(PresentMode::SharedDemandRefreshKHR) ==
                  VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR,
              "PresentMode must be updated");
static_assert(static_cast<uint32_t>(PresentMode::SharedContinuousRefreshKHR) ==
                  VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR,
              "PresentMode must be updated");

static_assert(static_cast<uint32_t>(PresentMode::GbmANGLE) == 6, "PresentMode must be updated");
}  // namespace

VkPresentModeKHR ConvertPresentModeToVkPresentMode(PresentMode presentMode)
{
    return presentMode == PresentMode::GbmANGLE ? VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR
                                                : static_cast<VkPresentModeKHR>(presentMode);
}

PresentMode ConvertVkPresentModeToPresentMode(VkPresentModeKHR vkPresentMode)
{
    return static_cast<PresentMode>(vkPresentMode);
}

}  // namespace vk
}  // namespace rx
