//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CLPlatformImpl.h: Defines the abstract rx::CLPlatformImpl class.

#ifndef LIBANGLE_RENDERER_CLPLATFORMIMPL_H_
#define LIBANGLE_RENDERER_CLPLATFORMIMPL_H_

#include "libANGLE/renderer/CLtypes.h"

#include <list>
#include <vector>

namespace rx
{

class CLPlatformImpl : angle::NonCopyable
{
  public:
    using Ptr           = std::unique_ptr<CLPlatformImpl>;
    using ImplList      = std::list<Ptr>;
    using ExtensionList = std::vector<cl_name_version>;

    CLPlatformImpl()          = default;
    virtual ~CLPlatformImpl() = default;

    virtual const char *getProfile() const                        = 0;
    virtual const char *getVersionString() const                  = 0;
    virtual cl_version getVersion() const                         = 0;
    virtual const char *getName() const                           = 0;
    virtual const char *getExtensions() const                     = 0;
    virtual const ExtensionList &getExtensionsWithVersion() const = 0;
    virtual cl_ulong getHostTimerResolution() const               = 0;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_CLPLATFORMIMPL_H_
