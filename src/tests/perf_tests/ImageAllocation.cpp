//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageAllocationBenchmark:
//   Test that we can allocate 4096 images, which is the maximum allocation count on some platforms.
//   Image suballocation should enable us to allocate more than this limit.
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
struct ImageAllocationParams final : public RenderTestParams
{
    ImageAllocationParams() { iterationsPerStep = 1; }

    std::string story() const override;
};

std::ostream &operator<<(std::ostream &os, const ImageAllocationParams &params)
{
    return os << params.backendAndStory().substr(1);
}

std::string ImageAllocationParams::story() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::story();

    return strstr.str();
}

class ImageAllocationBenchmark : public ANGLERenderTest,
                                 public ::testing::WithParamInterface<ImageAllocationParams>
{
  public:
    ImageAllocationBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  protected:
    void initShaders();

    GLuint mProgram = 0;
};

ImageAllocationBenchmark::ImageAllocationBenchmark()
    : ANGLERenderTest("ImageAllocation", GetParam())
{}

void ImageAllocationBenchmark::initializeBenchmark()
{
    initShaders();
    ASSERT_GL_NO_ERROR();
}

void ImageAllocationBenchmark::initShaders()
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

void ImageAllocationBenchmark::destroyBenchmark()
{
    glDeleteProgram(mProgram);
}

void ImageAllocationBenchmark::drawBenchmark()
{
    constexpr size_t kTextureCount = 4096;
    GLTexture texture[kTextureCount];
    for (size_t i = 0; i < kTextureCount; i++)
    {
        glBindTexture(GL_TEXTURE_2D, texture[i]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    ASSERT_GL_NO_ERROR();
}

ImageAllocationParams VulkanParams()
{
    ImageAllocationParams params;
    params.eglParameters = egl_platform::VULKAN();
    params.majorVersion  = 3;
    params.minorVersion  = 0;
    return params;
}

}  // anonymous namespace

// Runs the test to make sure we can allocate at least 4096 images.
TEST_P(ImageAllocationBenchmark, Run)
{
    run();
}

using namespace params;

ANGLE_INSTANTIATE_TEST(ImageAllocationBenchmark, VulkanParams());
