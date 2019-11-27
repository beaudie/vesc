//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TestSuite:
//   Basic implementation of a test harness in ANGLE.

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "util/test_utils.h"

namespace angle
{
struct TestIdentifier
{
    TestIdentifier();
    TestIdentifier(const std::string &caseNameIn, const std::string &nameIn);
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

inline bool operator==(const TestIdentifier &a, const TestIdentifier &b)
{
    return std::tie(a.testCaseName, a.testName) == std::tie(b.testCaseName, b.testName);
}

inline std::ostream &operator<<(std::ostream &os, const TestIdentifier &id)
{
    return os << id.testCaseName << "." << id.testName;
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

const char *TestResultTypeToString(TestResultType type);

struct TestResult
{
    TestResultType type;
    double elapsedTimeSeconds;
};

inline bool operator==(const TestResult &a, const TestResult &b)
{
    return a.type == b.type;
}

inline std::ostream &operator<<(std::ostream &os, const TestResult &result)
{
    return os << TestResultTypeToString(result.type);
}

struct TestResults
{
    TestResults();
    ~TestResults();

    std::map<TestIdentifier, TestResult> results;
    std::mutex currentTestMutex;
    TestIdentifier currentTest;
    Timer currentTestTimer;
    bool allDone = false;
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

    ProcessHandle process;
    std::vector<TestIdentifier> testsInBatch;
    std::string resultsFileName;
    std::string filterFileName;
    std::string commandLine;
};

class TestSuite
{
  public:
    TestSuite(int *argc, char **argv);
    ~TestSuite();

    int run();
    void onCrashOrTimeout(TestResultType crashOrTimeout);

  private:
    bool parseSingleArg(const char *argument);
    bool launchChildTestProcess(const std::vector<TestIdentifier> &testsInBatch);
    bool finishProcess(ProcessInfo *processInfo);
    int printUnexpectedFailuresAndReturnCount() const;
    void startWatchdog();

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
    int mBatchTimeout;
    std::vector<std::string> mArgv;
    std::map<TestIdentifier, FileLine> mTestFileLines;
    std::vector<ProcessInfo> mCurrentProcesses;
    std::thread mWatchdogThread;
};

bool GetTestResultsFromFile(const char *fileName, TestResults *resultsOut);
}  // namespace angle
