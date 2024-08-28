#include <gtest/gtest.h>
#if !defined(__APPLE__)
#    include <CL/cl.h>
#endif
#include <stdlib.h>
#include <cassert>
#include <fstream>

#include "angle_oclcts_gtest.h"
#include "common/string_utils.h"
#include "src/tests/test_utils/runner/TestSuite.h"
#include "util/OSWindow.h"

int *CtsSetup::globalArgC;
char **CtsSetup::globalArgV;

constexpr char kInfoTag[] = "*RESULT";

void CtsSetup::Init(int *argc, char **argv)
{
    globalArgC = argc;
    globalArgV = argv;
}

std::string TrimString(const std::string &input, const std::string &trimChars)
{
    auto begin = input.find_first_not_of(trimChars);
    if (begin == std::string::npos)
    {
        return "";
    }

    std::string::size_type end = input.find_last_not_of(trimChars);
    if (end == std::string::npos)
    {
        return input.substr(begin);
    }

    return input.substr(begin, end - begin + 1);
}

class openCLCaseList
{
  public:
    openCLCaseList();

    struct CaseInfo
    {
        CaseInfo(const std::string &testNameIn, int expectationIn)
            : testName(testNameIn), expectation(expectationIn)
        {}

        std::string testName;
        int expectation;
    };

    void initialize(angle::TestSuite *instance);

    const CaseInfo &getCaseInfo(size_t caseIndex) const
    {
        assert(mInitialized);
        assert(caseIndex < mCaseInfoList.size());
        return mCaseInfoList[caseIndex];
    }

    size_t numCases() const
    {
        assert(mInitialized);
        return mCaseInfoList.size();
    }

  private:
    std::vector<CaseInfo> mCaseInfoList;
    bool mInitialized = false;
};

openCLCaseList::openCLCaseList() {}

const openCLCaseList &GetTestList()
{
    angle::TestSuite *instance = angle::TestSuite::GetInstance();
    static openCLCaseList sCaseList;
    sCaseList.initialize(instance);
    return sCaseList;
}

void openCLCaseList::initialize(angle::TestSuite *instance)
{
    mInitialized                           = true;
    constexpr char kTestExpectationsPath[] = "src/tests/openclcts_expectations.txt";
    constexpr char kTestMustPassPath[]     = "src/tests/openclcts_mustpass.txt";
    constexpr size_t kMaxPath              = 512;
    std::array<char, kMaxPath> foundDataPath;
    if (!angle::FindTestDataPath(kTestExpectationsPath, foundDataPath.data(), foundDataPath.size()))
    {
        std::cerr << "Unable to find test expectations path (" << kTestExpectationsPath << ")\n";
        exit(EXIT_FAILURE);
    }
    if (!instance->loadAllTestExpectationsFromFile(std::string(foundDataPath.data())))
    {
        exit(EXIT_FAILURE);
    }
    if (!angle::FindTestDataPath(kTestMustPassPath, foundDataPath.data(), foundDataPath.size()))
    {
        std::cerr << "Unable to find test must pass list path (" << kTestMustPassPath << ")\n";
        exit(EXIT_FAILURE);
    }
    std::ifstream caseListStream(std::string(foundDataPath.data()));
    if (caseListStream.fail())
    {
        std::cerr << "Failed to load the case list." << std::endl;
        exit(EXIT_FAILURE);
    }

    while (!caseListStream.eof())
    {
        std::string inString;
        std::getline(caseListStream, inString);

        std::string testName = TrimString(inString, angle::kWhitespaceASCII);
        if (testName.empty())
            continue;
        int expectation = instance->getTestExpectation(testName);
        mCaseInfoList.push_back(CaseInfo(testName, expectation));
    }
}

class openCLTestSuiteStats
{
  public:
    openCLTestSuiteStats() {}

  private:
    void setUpTestStats();
    void printTestStats();
    void countTestResult(int result);

