//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// InitializeSharedVariablePerf.cpp:
//   Performance tests for initializing shared variables.
//

#include "ANGLEPerfTest.h"

#include <array>

#include "common/vector_utils.h"
#include "shader_utils.h"

using namespace angle;

namespace
{

enum class InitializeSharedVariableOption
{
    Basic,
    Initializer,
    Assignment,

    Unspecified
};

struct InitializeSharedVariableParams final : public RenderTestParams
{
    InitializeSharedVariableParams(InitializeSharedVariableOption optionIn)
    {
        majorVersion      = 3;
        minorVersion      = 1;
        windowWidth       = 256;
        windowHeight      = 256;
        iterationsPerStep = 128;
        option            = optionIn;
    }

    std::string suffix() const override
    {
        std::stringstream strstr;
        strstr << RenderTestParams::suffix();

        if (option == InitializeSharedVariableOption::Initializer)
        {
            strstr << "_basic";
        }
        if (option == InitializeSharedVariableOption::Initializer)
        {
            strstr << "_initializer";
        }
        else if (option == InitializeSharedVariableOption::Assignment)
        {
            strstr << "_assignment";
        }

        if (eglParameters.deviceType == EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE)
        {
            strstr << "_null";
        }

        return strstr.str();
    }

    InitializeSharedVariableOption option;
};

std::ostream &operator<<(std::ostream &os, const InitializeSharedVariableParams &params)
{
    os << params.suffix().substr(1);
    return os;
}

class InitializeSharedVariableBenchmark
    : public ANGLERenderTest,
      public ::testing::WithParamInterface<InitializeSharedVariableParams>
{
  public:
    InitializeSharedVariableBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  protected:
    GLuint mBasicProgram       = 0;
    GLuint mInitializerProgram = 0;
    GLuint mAssignmentProgram  = 0;
    GLuint mTexture            = 0;
};

InitializeSharedVariableBenchmark::InitializeSharedVariableBenchmark()
    : ANGLERenderTest("InitializeSharedVariable", GetParam())
{
}

void InitializeSharedVariableBenchmark::initializeBenchmark()
{
    const std::string &csBasicSource =
        R"(#version 310 es
        layout(local_size_x=4, local_size_y=4) in;
        precision highp float;
        layout(r32f, binding = 4) writeonly uniform highp image2D  outImage;

        void main() {
            imageStore(outImage, ivec2(gl_GlobalInvocationID.xy), vec4(0.0f));
        })";

    mBasicProgram = CompileComputeProgram(csBasicSource, true);
    ASSERT_NE(0u, mBasicProgram);

    std::string csInitializerSource1 =
        R"(#version 310 es
        layout(local_size_x=4, local_size_y=4) in;
        precision highp float;
        layout(r32f, binding = 4) writeonly uniform highp image2D  outImage;)";

    std::stringstream ss;
    ss << "\nshared float data[256] = float[256](0.0f";
    for (int i = 0; i < 255; i++)
    {
        ss << ", 0.0f";
    }
    ss << ");\n";

    std::string csInitializerSource2 =
        R"(
        void main() {
            uint i = (gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * 4u) * 16u;

            float value = data[i];
            imageStore(outImage, ivec2(gl_GlobalInvocationID.xy), vec4(value));
        })";

    std::string csInitializerSource = csInitializerSource1 + ss.str() + csInitializerSource2;

    mInitializerProgram = CompileComputeProgram(csInitializerSource, true);
    ASSERT_NE(0u, mInitializerProgram);

    const std::string &csAssignmentSource =
        R"(#version 310 es
        layout(local_size_x=4, local_size_y=4) in;
        precision highp float;
        layout(r32f, binding = 4) writeonly uniform highp image2D  outImage;
        shared float data[256];

        void main() {
            uint i = (gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * 4u) * 16u;
            uint end = i + 16u;
            for (; i < end; i++)
            {
                data[i] = 0.0f;
            }
            memoryBarrierShared();

            float value = data[i];
            imageStore(outImage, ivec2(gl_GlobalInvocationID.xy), vec4(value));
        })";

    mAssignmentProgram = CompileComputeProgram(csAssignmentSource, true);
    ASSERT_NE(0u, mAssignmentProgram);

    std::vector<GLfloat> textureData(256, 0.1f);
    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, 16, 16);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 16, 16, GL_RED, GL_FLOAT, textureData.data());
    ASSERT_GL_NO_ERROR();
};

void InitializeSharedVariableBenchmark::destroyBenchmark()
{
    glDeleteTextures(1, &mTexture);
    glDeleteProgram(mBasicProgram);
    glDeleteProgram(mInitializerProgram);
    glDeleteProgram(mAssignmentProgram);
}

void InitializeSharedVariableBenchmark::drawBenchmark()
{
    if (GetParam().option == InitializeSharedVariableOption::Basic)
    {
        glUseProgram(mBasicProgram);
    }

    if (GetParam().option == InitializeSharedVariableOption::Initializer)
    {
        glUseProgram(mInitializerProgram);
    }

    if (GetParam().option == InitializeSharedVariableOption::Assignment)
    {
        glUseProgram(mAssignmentProgram);
    }

    glBindImageTexture(4, mTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
    glDispatchCompute(8, 8, 8);
    glFinish();
}

using namespace egl_platform;

InitializeSharedVariableParams InitializeSharedVariableD3D11Params(
    InitializeSharedVariableOption option)
{
    InitializeSharedVariableParams params(option);
    params.eglParameters = D3D11();
    return params;
}

InitializeSharedVariableParams InitializeSharedVariableOpenGLOrGLESParams(
    InitializeSharedVariableOption option)
{
    InitializeSharedVariableParams params(option);
    params.eglParameters = OPENGL_OR_GLES(false);
    return params;
}

TEST_P(InitializeSharedVariableBenchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(
    InitializeSharedVariableBenchmark,
    InitializeSharedVariableD3D11Params(InitializeSharedVariableOption::Basic),
    InitializeSharedVariableOpenGLOrGLESParams(InitializeSharedVariableOption::Basic),
    InitializeSharedVariableD3D11Params(InitializeSharedVariableOption::Initializer),
    InitializeSharedVariableOpenGLOrGLESParams(InitializeSharedVariableOption::Initializer),
    InitializeSharedVariableD3D11Params(InitializeSharedVariableOption::Assignment),
    InitializeSharedVariableOpenGLOrGLESParams(InitializeSharedVariableOption::Assignment));

}  // anonymous namespace
