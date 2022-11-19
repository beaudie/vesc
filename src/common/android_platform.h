//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// android_platform.h: Android operating system specific includes and defines.
//

#ifndef COMMON_ANDROID_PLATFORM_H_
#define COMMON_ANDROID_PLATFORM_H_

#if defined(ANGLE_PLATFORM_ANDROID)

#   if defined(__has_include) && __has_include(<android/ndk-version.h>)
#       include <android/ndk-version.h>
#   endif

// Note: there are two kind of levels involved when building:
//
//   1. The level of the devices you are building *for*. This is
//      ANGLE_AHARDWAREBUFFER_BUILD_LEVEL below.
//
//   2. The level of the repository you are building *from*. This is
//      ANGLE_AHARDWAREBUFFER_TARGET_SUPPORT and
//      ANGLE_AHARDWAREBUFFER_TARGET_LOCK_PLANES_SUPPORT below.

#   if defined(__ANDROID_NDK__) && defined(__NDK_MAJOR__)
#       define ANGLE_AHARDWAREBUFFER_BUILD_LEVEL __NDK_MAJOR__
#   else
        // This is an Android platform build which will have the latest available.
#       define ANGLE_AHARDWAREBUFFER_BUILD_LEVEL 10000
#   endif

#if __ANDROID_API__ >= 26
#   define ANGLE_AHARDWAREBUFFER_TARGET_SUPPORT
#   include <android/hardware_buffer.h>
#endif // __ANDROID_API__ >= 26

#if __ANDROID_API__ >= 29
#   define ANGLE_AHARDWAREBUFFER_TARGET_LOCK_PLANES_SUPPORT
#endif

#endif // defined(ANGLE_PLATFORM_ANDROID)

#endif /* COMMON_ANDROID_PLATFORM_H_ */
