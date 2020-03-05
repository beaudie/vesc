//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VulkanExternalHelper.h : Helper for allocating & managing vulkan external objects.

#ifndef COMMON_VULKAN_VULKANINSTANCEHELPER_H_
#define COMMON_VULKAN_VULKANINSTANCEHELPER_H_

#include <string>

#include "common/Optional.h"
#include "common/angleutils.h"
#include "volk.h"

namespace angle
{

namespace vk
{

enum class ICD
{
    Default,
    Mock,
    SwiftShader,
};

class ScopedVkLoaderEnvironment : angle::NonCopyable
{
  public:
    ScopedVkLoaderEnvironment(bool enableValidationLayers, vk::ICD icd);
    ~ScopedVkLoaderEnvironment();

    bool canEnableValidationLayers() const { return mEnableValidationLayers; }
    vk::ICD getEnabledICD() const { return mICD; }

  private:
    bool setICDEnvironment(const char *icd);

    bool mEnableValidationLayers;
    vk::ICD mICD;
    bool mChangedCWD;
    Optional<std::string> mPreviousCWD;
    bool mChangedICDEnv;
    Optional<std::string> mPreviousICDEnv;
};

void ChoosePhysicalDevice(const std::vector<VkPhysicalDevice> &physicalDevices,
                          vk::ICD preferredICD,
                          VkPhysicalDevice *physicalDeviceOut,
                          VkPhysicalDeviceProperties *physicalDevicePropertiesOut);

}  // namespace vk

}  // namespace angle

#endif  // COMMON_VULKAN_VULKANINSTANCEHELPER_H_
