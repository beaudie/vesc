//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TestSuite:
//   Basic implementation of a test harness in ANGLE.

#include "TestSuite.h"

#include "common/platform.h"
#include "util/system_utils.h"

#include <time.h>

#include <gtest/gtest.h>
#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>

// We directly call into a function to register the parameterized tests. This saves spinning up
// a subprocess with a new gtest filter.
#include "third_party/googletest/src/googletest/src/gtest-internal-inl.h"

namespace js = rapidjson;

namespace angle
{
namespace
{
// FIXME: non-Windows paths
constexpr char kPathSeparator = '\\';
// FIXME: non-Windows environment vars

const char *ParseFlagValue(const char *flag, const char *argument)
{
    if (strstr(argument, flag) == argument)
    {
        return argument + strlen(flag);
    }

    return nullptr;
}

bool ParseIntArg(const char *flag, const char *argument, int *valueOut)
{
    const char *value = ParseFlagValue(flag, argument);
    if (!value)
    {
        return false;
    }

    char *end            = nullptr;
    const long longValue = strtol(value, &end, 10);

    if (*end != '\0')
    {
        printf("Error parsing integer flag value.\n");
        exit(1);
    }

    if (longValue == LONG_MAX || longValue == LONG_MIN || static_cast<int>(longValue) != longValue)
    {
        printf("OVerflow when parsing integer flag value.\n");
        exit(1);
    }

    *valueOut = static_cast<int>(longValue);
    return true;
}

bool ParseFlag(const char *expected, const char *actual, bool *flagOut)
{
    if (strcmp(expected, actual) == 0)
    {
        *flagOut = true;
        return true;
    }
    return false;
}

bool ParseStringArg(const char *flag, const char *argument, const char **valueOut)
{
    const char *value = ParseFlagValue(flag, argument);
    if (!value)
    {
        return false;
    }

    *valueOut = value;
    return true;
}

void DeleteArg(int *argc, char **argv, int argIndex)
{
    // Shift the remainder of the argv list left by one.  Note that argv has (*argc + 1) elements,
    // the last one always being NULL.  The following loop moves the trailing NULL element as well.
    for (int index = argIndex; index < *argc; ++index)
    {
        argv[index] = argv[index + 1];
    }
    (*argc)--;
}

void AddArg(int *argc, char **argv, const char *arg)
{
    // This unsafe const_cast is necessary to work around gtest limitations.
    argv[*argc]     = const_cast<char *>(arg);
    argv[*argc + 1] = nullptr;
    (*argc)++;
}

const char *ResultTypeToString(TestResultType type)
{
    switch (type)
    {
        case TestResultType::Crash:
            return "CRASH";
        case TestResultType::Fail:
            return "FAIL";
        case TestResultType::Pass:
            return "PASS";
        case TestResultType::Skip:
            return "SKIP";
        case TestResultType::Timeout:
            return "TIMEOUT";
    }
}

js::Value ResultTypeToJSString(TestResultType type, js::Document::AllocatorType *allocator)
{
    js::Value jsName;
    jsName.SetString(ResultTypeToString(type), *allocator);
    return jsName;
}

void WriteTestResults(bool interrupted,
                      const TestResults &testResults,
                      const char *outputDirectory,
                      const char *testSuiteName)
{
    time_t ltime;
    time(&ltime);
    struct tm *timeinfo = gmtime(&ltime);
    ltime               = mktime(timeinfo);

    js::Document doc;
    doc.SetObject();

    js::Document::AllocatorType &allocator = doc.GetAllocator();

    doc.AddMember("interrupted", interrupted, allocator);
    doc.AddMember("path_delimiter", ".", allocator);
    doc.AddMember("version", 3, allocator);
    doc.AddMember("seconds_since_epoch", ltime, allocator);

    js::Value testSuite;
    testSuite.SetObject();

    std::map<TestResultType, uint32_t> counts;

    for (const auto &resultIter : testResults.results)
    {
        const TestIdentifier &id = resultIter.first;
        const TestResult &result = resultIter.second;

        js::Value jsResult;
        jsResult.SetObject();

        counts[result.type]++;

        jsResult.AddMember("expected", "PASS", allocator);
        jsResult.AddMember("actual", ResultTypeToJSString(result.type, &allocator), allocator);

        js::Value times;
        times.SetArray();
        times.PushBack(result.elapsedTimeSeconds, allocator);

        jsResult.AddMember("times", times, allocator);

        char testName[500];
        sprintf(testName, "%s.%s", id.testCaseName.c_str(), id.testName.c_str());
        js::Value jsName;
        jsName.SetString(testName, allocator);

        testSuite.AddMember(jsName, jsResult, allocator);
    }

    js::Value numFailuresByType;
    numFailuresByType.SetObject();

    for (const auto &countIter : counts)
    {
        TestResultType type = countIter.first;
        uint32_t count      = countIter.second;

        js::Value jsCount(count);
        numFailuresByType.AddMember(ResultTypeToJSString(type, &allocator), jsCount, allocator);
    }

    doc.AddMember("num_failures_by_type", numFailuresByType, allocator);

    js::Value tests;
    tests.SetObject();
    tests.AddMember(js::StringRef(testSuiteName), testSuite, allocator);

    doc.AddMember("tests", tests, allocator);

    char buf[500];
    sprintf(buf, "%s%c%s", outputDirectory, kPathSeparator, "output.json");

    printf("opening %s\n", buf);

    FILE *fp = fopen(buf, "w");

    constexpr size_t kBufferSize = 0xFFFF;
    std::vector<char> writeBuffer(kBufferSize);
    js::FileWriteStream os(fp, writeBuffer.data(), kBufferSize);
    js::PrettyWriter<js::FileWriteStream> writer(os);
    doc.Accept(writer);

    fclose(fp);
}

void GetCurrentTestResult(const testing::TestResult &resultIn, TestResults *resultsOut)
{
    TestResult &resultOut = resultsOut->results[resultsOut->currentTest];

    if (resultIn.Skipped())
    {
        resultOut.type = TestResultType::Skip;
    }
    else if (resultIn.Failed())
    {
        resultOut.type = TestResultType::Fail;
    }
    else
    {
        resultOut.type = TestResultType::Pass;
    }

    resultOut.elapsedTimeSeconds = static_cast<double>(resultIn.elapsed_time()) / 1000.0;
}

TestIdentifier GetTestIdentifier(const testing::TestInfo &testInfo)
{
    TestIdentifier id;
    id.testName     = testInfo.name();
    id.testCaseName = testInfo.test_case_name();

    return id;
}

class TestEventListener : public testing::EmptyTestEventListener
{
  public:
    TestEventListener(const char *outputDirectory,
                      const char *testSuiteName,
                      TestResults *testResults)
        : mOutputDirectory(outputDirectory),
          mTestSuiteName(testSuiteName),
          mTestResults(testResults)
    {}

