//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// tls.cpp: Simple cross-platform interface for thread local storage.

#include "common/tls.h"

#include <assert.h>

#ifdef ANGLE_ENABLE_WINDOWS_STORE
#include <vector>
#include <set>
#include <map>
#include <mutex>

#include <wrl/client.h>
#include <wrl/async.h>
#include <Windows.System.Threading.h>

using namespace std;
using namespace Windows::Foundation;
using namespace ABI::Windows::System::Threading;
#endif

TLSIndex CreateTLSIndex()
{
    TLSIndex index;

#ifdef ANGLE_PLATFORM_WINDOWS
    index = TlsAlloc();
#elif defined(ANGLE_PLATFORM_POSIX)
    // Create global pool key
    if ((pthread_key_create(&index, NULL)) != 0)
    {
        index = TLS_INVALID_INDEX;
    }
#endif

    assert(index != TLS_INVALID_INDEX && "CreateTLSIndex(): Unable to allocate Thread Local Storage");
    return index;
}

bool DestroyTLSIndex(TLSIndex index)
{
    assert(index != TLS_INVALID_INDEX && "DestroyTLSIndex(): Invalid TLS Index");
    if (index == TLS_INVALID_INDEX)
    {
        return false;
    }

#ifdef ANGLE_PLATFORM_WINDOWS
    return (TlsFree(index) == TRUE);
#elif defined(ANGLE_PLATFORM_POSIX)
    return (pthread_key_delete(index) == 0);
#endif
}

bool SetTLSValue(TLSIndex index, void *value)
{
    assert(index != TLS_INVALID_INDEX && "SetTLSValue(): Invalid TLS Index");
    if (index == TLS_INVALID_INDEX)
    {
        return false;
    }

#ifdef ANGLE_PLATFORM_WINDOWS
    return (TlsSetValue(index, value) == TRUE);
#elif defined(ANGLE_PLATFORM_POSIX)
    return (pthread_setspecific(index, value) == 0);
#endif
}

void *GetTLSValue(TLSIndex index)
{
    assert(index != TLS_INVALID_INDEX && "GetTLSValue(): Invalid TLS Index");
    if (index == TLS_INVALID_INDEX)
    {
        return NULL;
    }

#ifdef ANGLE_PLATFORM_WINDOWS
    return TlsGetValue(index);
#elif defined(ANGLE_PLATFORM_POSIX)
    return pthread_getspecific(index);
#endif
}

#ifdef ANGLE_ENABLE_WINDOWS_STORE
// Thread local storage.
typedef vector<void*> ThreadLocalData;

static __declspec(thread) ThreadLocalData* currentThreadData = nullptr;
static set<ThreadLocalData*> allThreadData;
static DWORD nextTlsIndex = 0;
static vector<DWORD> freeTlsIndices;
static mutex tlsAllocationLock;

void Sleep(unsigned long dwMilliseconds)
{
    static HANDLE singletonEvent = nullptr;

    HANDLE sleepEvent = singletonEvent;

    // Demand create the event.
    if (!sleepEvent)
    {
        sleepEvent = CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);

        if (!sleepEvent)
            return;

        HANDLE previousEvent = InterlockedCompareExchangePointerRelease(&singletonEvent, sleepEvent, nullptr);

        if (previousEvent)
        {
            // Back out if multiple threads try to demand create at the same time.
            CloseHandle(sleepEvent);
            sleepEvent = previousEvent;
        }
    }

    // Emulate sleep by waiting with timeout on an event that is never signalled.
    WaitForSingleObjectEx(sleepEvent, dwMilliseconds, false);
}

unsigned long TlsAlloc()
{
    // Can we reuse a previously freed TLS slot?
    if (!freeTlsIndices.empty())
    {
        DWORD result = freeTlsIndices.back();
        freeTlsIndices.pop_back();
        return result;
    }

    // Allocate a new TLS slot.
    return nextTlsIndex++;
}

int TlsFree(unsigned long dwTlsIndex)
{
    assert(dwTlsIndex < nextTlsIndex);
    assert(find(freeTlsIndices.begin(), freeTlsIndices.end(), dwTlsIndex) == freeTlsIndices.end());

    // Store this slot for reuse by TlsAlloc.
    try
    {
        freeTlsIndices.push_back(dwTlsIndex);
    }
    catch (...)
    {
        return false;
    }

    // Zero the value for all threads that might be using this now freed slot.
    for each (auto threadData in allThreadData)
    {
        if (threadData->size() > dwTlsIndex)
        {
            threadData->at(dwTlsIndex) = nullptr;
        }
    }

    return true;
}

void* TlsGetValue(unsigned long dwTlsIndex)
{
    ThreadLocalData* threadData = currentThreadData;

    if (threadData && threadData->size() > dwTlsIndex)
    {
        // Return the value of an allocated TLS slot.
        return threadData->at(dwTlsIndex);
    }
    else
    {
        // Default value for unallocated slots.
        return nullptr;
    }
}

int TlsSetValue(unsigned long dwTlsIndex, void* lpTlsValue)
{
    ThreadLocalData* threadData = currentThreadData;

    if (!threadData)
    {
        // First time allocation of TLS data for this thread.
        try
        {
            threadData = new ThreadLocalData(dwTlsIndex + 1, nullptr);
            allThreadData.insert(threadData);
            currentThreadData = threadData;
        }
        catch (...)
        {
            if (threadData)
                delete threadData;

            return false;
        }
    }
    else if (threadData->size() <= dwTlsIndex)
    {
        // This thread already has a TLS data block, but it must be expanded to fit the specified slot.
        try
        {
            threadData->resize(dwTlsIndex + 1, nullptr);
        }
        catch (...)
        {
            return false;
        }
    }

    // Store the new value for this slot.
    threadData->at(dwTlsIndex) = lpTlsValue;

    return true;
}

// Called at thread exit to clean up TLS allocations.
void TlsShutdown()
{
    ThreadLocalData* threadData = currentThreadData;

    if (threadData)
    {
        {
            lock_guard<mutex> lock(tlsAllocationLock);
            allThreadData.erase(threadData);
        }

        currentThreadData = nullptr;

        delete threadData;
    }
}
#endif
