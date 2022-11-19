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

#    include <android/api-level.h>

#    if defined(__has_include)
#        warning "jasonjason __has_include available?"
#    else
#        warning "jasonjason __has_include not available?"
#    endif

#    if defined(__ANDROID_NDK__)
#        warning "jasonjason __ANDROID_NDK__ available via api-level"
#    else
#        warning "jasonjason __ANDROID_NDK__ not available via api-level?"
#    endif

#    if defined(__has_include) && __has_include(<android/ndk-version.h>)
#        include <android/ndk-version.h>
#        warning "jasonjason ndk-version available"
#    else
#        warning "jasonjason ndk-version not available?"
#    endif

#    if defined(__NDK_MAJOR__)
#        warning "jasonjason __NDK_MAJOR__ available"
#    else
#        warning "jasonjason __NDK_MAJOR__ not available"
#    endif

#    if defined(__ANDROID_API__)
#        warning "jasonjason __ANDROID_API__ available"
#        if __ANDROID_API__ == 15
#            warning "jasonjason __ANDROID_API__ == 15"
#        elif __ANDROID_API__ == 16
#            warning "jasonjason __ANDROID_API__ == 16"
#        elif __ANDROID_API__ == 17
#            warning "jasonjason __ANDROID_API__ == 17"
#        elif __ANDROID_API__ == 18
#            warning "jasonjason __ANDROID_API__ == 18"
#        elif __ANDROID_API__ == 19
#            warning "jasonjason __ANDROID_API__ == 19"
#        elif __ANDROID_API__ == 20
#            warning "jasonjason __ANDROID_API__ == 20"
#        elif __ANDROID_API__ == 21
#            warning "jasonjason __ANDROID_API__ == 21"
#        elif __ANDROID_API__ == 22
#            warning "jasonjason __ANDROID_API__ == 22"
#        elif __ANDROID_API__ == 23
#            warning "jasonjason __ANDROID_API__ == 23"
#        elif __ANDROID_API__ == 24
#            warning "jasonjason __ANDROID_API__ == 24"
#        elif __ANDROID_API__ == 25
#            warning "jasonjason __ANDROID_API__ == 25"
#        elif __ANDROID_API__ == 26
#            warning "jasonjason __ANDROID_API__ == 26"
#        elif __ANDROID_API__ == 27
#            warning "jasonjason __ANDROID_API__ == 27"
#        elif __ANDROID_API__ == 28
#            warning "jasonjason __ANDROID_API__ == 28"
#        elif __ANDROID_API__ == 29
#            warning "jasonjason __ANDROID_API__ == 29"
#        elif __ANDROID_API__ == 30
#            warning "jasonjason __ANDROID_API__ == 30"
#        elif __ANDROID_API__ == 31
#            warning "jasonjason __ANDROID_API__ == 31"
#        elif __ANDROID_API__ == 32
#            warning "jasonjason __ANDROID_API__ == 32"
#        elif __ANDROID_API__ == 33
#            warning "jasonjason __ANDROID_API__ == 33"
#        endif
#    else
#        warning "jasonjason __ANDROID_API__ not available"
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

#    if (defined(ANGLE_AHARDWAREBUFFER_NDK_BUILD_LEVEL) &&      \
         ANGLE_AHARDWAREBUFFER_BUILD_LEVEL >= 15) ||            \
        (defined(ANGLE_AHARDWAREBUFFER_PLATFORM_BUILD_LEVEL) && \
         ANGLE_AHARDWAREBUFFER_PLATFORM_BUILD_LEVEL >= 26)
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
