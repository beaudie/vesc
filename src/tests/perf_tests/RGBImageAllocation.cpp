//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RGBImageAllocationBenchmark:
//   Tests allocation of RGB8 image allocation and loading.
//

#include "ANGLEPerfTest.h"

#include <iostream>
#include <random>
#include <sstream>

#include "test_utils/gl_raii.h"
#include "util/shader_utils.h"

using namespace angle;

namespace
{
struct RGBImageAllocationParams final : public RenderTestParams
{
    RGBImageAllocationParams() { iterationsPerStep = 1; }

    std::string story() const override;
};

std::ostream &operator<<(std::ostream &os, const RGBImageAllocationParams &params)
{
    return os << params.backendAndStory().substr(1);
}

std::string RGBImageAllocationParams::story() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::story();

    return strstr.str();
}

class RGBImageAllocationBenchmark : public ANGLERenderTest,
                                    public ::testing::WithParamInterface<RGBImageAllocationParams>
{
  public:
    RGBImageAllocationBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  protected:
    void initShaders();

    GLuint mProgram = 0;
};

RGBImageAllocationBenchmark::RGBImageAllocationBenchmark()
    : ANGLERenderTest("RGBImageAllocation", GetParam())
{}

void RGBImageAllocationBenchmark::initializeBenchmark()
{
    initShaders();
    ASSERT_GL_NO_ERROR();
}

void RGBImageAllocationBenchmark::initShaders()
{
    constexpr char kVS[] = R"(void main()
{
    gl_Position = vec4(0);
})";

    constexpr char kFS[] = R"(void main(void)
{
    gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
})";

    mProgram = CompileProgram(kVS, kFS);
    ASSERT_NE(0u, mProgram);

    glUseProgram(mProgram);

    ASSERT_GL_NO_ERROR();
}

void RGBImageAllocationBenchmark::destroyBenchmark()
{
    glDeleteProgram(mProgram);
}

struct RGBColor
{
    RGBColor(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

void RGBImageAllocationBenchmark::drawBenchmark()
{
    constexpr size_t kNumTextures = 16;
    constexpr size_t kTextureDim  = 2048;
    GLTexture texture[kNumTextures];
    std::vector<RGBColor> colors(kTextureDim * kTextureDim, RGBColor(1, 2, 3));

    for (size_t i = 0; i < kNumTextures; i++)
    {
        glBindTexture(GL_TEXTURE_2D, texture[i]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, kTextureDim, kTextureDim);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTextureDim, kTextureDim, GL_RGB, GL_UNSIGNED_BYTE,
                        colors.data());
        glFinish();
    }

    ASSERT_GL_NO_ERROR();
}

RGBImageAllocationParams VulkanParams()
{
    RGBImageAllocationParams params;
    params.eglParameters = egl_platform::VULKAN();
    params.majorVersion  = 3;
    params.minorVersion  = 0;
    return params;
}

}  // anonymous namespace

// Runs the test to measure the performance of RGB8 image allocation and loading.
TEST_P(RGBImageAllocationBenchmark, Run)
{
    run();
}

using namespace params;

ANGLE_INSTANTIATE_TEST(RGBImageAllocationBenchmark, VulkanParams());
