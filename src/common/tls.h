//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// tls.h: Simple cross-platform interface for thread local storage.

#ifndef COMMON_TLS_H_
#define COMMON_TLS_H_

#include "common/angleutils.h"
#include "common/debug.h"
#include "common/platform.h"

#ifdef ANGLE_PLATFORM_WINDOWS

// TLS does not exist for Windows Store and needs to be emulated
#    ifdef ANGLE_ENABLE_WINDOWS_UWP
#        ifndef TLS_OUT_OF_INDEXES
#            define TLS_OUT_OF_INDEXES static_cast<DWORD>(0xFFFFFFFF)
#        endif
#        ifndef CREATE_SUSPENDED
#            define CREATE_SUSPENDED 0x00000004
#        endif
#    endif
typedef DWORD TLSIndex;
#    define TLS_INVALID_INDEX (TLS_OUT_OF_INDEXES)
#elif defined(ANGLE_PLATFORM_POSIX)
#    include <errno.h>
#    include <pthread.h>
#    include <semaphore.h>
typedef pthread_key_t TLSIndex;
#    define TLS_INVALID_INDEX (static_cast<TLSIndex>(-1))
#else
#    error Unsupported platform.
#endif

// TODO(kbr): for POSIX platforms this will have to be changed to take
// in a destructor function pointer, to allow the thread-local storage
// to be properly deallocated upon thread exit.
TLSIndex CreateTLSIndex();
bool DestroyTLSIndex(TLSIndex index);

bool SetTLSValue(TLSIndex index, void *value);
void *GetTLSValue(TLSIndex index);

#if defined(ANGLE_PLATFORM_ANDROID)
#    if defined(__aarch64__)
#        define __get_tls()                                 \
            ({                                              \
                void **__val;                               \
                __asm__("mrs %0, tpidr_el0" : "=r"(__val)); \
                __val;                                      \
            })
#    elif defined(__arm__)
#        define __get_tls()                                          \
            ({                                                       \
                void **__val;                                        \
                __asm__("mrc p15, 0, %0, c13, c0, 3" : "=r"(__val)); \
                __val;                                               \
            })
#    elif defined(__mips__)
#        define __get_tls()                                                                      \
            /* On mips32r1, this goes via a kernel illegal instruction trap that's optimized for \
             * v1. */                                                                            \
            ({                                                                                   \
                register void **__val asm("v1");                                                 \
                __asm__(                                                                         \
                    ".set    push\n"                                                             \
                    ".set    mips32r2\n"                                                         \
                    "rdhwr   %0,$29\n"                                                           \
                    ".set    pop\n"                                                              \
                    : "=r"(__val));                                                              \
                __val;                                                                           \
            })
#    elif defined(__i386__)
#        define __get_tls()                               \
            ({                                            \
                void **__val;                             \
                __asm__("movl %%gs:0, %0" : "=r"(__val)); \
                __val;                                    \
            })
#    elif defined(__x86_64__)
#        define __get_tls()                              \
            ({                                           \
                void **__val;                            \
                __asm__("mov %%fs:0, %0" : "=r"(__val)); \
                __val;                                   \
            })
#    else
#        error unsupported architecture
#    endif

//  - TLS_SLOT_OPENGL and TLS_SLOT_OPENGL_API: These two aren't used by bionic
//    itself, but allow the graphics code to access TLS directly rather than
//    using the pthread API.
//
// Choose the TLS_SLOT_OPENGL TLS slot with the value that matches value in the header file in
// bionic(bionic_asm_tls.h)
// #define TLS_SLOT_OPENGL           3
constexpr TLSIndex kContextTlsSlot = 3;

ANGLE_INLINE TLSIndex CreateContextTLSIndex()
{
    return kContextTlsSlot;
}

ANGLE_INLINE bool SetContextTls(TLSIndex index, void *value)
{
    ASSERT(kContextTlsSlot == index);
    __get_tls()[kContextTlsSlot] = value;
    return true;
}

ANGLE_INLINE void *GetContextTls(TLSIndex index)
{
    ASSERT(kContextTlsSlot == index);
    return __get_tls()[kContextTlsSlot];
}
#else
ANGLE_INLINE TLSIndex CreateContextTLSIndex()
{
    return CreateTLSIndex();
}

ANGLE_INLINE bool SetContextTls(TLSIndex index, void *value)
{
    return SetTLSValue(index, value);
}

ANGLE_INLINE void *GetContextTls(TLSIndex index)
{
    return GetTLSValue(index);
}
#endif

#endif  // COMMON_TLS_H_
