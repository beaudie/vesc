//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TestSuite:
//   Basic implementation of a test harness in ANGLE.

#include <map>
#include <memory>
#include <string>

#include "util/system_utils.h"

namespace angle
{
struct TestIdentifier
{
    TestIdentifier();
    TestIdentifier(const TestIdentifier &other);
    ~TestIdentifier();

    TestIdentifier &operator=(const TestIdentifier &other);

    bool valid() const { return !testName.empty(); }

    std::string testCaseName;
    std::string testName;
    std::string file;
    int line;
};

inline bool operator<(const TestIdentifier &a, const TestIdentifier &b)
{
    return std::tie(a.testCaseName, a.testName) < std::tie(b.testCaseName, b.testName);
}

enum class TestResultType
{
    Crash,
    Fail,
    Skip,
    Pass,
    Timeout,
};

struct TestResult
{
    TestResultType type;
    double elapsedTimeSeconds;
};

struct TestResults
{
    TestResults();
    ~TestResults();

    std::map<TestIdentifier, TestResult> results;
    TestIdentifier currentTest;
};

class TestSuite
{
  public:
    TestSuite(int *argc, char **argv);
    ~TestSuite();

    int run();
    void onCrash();

  private:
    bool parseSingleArg(const char *argument);

    std::string mTestExcutableName;
    std::string mTestSuiteName;
    std::string mFilterString;
    const char *mResultsDirectory;
    int mShardCount;
    int mShardIndex;
    angle::CrashCallback mCrashCallback;
    TestResults mTestResults;
    bool mBotMode;
    int mBatchSize;
};

}  // namespace angle
