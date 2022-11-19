//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// android_platform.h: Android operating system specific includes and defines.
//

#ifndef COMMON_ANDROID_PLATFORM_H_
#define COMMON_ANDROID_PLATFORM_H_

#include "common/platform.h"

#if defined(ANGLE_PLATFORM_ANDROID)

#    if defined(__has_include) && __has_include(<android/ndk-version.h>)
#        include <android/ndk-version.h>
#    endif

// Note: there are two kind of levels involved when building:
//
//   1. The level of the devices you are building *for*. This is
//      ANGLE_AHARDWAREBUFFER_TARGET_SUPPORT and
//      ANGLE_AHARDWAREBUFFER_TARGET_LOCK_PLANES_SUPPORT below.
//
//   2. The level of the repository you are building *from*. This is
//      ANGLE_AHARDWAREBUFFER_BUILD_LEVEL below.

#    if defined(__ANDROID_NDK__) && defined(__NDK_MAJOR__)
#        define ANGLE_AHARDWAREBUFFER_NDK_BUILD_LEVEL __NDK_MAJOR__
#    else
#        define ANGLE_AHARDWAREBUFFER_PLATFORM_BUILD_LEVEL __ANDROID_API__
#    endif

#    if (defined(ANGLE_AHARDWAREBUFFER_NDK_BUILD_LEVEL) && ANGLE_AHARDWAREBUFFER_BUILD_LEVEL >= 15) || \
        (defined(ANGLE_AHARDWAREBUFFER_PLATFORM_BUILD_LEVEL) && ANGLE_AHARDWAREBUFFER_PLATFORM_BUILD_LEVEL >= 26)
#        include <android/hardware_buffer.h>
#    endif

#    if __ANDROID_API__ >= 26
#        define ANGLE_AHARDWAREBUFFER_TARGET_SUPPORT
#    endif
#    if __ANDROID_API__ >= 29
#        define ANGLE_AHARDWAREBUFFER_TARGET_LOCK_PLANES_SUPPORT
#    endif

#endif  // defined(ANGLE_PLATFORM_ANDROID)

#endif /* COMMON_ANDROID_PLATFORM_H_ */
