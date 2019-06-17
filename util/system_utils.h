//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// system_utils.h: declaration of OS-specific utility functions

#ifndef UTIL_SYSTEM_UTILS_H_
#define UTIL_SYSTEM_UTILS_H_

#include <functional>
#include <string>

#include "common/system_utils.h"
#include "util/util_export.h"

namespace angle
{
// Cross platform equivalent of the Windows Sleep function
ANGLE_UTIL_EXPORT void Sleep(unsigned int milliseconds);

ANGLE_UTIL_EXPORT void SetLowPriorityProcess();

// Write a debug message, either to a standard output or Debug window.
ANGLE_UTIL_EXPORT void WriteDebugMessage(const char *format, ...);

// Set thread affinity and priority.
ANGLE_UTIL_EXPORT bool StabilizeCPUForBenchmarking();

// Set a crash handler to print stack traces.
using CrashCallback = std::function<void()>;
ANGLE_UTIL_EXPORT void InitCrashHandler(CrashCallback *callback);
ANGLE_UTIL_EXPORT void TerminateCrashHandler();

// Print a stack back trace.
ANGLE_UTIL_EXPORT void PrintStackBacktrace();

// Get temporary directory.
ANGLE_UTIL_EXPORT bool GetTempDir(std::string *tempDirOut);

// Creates a temporary file. The full path is placed in |path|, and the
// function returns true if was successful in creating the file. The file will
// be empty and all handles closed after this function returns.
ANGLE_UTIL_EXPORT bool CreateTemporaryFile(std::string *tempFileNameOut);

// Same as CreateTemporaryFile but the file is created in |dir|.
ANGLE_UTIL_EXPORT bool CreateTemporaryFileInDir(const std::string &dir,
                                                std::string *tempFileNameOut);

// Deletes a file or directory.
ANGLE_UTIL_EXPORT bool DeleteFile(const char *path);

// Reads a file contents into a string.
ANGLE_UTIL_EXPORT bool ReadEntireFileToString(const std::string &filePath,
                                              std::string *contentsOut);

}  // namespace angle

#endif  // UTIL_SYSTEM_UTILS_H_
