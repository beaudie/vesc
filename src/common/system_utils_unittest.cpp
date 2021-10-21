//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// system_utils_unittest.cpp: Unit tests for ANGLE's system utility functions

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "common/mathutil.h"
#include "common/system_utils.h"

using namespace angle;

namespace
{
// Test getting the executable path
TEST(SystemUtils, ExecutablePath)
{
    // TODO: fuchsia support. http://anglebug.com/3161
#if !defined(ANGLE_PLATFORM_FUCHSIA)
    std::string executablePath = GetExecutablePath();
    EXPECT_NE("", executablePath);
#endif
}

// Test getting the executable directory
TEST(SystemUtils, ExecutableDir)
{
    // TODO: fuchsia support. http://anglebug.com/3161
#if !defined(ANGLE_PLATFORM_FUCHSIA)
    std::string executableDir = GetExecutableDirectory();
    EXPECT_NE("", executableDir);

    std::string executablePath = GetExecutablePath();
    EXPECT_LT(executableDir.size(), executablePath.size());
    EXPECT_EQ(0, strncmp(executableDir.c_str(), executablePath.c_str(), executableDir.size()));
#endif
}

// Test setting environment variables
TEST(SystemUtils, Environment)
{
    constexpr char kEnvVarName[]  = "UNITTEST_ENV_VARIABLE";
    constexpr char kEnvVarValue[] = "The quick brown fox jumps over the lazy dog";

    bool setEnvDone = SetEnvironmentVar(kEnvVarName, kEnvVarValue);
    EXPECT_TRUE(setEnvDone);

    std::string readback = GetEnvironmentVar(kEnvVarName);
    EXPECT_EQ(kEnvVarValue, readback);

    bool unsetEnvDone = UnsetEnvironmentVar(kEnvVarName);
    EXPECT_TRUE(unsetEnvDone);

    readback = GetEnvironmentVar(kEnvVarName);
    EXPECT_EQ("", readback);
}

#if defined(ANGLE_PLATFORM_POSIX)
TEST(SystemUtils, ConcatenatePathSimple)
{
    std::string path1    = "/this/is/path1";
    std::string path2    = "this/is/path2";
    std::string expected = "/this/is/path1/this/is/path2";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, ConcatenatePath1Empty)
{
    std::string path1    = "";
    std::string path2    = "this/is/path2";
    std::string expected = "this/is/path2";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, ConcatenatePath2Empty)
{
    std::string path1    = "/this/is/path1";
    std::string path2    = "";
    std::string expected = "/this/is/path1";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, ConcatenatePath2FullPath)
{
    std::string path1    = "/this/is/path1";
    std::string path2    = "/this/is/path2";
    std::string expected = "/this/is/path1";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, ConcatenatePathRedundantSeparators)
{
    std::string path1    = "/this/is/path1/";
    std::string path2    = "this/is/path2";
    std::string expected = "/this/is/path1/this/is/path2";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, IsFullPath)
{
    std::string path1 = "/this/is/path1/";
    std::string path2 = "this/is/path2";
    EXPECT_TRUE(IsFullPath(path1));
    EXPECT_FALSE(IsFullPath(path2));
}
#elif defined(ANGLE_PLATFORM_WINDOWS)
TEST(SystemUtils, ConcatenatePathSimple)
{
    std::string path1    = "C:\\this\\is\\path1";
    std::string path2    = "this\\is\\path2";
    std::string expected = "C:\\this\\is\\path1\\this\\is\\path2";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, ConcatenatePath1Empty)
{
    std::string path1    = "";
    std::string path2    = "this\\is\\path2";
    std::string expected = "this\\is\\path2";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, ConcatenatePath2Empty)
{
    std::string path1    = "C:\\this\\is\\path1";
    std::string path2    = "";
    std::string expected = "C:\\this\\is\\path1";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, ConcatenatePath2FullPath)
{
    std::string path1    = "C:\\this\\is\\path1";
    std::string path2    = "C:\\this\\is\\path2";
    std::string expected = "C:\\this\\is\\path1";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, ConcatenatePathRedundantSeparators)
{
    std::string path1    = "C:\\this\\is\\path1\\";
    std::string path2    = "this\\is\\path2";
    std::string expected = "C:\\this\\is\\path1\\this\\is\\path2";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, ConcatenatePathRedundantSeparators2)
{
    std::string path1    = "C:\\this\\is\\path1\\";
    std::string path2    = "\\this\\is\\path2";
    std::string expected = "C:\\this\\is\\path1\\this\\is\\path2";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, ConcatenatePathRedundantSeparators3)
{
    std::string path1    = "C:\\this\\is\\path1";
    std::string path2    = "\\this\\is\\path2";
    std::string expected = "C:\\this\\is\\path1\\this\\is\\path2";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, IsFullPath)
{
    std::string path1 = "C:\\this\\is\\path1\\";
    std::string path2 = "this\\is\\path2";
    EXPECT_TRUE(IsFullPath(path1));
    EXPECT_FALSE(IsFullPath(path2));
}
#endif

TEST(SystemUtils, PageSize)
{
    size_t pageSize = GetPageSize();
    EXPECT_TRUE(pageSize > 0);
}

TEST(SystemUtils, PageFaultHandlerInit)
{
    PageFaultCallback callback = [](uintptr_t address) { return SignalRangeType::InRange; };

    PageFaultHandler *handler = CreatePageFaultHandler(callback);

    EXPECT_TRUE(handler->enable());
    EXPECT_TRUE(handler->disable());

    delete handler;
}

TEST(SystemUtils, PageFaultHandlerProtect)
{
    size_t pageSize = GetPageSize();
    EXPECT_TRUE(pageSize > 0);

    constexpr size_t numElements = 100;
    float *data                  = new float[numElements];
    size_t dataSize              = sizeof(float) * numElements;
    uintptr_t dataStart          = reinterpret_cast<uintptr_t>(data);
    uintptr_t protectionStart    = rx::roundDownPow2(dataStart, pageSize);
    uintptr_t protectionEnd      = rx::roundUpPow2(dataStart + dataSize, pageSize);

    bool handlerCalled = false;

    PageFaultCallback callback = [pageSize, dataStart, dataSize,
                                  &handlerCalled](uintptr_t address) {
        if (address >= dataStart && address < dataStart + dataSize)
        {
            uintptr_t pageStart = rx::roundDownPow2(address, pageSize);
            EXPECT_TRUE(UnprotectMemory(pageStart, pageSize));
            handlerCalled = true;
            return SignalRangeType::InRange;
        }
        else
        {
            return SignalRangeType::OutOfRange;
        }
    };

    PageFaultHandler *handler = CreatePageFaultHandler(callback);
    handler->enable();

    size_t protectionSize = protectionEnd - protectionStart;

    // Test Protect
    EXPECT_TRUE(ProtectMemory(protectionStart, protectionSize));

    data[0] = 0.0;

    EXPECT_TRUE(handlerCalled);

    // Test Protect and unprotect
    EXPECT_TRUE(ProtectMemory(protectionStart, protectionSize));
    EXPECT_TRUE(UnprotectMemory(protectionStart, protectionSize));

    handlerCalled = false;
    data[0]       = 0.0;
    EXPECT_FALSE(handlerCalled);

    // Clean up
    EXPECT_TRUE(handler->disable());

    delete[] data;
    delete handler;
}

}  // anonymous namespace
