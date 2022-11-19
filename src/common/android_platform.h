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

// Note: there are two levels involved when building:
//
//   1. The level of the devices you are building *for*.
#    if __ANDROID_API__ >= 26
#        define ANGLE_AHARDWAREBUFFER_TARGET_SUPPORT
#    endif
#    if __ANDROID_API__ >= 29
#        define ANGLE_AHARDWAREBUFFER_TARGET_LOCK_PLANES_SUPPORT
#    endif

//   2. The level of the repository you are building *from*.
#    if defined(__ANDROID_NDK__) && defined(__NDK_MAJOR__)
#        define ANGLE_AHARDWAREBUFFER_NDK_BUILD_LEVEL __NDK_MAJOR__
#    else
#        if defined(__ANDROID_API_U__)
#            define ANGLE_AHARDWAREBUFFER_PLATFORM_BUILD_LEVEL __ANDROID_API_U__
#        elif defined(__ANDROID_API_T__)
#            define ANGLE_AHARDWAREBUFFER_PLATFORM_BUILD_LEVEL __ANDROID_API_T__
#        elif defined(__ANDROID_API_S__)
#            define ANGLE_AHARDWAREBUFFER_PLATFORM_BUILD_LEVEL __ANDROID_API_S__
#        elif defined(__ANDROID_API_R__)
#            define ANGLE_AHARDWAREBUFFER_PLATFORM_BUILD_LEVEL __ANDROID_API_R__
#        elif defined(__ANDROID_API_Q__)
#            define ANGLE_AHARDWAREBUFFER_PLATFORM_BUILD_LEVEL __ANDROID_API_Q__
#        elif defined(__ANDROID_API_P__)
#            define ANGLE_AHARDWAREBUFFER_PLATFORM_BUILD_LEVEL __ANDROID_API_P__
#        elif defined(__ANDROID_API_O__)
#            define ANGLE_AHARDWAREBUFFER_PLATFORM_BUILD_LEVEL __ANDROID_API_O__
#        endif
#    endif

#    if (defined(ANGLE_AHARDWAREBUFFER_NDK_BUILD_LEVEL) &&      \
         ANGLE_AHARDWAREBUFFER_NDK_BUILD_LEVEL >= 15) ||        \
        (defined(ANGLE_AHARDWAREBUFFER_PLATFORM_BUILD_LEVEL) && \
         ANGLE_AHARDWAREBUFFER_PLATFORM_BUILD_LEVEL >= 26)
#        include <android/hardware_buffer.h>
#    endif

#endif  // defined(ANGLE_PLATFORM_ANDROID)

#endif /* COMMON_ANDROID_PLATFORM_H_ */
