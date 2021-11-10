//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ResultPerf:
//   Performance test for ANGLE's Error result class.
//

#include "ANGLEPerfTest.h"
#include "libANGLE/Error.h"
#include "util/test_utils.h"

#include <vector>

namespace
{
constexpr unsigned int kIterationsPerStep = 1000;

struct CpuTimeParams final : public RenderTestParams
{
    CpuTimeParams()
    {
        // Common default values
        majorVersion      = 3;
        minorVersion      = 0;
        windowWidth       = 512;
        windowHeight      = 512;
        iterationsPerStep = kIterationsPerStep;
        sleepy            = false;
    }

    std::string story() const override;
    bool sleepy;
};

std::string CpuTimeParams::story() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::story();

    strstr << "_steps" << iterationsPerStep;
    strstr << "_sleepy" << sleepy;

    return strstr.str();
}

std::ostream &operator<<(std::ostream &os, const CpuTimeParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

class CpuTimeBenchmark : public ANGLERenderTest, public ::testing::WithParamInterface<CpuTimeParams>
{
  public:
    CpuTimeBenchmark();
    void initializeBenchmark() override {}
    void destroyBenchmark() override {}
    void drawBenchmark() override
    {
        const auto &params = GetParam();
        if (params.sleepy)
        {
            for (unsigned int i = 0; i < params.iterationsPerStep; ++i)
            {
                angle::Sleep(1);
            }
        }
        else
        {
            constexpr size_t bufferSize1Mb = 1 << 20;
            std::vector<uint8_t> buffer(bufferSize1Mb);
            std::vector<uint8_t> zeroBuffer(bufferSize1Mb, 0);
            for (unsigned int i = 0; i < params.iterationsPerStep; ++i)
            {
                memset(buffer.data(), 0, buffer.size());
                ASSERT_EQ(0, memcmp(buffer.data(), zeroBuffer.data(), buffer.size()));
            }
        }
    }
};

CpuTimeBenchmark::CpuTimeBenchmark() : ANGLERenderTest("CpuTimeBenchmark", GetParam()) {}

CpuTimeParams CpuTimeNull()
{
    CpuTimeParams params;
    params.eglParameters = angle::egl_platform::OPENGL_OR_GLES_NULL();
    return params;
}

CpuTimeParams CpuTimeNullSleepy()
{
    CpuTimeParams params;
    params.eglParameters = angle::egl_platform::OPENGL_OR_GLES_NULL();
    params.sleepy        = true;
    return params;
}

// Tests CPU time measurement against something that takes a lot of CPU cycles
TEST_P(CpuTimeBenchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(CpuTimeBenchmark, CpuTimeNull(), CpuTimeNullSleepy());

}  // anonymous namespace
