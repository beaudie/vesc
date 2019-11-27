//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TestSuite:
//   Basic implementation of a test harness in ANGLE.

#include "TestSuite.h"

#include "common/platform.h"
#include "common/system_utils.h"
#include "util/Timer.h"

#include <time.h>
#include <fstream>

#include <gtest/gtest.h>
#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/istreamwrapper.h>
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
constexpr char kTestTimeoutArg[]  = "--test-timeout=";
constexpr char kFilterFileArg[]   = "--filter-file=";
constexpr char kResultFileArg[]   = "--results-file=";
constexpr int kDefaultTestTimeout = 10;
constexpr int kDefaultBatchSize   = 1000;

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
        printf("Overflow when parsing integer flag value.\n");
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

bool ParseStringArg(const char *flag, const char *argument, std::string *valueOut)
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
        case TestResultType::Unknown:
            return "UNKNOWN";
    }
}

TestResultType GetResultTypeFromString(const std::string &str)
{
    if (str == "CRASH")
        return TestResultType::Crash;
    if (str == "FAIL")
        return TestResultType::Fail;
    if (str == "PASS")
        return TestResultType::Pass;
    if (str == "SKIP")
        return TestResultType::Skip;
    if (str == "TIMEOUT")
        return TestResultType::Timeout;
    return TestResultType::Unknown;
}

js::Value ResultTypeToJSString(TestResultType type, js::Document::AllocatorType *allocator)
{
    js::Value jsName;
    jsName.SetString(ResultTypeToString(type), *allocator);
    return jsName;
}

void WriteTestResults(bool interrupted,
                      const TestResults &testResults,
                      const std::string &outputFile,
                      const char *testSuiteName)
{
    time_t ltime;
    time(&ltime);
    struct tm *timeinfo = gmtime(&ltime);
    ltime               = mktime(timeinfo);

    uint64_t secondsSinceEpoch = static_cast<uint64_t>(ltime);

    js::Document doc;
    doc.SetObject();

    js::Document::AllocatorType &allocator = doc.GetAllocator();

    doc.AddMember("interrupted", interrupted, allocator);
    doc.AddMember("path_delimiter", ".", allocator);
    doc.AddMember("version", 3, allocator);
    doc.AddMember("seconds_since_epoch", secondsSinceEpoch, allocator);

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

    printf("Writing test results to %s\n", outputFile.c_str());

    FILE *fp = fopen(outputFile.c_str(), "w");

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

    resultOut.elapsedTimeSeconds = resultsOut->currentTestTimer.getElapsedTime();
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
    TestEventListener(const std::string &outputFile,
                      const char *testSuiteName,
                      TestResults *testResults)
        : mResultsFile(outputFile), mTestSuiteName(testSuiteName), mTestResults(testResults)
    {}

    void OnTestStart(const testing::TestInfo &testInfo) override
    {
        std::lock_guard<std::mutex> guard(mTestResults->currentTestMutex);
        mTestResults->currentTest = GetTestIdentifier(testInfo);
        mTestResults->currentTestTimer.start();
    }

    void OnTestEnd(const testing::TestInfo &testInfo) override
    {
        std::lock_guard<std::mutex> guard(mTestResults->currentTestMutex);
        mTestResults->currentTestTimer.stop();
        const testing::TestResult &resultIn = *testInfo.result();
        GetCurrentTestResult(resultIn, mTestResults);
        mTestResults->currentTest = TestIdentifier();
    }

    void OnTestProgramEnd(const testing::UnitTest &testProgramInfo) override
    {
        std::lock_guard<std::mutex> guard(mTestResults->currentTestMutex);
        mTestResults->allDone = true;
        WriteTestResults(false, *mTestResults, mResultsFile, mTestSuiteName);
    }

  private:
    std::string mResultsFile;
    const char *mTestSuiteName;
    TestResults *mTestResults;
};

std::vector<TestIdentifier> GetCompiledInTests(std::map<TestIdentifier, FileLine> *fileLinesOut)
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
            tests.push_back(testData);

            (*fileLinesOut)[testData] = {testInfo->file(), testInfo->line()};
        }
    }
    return tests;
}

