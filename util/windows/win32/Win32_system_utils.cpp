//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Win32_system_utils.cpp: Implementation of OS-specific functions for Win32 (Windows)

#include "util/system_utils.h"

#include <windows.h>
#include <array>

#include "util/random_utils.h"

namespace angle
{
namespace
{
// Returns the Win32 last error code or ERROR_SUCCESS if the last error code is
// ERROR_FILE_NOT_FOUND or ERROR_PATH_NOT_FOUND. This is useful in cases where
// the absence of a file or path is a success condition (e.g., when attempting
// to delete an item in the filesystem).
bool ReturnSuccessOnNotFound()
{
    const DWORD error_code = ::GetLastError();
    return (error_code == ERROR_FILE_NOT_FOUND || error_code == ERROR_PATH_NOT_FOUND);
}
}  // namespace

void SetLowPriorityProcess()
{
    SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
}

bool StabilizeCPUForBenchmarking()
{
    if (::SetThreadAffinityMask(::GetCurrentThread(), 1) == 0)
    {
        return false;
    }
    if (::SetPriorityClass(::GetCurrentProcess(), REALTIME_PRIORITY_CLASS) == FALSE)
    {
        return false;
    }
    if (::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL) == FALSE)
    {
        return false;
    }

    return true;
}

bool GetTempDir(std::string *tempDirOut)
{
    char tempPath[MAX_PATH + 1];
    DWORD pathLen = ::GetTempPathA(MAX_PATH, tempPath);
    if (pathLen >= MAX_PATH || pathLen <= 0)
        return false;
    *tempDirOut = tempPath;
    return true;
}

bool CreateTemporaryFileInDir(const std::string &dir, std::string *tempFileNameOut)
{
    char fileName[MAX_PATH + 1];
    if (::GetTempFileNameA(dir.c_str(), "ANGLE", 0, fileName) == 0)
        return false;

    *tempFileNameOut = fileName;
    return true;
}

bool DeleteFile(const char *path)
{
    if (strlen(path) >= MAX_PATH)
        return false;

    const DWORD attr = ::GetFileAttributesA(path);
    // Report success if the file or path does not exist.
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        return ReturnSuccessOnNotFound();
    }

    // Clear the read-only bit if it is set.
    if ((attr & FILE_ATTRIBUTE_READONLY) &&
        !::SetFileAttributesA(path, attr & ~FILE_ATTRIBUTE_READONLY))
    {
        // It's possible for |path| to be gone now under a race with other deleters.
        return ReturnSuccessOnNotFound();
    }

    // We don't handle directories right now.
    if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0)
    {
        return false;
    }

    return !!::DeleteFileA(path) ? true : ReturnSuccessOnNotFound();
}
}  // namespace angle
