//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLPlatformVk.h:
//    Defines the class interface for CLPlatformVk, implementing CLPlatformImpl.
//

#ifndef LIBANGLE_RENDERER_VULKAN_CLPLATFORMVK_H_
#define LIBANGLE_RENDERER_VULKAN_CLPLATFORMVK_H_

#include "libANGLE/renderer/CLPlatformImpl.h"

#include <string>

namespace rx
{

class CLPlatformVk : public CLPlatformImpl
{
  public:
    CLPlatformVk();
    ~CLPlatformVk() override;

    const char *getProfile() const override;
    const char *getVersionString() const override;
    cl_version getVersion() const override;
    const char *getName() const override;
    const char *getExtensions() const override;
    const ExtensionList &getExtensionsWithVersion() const override;
    cl_ulong getHostTimerResolution() const override;

    static ImplList GetPlatforms();
    static constexpr cl_version GetVersion();
    static const std::string &GetVersionString();

  private:
    const ExtensionList mExtensionList;
    const std::string mExtensions;
};

inline const char *CLPlatformVk::getProfile() const
{
    return "FULL_PROFILE";
}

inline const char *CLPlatformVk::getVersionString() const
{
    return GetVersionString().c_str();
}

inline cl_version CLPlatformVk::getVersion() const
{
    return GetVersion();
}

constexpr cl_version CLPlatformVk::GetVersion()
{
    return CL_MAKE_VERSION(1, 2, 0);
}

inline const char *CLPlatformVk::getName() const
{
    return "ANGLE Vulkan";
}

inline const char *CLPlatformVk::getExtensions() const
{
    return mExtensions.c_str();
}

inline const CLPlatformVk::ExtensionList &CLPlatformVk::getExtensionsWithVersion() const
{
    return mExtensionList;
}

inline cl_ulong CLPlatformVk::getHostTimerResolution() const
{
    return 0u;
}

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_CLPLATFORMVK_H_
