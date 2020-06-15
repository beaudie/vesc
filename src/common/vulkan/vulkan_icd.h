//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vulkan_icd.h : Helper for creating vulkan instances & selecting physical device.

#ifndef COMMON_VULKAN_VULKAN_ICD_H_
#define COMMON_VULKAN_VULKAN_ICD_H_

#include <string>
#include <vector>

#include "common/Optional.h"
#include "common/angleutils.h"
#include "libANGLE/renderer/vulkan/vk_headers.h"

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
    struct CustomExtension
    {
        VkStructureType type;
        size_t size;
    };

    ScopedVkLoaderEnvironment(bool enableValidationLayers,
                              vk::ICD icd,
                              std::vector<CustomExtension> customExtensions);
    ~ScopedVkLoaderEnvironment();

    bool canEnableValidationLayers() const { return mEnableValidationLayers; }
    vk::ICD getEnabledICD() const { return mICD; }

  private:
    bool setICDEnvironment(const char *icd);
    bool setCustomExtensionsEnvironment();

    bool mEnableValidationLayers;
    vk::ICD mICD;
    bool mChangedCWD;
    Optional<std::string> mPreviousCWD;
    bool mChangedICDEnv;
    Optional<std::string> mPreviousICDEnv;
    std::vector<CustomExtension> mCustomExtensions;
    Optional<std::string> mPreviousCustomExtensionsEnv;
};

void ChoosePhysicalDevice(const std::vector<VkPhysicalDevice> &physicalDevices,
                          vk::ICD preferredICD,
                          VkPhysicalDevice *physicalDeviceOut,
                          VkPhysicalDeviceProperties *physicalDevicePropertiesOut);

}  // namespace vk

}  // namespace angle

#endif  // COMMON_VULKAN_VULKAN_ICD_H_
