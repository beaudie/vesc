//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// platform_helpers.h: common platform detection helper functions

#ifndef COMMON_PLATFORM_HELPERS_H_
#define COMMON_PLATFORM_HELPERS_H_

#include "common/platform.h"

namespace angle
{

inline constexpr bool IsAndroid()
{
#if defined(ANGLE_PLATFORM_ANDROID)
    return true;
#else
    return false;
#endif
}

inline constexpr bool IsApple()
{
    // This means any Apple-vended OS (including iOS, macOS, etc)
#if defined(ANGLE_PLATFORM_APPLE)
    return true;
#else
    return false;
#endif
}

inline constexpr bool IsChromeOS()
{
#if defined(ANGLE_PLATFORM_CHROMEOS)
    return true;
#else
    return false;
#endif
}

inline constexpr bool IsFuchsia()
{
#if defined(ANGLE_PLATFORM_FUCHSIA)
    return true;
#else
    return false;
#endif
}

inline constexpr bool IsIOS()
{
#if ANGLE_PLATFORM_IOS_FAMILY
    return true;
#else
    return false;
#endif
}

inline constexpr bool IsLinux()
{
#if defined(ANGLE_PLATFORM_LINUX)
    return true;
#else
    return false;
#endif
}

inline constexpr bool IsMac()
{
#if defined(ANGLE_PLATFORM_MACOS)
    return true;
#else
    return false;
#endif
}

inline constexpr bool IsWindows()
{
#if defined(ANGLE_PLATFORM_WINDOWS)
    return true;
#else
    return false;
#endif
}

// Helper for version number comparisons
struct VersionTriple
{
    constexpr VersionTriple() {}

    constexpr VersionTriple(int major, int minor, int patch)
        : majorVersion(major), minorVersion(minor), patchVersion(patch)
    {}

    int majorVersion = 0;
    int minorVersion = 0;
    int patchVersion = 0;
};

bool operator==(const VersionTriple &a, const VersionTriple &b);
bool operator!=(const VersionTriple &a, const VersionTriple &b);
bool operator<(const VersionTriple &a, const VersionTriple &b);
bool operator>=(const VersionTriple &a, const VersionTriple &b);

//
// Windows version check helpers
//

// Windows version constants, for range check functions
static constexpr VersionTriple kVersionWindowsXP    = VersionTriple(5, 1, 0);
static constexpr VersionTriple kVersionWindowsVista = VersionTriple(6, 0, 0);
static constexpr VersionTriple kVersionWindows7     = VersionTriple(6, 1, 0);
static constexpr VersionTriple kVersionWindows8     = VersionTriple(6, 2, 0);
static constexpr VersionTriple kVersionWindows10    = VersionTriple(10, 0, 0);
static constexpr VersionTriple kVersionWindows11    = VersionTriple(10, 0, 22000);

// Range checks
bool IsWindowsVersionOrLater(VersionTriple greaterOrEqual);
bool IsWindowsVersionInRange(VersionTriple greaterOrEqual, VersionTriple lessThan);

// Exact version checks
bool IsWindowsXP();
bool IsWindowsVista();
bool IsWindows7();
bool IsWindows8();
bool IsWindows10();
bool IsWindows11();

// Windows version or later helpers
bool IsWindowsXPOrLater();
bool IsWindowsVistaOrLater();
bool IsWindows7OrLater();
bool IsWindows8OrLater();
bool IsWindows10OrLater();
bool IsWindows11OrLater();

}  // namespace angle

#endif  // COMMON_PLATFORM_HELPERS_H_