std::vector<TestIdentifier> GetShardTests(int shardIndex,
                                          int shardCount,
                                          std::map<TestIdentifier, FileLine> *fileLinesOut)
{
    std::vector<TestIdentifier> allTests = GetCompiledInTests(fileLinesOut);
    std::vector<TestIdentifier> shardTests;

    for (int testIndex = shardIndex; testIndex < static_cast<int>(allTests.size());
         testIndex += shardCount)
    {
        shardTests.emplace_back(allTests[testIndex]);
    }

    return shardTests;
}

std::vector<TestIdentifier> GetFilteredTests(std::map<TestIdentifier, FileLine> *fileLinesOut)
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

                if (fileLinesOut)
                {
                    (*fileLinesOut)[id] = {testInfo.file(), testInfo.line()};
                }
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

bool GetTestResultsFromJSON(const js::Document &document, TestResults *resultsOut)
{
    if (!document.HasMember("tests") || !document["tests"].IsObject())
    {
        return false;
    }

    const js::Value::ConstObject &tests = document["tests"].GetObject();
    if (tests.MemberCount() != 1)
    {
        return false;
    }

    const js::Value::Member &suite = *tests.MemberBegin();
    if (!suite.value.IsObject())
    {
        return false;
    }

    const js::Value::ConstObject &actual = suite.value.GetObject();

    for (auto iter = actual.MemberBegin(); iter != actual.MemberEnd(); ++iter)
    {
        // Get test identifier.
        const js::Value &name = iter->name;
        if (!name.IsString())
        {
            return false;
        }

        std::string testName = name.GetString();
        size_t separator     = testName.find(".");
        if (separator == std::string::npos)
        {
            return false;
        }

        TestIdentifier id;
        id.testCaseName = testName.substr(0, separator);
        id.testName     = testName.substr(separator + 1, testName.length() - separator - 1);

        // Get test result.
        const js::Value &value = iter->value;
        if (!value.IsObject())
        {
            return false;
        }

        const js::Value::ConstObject &obj = value.GetObject();
        if (!obj.HasMember("expected") || !obj.HasMember("actual"))
        {
            return false;
        }

        const js::Value &expected = obj["expected"];
        const js::Value &actual   = obj["actual"];

        if (!expected.IsString() || !actual.IsString())
        {
            return false;
        }

        const std::string expectedStr = expected.GetString();
        const std::string actualStr   = actual.GetString();

        if (expectedStr != "PASS")
        {
            return false;
        }

        TestResultType resultType = GetResultTypeFromString(actualStr);
        if (resultType == TestResultType::Unknown)
        {
            return false;
        }

        double elapsedTimeSeconds = 0.0;
        if (obj.HasMember("times"))
        {
            const js::Value &times = obj["times"];
            if (!times.IsArray())
            {
                return false;
            }

            const js::Value::ConstArray &timesArray = times.GetArray();
            if (timesArray.Size() != 1 || !timesArray[0].IsDouble())
            {
                return false;
            }

            elapsedTimeSeconds = timesArray[0].GetDouble();
        }

        TestResult &result        = resultsOut->results[id];
        result.elapsedTimeSeconds = elapsedTimeSeconds;
        result.type               = resultType;
    }

    return true;
}

bool MergeTestResults(const TestResults &input, TestResults *output)
{
    for (const auto &resultsIter : input.results)
    {
        const TestIdentifier &id      = resultsIter.first;
        const TestResult &inputResult = resultsIter.second;
        TestResult &outputResult      = output->results[id];

        // This should probably handle situations where a test is run more than once.
        if (inputResult.type != TestResultType::Skip)
        {
            if (outputResult.type != TestResultType::Skip)
            {
                printf("Warning: duplicate entry for %s.%s.\n", id.testCaseName.c_str(),
                       id.testName.c_str());
                return false;
            }

            outputResult.elapsedTimeSeconds = inputResult.elapsedTimeSeconds;
            outputResult.type               = inputResult.type;
        }
    }

    return true;
}

