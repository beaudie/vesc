//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLPlatform.h: Defines the cl::Platform class, which provides information about platform-specific
// OpenCL features.

#ifndef LIBANGLE_CLPLATFORM_H_
#define LIBANGLE_CLPLATFORM_H_

#include "libANGLE/CLContext.h"
#include "libANGLE/CLDevice.h"
#include "libANGLE/renderer/CLPlatformImpl.h"

#include "anglebase/no_destructor.h"

#include <algorithm>

namespace cl
{

class Platform final : public _cl_platform_id, public Object
{
  public:
    using Ptr  = std::unique_ptr<Platform>;
    using List = std::vector<Ptr>;

    ~Platform();

    bool hasDevice(const Device *device) const;
    const Device::List &getDevices() const;
    Device::RefList mapDevices(const rx::CLDeviceImpl::Array &deviceImpls) const;

    bool hasContext(const Context *context) const;

    cl_int getInfo(PlatformInfo name, size_t valueSize, void *value, size_t *valueSizeRet);

    cl_int getDeviceIDs(cl_device_type deviceType,
                        cl_uint numEntries,
                        Device **devices,
                        cl_uint *numDevices) const;

    Context *createContext(Context::PropArray &&properties,
                           cl_uint numDevices,
                           Device *const *devices,
                           ContextErrorCB notify,
                           void *userData,
                           bool userSync,
                           cl_int *errcodeRet);

    Context *createContextFromType(Context::PropArray &&properties,
                                   cl_device_type deviceType,
                                   ContextErrorCB notify,
                                   void *userData,
                                   bool userSync,
                                   cl_int *errcodeRet);

    static void CreatePlatform(const cl_icd_dispatch &dispatch,
                               rx::CLPlatformImpl::ImplTuple &implTuple);

    static const List &GetPlatforms();
    static Platform *GetDefault();
    static bool IsValid(const Platform *platform);
    static bool IsValidOrDefault(const Platform *platform);

    static constexpr const char *GetVendor();

  private:
    Platform(const cl_icd_dispatch &dispatch, rx::CLPlatformImpl::ImplTuple &implTuple);

    rx::CLContextImpl::Ptr createContext(const Device::RefList &devices,
                                         ContextErrorCB notify,
                                         void *userData,
                                         bool userSync,
                                         cl_int *errcodeRet);

    void destroyContext(Context *context);

    static List &GetList();

    const rx::CLPlatformImpl::Ptr mImpl;
    const rx::CLPlatformImpl::Info mInfo;
    const Device::List mDevices;

    Context::List mContexts;

    static constexpr char kVendor[]    = "ANGLE";
    static constexpr char kIcdSuffix[] = "ANGLE";

    friend class Context;
};

inline bool Platform::hasDevice(const Device *device) const
{
    return std::find_if(mDevices.cbegin(), mDevices.cend(), [=](const Device::Ptr &ptr) {
               return ptr.get() == device || ptr->hasSubDevice(device);
           }) != mDevices.cend();
}

inline const Device::List &Platform::getDevices() const
{
    return mDevices;
}

inline bool Platform::hasContext(const Context *context) const
{
    return std::find_if(mContexts.cbegin(), mContexts.cend(), [=](const Context::Ptr &ptr) {
               return ptr.get() == context;
           }) != mContexts.cend();
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

inline Platform *Platform::GetDefault()
{
    return GetList().empty() ? nullptr : GetList().front().get();
}

inline bool Platform::IsValid(const Platform *platform)
{
    const List &platforms = GetPlatforms();
    return std::find_if(platforms.cbegin(), platforms.cend(),
                        [=](const Ptr &ptr) { return ptr.get() == platform; }) != platforms.cend();
}

inline bool Platform::IsValidOrDefault(const Platform *platform)
{
    return platform != nullptr ? IsValid(platform) : GetDefault() != nullptr;
}

constexpr const char *Platform::GetVendor()
{
    return kVendor;
}

}  // namespace cl

#endif  // LIBANGLE_CLPLATFORM_H_
