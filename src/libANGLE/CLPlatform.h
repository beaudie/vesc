//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLPlatform.h: Defines the cl::Platform class, which provides information about platform-specific
// OpenCL features.

#ifndef LIBANGLE_CLPLATFORM_H_
#define LIBANGLE_CLPLATFORM_H_

#include "libANGLE/CLObject.h"
#include "libANGLE/renderer/CLPlatformImpl.h"

#include "anglebase/no_destructor.h"

#include <algorithm>
#include <list>
#include <string>

namespace cl
{

class Platform final : public _cl_platform_id, public Object
{
  public:
    using Ptr  = std::unique_ptr<Platform>;
    using List = std::list<const Ptr>;

    ~Platform();

    const char *getProfile() const;
    const char *getVersionString() const;
    cl_version getVersion() const;
    const char *getName() const;
    const char *getExtensions() const;
    const rx::CLPlatformImpl::ExtensionList &getExtensionsWithVersion() const;
    cl_ulong getHostTimerResolution() const;

    static void CreatePlatform(const cl_icd_dispatch &dispatch, rx::CLPlatformImpl::Ptr &&impl);
    static const List &GetPlatforms();
    static bool IsValid(const Platform *platform);

    static constexpr const char *GetVendor() { return kVendor; }

  private:
    Platform(const cl_icd_dispatch &dispatch, rx::CLPlatformImpl::Ptr &&impl);

    static List &GetList();

    const rx::CLPlatformImpl::Ptr mImpl;

    static constexpr char kVendor[] = "ANGLE";
};

inline const char *Platform::getProfile() const
{
    return mImpl->getProfile();
}

inline const char *Platform::getVersionString() const
{
    return mImpl->getVersionString();
}

inline cl_version Platform::getVersion() const
{
    return mImpl->getVersion();
}

inline const char *Platform::getName() const
{
    return mImpl->getName();
}

inline const char *Platform::getExtensions() const
{
    return mImpl->getExtensions();
}

inline const rx::CLPlatformImpl::ExtensionList &Platform::getExtensionsWithVersion() const
{
    return mImpl->getExtensionsWithVersion();
}

inline cl_ulong Platform::getHostTimerResolution() const
{
    return mImpl->getHostTimerResolution();
}

inline Platform::List &Platform::GetList()
{
    static angle::base::NoDestructor<List> sList;
    return *sList;
}

inline const Platform::List &Platform::GetPlatforms()
{
    return GetList();
}

inline bool Platform::IsValid(const Platform *platform)
{
    const List &platforms = GetPlatforms();
    return std::find_if(platforms.cbegin(), platforms.cend(),
                        [=](const Ptr &ptr) { return ptr.get() == platform; }) != platforms.cend();
}

}  // namespace cl

#endif  // LIBANGLE_CLPLATFORM_H_
