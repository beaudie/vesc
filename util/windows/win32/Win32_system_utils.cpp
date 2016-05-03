//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Win32_system_utils.cpp: Implementation of OS-specific functions for Win32 (Windows)

#include "system_utils.h"

#include <windows.h>
#include <array>

namespace angle
{

void SetLowPriorityProcess()
{
    SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
}

void *LoadSymbol(const std::string &moduleName, const std::string &symbolName)
{
    const auto &libraryName = moduleName + ".dll";
    auto library = LoadLibrary(libraryName.c_str());
    if (!library)
    {
        return nullptr;
    }
    return GetProcAddress(library, symbolName.c_str());
}

}  // namespace angle
