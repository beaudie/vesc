//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// system_utils_unittest.cpp: Unit tests for ANGLE's system utility functions

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "common/system_utils.h"

using namespace angle;

namespace
{
constexpr char kRunAppTestEnvVarName[]  = "RUN_APP_TEST_ENV";
constexpr char kRunAppTestEnvVarValue[] = "RunAppTest environment variable value\n";
constexpr char kRunAppTestStdout[]      = "RunAppTest stdout test\n";
constexpr char kRunAppTestStderr[] = "RunAppTest stderr test\n  .. that expands multiple lines\n";

// Test getting the executable path
TEST(SystemUtils, ExecutablePath)
{
    std::string executablePath = GetExecutablePath();
    EXPECT_NE(executablePath, "");
}

// Test getting the executable directory
TEST(SystemUtils, ExecutableDir)
{
    std::string executableDir = GetExecutableDirectory();
    EXPECT_NE(executableDir, "");

    std::string executablePath = GetExecutablePath();
    EXPECT_LT(executableDir.size(), executablePath.size());
    EXPECT_EQ(strncmp(executableDir.c_str(), executablePath.c_str(), executableDir.size()), 0);
}

// Test setting environment variables
TEST(SystemUtils, Environment)
{
    constexpr char kEnvVarName[]  = "UNITTEST_ENV_VARIABLE";
    constexpr char kEnvVarValue[] = "The quick brown fox jumps over the lazy dog";

    bool setEnvDone = SetEnvironmentVar(kEnvVarName, kEnvVarValue);
    EXPECT_TRUE(setEnvDone);

    std::string readback = GetEnvironmentVar(kEnvVarName);
    EXPECT_EQ(readback, kEnvVarValue);

    bool unsetEnvDone = UnsetEnvironmentVar(kEnvVarName);
    EXPECT_TRUE(unsetEnvDone);

    readback = GetEnvironmentVar(kEnvVarName);
    EXPECT_EQ(readback, "");
}

TEST(SystemUtils, RunApp)
{
#if defined(ANGLE_PLATFORM_ANDROID)
    // TODO: android support. http://anglebug.com/3125
    return;
#endif

    std::string executablePath = GetExecutablePath();
    EXPECT_NE(executablePath, "");

    std::vector<const char *> args = {executablePath.c_str(),
                                      "--gtest_filter=SystemUtils.RunAppTestTarget", nullptr};

    std::string stdoutOutput;
    std::string stderrOutput;
    int exitCode = EXIT_FAILURE;

    // Test that the application can be executed.
    bool ranApp = RunApp(args, &stdoutOutput, &stderrOutput, &exitCode);
    EXPECT_TRUE(ranApp);
    // Note that stdout includes gtest output as well
    EXPECT_NE(stdoutOutput.find(kRunAppTestStdout), std::string::npos);
    EXPECT_EQ(stderrOutput, kRunAppTestStderr);
    EXPECT_EQ(exitCode, EXIT_SUCCESS);

    // Test that environment variables reach the cild.
    bool setEnvDone = SetEnvironmentVar(kRunAppTestEnvVarName, kRunAppTestEnvVarValue);
    EXPECT_TRUE(setEnvDone);

    ranApp = RunApp(args, &stdoutOutput, &stderrOutput, &exitCode);
    EXPECT_TRUE(ranApp);
    EXPECT_EQ(stderrOutput, kRunAppTestEnvVarValue);
    EXPECT_EQ(exitCode, EXIT_SUCCESS);
}

TEST(SystemUtils, RunAppTestTarget)
{
    std::string env = GetEnvironmentVar(kRunAppTestEnvVarName);
    if (env == "")
    {
        printf("%s", kRunAppTestStdout);
        fprintf(stderr, "%s", kRunAppTestStderr);
    }
    else
    {
        fprintf(stderr, "%s", env.c_str());
    }
}

}  // anonymous namespace