std::string GetTestOutputSnippet(const TestIdentifier &id,
                                 const TestResult &result,
                                 const std::string &fullOutput)
{
    std::string fullName = id.testCaseName + "." + id.testName;

    size_t runPos = fullOutput.find(std::string("[ RUN      ] ") + fullName);
    if (runPos == std::string::npos)
        return std::string();

    size_t endPos = fullOutput.find(std::string("[  FAILED  ] ") + fullName, runPos);
    // Only clip the snippet to the "OK" message if the test really
    // succeeded. It still might have e.g. crashed after printing it.
    if (endPos == std::string::npos && result.type == TestResultType::Pass)
    {
        endPos = fullOutput.find(std::string("[       OK ] ") + fullName, runPos);
    }
    if (endPos != std::string::npos)
    {
        size_t newline_pos = fullOutput.find("\n", endPos);
        if (newline_pos != std::string::npos)
            endPos = newline_pos + 1;
    }

    std::string snippet(fullOutput.substr(runPos));
    if (endPos != std::string::npos)
        snippet = fullOutput.substr(runPos, endPos - runPos);

    return snippet;
}
}  // namespace

TestIdentifier::TestIdentifier() = default;

TestIdentifier::TestIdentifier(const std::string &caseNameIn, const std::string &nameIn)
    : testCaseName(caseNameIn), testName(nameIn)
{}

TestIdentifier::TestIdentifier(const TestIdentifier &other) = default;

TestIdentifier::~TestIdentifier() = default;

TestIdentifier &TestIdentifier::operator=(const TestIdentifier &other) = default;

TestResults::TestResults() = default;

TestResults::~TestResults() = default;

ProcessInfo::ProcessInfo() = default;

ProcessInfo &ProcessInfo::operator=(ProcessInfo &&rhs)
{
    process         = std::move(rhs.process);
    testsInBatch    = std::move(rhs.testsInBatch);
    resultsFileName = std::move(rhs.resultsFileName);
    filterFileName  = std::move(rhs.filterFileName);
    commandLine     = std::move(rhs.commandLine);
    return *this;
}

ProcessInfo::~ProcessInfo() = default;

ProcessInfo::ProcessInfo(ProcessInfo &&other)
{
    *this = std::move(other);
}

TestSuite::TestSuite(int *argc, char **argv)
    : mShardCount(-1),
      mShardIndex(-1),
      mBotMode(false),
      mBatchSize(kDefaultBatchSize),
      mCurrentResultCount(0),
      mTotalResultCount(0),
      mMaxProcesses(NumberOfProcessors()),
      mTestTimeout(kDefaultTestTimeout),
      mBatchTimeout(60)
{
    bool hasFilter = false;

#if defined(ANGLE_PLATFORM_WINDOWS)
    testing::GTEST_FLAG(catch_exceptions) = false;
#endif

    // Note that the crash callback must be owned and not use global constructors.
    mCrashCallback = [this]() { onCrashOrTimeout(TestResultType::Crash); };
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
            continue;
        }

        if (ParseFlagValue("--gtest_filter=", argv[argIndex]))
        {
            hasFilter = true;
        }
        else
        {
            mArgv.push_back(argv[argIndex]);
        }
        ++argIndex;
    }

    if ((mShardIndex >= 0) != (mShardCount > 1))
    {
        printf("Shard index and shard count must be specified together.\n");
        exit(1);
    }

    if (!mFilterFile.empty())
    {
        if (hasFilter)
        {
            printf("Cannot use gtest_filter in conjunction with a filter file.\n");
            exit(1);
        }

        if (mShardCount > 0)
        {
            printf("Cannot use filter file in conjunction with sharding parameters.\n");
            exit(1);
        }

        uint32_t fileSize = 0;
        if (!GetFileSize(mFilterFile.c_str(), &fileSize))
        {
            printf("Error getting filter file size: %s\n", mFilterFile.c_str());
            exit(1);
        }

        std::vector<char> fileContents(fileSize + 1, 0);
        if (!ReadEntireFileToString(mFilterFile.c_str(), fileContents.data(), fileSize))
        {
            printf("Error loading filter file: %s\n", mFilterFile.c_str());
            exit(1);
        }
        mFilterString.assign(fileContents.data());

        // Note that we only add a filter string if we previously deleted a shader filter file
        // argument. So we will have space for the new filter string in argv.
        AddArg(argc, argv, mFilterString.c_str());
    }

    // Call into gtest internals to force parameterized test name registration.
    // TODO(jmadill): Clean this up so we don't need to call it.
    testing::internal::UnitTestImpl *impl = testing::internal::GetUnitTestImpl();
    impl->RegisterParameterizedTests();

    if (mShardCount > 0)
    {
        if (hasFilter)
        {
            printf("Cannot use gtest_filter in conjunction with sharding parameters.\n");
            exit(1);
        }

        mTestQueue    = GetShardTests(mShardIndex, mShardCount, &mTestFileLines);
        mFilterString = GetTestFilter(mTestQueue);

        // Note that we only add a filter string if we previously deleted a shader index/count
        // argument. So we will have space for the new filter string in argv.
        AddArg(argc, argv, mFilterString.c_str());
    }

    testing::InitGoogleTest(argc, argv);

    if (mShardCount <= 0)
    {
        mTestQueue = GetFilteredTests(&mTestFileLines);
    }

    mTotalResultCount = mTestQueue.size();

    if (!mResultsDirectory.empty())
    {
        std::stringstream resultFileName;
        resultFileName << mResultsDirectory << kPathSeparator << "output.json";
        mResultsFile = resultFileName.str();
    }

    if (mBotMode && mResultsFile.empty())
    {
        // Create a default output file in bot mode.
        mResultsFile = "output.json";
    }

    if (!mResultsFile.empty())
    {
        testing::TestEventListeners &listeners = testing::UnitTest::GetInstance()->listeners();
        listeners.Append(
            new TestEventListener(mResultsFile, mTestSuiteName.c_str(), &mTestResults));

        std::vector<TestIdentifier> testList = GetFilteredTests(nullptr);

        for (const TestIdentifier &id : testList)
        {
            mTestResults.results[id].type = TestResultType::Skip;
        }
    }
}

