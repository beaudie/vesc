//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// OSX_system_utils.cpp: Implementation of OS-specific functions for OSX

#include "system_utils.h"

#include <cstdlib>
#include <mach-o/dyld.h>

namespace angle
{

std::string GetExecutablePath()
{
    std::string result;

    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);

    char* buffer = new char[size + 1];
    if (!buffer)
    {
        return "";
    }

    _NSGetExecutablePath(buffer, &size);
    buffer[size] = '\0';

    if (!strrchr(buffer, '/'))
    {
        result = "";
    }
    else
    {
        result = buffer;
    }

    delete[] buffer;
    return result;
}

std::string GetExecutableDirectory()
{
    std::string executablePath = GetExecutablePath();
    size_t lastPathSepLoc = executablePath.find_last_of("/");
    return (lastPathSepLoc != std::string::npos) ? executablePath.substr(0, lastPathSepLoc) : "";
}

} // namespace angle
