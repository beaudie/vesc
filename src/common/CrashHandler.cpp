//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CrashHandler:
//    ANGLE's crash handling and stack walking code. Modified from Skia's:
//     https://github.com/google/skia/blob/master/tools/CrashHandler.cpp
//

#include "CrashHandler.h"
#include "common/angleutils.h"

#include <stdio.h>
#include <stdlib.h>

#if defined(ANGLE_PLATFORM_APPLE)
// We only use local unwinding, so we can define this to select a faster implementation.
#    define UNW_LOCAL_ONLY
#    include <cxxabi.h>
#    include <libunwind.h>
#    include <signal.h>
#elif defined(ANGLE_PLATFORM_POSIX)
// We'd use libunwind here too, but it's a pain to get installed for
// both 32 and 64 bit on bots.  Doesn't matter much: catchsegv is best anyway.
#    include <cxxabi.h>
#    include <dlfcn.h>
#    include <execinfo.h>
#    include <signal.h>
#    include <string.h>
#endif  // defined(ANGLE_PLATFORM_APPLE)

template <typename T>
void bzero(T *mem, size_t size)
{
    memset(reinterpret_cast<void *>(mem), 0, size);
}

#if defined(ANGLE_PLATFORM_APPLE)

static void handler(int sig)
{
    unw_context_t context;
    unw_getcontext(&context);

    unw_cursor_t cursor;
    unw_init_local(&cursor, &context);

    printf("\nSignal %d:\n", sig);
    while (unw_step(&cursor) > 0)
    {
        static const size_t kMax = 256;
        char mangled[kMax], demangled[kMax];
        unw_word_t offset;
        unw_get_proc_name(&cursor, mangled, kMax, &offset);

        int ok;
        size_t len = kMax;
        abi::__cxa_demangle(mangled, demangled, &len, &ok);

        printf("%s (+0x%zx)\n", ok == 0 ? demangled : mangled, (size_t)offset);
    }
    printf("\n");

    // Exit NOW.  Don't notify other threads, don't call anything registered with atexit().
    _Exit(sig);
}

#elif defined(ANGLE_PLATFORM_POSIX)

static void handler(int sig)
{
    void *stack[64];
    const int count = backtrace(stack, ArraySize(stack));
    char **symbols  = backtrace_symbols(stack, count);

    printf("\nSignal %d [%s]:\n", sig, strsignal(sig));
    for (int i = 0; i < count; i++)
    {
        Dl_info info;
        if (dladdr(stack[i], &info) && info.dli_sname)
        {
            char demangled[256];
            size_t len = ArraySize(demangled);
            int ok;

            abi::__cxa_demangle(info.dli_sname, demangled, &len, &ok);
            if (ok == 0)
            {
                printf("    %s\n", demangled);
                continue;
            }
        }
        printf("    %s\n", symbols[i]);
    }

    // Exit NOW.  Don't notify other threads, don't call anything registered with atexit().
    _Exit(sig);
}

#endif

#if defined(ANGLE_PLATFORM_APPLE) || defined(ANGLE_PLATFORM_POSIX)

void SetupCrashHandler()
{
    static const int kSignals[] = {
        SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGTRAP,
    };

    for (size_t i = 0; i < sizeof(kSignals) / sizeof(kSignals[0]); i++)
    {
        // Register our signal handler unless something's already done so (e.g. catchsegv).
        void (*prev)(int) = signal(kSignals[i], handler);
        if (prev != SIG_DFL)
        {
            signal(kSignals[i], prev);
        }
    }
}

#elif defined(ANGLE_PLATFORM_WINDOWS)

// Defined in system_utils_win.cpp.

#else

void SetupCrashHandler() {}

#endif
