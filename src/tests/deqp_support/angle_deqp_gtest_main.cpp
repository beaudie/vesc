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

namespace
{

class dEQPCaseList
{
  public:
    dEQPCaseList(const char *caseListPath);

    struct CaseInfo
    {
        CaseInfo(const std::string &dEQPName,
                 const std::string &gTestName)
            : mDEQPName(dEQPName),
              mGTestName(gTestName)
        {
        }

        std::string mDEQPName;
        std::string mGTestName;
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

    static dEQPCaseList *mInstance;
};

// static
dEQPCaseList *dEQPCaseList::mInstance = nullptr;

// static
dEQPCaseList *dEQPCaseList::GetInstance()
{
    if (mInstance == nullptr)
    {
        mInstance = new dEQPCaseList("deqp_support/dEQP-GLES2-cases.txt.gz");
    }
    return mInstance;
}

// static
void dEQPCaseList::FreeInstance()
{
    SafeDelete(mInstance);
}

dEQPCaseList::dEQPCaseList(const char *caseListPath)
{
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
            std::string gTestName = dEQPName.substr(11);
            std::replace(gTestName.begin(), gTestName.end(), '.', '_');

            // Occurs in some luminance tests
            gTestName.erase(std::remove(gTestName.begin(), gTestName.end(), '-'), gTestName.end());

            mCaseInfoList.push_back(CaseInfo(dEQPName, gTestName));
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
        std::cout << caseInfo.mDEQPName << std::endl;
        ASSERT_TRUE(deqp_libtester_run(caseInfo.mDEQPName.c_str()));
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

INSTANTIATE_TEST_CASE_P(, dEQP_GLES2, GetTestingRange());

}

int main(int argc, char **argv)
{
    deqp_libtester_init_platform(argc, argv, ANGLE_DEQP_DIR "/data");
    testing::InitGoogleTest(&argc, argv);
    int rt = RUN_ALL_TESTS();
    dEQPCaseList::FreeInstance();
    deqp_libtester_shutdown_platform();
    return rt;
}
