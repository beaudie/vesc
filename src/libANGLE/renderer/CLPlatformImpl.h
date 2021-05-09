//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLPlatformImpl.h: Defines the abstract rx::CLPlatformImpl class.

#ifndef LIBANGLE_RENDERER_CLPLATFORMIMPL_H_
#define LIBANGLE_RENDERER_CLPLATFORMIMPL_H_

#include "libANGLE/renderer/CLContextImpl.h"
#include "libANGLE/renderer/CLDeviceImpl.h"

#include <list>
#include <string>
#include <tuple>

namespace rx
{

class CLPlatformImpl : angle::NonCopyable
{
  public:
    struct Info
    {
        Info();
        ~Info();

        Info(const Info &) = delete;
        Info &operator=(const Info &) = delete;

        Info(Info &&);
        Info &operator=(Info &&);

        bool isValid() const;

        std::string mProfile;
        std::string mVersionStr;
        cl_version mVersion;
        std::string mName;
        std::string mExtensions;
        NameVersionArray mExtensionsWithVersion;
        cl_ulong mHostTimerRes;
    };

    using Ptr       = std::unique_ptr<CLPlatformImpl>;
    using ImplTuple = std::tuple<Ptr, Info, CLDeviceImpl::List>;
    using ImplList  = std::list<ImplTuple>;

    explicit CLPlatformImpl(CLDeviceImpl::Array &&devices);
    virtual ~CLPlatformImpl();

    const CLDeviceImpl::Array &getDevices() const;

    virtual CLContextImpl::Ptr createContext(CLDeviceImpl::Array &&devices,
                                             cl::ContextErrorCB notify,
                                             void *userData,
                                             bool userSync,
                                             cl_int *errcodeRet) = 0;

    virtual CLContextImpl::Ptr createContextFromType(cl_device_type deviceType,
                                                     cl::ContextErrorCB notify,
                                                     void *userData,
                                                     bool userSync,
                                                     cl_int *errcodeRet) = 0;

  protected:
    const CLDeviceImpl::Array mDevices;

    CLContextImpl::Array mContexts;

    friend class CLContextImpl;
};

inline const CLDeviceImpl::Array &CLPlatformImpl::getDevices() const
{
    return mDevices;
}

}  // namespace rx

#endif  // LIBANGLE_RENDERER_CLPLATFORMIMPL_H_
