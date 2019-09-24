//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// test_utils_unittest.cpp: Unit tests for ANGLE's test utility functions

#include "gtest/gtest.h"

#include "common/system_utils.h"
#include "util/test_utils.h"
#include "util/test_utils_unittest_helper.h"

using namespace angle;

namespace
{
#if defined(ANGLE_PLATFORM_WINDOWS)
constexpr char kRunAppHelperExecutable[] = "angle_unittests_helper.exe";
#else
constexpr char kRunAppHelperExecutable[] = "angle_unittests_helper";
#endif

// Transforms various line endings into C/Unix line endings:
//
// - A\nB -> A\nB
// - A\rB -> A\nB
// - A\r\nB -> A\nB
std::string NormalizeNewLines(const std::string &str)
{
    std::string result;

    for (size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] == '\r')
        {
            if (i + 1 < str.size() && str[i + 1] == '\n')
            {
                ++i;
            }
            result += '\n';
        }
        else
        {
            result += str[i];
        }
    }

    return result;
}

// Test running an external application and receiving its output
TEST(TestUtils, RunApp)
{
#if defined(ANGLE_PLATFORM_ANDROID)
    // TODO: android support. http://anglebug.com/3125
    return;
#endif

#if defined(ANGLE_PLATFORM_FUCHSIA)
    // TODO: fuchsia support. http://anglebug.com/3161
    return;
#endif

    std::string executablePath = GetExecutableDirectory();
    EXPECT_NE(executablePath, "");
    executablePath += "/";
    executablePath += kRunAppHelperExecutable;

    std::vector<const char *> args = {executablePath.c_str(), kRunAppTestArg1, kRunAppTestArg2,
                                      nullptr};

    // Test that the application can be executed.
    Process *process = LaunchProcess(args, true, true);
    EXPECT_TRUE(process->started());
    EXPECT_TRUE(process->finish());

    std::string stdoutOutput;
    std::string stderrOutput;
    int exitCode = process->getExitCode();
    process->getStdout(&stdoutOutput);
    process->getStderr(&stderrOutput);

    EXPECT_EQ(kRunAppTestStdout, NormalizeNewLines(stdoutOutput));
    EXPECT_EQ(kRunAppTestStderr, NormalizeNewLines(stderrOutput));
    EXPECT_EQ(EXIT_SUCCESS, exitCode);

    Process::Delete(&process);

    // Test that environment variables reach the cild.
    bool setEnvDone = SetEnvironmentVar(kRunAppTestEnvVarName, kRunAppTestEnvVarValue);
    EXPECT_TRUE(setEnvDone);

    process = LaunchProcess(args, true, true);
    EXPECT_TRUE(process->started());
    EXPECT_TRUE(process->finish());

    exitCode = process->getExitCode();
    process->getStdout(&stdoutOutput);
    process->getStderr(&stderrOutput);

    EXPECT_EQ("", stdoutOutput);
    EXPECT_EQ(kRunAppTestEnvVarValue, NormalizeNewLines(stderrOutput));
    EXPECT_EQ(EXIT_SUCCESS, exitCode);
}
}  // namespace
