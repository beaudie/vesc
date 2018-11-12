//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ZeroInitializerPerf.cpp:
//

#include "ANGLEPerfTest.h"

#include <array>
#include <cstdlib>
#include <ctime>

#include "common/vector_utils.h"
#include "shader_utils.h"

using namespace angle;

namespace
{

enum class ZeroInitializerOption
{
    Local,
    Shared,
    Unspecified
};

struct ZeroInitializerParams final : public RenderTestParams
{
    ZeroInitializerParams(ZeroInitializerOption optionIn)
    {
        majorVersion      = 3;
        minorVersion      = 1;
        windowWidth       = 256;
        windowHeight      = 256;
        iterationsPerStep = 1;
        option            = optionIn;
    }

    std::string suffix() const override
    {
        std::stringstream strstr;
        strstr << RenderTestParams::suffix();

        if (option == ZeroInitializerOption::Local)
        {
            strstr << "_local";
        }
        else if (option == ZeroInitializerOption::Shared)
        {
            strstr << "_shared";
        }

        if (eglParameters.deviceType == EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE)
        {
            strstr << "_null";
        }

        return strstr.str();
    }

    ZeroInitializerOption option;
};

std::ostream &operator<<(std::ostream &os, const ZeroInitializerParams &params)
{
    os << params.suffix().substr(1);
    return os;
}

class ZeroInitializerBenchmark : public ANGLERenderTest,
                                    public ::testing::WithParamInterface<ZeroInitializerParams>
{
  public:
    ZeroInitializerBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  protected:
    std::string mLocalSource;
    std::string mSharedSource;
};

ZeroInitializerBenchmark::ZeroInitializerBenchmark()
    : ANGLERenderTest("ZeroInitializer", GetParam())
{
    mLocalSource =
        R"(#version 310 es
        layout(local_size_x=1) in;
        void main()
        {
            uint data[1024 * 16];
            data[0] += 1u;
        })";

    mSharedSource =
        R"(#version 310 es
        layout(local_size_x=1) in;
        shared uint data[1024 * 16];
        void main()
        {
            data[0] += 1u;
        })";
}

void ZeroInitializerBenchmark::initializeBenchmark(){

};

void ZeroInitializerBenchmark::destroyBenchmark()
{
}

void ZeroInitializerBenchmark::drawBenchmark()
{
    std::srand(std::time(nullptr));
    int rand = std::rand();
    std::string csSource =
        (GetParam().option == ZeroInitializerOption::Local ? mLocalSource : mSharedSource) +
        "// random numbers to fool cache. " + std::to_string(rand) + "\n";
    GLuint program = glCreateProgram();

    GLuint cs = CompileShader(GL_COMPUTE_SHADER, csSource);
    EXPECT_NE(0u, cs);

    glAttachShader(program, cs);
    glDeleteShader(cs);

    glLinkProgram(program);
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    glDetachShader(program, cs);

    glUseProgram(program);
    // glDispatchCompute(8, 4, 2);
    // glUseProgram(0);
    glDeleteProgram(program);
}

ZeroInitializerParams ZeroInitializerD3D11Params(ZeroInitializerOption option)
{
    ZeroInitializerParams params(option);
    params.eglParameters = egl_platform::D3D11();
    return params;
}

TEST_P(ZeroInitializerBenchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(ZeroInitializerBenchmark,
                       ZeroInitializerD3D11Params(ZeroInitializerOption::Local),
                       ZeroInitializerD3D11Params(ZeroInitializerOption::Shared));

}  // anonymous namespace
