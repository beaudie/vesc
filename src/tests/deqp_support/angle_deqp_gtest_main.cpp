//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <gtest/gtest.h>
#include <stdint.h>
#include <zlib.h>

#include <algorithm>
#include <fstream>

#include "angle_deqp_libtester.h"
#include "common/angleutils.h"
#include "common/debug.h"
#include "gpu_test_expectations_parser.h"

namespace
{

std::string dEQPToGTestName(const std::string &dEQPName)
{
    std::string gTestName = dEQPName.substr(11);
    std::replace(gTestName.begin(), gTestName.end(), '.', '_');

    // Occurs in some luminance tests
    gTestName.erase(std::remove(gTestName.begin(), gTestName.end(), '-'), gTestName.end());
    return gTestName;
}

class dEQPCaseList
{
  public:
    dEQPCaseList(const char *caseListPath, const char *testExpectationsPath);

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

    static dEQPCaseList *GetInstance();
    static void FreeInstance();

  private:
    std::vector<CaseInfo> mCaseInfoList;
    gpu::GPUTestExpectationsParser mTestExpectationsParser;
    gpu::GPUTestBotConfig mTestConfig;

    static dEQPCaseList *mInstance;
};

// static
dEQPCaseList *dEQPCaseList::mInstance = nullptr;

// static
dEQPCaseList *dEQPCaseList::GetInstance()
{
    if (mInstance == nullptr)
    {
        mInstance = new dEQPCaseList("deqp_support/dEQP-GLES2-cases.txt.gz",
                                     "deqp_support/deqp_test_expectations.txt");
    }
    return mInstance;
}

// static
void dEQPCaseList::FreeInstance()
{
    SafeDelete(mInstance);
}

dEQPCaseList::dEQPCaseList(const char *caseListPath, const char *testExpectationsPath)
{
    if (!mTestExpectationsParser.LoadTestExpectationsFromFile(std::string(testExpectationsPath)))
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

    std::stringstream strstr;

    char *buf = new char[1024 * 1024 * 16];
    gzFile *fi = static_cast<gzFile *>(gzopen(caseListPath, "rb"));

    if (fi == nullptr)
    {
        return;
    }

    gzrewind(fi);
    while (!gzeof(fi))
    {
        int len = gzread(fi, buf, sizeof(buf) - 1);
        buf[len] = '\0';
        strstr << buf;
    }
    gzclose(fi);

    while (!strstr.eof())
    {
        std::string inString;
        std::getline(strstr, inString);

        if (inString.substr(0, 4) == "TEST")
        {
            std::string dEQPName = inString.substr(6);
            std::string gTestName = dEQPToGTestName(dEQPName);
            int expectation = mTestExpectationsParser.GetTestExpectation(dEQPName, mTestConfig);
            if (expectation != gpu::GPUTestExpectationsParser::kGpuTestSkip)
            {
                mCaseInfoList.push_back(CaseInfo(dEQPName, gTestName, expectation));
            }
        }
    }
}

class dEQP_GLES2 : public testing::TestWithParam<size_t>
{
  protected:
    dEQP_GLES2() {}

    void SetUp() override
    {

    }

    void runTest()
    {
        const auto &caseInfo = dEQPCaseList::GetInstance()->getCaseInfo(GetParam());

        bool result = deqp_libtester_run(caseInfo.mDEQPName.c_str());
        if (caseInfo.mExpectation == gpu::GPUTestExpectationsParser::kGpuTestPass)
        {
            ASSERT_TRUE(result);
        }
        else if (result)
        {
            std::cout << "Test exepected to fail but passed!" << std::endl;
        }
    }
};

TEST_P(dEQP_GLES2, D3D11)
{
    runTest();
}

testing::internal::ParamGenerator<size_t> GetTestingRange()
{
    return testing::Range<size_t>(0, dEQPCaseList::GetInstance()->numCases());
}

std::string GetTestName(const testing::ParamNameArgs<size_t> &nameArgs)
{
    return dEQPCaseList::GetInstance()->getCaseInfo(nameArgs.index).mGTestName;
}

INSTANTIATE_TEST_CASE_P(, dEQP_GLES2, GetTestingRange(), GetTestName);

}

int main(int argc, char **argv)
{
    std::vector<std::string> args;
    std::vector<char *> argVector;
    for (int argIndex = 0; argIndex < argc; argIndex++)
    {
        size_t maxCount = std::min(strlen("--deqp_filter="), strlen(argv[argIndex]));
        if (strncmp(argv[argIndex], "--deqp_filter=", maxCount) == 0)
        {
            std::string baseString(argv[argIndex] + strlen("--deqp_filter="));
            args.push_back(std::string("--gtest_filter=*") + dEQPToGTestName(baseString));
        }
        else
        {
            args.push_back(argv[argIndex]);
        }
        argVector.push_back(const_cast<char *>(args[argIndex].c_str()));
    }

    int argCount = static_cast<int>(argVector.size());
    testing::InitGoogleTest(&argCount, &argVector[0]);
    deqp_libtester_init_platform(argCount, &argVector[0], ANGLE_DEQP_DIR "/data");
    int rt = RUN_ALL_TESTS();
    dEQPCaseList::FreeInstance();
    deqp_libtester_shutdown_platform();
    return rt;
}
