//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// systeminfo.cpp: Implementation for system status queries.
//   Based on chromium's platform/win/SystemInfo.cpp
//

#include "libGLESv2/systeminfo.h"

#if _WIN32_WINNT_WINBLUE
#include <versionhelpers.h>
#endif

#include <windows.h>

namespace rx
{

#ifndef _WIN32_WINNT_WINBLUE
static bool IsWindowsVistaOrGreater()
{
    OSVERSIONINFOEXW osvi = {};
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.dwMajorVersion = HIBYTE(_WIN32_WINNT_VISTA);
    osvi.dwMinorVersion = LOBYTE(_WIN32_WINNT_VISTA);
    DWORDLONG conditoin = 0;
    VER_SET_CONDITION(conditoin, VER_MAJORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(conditoin, VER_MINORVERSION, VER_GREATER_EQUAL);
    return !!::VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION, conditoin);
}
#endif

bool isWindowsVistaOrGreater()
{
    static bool initialized = false;
    static bool cachedIsWindowsVistaOrGreater;

    if (!initialized) {
        initialized = true;
        cachedIsWindowsVistaOrGreater = IsWindowsVistaOrGreater();
    }
    return cachedIsWindowsVistaOrGreater;
}

}
