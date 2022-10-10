//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AstcDecompressorPerf: Performance test for the ASTC decompressor.
//

#include "ANGLEPerfTest.h"

#include <gmock/gmock.h>

#include "image_util/AstcDecompressor.h"
#include "image_util/AstcDecompressorTestUtils.h"

using namespace testing;

namespace
{
using angle::AstcDecompressor;

struct AstcDecompressorParams
{
    AstcDecompressorParams(uint32_t width, uint32_t height) : width(width), height(height) {}

    uint32_t width;
    uint32_t height;
};

std::ostream &operator<<(std::ostream &os, const AstcDecompressorParams &params)
{
    os << params.width << "x" << params.height;
    return os;
}

class AstcDecompressorPerfTest : public ANGLEPerfTest,
                                 public WithParamInterface<AstcDecompressorParams>
{
  public:
    AstcDecompressorPerfTest();

    void step() override;

    std::string getName();

    AstcDecompressor &mDecompressor;
    std::vector<uint8_t> mInput;
    std::vector<uint8_t> mOutput;
};

AstcDecompressorPerfTest::AstcDecompressorPerfTest()
    : ANGLEPerfTest(getName(), "", "_run", 1, "us"),
      mDecompressor(AstcDecompressor::get()),
      mInput(makeAstcCheckerboard(GetParam().width, GetParam().height)),
      mOutput(GetParam().width * GetParam().height * 4)
{
    // const auto &param = GetParam();
}

void AstcDecompressorPerfTest::step()
{
    mDecompressor.decompress(GetParam().width, GetParam().height, 8, 8, mInput.data(),
                             mInput.size(), mOutput.data());
}

std::string AstcDecompressorPerfTest::getName()
{
    std::stringstream ss;
    ss << UnitTest::GetInstance()->current_test_case()->name() << "/" << GetParam();
    return ss.str();
}

TEST_P(AstcDecompressorPerfTest, Run)
{
    if (!mDecompressor.available())
        skipTest("ASTC decompressor not available");

    this->run();
}

INSTANTIATE_TEST_SUITE_P(,
                         AstcDecompressorPerfTest,
                         Values(AstcDecompressorParams(16, 16),
                                AstcDecompressorParams(256, 256),
                                AstcDecompressorParams(1024, 1024)),
                         PrintToStringParamName());

}  // anonymous namespace
