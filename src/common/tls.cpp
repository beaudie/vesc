//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// tls.cpp: Simple cross-platform interface for thread local storage.

#include "common/tls.h"

#include "common/debug.h"

#ifdef ANGLE_ENABLE_WINDOWS_UWP
#    include <map>
#    include <mutex>
#    include <set>
#    include <vector>

#    include <Windows.System.Threading.h>
#    include <wrl/async.h>
#    include <wrl/client.h>

using namespace std;
using namespace Windows::Foundation;
using namespace ABI::Windows::System::Threading;

// Thread local storage for Windows Store support
typedef vector<void *> ThreadLocalData;

static __declspec(thread) ThreadLocalData *currentThreadData = nullptr;
static set<ThreadLocalData *> allThreadData;
static DWORD nextTlsIndex = 0;
static vector<DWORD> freeTlsIndices;

#endif

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

TLSIndex CreateContextTLSIndexFast()
{
    return kContextTlsSlot;
}

bool DestroyContextTLSIndexFast(TLSIndex index)
{
    ANGLE_UNUSED_VARIABLE(index);
    return true;
}

bool SetContextTLSFast(TLSIndex index, void *value)
{
    ASSERT(kContextTlsSlot == index);
    __get_tls()[kContextTlsSlot] = value;
    return true;
}

void *GetContextTLSFast(TLSIndex index)
{
    ASSERT(kContextTlsSlot == index);
    return __get_tls()[kContextTlsSlot];
}
#endif

TLSIndex CreateTLSIndex()
{
    TLSIndex index;

#ifdef ANGLE_PLATFORM_WINDOWS
#    ifdef ANGLE_ENABLE_WINDOWS_UWP
    if (!freeTlsIndices.empty())
    {
        DWORD result = freeTlsIndices.back();
        freeTlsIndices.pop_back();
        index = result;
    }
    else
    {
        index = nextTlsIndex++;
    }
#    else
    index = TlsAlloc();
#    endif

#elif defined(ANGLE_PLATFORM_POSIX)
    // Create global pool key
    if ((pthread_key_create(&index, nullptr)) != 0)
    {
        index = TLS_INVALID_INDEX;
    }
#endif

    ASSERT(index != TLS_INVALID_INDEX &&
           "CreateTLSIndex(): Unable to allocate Thread Local Storage");
    return index;
}

bool DestroyTLSIndex(TLSIndex index)
{
    ASSERT(index != TLS_INVALID_INDEX && "DestroyTLSIndex(): Invalid TLS Index");
    if (index == TLS_INVALID_INDEX)
    {
        return false;
    }

#ifdef ANGLE_PLATFORM_WINDOWS
#    ifdef ANGLE_ENABLE_WINDOWS_UWP
    ASSERT(index < nextTlsIndex);
    ASSERT(find(freeTlsIndices.begin(), freeTlsIndices.end(), index) == freeTlsIndices.end());

    freeTlsIndices.push_back(index);
    for (auto threadData : allThreadData)
    {
        if (threadData->size() > index)
        {
            threadData->at(index) = nullptr;
        }
    }
    return true;
#    else
    return (TlsFree(index) == TRUE);
#    endif
#elif defined(ANGLE_PLATFORM_POSIX)
    return (pthread_key_delete(index) == 0);
#endif
}

bool SetTLSValue(TLSIndex index, void *value)
{
    ASSERT(index != TLS_INVALID_INDEX && "SetTLSValue(): Invalid TLS Index");
    if (index == TLS_INVALID_INDEX)
    {
        return false;
    }

#ifdef ANGLE_PLATFORM_WINDOWS
#    ifdef ANGLE_ENABLE_WINDOWS_UWP
    ThreadLocalData *threadData = currentThreadData;
    if (!threadData)
    {
        threadData = new ThreadLocalData(index + 1, nullptr);
        allThreadData.insert(threadData);
        currentThreadData = threadData;
    }
    else if (threadData->size() <= index)
    {
        threadData->resize(index + 1, nullptr);
    }

    threadData->at(index) = value;
    return true;
#    else
    return (TlsSetValue(index, value) == TRUE);
#    endif
#elif defined(ANGLE_PLATFORM_POSIX)
    return (pthread_setspecific(index, value) == 0);
#endif
}

void *GetTLSValue(TLSIndex index)
{
    ASSERT(index != TLS_INVALID_INDEX && "GetTLSValue(): Invalid TLS Index");
    if (index == TLS_INVALID_INDEX)
    {
        return nullptr;
    }

#ifdef ANGLE_PLATFORM_WINDOWS
#    ifdef ANGLE_ENABLE_WINDOWS_UWP
    ThreadLocalData *threadData = currentThreadData;
    if (threadData && threadData->size() > index)
    {
        return threadData->at(index);
    }
    else
    {
        return nullptr;
    }
#    else
    return TlsGetValue(index);
#    endif
#elif defined(ANGLE_PLATFORM_POSIX)
    return pthread_getspecific(index);
#endif
}

CreateContextTlsIndexImpl createContextTlsIndexImpl   = CreateTLSIndex;
DestroyContextTLSIndexImpl destroyContextTlsIndexImpl = DestroyTLSIndex;
SetContextTlsValueImpl setContextTlsValueImpl         = SetTLSValue;
GetContextTlsValueImpl getContextTlsValueImpl         = GetTLSValue;

void SetPlatformType(bool platformTypeVulkan)
{
#if defined(ANGLE_PLATFORM_ANDROID)
    if (platformTypeVulkan == true)
    {
        createContextTlsIndexImpl  = CreateContextTLSIndexFast;
        destroyContextTlsIndexImpl = DestroyContextTLSIndexFast;
        setContextTlsValueImpl     = SetContextTLSFast;
        getContextTlsValueImpl     = GetContextTLSFast;
        return;
    }
#endif  // ANGLE_PLATFORM_ANDROID

    createContextTlsIndexImpl  = CreateTLSIndex;
    destroyContextTlsIndexImpl = DestroyTLSIndex;
    setContextTlsValueImpl     = SetTLSValue;
    getContextTlsValueImpl     = GetTLSValue;
}

TLSIndex CreateContextTLSIndex()
{
    return createContextTlsIndexImpl();
}

bool DestroyContextTLSIndex(TLSIndex index)
{
    return destroyContextTlsIndexImpl(index);
}

bool SetContextTLSValue(TLSIndex index, void *value)
{
    return setContextTlsValueImpl(index, value);
}

void *GetContextTLSValue(TLSIndex index)
{
    return getContextTlsValueImpl(index);
}
