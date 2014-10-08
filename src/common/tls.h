//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// tls.h: Simple cross-platform interface for thread local storage.

#ifndef COMMON_TLS_H_
#define COMMON_TLS_H_

#include "common/platform.h"

#ifdef ANGLE_PLATFORM_WINDOWS

// TLS does not exist for Windows Store and needs to be emulated
#   ifdef ANGLE_ENABLE_WINDOWS_STORE
#       define TLS_OUT_OF_INDEXES -1
#       ifndef CREATE_SUSPENDED
#           define CREATE_SUSPENDED 0x00000004
#       endif

        void Sleep(_In_ unsigned long dwMilliseconds);
        unsigned long TlsAlloc();
        int TlsFree(_In_ unsigned long dwTlsIndex);
        void* TlsGetValue(_In_ unsigned long dwTlsIndex);
        int TlsSetValue(_In_ unsigned long dwTlsIndex, _In_opt_ void* lpTlsValue);
        void TlsShutdown();

#   endif
    typedef DWORD TLSIndex;
#   define TLS_INVALID_INDEX (TLS_OUT_OF_INDEXES)
#elif defined(ANGLE_PLATFORM_POSIX)
#   include <pthread.h>
#   include <semaphore.h>
#   include <errno.h>
    typedef pthread_key_t TLSIndex;
#   define TLS_INVALID_INDEX (static_cast<TLSIndex>(-1))
#else
#   error Unsupported platform.
#endif

TLSIndex CreateTLSIndex();
bool DestroyTLSIndex(TLSIndex index);

bool SetTLSValue(TLSIndex index, void *value);
void *GetTLSValue(TLSIndex index);

#endif // COMMON_TLS_H_
