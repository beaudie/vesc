//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// platform_utils.cpp: Global helper methods for angle::Platform.

#include "libANGLE/platform_utils.h"

#include "common/debug.h"
#include "platform/Platform.h"

namespace angle
{

namespace
{

class DefaultPlatform : public Platform
{
  public:
    DefaultPlatform() {}
    ~DefaultPlatform() override {}
};

Platform *currentPlatform = nullptr;
DefaultPlatform *defaultPlatform = nullptr;

}

void InitDefaultPlatformImpl()
{
    if (defaultPlatform == nullptr)
    {
        defaultPlatform = new DefaultPlatform();
    }

    if (currentPlatform == nullptr)
    {
        Platform::initialize(defaultPlatform);
    }
}

void DeinitDefaultPlatformImpl()
{
    if (currentPlatform == defaultPlatform)
    {
        currentPlatform = nullptr;
    }

    SafeDelete(defaultPlatform);
}

// static
ANGLE_EXPORT Platform *Platform::current()
{
    ASSERT(currentPlatform);
    return currentPlatform;
}

// static
ANGLE_EXPORT void Platform::initialize(Platform *platformImpl)
{
    ASSERT(platformImpl);
    currentPlatform = platformImpl;
}

// static
ANGLE_EXPORT void Platform::shutdown()
{
    currentPlatform = defaultPlatform;
}

}
