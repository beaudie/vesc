//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// android_tls.h : Helper functions for fast TLS operations on the Android platform.

#ifndef COMMON_ANDROID_TLS_H_
#define COMMON_ANDROID_TLS_H_

#include "angleutils.h"

namespace gl
{
class Context;
}  // namespace gl

namespace angle
{
namespace android
{
//  - TLS_SLOT_OPENGL and TLS_SLOT_OPENGL_API: These two aren't used by bionic
//    itself, but allow the graphics code to access TLS directly rather than
//    using the pthread API.
//
// Choose the TLS_SLOT_OPENGL TLS slot with the value that matches value in the header file in
// bionic(tls_defines.h)
constexpr size_t kAndroidOpenGLTlsSlot = 3;

#if defined(ANGLE_PLATFORM_ANDROID)

// The following ASM variant provides a much more performant store/retrieve interface
// compared to those provided by the pthread library. These have been derived from code
// in the bionic module of Android ->
// https://cs.android.com/android/platform/superproject/+/master:bionic/libc/platform/bionic/tls.h;l=30

#    if defined(__aarch64__)
#        define ANGLE_ANDROID_GET_GL_TLS()                  \
            ({                                              \
                void **__val;                               \
                __asm__("mrs %0, tpidr_el0" : "=r"(__val)); \
                __val;                                      \
            })
#    elif defined(__arm__)
#        define ANGLE_ANDROID_GET_GL_TLS()                           \
            ({                                                       \
                void **__val;                                        \
                __asm__("mrc p15, 0, %0, c13, c0, 3" : "=r"(__val)); \
                __val;                                               \
            })
#    elif defined(__mips__)
// On mips32r1, this goes via a kernel illegal instruction trap that's
// optimized for v1
#        define ANGLE_ANDROID_GET_GL_TLS()       \
            ({                                   \
                register void **__val asm("v1"); \
                __asm__(                         \
                    ".set    push\n"             \
                    ".set    mips32r2\n"         \
                    "rdhwr   %0,$29\n"           \
                    ".set    pop\n"              \
                    : "=r"(__val));              \
                __val;                           \
            })
#    elif defined(__i386__)
#        define ANGLE_ANDROID_GET_GL_TLS()                \
            ({                                            \
                void **__val;                             \
                __asm__("movl %%gs:0, %0" : "=r"(__val)); \
                __val;                                    \
            })
#    elif defined(__x86_64__)
#        define ANGLE_ANDROID_GET_GL_TLS()               \
            ({                                           \
                void **__val;                            \
                __asm__("mov %%fs:0, %0" : "=r"(__val)); \
                __val;                                   \
            })
#    else
#        error unsupported architecture
#    endif

#endif  // ANGLE_PLATFORM_ANDROID

ANGLE_INLINE gl::Context *GetTlsContextAndroid()
{
#if defined(ANGLE_PLATFORM_ANDROID)
    return static_cast<gl::Context *>(ANGLE_ANDROID_GET_GL_TLS()[kAndroidOpenGLTlsSlot]);
#else
    return nullptr;
#endif
}

ANGLE_INLINE void SetTlsContextAndroid(gl::Context *value)
{
#if defined(ANGLE_PLATFORM_ANDROID)
    ANGLE_ANDROID_GET_GL_TLS()[kAndroidOpenGLTlsSlot] = static_cast<void *>(value);
#endif
}

}  // namespace android
}  // namespace angle

#endif  // COMMON_ANDROID_TLS_H_
