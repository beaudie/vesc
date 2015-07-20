//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// angle_deqp_gtest:
//   dEQP and GoogleTest integration logic. Calls through to the random
//   order executor.

#include <stdint.h>
#include <zlib.h>

#include <gtest/gtest.h>

#include "angle_deqp_libtester.h"
#include "common/angleutils.h"
#include "common/debug.h"
#include "gpu_test_expectations_parser.h"
#include "system_utils.h"

namespace
{

const char *g_CaseListFiles[] =
{
    "dEQP-GLES2-cases.txt.gz",
    "dEQP-GLES3-cases.txt.gz",
};

template <size_t CaseListIndex>
class dEQPCaseList
{
  public:
    dEQPCaseList(const std::string &testExpectationsFile);

    struct CaseInfo
    {
        CaseInfo(const std::string &dEQPName,
                 const std::string &gTestName,
                 int expectation)
            : mDEQPName(dEQPName),
              mGTestName(gTestName),
              mExpectation(expectation)
        {
        }

        std::string mDEQPName;
        std::string mGTestName;
        int mExpectation;
    };

    const CaseInfo &getCaseInfo(size_t caseIndex) const
    {
        ASSERT(caseIndex < mCaseInfoList.size());
        return mCaseInfoList[caseIndex];
    }

    size_t numCases() const
    {
        return mCaseInfoList.size();
    }

    static dEQPCaseList<CaseListIndex> *GetInstance();
    static void FreeInstance();

  private:
    std::vector<CaseInfo> mCaseInfoList;
    gpu::GPUTestExpectationsParser mTestExpectationsParser;
    gpu::GPUTestBotConfig mTestConfig;

    static dEQPCaseList<CaseListIndex> *mInstance;
};

// static
template <size_t CaseListIndex>
dEQPCaseList<CaseListIndex> *dEQPCaseList<CaseListIndex>::mInstance = nullptr;

// static
template <size_t CaseListIndex>
dEQPCaseList<CaseListIndex> *dEQPCaseList<CaseListIndex>::GetInstance()
{
    if (mInstance == nullptr)
    {
        mInstance = new dEQPCaseList<CaseListIndex>("deqp_test_expectations.txt");
    }
    return mInstance;
}

// Not currently called, assumed to be freed on program exit.
// static
template <size_t CaseListIndex>
void dEQPCaseList<CaseListIndex>::FreeInstance()
{
    SafeDelete(mInstance);
}

template <size_t CaseListIndex>
dEQPCaseList<CaseListIndex>::dEQPCaseList(const std::string &testExpectationsFile)
{
    std::string exeDir = angle::GetExecutableDirectory();

    std::stringstream caseListPathStr;
    caseListPathStr << exeDir << "/deqp_support/" << g_CaseListFiles[CaseListIndex];

    std::string caseListPath = caseListPathStr.str();
    std::string testExpectationsPath = exeDir + "/deqp_support/" + testExpectationsFile;

    if (!mTestExpectationsParser.LoadTestExpectationsFromFile(testExpectationsPath))
    {
        std::cerr << "Failed to load test expectations." << std::endl;
        for (const auto &message : mTestExpectationsParser.GetErrorMessages())
        {
            std::cerr << " " << message << std::endl;
        }
        return;
    }

    if (!mTestConfig.LoadCurrentConfig(nullptr))
    {
        std::cerr << "Failed to load test configuration." << std::endl;
        return;
    }

    std::stringstream caseListStream;

    std::vector<char> buf(1024 * 1024 * 16);
    gzFile *fi = static_cast<gzFile *>(gzopen(caseListPath.c_str(), "rb"));

    if (fi == nullptr)
    {
        return;
    }

    gzrewind(fi);
    while (!gzeof(fi))
    {
        int len = gzread(fi, &buf[0], static_cast<unsigned int>(buf.size()) - 1);
        buf[len] = '\0';
        caseListStream << &buf[0];
    }
    gzclose(fi);

    while (!caseListStream.eof())
    {
        std::string inString;
        std::getline(caseListStream, inString);

        if (inString.substr(0, 4) == "TEST")
        {
            std::string dEQPName = inString.substr(6);
            std::string gTestName = dEQPName.substr(11);
            std::replace(gTestName.begin(), gTestName.end(), '.', '_');

            // Occurs in some luminance tests
            gTestName.erase(std::remove(gTestName.begin(), gTestName.end(), '-'), gTestName.end());

            int expectation = mTestExpectationsParser.GetTestExpectation(dEQPName, mTestConfig);
            if (expectation != gpu::GPUTestExpectationsParser::kGpuTestSkip)
            {
                mCaseInfoList.push_back(CaseInfo(dEQPName, gTestName, expectation));
            }
        }
    }
}

template <size_t CaseListIndex>
class dEQPTest : public testing::TestWithParam<size_t>
{
  public:
    static testing::internal::ParamGenerator<size_t> GetTestingRange()
    {
        return testing::Range<size_t>(0, dEQPCaseList<CaseListIndex>::GetInstance()->numCases());
    }

  protected:
    void runTest()
    {
        const auto &caseInfo = dEQPCaseList<CaseListIndex>::GetInstance()->getCaseInfo(GetParam());
        std::cout << caseInfo.mDEQPName << std::endl;

        bool result = deqp_libtester_run(caseInfo.mDEQPName.c_str());

        if (!result)
        {
            FILE *fp = fopen("fails.txt", "at+");
            fprintf(fp, caseInfo.mDEQPName.c_str());
            fprintf(fp, "\n");
            fclose(fp);
        }

        if (caseInfo.mExpectation == gpu::GPUTestExpectationsParser::kGpuTestPass)
        {
            EXPECT_TRUE(result);
        }
        else if (result)
        {
            std::cout << "Test expected to fail but passed!" << std::endl;
        }
    }
};

class dEQP_GLES2 : public dEQPTest<0> {};
class dEQP_GLES3 : public dEQPTest<1> {};

#ifdef ANGLE_DEQP_GLES2_TESTS
// TODO(jmadill): add different platform configs, or ability to choose platform
TEST_P(dEQP_GLES2, Default)
{
    runTest();
}

INSTANTIATE_TEST_CASE_P(, dEQP_GLES2, dEQP_GLES2::GetTestingRange());
#endif

#ifdef ANGLE_DEQP_GLES3_TESTS
TEST_P(dEQP_GLES3, Default)
{
    runTest();
}

INSTANTIATE_TEST_CASE_P(, dEQP_GLES3, dEQP_GLES3::GetTestingRange());
#endif

} // anonymous namespace
