//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RGBImageAllocationBenchmark:
//   GL_RGB8 image allocation and loading using GL_UNSIGNED_BYTE.
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

struct RGBColor
{
    RGBColor() : r(0), g(0), b(0) {}
    RGBColor(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class RGBImageAllocationBenchmark : public ANGLERenderTest,
                                    public ::testing::WithParamInterface<RGBImageAllocationParams>
{
  public:
    RGBImageAllocationBenchmark();

    void initializeBenchmark() override;
    void drawBenchmark() override;
    void destroyBenchmark() override;

  protected:
    GLuint mTexture;
    std::vector<RGBColor> mColors1;
    std::vector<RGBColor> mColors2;

    static constexpr size_t kTextureDim = 2048;
};

RGBImageAllocationBenchmark::RGBImageAllocationBenchmark()
    : ANGLERenderTest("RGBImageAllocation", GetParam())
{}

void RGBImageAllocationBenchmark::initializeBenchmark()
{
    // Initialize texture
    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, kTextureDim, kTextureDim);

    // Initialize color data
    size_t textureSize = kTextureDim * kTextureDim;
    mColors1.resize(textureSize);
    mColors2.resize(textureSize);

    for (size_t i = 0; i < textureSize; i++)
    {
        mColors1[i] = RGBColor(1, 2, 3);
        mColors2[i] = RGBColor(4, 5, 6);
    }
}

void RGBImageAllocationBenchmark::destroyBenchmark()
{
    glDeleteTextures(1, &mTexture);
}

void RGBImageAllocationBenchmark::drawBenchmark()
{
    // Copy the first color data
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTextureDim, kTextureDim, GL_RGB, GL_UNSIGNED_BYTE,
                    mColors1.data());
    glFinish();
    ASSERT_GL_NO_ERROR();

    // Copy the second color data
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTextureDim, kTextureDim, GL_RGB, GL_UNSIGNED_BYTE,
                    mColors2.data());
    glFinish();
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
