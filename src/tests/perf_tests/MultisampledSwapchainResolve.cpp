//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MultisampledSwapchainResolveBenchmark:
//   Performance test for resolving multisample swapchains in subpass
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
struct MultisampledSwapchainResolveParams final : public RenderTestParams
{
    MultisampledSwapchainResolveParams()
    {
        iterationsPerStep = 1;
        trackGpuTime      = true;

        windowWidth  = 1920;
        windowHeight = 1080;
        multisample  = true;
        samples      = 4;
    }

    std::string story() const override;
};

std::ostream &operator<<(std::ostream &os, const MultisampledSwapchainResolveParams &params)
{
    return os << params.backendAndStory().substr(1);
}

std::string MultisampledSwapchainResolveParams::story() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::story();

    return strstr.str();
}

class MultisampledSwapchainResolveBenchmark
    : public ANGLERenderTest,
      public ::testing::WithParamInterface<MultisampledSwapchainResolveParams>
{
  public:
    MultisampledSwapchainResolveBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  protected:
    void initShaders();

    GLuint mProgram = 0;
};

MultisampledSwapchainResolveBenchmark::MultisampledSwapchainResolveBenchmark()
    : ANGLERenderTest("MultisampledSwapchainResolve", GetParam())
{
    if (GetParam().getRenderer() == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        skipTest("http://crbug.com/945415 Crashes on nvidia+d3d11");
    }

    if (IsLinux() && GetParam().eglParameters.renderer == EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE)
    {
        skipTest("Fails on Linux GL");
    }
}

void MultisampledSwapchainResolveBenchmark::initializeBenchmark()
{
    initShaders();
    glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

    ASSERT_GL_NO_ERROR();
}

void MultisampledSwapchainResolveBenchmark::initShaders()
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

void MultisampledSwapchainResolveBenchmark::destroyBenchmark()
{
    glDeleteProgram(mProgram);
}

void MultisampledSwapchainResolveBenchmark::drawBenchmark()
{
    // Initially clear the color attachment to avoid having to load from the resolved image.
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Perform a draw just to have something in the render pass.  With the position attributes
    // not set, a constant default value is used, resulting in a very cheap draw.
    glDrawArrays(GL_TRIANGLES, 0, 3);

    ASSERT_GL_NO_ERROR();
}

MultisampledSwapchainResolveParams D3D11Params()
{
    MultisampledSwapchainResolveParams params;
    params.eglParameters = egl_platform::D3D11();
    params.majorVersion  = 3;
    params.minorVersion  = 0;
    return params;
}

MultisampledSwapchainResolveParams OpenGLOrGLESParams()
{
    MultisampledSwapchainResolveParams params;
    params.eglParameters = egl_platform::OPENGL_OR_GLES();
    params.majorVersion  = 3;
    params.minorVersion  = 0;
    return params;
}

MultisampledSwapchainResolveParams VulkanParams()
{
    MultisampledSwapchainResolveParams params;
    params.eglParameters = egl_platform::VULKAN();
    params.majorVersion  = 3;
    params.minorVersion  = 0;
    return params;
}

}  // anonymous namespace

TEST_P(MultisampledSwapchainResolveBenchmark, Run)
{
    run();
}

using namespace params;

ANGLE_INSTANTIATE_TEST(MultisampledSwapchainResolveBenchmark,
                       D3D11Params(),
                       OpenGLOrGLESParams(),
                       VulkanParams());
