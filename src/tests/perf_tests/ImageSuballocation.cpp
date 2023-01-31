//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageSuballocationBenchmark:
//   Tests allocation of at least 4096 images.
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
struct ImageSuballocationParams final : public RenderTestParams
{
    ImageSuballocationParams() { iterationsPerStep = 1; }

    std::string story() const override;
};

std::ostream &operator<<(std::ostream &os, const ImageSuballocationParams &params)
{
    return os << params.backendAndStory().substr(1);
}

std::string ImageSuballocationParams::story() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::story();

    return strstr.str();
}

class ImageSuballocationBenchmark : public ANGLERenderTest,
                                    public ::testing::WithParamInterface<ImageSuballocationParams>
{
  public:
    ImageSuballocationBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  protected:
    void initShaders();

    GLuint mProgram = 0;
};

ImageSuballocationBenchmark::ImageSuballocationBenchmark()
    : ANGLERenderTest("ImageSuballocation", GetParam())
{}

void ImageSuballocationBenchmark::initializeBenchmark()
{
    initShaders();
    ASSERT_GL_NO_ERROR();
}

void ImageSuballocationBenchmark::initShaders()
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

void ImageSuballocationBenchmark::destroyBenchmark()
{
    glDeleteProgram(mProgram);
}

void ImageSuballocationBenchmark::drawBenchmark()
{
    constexpr size_t kNumTextures = 4096;
    GLTexture texture[kNumTextures];
    for (size_t i = 0; i < kNumTextures; i++)
    {
        glBindTexture(GL_TEXTURE_2D, texture[i]);
        glTexStorage2D(GL_TEXTURE_2D, 2, GL_RGBA8, 2, 2);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    ASSERT_GL_NO_ERROR();
}

ImageSuballocationParams VulkanParams()
{
    ImageSuballocationParams params;
    params.eglParameters = egl_platform::VULKAN();
    params.majorVersion  = 3;
    params.minorVersion  = 0;
    return params;
}

}  // anonymous namespace

// Runs the test to make sure we can allocate at least 4096 images.
TEST_P(ImageSuballocationBenchmark, Run)
{
    run();
}

using namespace params;

ANGLE_INSTANTIATE_TEST(ImageSuballocationBenchmark, VulkanParams());