TestSuite::~TestSuite()
{
    if (mWatchdogThread.joinable())
    {
        mWatchdogThread.detach();
    }
    TerminateCrashHandler();
}

bool TestSuite::parseSingleArg(const char *argument)
{
    return (ParseIntArg("--shard-count=", argument, &mShardCount) ||
            ParseIntArg("--shard-index=", argument, &mShardIndex) ||
            ParseIntArg("--batch-size=", argument, &mBatchSize) ||
            ParseIntArg("--max-processes=", argument, &mMaxProcesses) ||
            ParseIntArg(kTestTimeoutArg, argument, &mTestTimeout) ||
            ParseIntArg("--batch-timeout=", argument, &mBatchTimeout) ||
            ParseStringArg("--results-directory=", argument, &mResultsDirectory) ||
            ParseStringArg(kResultFileArg, argument, &mResultsFile) ||
            ParseStringArg(kFilterFileArg, argument, &mFilterFile) ||
            ParseFlag("--bot-mode", argument, &mBotMode));
}

void TestSuite::onCrashOrTimeout(TestResultType crashOrTimeout)
{
    if (mTestResults.currentTest.valid())
    {
        mTestResults.results[mTestResults.currentTest].type = crashOrTimeout;
    }

    if (mResultsFile.empty())
    {
        printf("No results file specified.\n");
        return;
    }

    WriteTestResults(true, mTestResults, mResultsFile, mTestSuiteName.c_str());
}

bool TestSuite::launchChildTestProcess(const std::vector<TestIdentifier> &testsInBatch)
{
    constexpr uint32_t kMaxPath = 1000;

    // Create a temporary file to store the test list
    ProcessInfo processInfo;

    char filterBuffer[kMaxPath] = {};
    if (!CreateTemporaryFile(filterBuffer, kMaxPath))
    {
        std::cerr << "Error creating temporary file for test list.\n";
        return false;
    }
    processInfo.filterFileName.assign(filterBuffer);

    std::string filterString = GetTestFilter(testsInBatch);

    FILE *fp = fopen(processInfo.filterFileName.c_str(), "w");
    if (!fp)
    {
        std::cerr << "Error opening temporary file for test list.\n";
        return false;
    }
    fprintf(fp, "%s", filterString.c_str());
    fclose(fp);

    std::string filterFileArg = kFilterFileArg + processInfo.filterFileName;

    // Create a temporary file to store the test output.
    char resultsBuffer[kMaxPath] = {};
    if (!CreateTemporaryFile(resultsBuffer, kMaxPath))
    {
        std::cerr << "Error creating temporary file for test list.\n";
        return false;
    }
    processInfo.resultsFileName.assign(resultsBuffer);

    std::string resultsFileArg = kResultFileArg + processInfo.resultsFileName;

    // Construct commandline for child process.
    std::vector<const char *> args;

    args.push_back(mTestExcutableName.c_str());
    args.push_back(filterFileArg.c_str());
    args.push_back(resultsFileArg.c_str());

    for (const std::string &arg : mArgv)
    {
        args.push_back(arg.c_str());
    }

    std::string timeoutStr;
    if (mTestTimeout != kDefaultTestTimeout)
    {
        std::stringstream timeoutStream;
        timeoutStream << kTestTimeoutArg << mTestTimeout;
        timeoutStr = timeoutStream.str();
        args.push_back(timeoutStr.c_str());
    }

    // Launch child process and wait for completion.
    processInfo.process = LaunchProcess(args, true, true);

    if (!processInfo.process->started())
    {
        std::cerr << "Error launching child process.\n";
        return false;
    }

    std::stringstream commandLineStr;
    commandLineStr << mTestExcutableName << " " << filterFileArg << " " << resultsFileArg;
    processInfo.commandLine  = commandLineStr.str();
    processInfo.testsInBatch = testsInBatch;
    mCurrentProcesses.emplace_back(std::move(processInfo));
    return true;
}