    uint32_t mTestCount;
    uint32_t mPassedTestCount;
    uint32_t mFailedTestCount;
    uint32_t mTestExceptionCount;
    uint32_t mNotSupportedTestCount;
    uint32_t mSkippedTestCount;

    std::vector<std::string> mUnexpectedFailed;
    std::vector<std::string> mUnexpectedPasses;

    friend class openCLTest;
};

void openCLTestSuiteStats::setUpTestStats()
{
    mPassedTestCount       = 0;
    mFailedTestCount       = 0;
    mNotSupportedTestCount = 0;
    mTestExceptionCount    = 0;
    mTestCount             = 0;
    mSkippedTestCount      = 0;
    mUnexpectedPasses.clear();
    mUnexpectedFailed.clear();
}

std::string GetTestStatLine(const std::string &key, const std::string &value)
{
    return std::string(kInfoTag) + ": " + key + ": " + value + "\n";
}

void openCLTestSuiteStats::printTestStats()
{
    uint32_t crashedCount =
        mTestCount - (mPassedTestCount + mFailedTestCount + mNotSupportedTestCount +
                      mTestExceptionCount + mSkippedTestCount);

    std::cout << GetTestStatLine("Total", std::to_string(mTestCount));
    std::cout << GetTestStatLine("Passed", std::to_string(mPassedTestCount));
    std::cout << GetTestStatLine("Failed", std::to_string(mFailedTestCount));
    std::cout << GetTestStatLine("Skipped", std::to_string(mSkippedTestCount));
    std::cout << GetTestStatLine("Not Supported", std::to_string(mNotSupportedTestCount));
    std::cout << GetTestStatLine("Exception", std::to_string(mTestExceptionCount));
    std::cout << GetTestStatLine("Crashed", std::to_string(crashedCount));

    if (!mUnexpectedPasses.empty())
    {
        std::cout << GetTestStatLine("Unexpected Passed Count",
                                     std::to_string(mUnexpectedPasses.size()));
        for (const std::string &testName : mUnexpectedPasses)
        {
            std::cout << GetTestStatLine("Unexpected Passed Tests", testName);
        }
    }

    if (!mUnexpectedFailed.empty())
    {
        std::cout << GetTestStatLine("Unexpected Failed Count",
                                     std::to_string(mUnexpectedFailed.size()));
        for (const std::string &testName : mUnexpectedFailed)
        {
            std::cout << GetTestStatLine("Unexpected Failed Tests", testName);
        }
    }
}

void openCLTestSuiteStats::countTestResult(int result)
{
    switch (result)
    {
        case TEST_SUCCESS:
            mPassedTestCount++;
            break;
        case TEST_FAIL:
            mFailedTestCount++;
            break;
        default:
            std::cerr << "Unexpected test result code: " << static_cast<int>(result) << "\n";
            break;
    }
}

class openCLTest : public testing::Test
{
  public:
    openCLTest(size_t caseIndex) : mTestCaseIndex(caseIndex) {}

    static void SetUpTestSuite();
    static void TearDownTestSuite();

  protected:
    void TestBody() override;

  private:
    size_t mTestCaseIndex = 0;

    static openCLTestSuiteStats sTestSuiteData;
};

openCLTestSuiteStats openCLTest::sTestSuiteData = openCLTestSuiteStats();

// static function called once before running all of openCLTest under the same test suite
void openCLTest::SetUpTestSuite()
{
    sTestSuiteData.setUpTestStats();
}

// static function called once after running all of openCLTest under the same test suite
void openCLTest::TearDownTestSuite()
{
    sTestSuiteData.printTestStats();
}