    void OnTestStart(const testing::TestInfo &testInfo) override
    {
        mTestResults->currentTest = GetTestIdentifier(testInfo);
    }

    void OnTestEnd(const testing::TestInfo &testInfo) override
    {
        const testing::TestResult &resultIn = *testInfo.result();
        GetCurrentTestResult(resultIn, mTestResults);
        mTestResults->currentTest = TestIdentifier();
    }

    void OnTestProgramEnd(const testing::UnitTest &testProgramInfo) override
    {
        WriteTestResults(false, *mTestResults, mOutputDirectory, mTestSuiteName);
    }

  private:
    const char *mOutputDirectory;
    const char *mTestSuiteName;
    TestResults *mTestResults;
};

std::vector<TestIdentifier> GetCompiledInTests()
{
    testing::UnitTest *const unitTest = testing::UnitTest::GetInstance();

    std::vector<TestIdentifier> tests;
    for (int i = 0; i < unitTest->total_test_case_count(); ++i)
    {
        const testing::TestCase *testCase = unitTest->GetTestCase(i);
        for (int j = 0; j < testCase->total_test_count(); ++j)
        {
            const testing::TestInfo *testInfo = testCase->GetTestInfo(j);
            TestIdentifier testData;
            testData.testCaseName = testCase->name();
            testData.testName     = testInfo->name();
            testData.file         = testInfo->file();
            testData.line         = testInfo->line();
            tests.push_back(testData);
        }
    }
    return tests;
}

std::vector<TestIdentifier> GetShardTests(int shardIndex, int shardCount)
{
    std::vector<TestIdentifier> allTests = GetCompiledInTests();
    std::vector<TestIdentifier> shardTests;

    for (int testIndex = shardIndex; testIndex < static_cast<int>(allTests.size());
         testIndex += shardCount)
    {
        shardTests.emplace_back(allTests[testIndex]);
    }

    return shardTests;
}

std::vector<TestIdentifier> GetFilteredTests()
{
    testing::internal::UnitTestImpl *impl = testing::internal::GetUnitTestImpl();

    // Force the gtest_filter to be evaluated so we can set up test results.
    impl->FilterTests(testing::internal::UnitTestImpl::IGNORE_SHARDING_PROTOCOL);

    std::vector<TestIdentifier> tests;

    // Set initial test results to "SKIP". Account for gtest_filter and disabled tests.
    const testing::UnitTest &testProgramInfo = *testing::UnitTest::GetInstance();
    for (int caseIndex = 0; caseIndex < testProgramInfo.total_test_case_count(); ++caseIndex)
    {
        const testing::TestCase &testCase = *testProgramInfo.GetTestCase(caseIndex);
        for (int testIndex = 0; testIndex < testCase.total_test_count(); ++testIndex)
        {
            const testing::TestInfo &testInfo = *testCase.GetTestInfo(testIndex);
            if (testInfo.should_run())
            {
                TestIdentifier id = GetTestIdentifier(testInfo);
                tests.emplace_back(id);
            }
        }
    }

    return tests;
}

std::string GetTestFilter(const std::vector<TestIdentifier> &tests)
{
    std::stringstream filterStream;

    filterStream << "--gtest_filter=";

    for (size_t testIndex = 0; testIndex < tests.size(); ++testIndex)
    {
        if (testIndex != 0)
        {
            filterStream << ":";
        }

        filterStream << tests[testIndex].testCaseName;
        filterStream << ".";
        filterStream << tests[testIndex].testName;
    }

    return filterStream.str();
}

std::string ParseTestSuiteName(const char *executable)
{
    const char *baseNameStart = strrchr(executable, kPathSeparator);
    if (!baseNameStart)
    {
        baseNameStart = executable;
    }
    else
    {
        baseNameStart++;
    }

    const char kSuffix[]       = ".exe";
    const char *baseNameSuffix = strstr(baseNameStart, kSuffix);
    if (baseNameSuffix == (baseNameStart + strlen(baseNameStart) - strlen(kSuffix)))
    {
        return std::string(baseNameStart, baseNameSuffix);
    }
    else
    {
        return baseNameStart;
    }
}
}  // namespace

TestIdentifier::TestIdentifier() = default;

TestIdentifier::TestIdentifier(const TestIdentifier &other) = default;

TestIdentifier::~TestIdentifier() = default;

TestIdentifier &TestIdentifier::operator=(const TestIdentifier &other) = default;

TestResults::TestResults() = default;

TestResults::~TestResults() = default;

TestSuite::TestSuite(int *argc, char **argv)
    : mResultsDirectory(nullptr), mShardCount(-1), mShardIndex(-1), mBotMode(false), mBatchSize(0)
{
    bool hasFilter = false;

#if defined(ANGLE_PLATFORM_WINDOWS)
    testing::GTEST_FLAG(catch_exceptions) = false;
#endif

    // Note that the crash callback must be owned and not use global constructors.
    mCrashCallback = [this]() { this->onCrash(); };
    InitCrashHandler(&mCrashCallback);

    if (*argc <= 0)
    {
        printf("Missing test arguments.\n");
        exit(1);
    }

    mTestExcutableName = argv[0];
    mTestSuiteName     = ParseTestSuiteName(mTestExcutableName.c_str());

    for (int argIndex = 1; argIndex < *argc;)
    {
        if (parseSingleArg(argv[argIndex]))
        {
            DeleteArg(argc, argv, argIndex);
        }
        else
        {
            if (ParseFlagValue("--gtest_filter=", argv[argIndex]))
            {
                hasFilter = true;
            }
            ++argIndex;
        }
    }

    if ((mShardIndex >= 0) != (mShardCount > 1))
    {
        printf("Shard index and shard count must be specified together.\n");
        exit(1);
    }

    if (mShardCount > 0)
    {
        if (hasFilter)
        {
            printf("Cannot use gtest_filter in conjunction with sharding parameters.\n");
            exit(1);
        }

        testing::internal::UnitTestImpl *impl = testing::internal::GetUnitTestImpl();
        impl->RegisterParameterizedTests();

        std::vector<TestIdentifier> testList = GetShardTests(mShardIndex, mShardCount);
        mFilterString                        = GetTestFilter(testList);

        // Note that we only add a filter string if we previously deleted a shader index/count
        // argument. So we will have space for the new filter string in argv.
        AddArg(argc, argv, mFilterString.c_str());
    }

    testing::InitGoogleTest(argc, argv);

    if (mResultsDirectory)
    {
        testing::TestEventListeners &listeners = testing::UnitTest::GetInstance()->listeners();
        listeners.Append(
            new TestEventListener(mResultsDirectory, mTestSuiteName.c_str(), &mTestResults));

        std::vector<TestIdentifier> testList = GetFilteredTests();

        for (const TestIdentifier &id : testList)
        {
            mTestResults.results[id].type = TestResultType::Skip;
        }
    }
}

TestSuite::~TestSuite()
{
    TerminateCrashHandler();
}

bool TestSuite::parseSingleArg(const char *argument)
{
    return (ParseIntArg("--shard-count=", argument, &mShardCount) ||
            ParseIntArg("--shard-index=", argument, &mShardIndex) ||
            ParseIntArg("--batch-size=", argument, &mBatchSize) ||
            ParseStringArg("--results-directory=", argument, &mResultsDirectory) ||
            ParseFlag("--bot-mode", argument, &mBotMode));
}

void TestSuite::onCrash()
{
    if (mTestResults.currentTest.valid())
    {
        mTestResults.results[mTestResults.currentTest].type = TestResultType::Crash;
    }

    if (!mResultsDirectory)
        return;

    WriteTestResults(true, mTestResults, mResultsDirectory, mTestSuiteName.c_str());
}

int TestSuite::run()
{
    // Run tests serially.
    if (!mBotMode)
    {
        return RUN_ALL_TESTS();
    }

    // Spawn a child process that runs the tests.

    std::vector<TestIdentifier> testList = GetFilteredTests();
    std::string filterString             = GetTestFilter(testList);

    std::vector<const char *> args;
    int exitCode = 0;
    args.push_back(mTestExcutableName.c_str());
    args.push_back(filterString.c_str());

    printf("filter: %s\n", filterString.c_str());

    if (!RunApp(args, nullptr, nullptr, &exitCode))
    {
        std::cerr << "Error launching child process.\n";
        return 1;
    }

    return exitCode;
}
}  // namespace angle