bool TestSuite::finishProcess(ProcessInfo *processInfo)
{
    // Get test results and merge into master list.
    TestResults batchResults;

    if (!GetTestResultsFromFile(processInfo->resultsFileName.c_str(), &batchResults))
    {
        std::cerr << "Error reading test results from child process.\n";
        return false;
    }

    if (!MergeTestResults(batchResults, &mTestResults))
    {
        std::cerr << "Error merging batch test results.\n";
        return false;
    }

    // Process results and print unexpected errors.
    for (const auto &resultIter : batchResults.results)
    {
        const TestIdentifier &id = resultIter.first;
        const TestResult &result = resultIter.second;

        if (result.type == TestResultType::Skip)
        {
            continue;
        }

        mCurrentResultCount++;
        printf("[%d/%d] %s.%s", mCurrentResultCount, mTotalResultCount, id.testCaseName.c_str(),
               id.testName.c_str());

        if (result.type == TestResultType::Pass)
        {
            printf(" (%g ms)\n", result.elapsedTimeSeconds * 1000.0);
        }
        else
        {
            printf(" (%s)\n", ResultTypeToString(result.type));

            const std::string &batchStdout = processInfo->process->getStdout();
            std::string outputSnippet      = GetTestOutputSnippet(id, result, batchStdout);
            printf("\n%s\n", outputSnippet.c_str());
        }
    }

    // On unexpected exit, re-queue any unfinished tests.
    if (processInfo->process->getExitCode() != 0)
    {
        for (const auto &resultIter : batchResults.results)
        {
            const TestIdentifier &id = resultIter.first;
            const TestResult &result = resultIter.second;

            if (result.type == TestResultType::Skip)
            {
                mTestQueue.emplace_back(id);
            }
        }
    }

    // Clean up any dirty temporary files.
    for (const std::string &tempFile : {processInfo->filterFileName, processInfo->resultsFileName})
    {
        if (!angle::DeleteFile(tempFile.c_str()))
        {
            std::cerr << "Warning: Error cleaning up temp file: " << tempFile << "\n";
        }
    }

    processInfo->process.reset();
    return true;
}