void openCLTest::TestBody()
{
    const auto &caseInfo = GetTestList().getCaseInfo(mTestCaseIndex);

    // Tests that crash exit the harness before collecting the result. To tally the number of
    // crashed tests we track how many tests we "tried" to run.
    sTestSuiteData.mTestCount++;

    if (caseInfo.expectation == angle::GPUTestExpectationsParser::kGpuTestSkip)
    {
        sTestSuiteData.mSkippedTestCount++;
        std::cout << "Test skipped.\n";
        return;
    }

    size_t pos            = caseInfo.testName.find('.');
    std::string testSuite = caseInfo.testName.substr(0, pos);
    std::string testName  = caseInfo.testName.substr(pos + 1);

#if defined(ANGLE_PLATFORM_ANDROID)
    testSuite = "/data/local/tmp/" + testSuite;
#else
    constexpr size_t kMaxPath = 512;
    std::array<char, kMaxPath> foundDataPath;
    if (!angle::FindTestDataPath(testSuite.c_str(), foundDataPath.data(), foundDataPath.size()))
    {
        std::cerr << "Unable to find test path (" << testSuite << ")\n";
        exit(EXIT_FAILURE);
    }
#endif

    char path[MAX_LINE_LENGTH];
    char **originalArgV = CtsSetup::getArgV();
    char command[MAX_LINE_LENGTH];
    for (int index = 0; index < *CtsSetup::getArgC(); index++)
    {
        if (index == 0)
        {
#if defined(ANGLE_PLATFORM_ANDROID)
            strcpy(command, testSuite.c_str());
#else
            strcpy(command, foundDataPath.data());
#endif
        }
        else
        {
            strcat(command, " ");
            strcat(command, originalArgV[index]);
        }
    }
    strcat(command, " ");
    strcat(command, testName.c_str());
    FILE *fp   = popen(command, "r");
    int result = TEST_FAIL;
    if (fp != NULL)
    {
        while (fgets(path, 300, fp) != NULL)
        {
            printf("%s", path);
            if (strstr(path, "PASSED test."))
            {
                result = TEST_SUCCESS;
                break;
            }
            else if (strstr(path, "FAILED test."))
            {
                break;
            }
        }
        pclose(fp);
    }
    sTestSuiteData.countTestResult(result);
    if (caseInfo.expectation == angle::GPUTestExpectationsParser::kGpuTestPass ||
        caseInfo.expectation == angle::GPUTestExpectationsParser::kGpuTestFlaky)
    {
        EXPECT_TRUE((result == TEST_SUCCESS));
        if (result != TEST_SUCCESS)
        {
            sTestSuiteData.mUnexpectedFailed.push_back(caseInfo.testName);
        }
    }
    else if (result == TEST_SUCCESS)
    {
        std::cout << "Test expected to fail but passed!" << std::endl;
        sTestSuiteData.mUnexpectedPasses.push_back(caseInfo.testName);
    }
}

void RegisterCLCTSTests()
{
    const openCLCaseList &testList = GetTestList();
    for (size_t caseIndex = 0; caseIndex < testList.numCases(); caseIndex++)
    {
        auto factory = [caseIndex]() { return new openCLTest(caseIndex); };

        const std::string testCaseName = testList.getCaseInfo(caseIndex).testName;
        size_t pos                     = testCaseName.find('.');
        ASSERT(pos != std::string::npos);
        // testCaseName comes from one of the mustpass files in gCaseListFiles.
        // All of the testCaseName in the same mustpass file starts with the same testSuiteName
        // prefix. Which mustpass file to load the set of testCaseName depends on testModuleIndex
        // compiled into the deqp test application binary. For now, only one testModuleIndex is
        // compiled in a deqp test application binary, meaning all of the tests invoked by same deqp
        // test application binary are under the same test suite.
        std::string testSuiteName = testCaseName.substr(0, pos);
        std::string testName      = testCaseName.substr(pos + 1);

        testing::RegisterTest(testSuiteName.c_str(), testName.c_str(), nullptr, nullptr, __FILE__,
                              __LINE__, factory);
    }
}

int main(int argc, char **argv)
{
    CtsSetup::Init(&argc, argv);
    ::testing::InitGoogleTest(&argc, argv);

    angle::TestSuite testSuite(&argc, argv);
    RegisterCLCTSTests();
    testSuite.run();
}
