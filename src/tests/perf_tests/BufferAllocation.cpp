//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BufferAllocation:
//   Performance test for buffer allocation and deallocation repeatedly.
//

#include "ANGLEPerfTest.h"
#include "test_utils/gl_raii.h"

#include <iostream>
#include <random>
#include <sstream>

namespace angle
{
constexpr unsigned int kIterationsPerStep = 1;

struct BufferAllocationParams final : public RenderTestParams
{
    BufferAllocationParams()
    {
        iterationsPerStep = kIterationsPerStep;

        // Common default params
        majorVersion = 3;
        minorVersion = 0;
    }

    std::string story() const override;
};

std::ostream &operator<<(std::ostream &os, const BufferAllocationParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

std::string BufferAllocationParams::story() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::story();

    return strstr.str();
}

class BufferAllocationBenchmark : public ANGLERenderTest,
                                  public ::testing::WithParamInterface<BufferAllocationParams>
{
  public:
    BufferAllocationBenchmark() : ANGLERenderTest("BufferAllocation", GetParam()) {}
    void initializeBenchmark() override;
    void drawBenchmark() override;

  protected:
    GLuint mProgram = 0;
};

void BufferAllocationBenchmark::initializeBenchmark()
{
    constexpr char kVS[] = R"(void main()
{
    gl_Position = vec4(0);
})";

    constexpr char kFS[] = R"(void main(void)
{
    gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
})";

    mProgram = CompileProgram(kVS, kFS);
    ASSERT_NE(0u, mProgram);
    glUseProgram(mProgram);
}

void BufferAllocationBenchmark::drawBenchmark()
{
    constexpr uint64_t kBufferSize  = 1;
    constexpr uint64_t kBufferCount = 0xAB54;
    const auto &params              = GetParam();

    for (size_t it = 0; it < params.iterationsPerStep; ++it)
    {
        GLBuffer buffer[kBufferCount];

        for (uint64_t i = 0; i < kBufferCount; i++)
        {
            glBindBuffer(GL_ARRAY_BUFFER, buffer[i]);
            glBufferData(GL_ARRAY_BUFFER, kBufferSize, nullptr, GL_STATIC_DRAW);
        }
        glDrawArrays(GL_POINTS, 1, 0);
    }

    ASSERT_GL_NO_ERROR();
}

BufferAllocationParams VulkanTestParams()
{
    BufferAllocationParams params;
    params.eglParameters = egl_platform::VULKAN();

    return params;
}

// Runs tests to measure buffer allocation performance
TEST_P(BufferAllocationBenchmark, Run)
{
    run();
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(BufferAllocationBenchmark);
ANGLE_INSTANTIATE_TEST(BufferAllocationBenchmark, VulkanTestParams());

}  // namespace angle
