//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLPlatformCL.h:
//    Defines the class interface for CLPlatformCL, implementing CLPlatformImpl.
//

#ifndef LIBANGLE_RENDERER_CL_CLPLATFORMCL_H_
#define LIBANGLE_RENDERER_CL_CLPLATFORMCL_H_

#include "libANGLE/renderer/CLPlatformImpl.h"

#include "common/system_utils.h"

#include <string>

namespace rx
{

class CLPlatformCL : public CLPlatformImpl
{
  public:
    ~CLPlatformCL() override;

    cl_platform_id getNative();

    const char *getProfile() const override;
    const char *getVersionString() const override;
    cl_version getVersion() const override;
    const char *getName() const override;
    const char *getExtensions() const override;
    const ExtensionList &getExtensionsWithVersion() const override;
    cl_ulong getHostTimerResolution() const override;

    static ImplList GetPlatforms(bool isIcd);

  private:
    CLPlatformCL(cl_platform_id platform,
                 std::string &&profile,
                 std::string &&versionString,
                 cl_version version,
                 std::string &&name,
                 std::string &&extensions,
                 ExtensionList &&extensionList,
                 cl_ulong hostTimerRes);

    static std::unique_ptr<angle::Library> &Library();

    const cl_platform_id mPlatform;
    const std::string mProfile, mVersionString, mName, mExtensions;
    const ExtensionList mExtensionList;
    const cl_version mVersion;
    const cl_ulong mHostTimerRes;
};

inline cl_platform_id CLPlatformCL::getNative()
{
    return mPlatform;
}

inline const char *CLPlatformCL::getProfile() const
{
    return mProfile.c_str();
}

inline const char *CLPlatformCL::getVersionString() const
{
    return mVersionString.c_str();
}

inline cl_version CLPlatformCL::getVersion() const
{
    return mVersion;
}

inline const char *CLPlatformCL::getName() const
{
    return mName.c_str();
}

inline const char *CLPlatformCL::getExtensions() const
{
    return mExtensions.c_str();
}

inline const CLPlatformCL::ExtensionList &CLPlatformCL::getExtensionsWithVersion() const
{
    return mExtensionList;
}

inline cl_ulong CLPlatformCL::getHostTimerResolution() const
{
    return mHostTimerRes;
}

}  // namespace rx

#endif  // LIBANGLE_RENDERER_CL_CLPLATFORMCL_H_
