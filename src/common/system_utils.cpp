//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// system_utils.cpp: Implementation of common functions

#include "common/system_utils.h"

namespace angle
{
#if !defined(ANGLE_PLATFORM_ANDROID)
std::string GetExecutableName()
{
    std::string executableName = GetExecutablePath();
    size_t lastPathSepLoc      = executableName.find_last_of(GetPathSeparator());
    return (lastPathSepLoc > 0 ? executableName.substr(lastPathSepLoc + 1, executableName.length())
                               : "ANGLE");
}
#endif  // ANGLE_PLATFORM_ANDROID

bool PrependPathToEnvironmentVar(const char *variableName, const char *path)
{
    std::string oldValue = GetEnvironmentVar(variableName);
    const char *newValue = nullptr;
    std::string buf;
    if (oldValue.empty())
    {
        newValue = path;
    }
    else
    {
        buf = path;
        buf += GetPathSeparatorForEnvironmentVar();
        buf += oldValue;
        newValue = buf.c_str();
    }
    return SetEnvironmentVar(variableName, newValue);
}
}  // namespace angle
