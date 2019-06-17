//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// test_utils.h: declaration of OS-specific utility functions

#ifndef UTIL_TEST_UTILS_H_
#define UTIL_TEST_UTILS_H_

#include <functional>
#include <string>
#include <vector>

#include "common/angleutils.h"
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

class ANGLE_UTIL_EXPORT Process : angle::NonCopyable
{
  public:
    virtual ~Process() {}

    virtual bool started()                         = 0;
    virtual bool finished()                        = 0;
    virtual bool finish()                          = 0;
    virtual bool kill()                            = 0;
    virtual int getExitCode()                      = 0;
    virtual double getElapsedTimeSeconds()         = 0;
    virtual void getStdout(std::string *stdoutOut) = 0;
    virtual void getStderr(std::string *stderrOut) = 0;
};

ANGLE_UTIL_EXPORT Process *RunProcessAsync(const std::vector<const char *> &args,
                                           bool captureStdout,
                                           bool captureStderr);

ANGLE_UTIL_EXPORT int NumberOfProcessors();

}  // namespace angle

#endif  // UTIL_TEST_UTILS_H_
