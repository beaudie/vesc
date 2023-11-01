//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageSuballocationBenchmark:
//   Tests allocation of RGB/RGBA images.
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

// For RGBA
// struct Color
//{
//    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}
//    uint8_t r;
//    uint8_t g;
//    uint8_t b;
//    uint8_t a;
//};
//
// void ImageSuballocationBenchmark::drawBenchmark()
//{
//    constexpr size_t kNumTextures = 16;
//    GLTexture texture[kNumTextures];
//    std::vector<Color> colors(2048 * 2048, Color(255, 0, 0, 255));
//    for (size_t i = 0; i < kNumTextures; i++)
//    {
//        glBindTexture(GL_TEXTURE_2D, texture[i]);
//        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 2048, 2048);
//        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2048, 2048, GL_RGBA, GL_UNSIGNED_BYTE,
//        colors.data());
//        glFinish();
//    }
//
//    ASSERT_GL_NO_ERROR();
//}
// End of RGBA

// For RGB
struct Color
{
    Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

void ImageSuballocationBenchmark::drawBenchmark()
{
    constexpr size_t kNumTextures = 16;
    GLTexture texture[kNumTextures];
    std::vector<Color> colors(2048 * 2048, Color(250, 0, 0));

    // Test: Marking the beginning and the end of the data.
    colors[0]               = Color(1, 2, 3);
    colors[1]               = Color(4, 5, 6);
    colors[2048 * 2048 - 1] = Color(254, 253, 252);

    for (size_t i = 0; i < kNumTextures; i++)
    {
        glBindTexture(GL_TEXTURE_2D, texture[i]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, 2048, 2048);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2048, 2048, GL_RGB, GL_UNSIGNED_BYTE,
                        colors.data());
        glFinish();
    }

    ASSERT_GL_NO_ERROR();
}
// End of RGB

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
