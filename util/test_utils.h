//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// test_utils.h: declaration of OS-specific utility functions

#ifndef UTIL_TEST_UTILS_H_
#define UTIL_TEST_UTILS_H_

#include <string>
#include <vector>

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
ANGLE_UTIL_EXPORT void InitCrashHandler();
ANGLE_UTIL_EXPORT void TerminateCrashHandler();

// Print a stack back trace.
ANGLE_UTIL_EXPORT void PrintStackBacktrace();

// Run an application and get the output.  Gets a nullptr-terminated set of args to execute the
// application with, and returns the stdout and stderr outputs as well as the exit code.
//
// Pass nullptr for stdoutOut/stderrOut if you don't need to capture. exitCodeOut is required.
//
// Returns false if it fails to actually execute the application.
ANGLE_UTIL_EXPORT bool RunApp(const std::vector<const char *> &args,
                              std::string *stdoutOut,
                              std::string *stderrOut,
                              int *exitCodeOut);

}  // namespace angle

#endif  // UTIL_TEST_UTILS_H_
