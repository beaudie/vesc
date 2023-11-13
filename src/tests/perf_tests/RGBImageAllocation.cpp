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

    bool usesSameDataPerUpdate = false;
};

std::ostream &operator<<(std::ostream &os, const RGBImageAllocationParams &params)
{
    return os << params.backendAndStory().substr(1);
}

std::string RGBImageAllocationParams::story() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::story()
           << (usesSameDataPerUpdate ? "_sameDataPerUpdate" : "_changeDataPerUpdate");

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
    uint32_t mIteration;
    std::vector<std::vector<RGBColor>> mColors;
    bool mUsesSameDataPerUpdate;

    // The size is set to a power of two for easier data allocation and avoiding alignment issues.
    static constexpr size_t kTextureDim = 2048;

    static constexpr size_t kColorArrayCount = 64;
};

RGBImageAllocationBenchmark::RGBImageAllocationBenchmark()
    : ANGLERenderTest("RGBImageAllocation", GetParam())
{
    mUsesSameDataPerUpdate = GetParam().usesSameDataPerUpdate;
}

void RGBImageAllocationBenchmark::initializeBenchmark()
{
    mIteration = 0;

    // Initialize texture
    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, kTextureDim, kTextureDim);

    // Initialize color data
    mColors.resize(kColorArrayCount);
    for (size_t n = 0; n < mColors.size(); n++)
    {
        mColors[n].resize(kTextureDim * kTextureDim);
        for (size_t i = 0; i < kTextureDim * kTextureDim; i++)
        {
            mColors[n][i] = RGBColor(n + 1, n + 2, n + 3);
        }
    }
}

void RGBImageAllocationBenchmark::destroyBenchmark()
{
    glDeleteTextures(1, &mTexture);
}

void RGBImageAllocationBenchmark::drawBenchmark()
{
    // Copy the next color data
    mIteration++;
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTextureDim, kTextureDim, GL_RGB, GL_UNSIGNED_BYTE,
                    mColors[mIteration % kColorArrayCount].data());
    glFinish();
    ASSERT_GL_NO_ERROR();
}

RGBImageAllocationParams VulkanParams(bool usesSameDataPerUpdate)
{
    RGBImageAllocationParams params;
    params.eglParameters         = egl_platform::VULKAN();
    params.majorVersion          = 3;
    params.minorVersion          = 0;
    params.usesSameDataPerUpdate = usesSameDataPerUpdate;
    return params;
}

RGBImageAllocationParams OpenGLOrGLESParams(bool usesSameDataPerUpdate)
{
    RGBImageAllocationParams params;
    params.eglParameters         = egl_platform::OPENGL_OR_GLES();
    params.majorVersion          = 3;
    params.minorVersion          = 0;
    params.usesSameDataPerUpdate = usesSameDataPerUpdate;
    return params;
}

RGBImageAllocationParams MetalParams(bool usesSameDataPerUpdate)
{
    RGBImageAllocationParams params;
    params.eglParameters         = egl_platform::METAL();
    params.majorVersion          = 3;
    params.minorVersion          = 0;
    params.usesSameDataPerUpdate = usesSameDataPerUpdate;
    return params;
}

RGBImageAllocationParams D3D11Params(bool usesSameDataPerUpdate)
{
    RGBImageAllocationParams params;
    params.eglParameters         = egl_platform::D3D11();
    params.majorVersion          = 3;
    params.minorVersion          = 0;
    params.usesSameDataPerUpdate = usesSameDataPerUpdate;
    return params;
}

}  // anonymous namespace

// Runs the test to measure the performance of RGB8 image allocation and loading.
TEST_P(RGBImageAllocationBenchmark, Run)
{
    run();
}

using namespace params;

ANGLE_INSTANTIATE_TEST(RGBImageAllocationBenchmark,
                       VulkanParams(true),
                       VulkanParams(false),
                       OpenGLOrGLESParams(true),
                       OpenGLOrGLESParams(false),
                       MetalParams(true),
                       MetalParams(false),
                       D3D11Params(true),
                       D3D11Params(false));