int TestSuite::run()
{
    // Run tests serially.
    if (!mBotMode)
    {
        startWatchdog();
        return RUN_ALL_TESTS();
    }

    constexpr double kIdleMessageTimeout = 5.0;

    Timer messageTimer;
    messageTimer.start();

    while (!mTestQueue.empty() || !mCurrentProcesses.empty())
    {
        bool progress = false;

        // Spawn a process if needed and possible.
        while (static_cast<int>(mCurrentProcesses.size()) < mMaxProcesses && !mTestQueue.empty())
        {
            int numTests = std::min<int>(mTestQueue.size(), mBatchSize);

            std::vector<TestIdentifier> testsInBatch;
            testsInBatch.assign(mTestQueue.begin(), mTestQueue.begin() + numTests);
            mTestQueue.erase(mTestQueue.begin(), mTestQueue.begin() + numTests);

            if (!launchChildTestProcess(testsInBatch))
            {
                return 1;
            }

            progress = true;
        }

        // Check for process completion.
        for (auto processIter = mCurrentProcesses.begin(); processIter != mCurrentProcesses.end();)
        {
            ProcessInfo &processInfo = *processIter;
            if (processInfo.process->finished())
            {
                if (!finishProcess(&processInfo))
                {
                    return 1;
                }
                processIter = mCurrentProcesses.erase(processIter);
                progress    = true;
            }
            else if (processInfo.process->getElapsedTimeSeconds() > mBatchTimeout)
            {
                // Terminate the process and record timeouts for the batch.
                // Because we can't determine which sub-test caused a timeout, record the whole
                // batch as a timeout failure. Can be improved by using socket message passing.
                if (!processInfo.process->kill())
                {
                    return 1;
                }
                for (const TestIdentifier &testIdentifier : processInfo.testsInBatch)
                {
                    mTestResults.results[testIdentifier].type = TestResultType::Timeout;
                }

                processIter = mCurrentProcesses.erase(processIter);
                progress    = true;
            }
            else
            {
                processIter++;
            }
        }

        if (!progress && messageTimer.getElapsedTime() > kIdleMessageTimeout)
        {
            for (const ProcessInfo &processInfo : mCurrentProcesses)
            {
                double processTime = processInfo.process->getElapsedTimeSeconds();
                if (processTime > kIdleMessageTimeout)
                {
                    printf("Running for %d seconds: %s\n", static_cast<int>(processTime),
                           processInfo.commandLine.c_str());
                }
            }

            messageTimer.start();
        }

        // Sleep briefly and continue.
        angle::Sleep(10);
    }

    // Dump combined results.
    WriteTestResults(true, mTestResults, mResultsFile, mTestSuiteName.c_str());

    return printUnexpectedFailuresAndReturnCount() == 0;
}

int TestSuite::printUnexpectedFailuresAndReturnCount() const
{
    std::vector<std::string> unexpectedFailures;

    for (const auto &resultIter : mTestResults.results)
    {
        const TestIdentifier &id = resultIter.first;
        const TestResult &result = resultIter.second;

        if (result.type != TestResultType::Pass)
        {
            const FileLine &fileLine = mTestFileLines.find(id)->second;

            std::stringstream failureMessage;
            failureMessage << id.testCaseName << "." << id.testName << " (" << fileLine.file << ":"
                           << fileLine.line << ") (" << ResultTypeToString(result.type) << ")";
            unexpectedFailures.emplace_back(failureMessage.str());
        }
    }

    if (unexpectedFailures.empty())
        return 0;

    printf("%zu unexpected failures:\n", unexpectedFailures.size());
    for (const std::string &failure : unexpectedFailures)
    {
        printf("    %s\n", failure.c_str());
    }

    return static_cast<int>(unexpectedFailures.size());
}

void TestSuite::startWatchdog()
{
    // if (!IsDebuggerAttached())
    auto watchdogMain = [this]() {
        do
        {
            {
                std::lock_guard<std::mutex> guard(mTestResults.currentTestMutex);
                if (mTestResults.currentTestTimer.getElapsedTime() >
                    static_cast<double>(mTestTimeout))
                {
                    onCrashOrTimeout(TestResultType::Timeout);
                    exit(2);
                }

                if (mTestResults.allDone)
                    return;
            }

            angle::Sleep(1000);
        } while (true);
    };
    mWatchdogThread = std::thread(watchdogMain);
}

bool GetTestResultsFromFile(const char *fileName, TestResults *resultsOut)
{
    std::ifstream ifs(fileName);
    if (!ifs.is_open())
    {
        std::cerr << "Error opening " << fileName << "\n";
        return false;
    }

    js::IStreamWrapper ifsWrapper(ifs);
    js::Document document;
    document.ParseStream(ifsWrapper);

    if (document.HasParseError())
    {
        std::cerr << "Parse error reading JSON document: " << document.GetParseError() << "\n";
        return false;
    }

    if (!GetTestResultsFromJSON(document, resultsOut))
    {
        std::cerr << "Error getting test results from JSON.\n";
        return false;
    }

    ifs.close();
    return true;
}

const char *TestResultTypeToString(TestResultType type)
{
    switch (type)
    {
        case TestResultType::Crash:
            return "Crash";
        case TestResultType::Fail:
            return "Fail";
        case TestResultType::Skip:
            return "Skip";
        case TestResultType::Pass:
            return "Pass";
        case TestResultType::Timeout:
            return "Timeout";
        case TestResultType::Unknown:
            return "Unknown";
    }
}
}  // namespace angle
