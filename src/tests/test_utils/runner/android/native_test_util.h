// Copyright (c) 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ANGLE_TESTS_TEST_UTILS_RUNNER_ANDROID_NATIVE_TEST_UTIL_H_
#define ANGLE_TESTS_TEST_UTILS_RUNNER_ANDROID_NATIVE_TEST_UTIL_H_

#include <stdio.h>
#include <string>
#include <vector>

// Helper methods for setting up environment for running gtest tests
// inside an APK.
namespace angle
{

class ScopedMainEntryLogger
{
  public:
    ScopedMainEntryLogger() { printf(">>ScopedMainEntryLogger\n"); }

    ~ScopedMainEntryLogger()
    {
        printf("<<ScopedMainEntryLogger\n");
        fflush(stdout);
        fflush(stderr);
    }
};

void ParseArgsFromString(const std::string &command_line, std::vector<std::string> *args);
void ParseArgsFromCommandLineFile(const std::string &path, std::vector<std::string> *args);
int ArgsToArgv(const std::vector<std::string> &args, std::vector<char *> *argv);

}  // namespace angle

#endif  // ANGLE_TESTS_TEST_UTILS_RUNNER_ANDROID_NATIVE_TEST_UTIL_H_
