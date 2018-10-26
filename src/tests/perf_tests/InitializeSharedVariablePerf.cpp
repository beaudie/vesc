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
    Initializer,
    Assignment,

    Unspecified
};

struct InitializeSharedVariableParams final : public RenderTestParams
{
    InitializeSharedVariableParams(InitializeSharedVariableOption optionIn)
    {
        majorVersion = 3;
        minorVersion = 1;
        windowWidth  = 256;
        windowHeight = 256;
        option       = optionIn;
    }

    std::string suffix() const override
    {
        std::stringstream strstr;
        strstr << RenderTestParams::suffix();

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

class InitializeSharedVariableBenchmark : public ANGLERenderTest,
                             public ::testing::WithParamInterface<InitializeSharedVariableParams>
{
  public:
    InitializeSharedVariableBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  protected:
    GLuint mInitializerProgram = 0;
    GLuint mAssignmentProgram = 0;
    GLuint mTexture = 0;
};

InitializeSharedVariableBenchmark::InitializeSharedVariableBenchmark() : ANGLERenderTest("InitializeSharedVariable", GetParam())
{
}

void InitializeSharedVariableBenchmark::initializeBenchmark()
{

    const std::string &csInitializerSource =
        R"(#version 310 es
        layout(local_size_x=32, local_size_y=32) in;

        #define ZERO1 0.0f
        #define ZERO2 ZERO1,ZERO1
        #define ZERO4 ZERO2,ZERO2
        #define ZERO8 ZERO4,ZERO4
        #define ZERO16 ZERO8,ZERO8
        #define ZERO32 ZERO16,ZERO16
        #define ZERO64 ZERO32,ZERO32
        #define ZERO128 ZERO64,ZERO64
        #define ZERO256 ZERO128,ZERO128
        #define ZERO512 ZERO256,ZERO256
        #define ZERO1024 ZERO512,ZERO512

        precision highp float;
        layout(r32f, binding = 4) writeonly uniform highp image2D  outImage;
        shared float data[1024] = float[1024](ZERO1024);

        void main() {
            uint i = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * 32u;
            float value = data[i];
            imageStore(outImage, ivec2(gl_GlobalInvocationID.xy), vec4(value));
        })";

    mInitializerProgram = CompileComputeProgram(csInitializerSource, false);
    ASSERT_NE(0u, mInitializerProgram);

    const std::string &csAssignmentSource =
        R"(#version 310 es
        layout(local_size_x=32, local_size_y=32) in;
        precision highp float;
        layout(r32f, binding = 4) writeonly uniform highp image2D  outImage;
        shared float data[1024];

        void main() {
            uint i = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * 32u;

            data[i] = 0.0f;
            memoryBarrierShared();

            float value = data[i]; 
            imageStore(outImage, ivec2(gl_GlobalInvocationID.xy), vec4(value));
        })";

    mAssignmentProgram = CompileComputeProgram(csAssignmentSource, false);
    ASSERT_NE(0u, mAssignmentProgram);

    std::vector<GLfloat> textureData(1024, 0.1f);
    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, 32, 32);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 32, 32, GL_RED,
                    GL_FLOAT, textureData.data());
    ASSERT_GL_NO_ERROR();

};

void InitializeSharedVariableBenchmark::destroyBenchmark()
{
    glDeleteTextures(1, &mTexture);
    glDeleteProgram(mInitializerProgram);
    glDeleteProgram(mAssignmentProgram);
}

void InitializeSharedVariableBenchmark::drawBenchmark()
{
    if (GetParam().option == InitializeSharedVariableOption::Initializer)
    {
        glUseProgram(mInitializerProgram);
    }
    else
    {
        glUseProgram(mAssignmentProgram);
    }

    glBindImageTexture(4, mTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

    glDispatchCompute(8, 8, 8);
}

using namespace egl_platform;

InitializeSharedVariableParams InitializeSharedVariableD3D11Params(InitializeSharedVariableOption option)
{
    InitializeSharedVariableParams params(option);
    params.eglParameters = D3D11();
    return params;
}

InitializeSharedVariableParams InitializeSharedVariableOpenGLOrGLESParams(InitializeSharedVariableOption option)
{
    InitializeSharedVariableParams params(option);
    params.eglParameters = OPENGL_OR_GLES(false);
    return params;
}

TEST_P(InitializeSharedVariableBenchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(InitializeSharedVariableBenchmark,
                       InitializeSharedVariableD3D11Params(InitializeSharedVariableOption::Initializer),
                       InitializeSharedVariableOpenGLOrGLESParams(InitializeSharedVariableOption::Initializer),
                       InitializeSharedVariableD3D11Params(InitializeSharedVariableOption::Assignment),
                       InitializeSharedVariableOpenGLOrGLESParams(InitializeSharedVariableOption::Assignment));

}  // anonymous namespace
