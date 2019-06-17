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

#include "util/test_utils.h"

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
    Unknown,
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

struct FileLine
{
    const char *file;
    int line;
};

struct ProcessInfo : angle::NonCopyable
{
    ProcessInfo();
    ~ProcessInfo();
    ProcessInfo(ProcessInfo &&other);
    ProcessInfo &operator=(ProcessInfo &&rhs);

    std::unique_ptr<Process> process;
    std::vector<TestIdentifier> testsInBatch;
    std::string resultsFileName;
    std::string filterFileName;
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
    bool launchChildTestProcess(const std::vector<TestIdentifier> &testsInBatch);
    bool finishProcess(ProcessInfo *processInfo);
    int printUnexpectedFailuresAndReturnCount() const;

    std::string mTestExcutableName;
    std::string mTestSuiteName;
    std::vector<TestIdentifier> mTestQueue;
    std::string mFilterString;
    std::string mFilterFile;
    std::string mResultsDirectory;
    std::string mResultsFile;
    int mShardCount;
    int mShardIndex;
    angle::CrashCallback mCrashCallback;
    TestResults mTestResults;
    bool mBotMode;
    int mBatchSize;
    int mCurrentResultCount;
    int mTotalResultCount;
    int mMaxProcesses;
    int mTestTimeout;
    std::map<TestIdentifier, FileLine> mTestFileLines;
    std::vector<ProcessInfo> mCurrentProcesses;
};

}  // namespace angle
