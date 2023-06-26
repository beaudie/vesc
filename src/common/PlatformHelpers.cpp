//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PlatformHelpers.cpp: implementations for platform identification functions
// which require runtime detections.

#include "common/PlatformHelpers.h"

#ifdef ANGLE_PLATFORM_WINDOWS
#    include <versionhelpers.h>
#endif

namespace angle
{
bool IsWindowsXP()
{
#if defined(ANGLE_PLATFORM_WINDOWS)
    return IsWindowsXPOrGreater() && !IsWindowsVistaOrGreater();
#else
    return false;
#endif
}

bool IsWindowsVista()
{
#if defined(ANGLE_PLATFORM_WINDOWS)
    return IsWindowsVistaOrGreater() && !IsWindows7OrGreater();
#else
    return false;
#endif
}

bool IsWindows7()
{
#if defined(ANGLE_PLATFORM_WINDOWS)
    return IsWindows7OrGreater() && !IsWindows8OrGreater();
#else
    return false;
#endif
}

bool IsWindows8()
{
#if defined(ANGLE_PLATFORM_WINDOWS)
    return IsWindows8OrGreater() && !IsWindows10OrGreater();
#else
    return false;
#endif
}

bool IsWindows10()
{
#if defined(ANGLE_PLATFORM_WINDOWS)
    return IsWindows10OrGreater();
#else
    return false;
#endif
}

}  // namespace angle
