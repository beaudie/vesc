//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// crash_handler_posix:
//    ANGLE's crash handling and stack walking code. Modified from Skia's:
//     https://github.com/google/skia/blob/master/tools/CrashHandler.cpp
//

#include "util/test_utils.h"

#include "common/angleutils.h"
#include "common/system_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#if !defined(ANGLE_PLATFORM_ANDROID) && !defined(ANGLE_PLATFORM_FUCHSIA)
#    if defined(ANGLE_PLATFORM_APPLE)
// We only use local unwinding, so we can define this to select a faster implementation.
#        define UNW_LOCAL_ONLY
#        include <cxxabi.h>
#        include <libunwind.h>
#        include <signal.h>
#    elif defined(ANGLE_PLATFORM_POSIX)
// We'd use libunwind here too, but it's a pain to get installed for
// both 32 and 64 bit on bots.  Doesn't matter much: catchsegv is best anyway.
#        include <cxxabi.h>
#        include <dlfcn.h>
#        include <execinfo.h>
#        include <libgen.h>
#        include <signal.h>
#        include <string.h>
#    endif  // defined(ANGLE_PLATFORM_APPLE)
#endif      // !defined(ANGLE_PLATFORM_ANDROID) && !defined(ANGLE_PLATFORM_FUCHSIA)

namespace angle
{
#if defined(ANGLE_PLATFORM_ANDROID) || defined(ANGLE_PLATFORM_FUCHSIA)

void PrintStackBacktrace()
{
    // No implementations yet.
}

void InitCrashHandler(CrashCallback *callback)
{
    // No implementations yet.
}

void TerminateCrashHandler()
{
    // No implementations yet.
}

#else
namespace
{
CrashCallback *gCrashHandlerCallback;
}  // namespace

#    if defined(ANGLE_PLATFORM_APPLE)

void PrintStackBacktrace()
{
    printf("Backtrace:\n");

    unw_context_t context;
    unw_getcontext(&context);

    unw_cursor_t cursor;
    unw_init_local(&cursor, &context);

    while (unw_step(&cursor) > 0)
    {
        static const size_t kMax = 256;
        char mangled[kMax], demangled[kMax];
        unw_word_t offset;
        unw_get_proc_name(&cursor, mangled, kMax, &offset);

        int ok;
        size_t len = kMax;
        abi::__cxa_demangle(mangled, demangled, &len, &ok);

        printf("    %s (+0x%zx)\n", ok == 0 ? demangled : mangled, (size_t)offset);
    }
    printf("\n");
}

static void Handler(int sig)
{
    if (gCrashHandlerCallback)
    {
        (*gCrashHandlerCallback)();
    }

    printf("\nSignal %d:\n", sig);
    PrintStackBacktrace();

    // Exit NOW.  Don't notify other threads, don't call anything registered with atexit().
    _Exit(sig);
}

#    elif defined(ANGLE_PLATFORM_POSIX)

// Can control this at a higher level if required.
#        define ANGLE_HAS_ADDR2LINE

void PrintStackBacktrace()
{
    printf("Backtrace:\n");

    void *stack[64];
    const int count = backtrace(stack, ArraySize(stack));
    char **symbols  = backtrace_symbols(stack, count);

    for (int i = 0; i < count; i++)
    {
#        if defined(ANGLE_HAS_ADDR2LINE)
        std::string module(symbols[i], strchr(symbols[i], '('));

        // We need an absolute path to get to the executable and all of the various shared objects,
        // but the caller may have used a relative path to launch the executable, so build one up if
        // we don't see a leading '/'.
        if (module.at(0) != GetPathSeparator())
        {
            // https://stackoverflow.com/a/23943306
            char result[PATH_MAX];
            ssize_t pathLength = readlink("/proc/self/exe", result, PATH_MAX);
            const char *path;
            if (pathLength != -1)
            {
                path                  = dirname(result);
                size_t lastPathSepLoc = module.find_last_of(GetPathSeparator());
                module                = path + module.substr(lastPathSepLoc, module.length());
            }
            else
            {
                std::cout << "Failed to get executable's path.";
            }
        }

        std::string substring(strchr(symbols[i], '+') + 1, strchr(symbols[i], ')'));
        std::string command = "addr2line -s -p -f -C -e " + module + " " + substring;
        (void)system(command.c_str());
#        else
        Dl_info info;
        if (dladdr(stack[i], &info) && info.dli_sname)
        {
            // Make sure this is large enough to hold the fully demangled names, otherwise we could
            // segault/hang here. For example, Vulkan validation layer errors can be deep enough
            // into the stack that very large symbol names are generated.
            char demangled[4096];
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
#        endif  // defined(ANGLE_HAS_ADDR2LINE)
    }
}

static void Handler(int sig)
{
    if (gCrashHandlerCallback)
    {
        (*gCrashHandlerCallback)();
    }

    printf("\nSignal %d [%s]:\n", sig, strsignal(sig));
    PrintStackBacktrace();

    // Exit NOW.  Don't notify other threads, don't call anything registered with atexit().
    _Exit(sig);
}

#    endif  // defined(ANGLE_PLATFORM_APPLE)

static constexpr int kSignals[] = {
    SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGTRAP,
};

void InitCrashHandler(CrashCallback *callback)
{
    gCrashHandlerCallback = callback;
    for (int sig : kSignals)
    {
        // Register our signal handler unless something's already done so (e.g. catchsegv).
        void (*prev)(int) = signal(sig, Handler);
        if (prev != SIG_DFL)
        {
            signal(sig, prev);
        }
    }
}

void TerminateCrashHandler()
{
    gCrashHandlerCallback = nullptr;
    for (int sig : kSignals)
    {
        void (*prev)(int) = signal(sig, SIG_DFL);
        if (prev != Handler && prev != SIG_DFL)
        {
            signal(sig, prev);
        }
    }
}

#endif  // defined(ANGLE_PLATFORM_ANDROID) || defined(ANGLE_PLATFORM_FUCHSIA)

}  // namespace angle
