//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// LinkProgramPerfTest:
//   Performance tests compiling a lot of shaders.
//

#include "ANGLEPerfTest.h"

#include <array>

#include "common/vector_utils.h"
#include "util/EGLWindow.h"
#include "util/shader_utils.h"

#include "LinkProgramPerfTest_shaders.h"

using namespace angle;

namespace
{

enum class TaskOption
{
    CompileOnly,
    CompileAndLink,
    ProgramBinary,

    Unspecified
};

enum class ThreadOption
{
    SingleThread,
    MultiThread,

    Unspecified
};

enum class ProgramCacheOption
{
    MetalBinaryArchive,

    Unspecified
};

struct LinkProgramParams final : public RenderTestParams
{
    LinkProgramParams(TaskOption taskOptionIn,
                      ThreadOption threadOptionIn,
                      ProgramCacheOption programCacheOptionIn = ProgramCacheOption::Unspecified)
    {
        iterationsPerStep = 1;

        majorVersion = 3;
        minorVersion = 0;
        windowWidth  = 256;
        windowHeight = 256;
        taskOption   = taskOptionIn;
        threadOption = threadOptionIn;
        programIndex = 0;

        switch (programCacheOptionIn)
        {
            case ProgramCacheOption::MetalBinaryArchive:
                mtlBinaryArchiveCache = true;
                break;
            default:
                break;
        }
    }

    std::string story() const override
    {
        std::stringstream strstr;
        strstr << RenderTestParams::story();

        if (taskOption == TaskOption::CompileOnly)
        {
            strstr << "_compile_only";
        }
        else if (taskOption == TaskOption::CompileAndLink)
        {
            strstr << "_compile_and_link";
        }
        else if (taskOption == TaskOption::ProgramBinary)
        {
            strstr << "_program_binary";
        }

        if (threadOption == ThreadOption::SingleThread)
        {
            strstr << "_single_thread";
        }
        else if (threadOption == ThreadOption::MultiThread)
        {
            strstr << "_multi_thread";
        }

        strstr << "_program_" << kPrograms[programIndex].name;

        if (eglParameters.deviceType == EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE)
        {
            strstr << "_null";
        }

        return strstr.str();
    }

    TaskOption taskOption;
    ThreadOption threadOption;
    size_t programIndex;
};

std::ostream &operator<<(std::ostream &os, const LinkProgramParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

class LinkProgramBenchmark : public ANGLERenderTest,
                             public ::testing::WithParamInterface<LinkProgramParams>
{
  public:
    LinkProgramBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  protected:
    GLuint compileAndLinkProgram();

    GLuint mVertexBuffer = 0;

    GLenum mBinaryFormat = 0;
    std::vector<uint8_t> mBinary;
};

LinkProgramBenchmark::LinkProgramBenchmark() : ANGLERenderTest("LinkProgram", GetParam()) {}

GLuint LinkProgramBenchmark::compileAndLinkProgram()
{
    const ProgramInfo &programInfo = kPrograms[GetParam().programIndex];

    GLuint program = glCreateProgram();
    EXPECT_NE(0u, program);

    for (const auto &shaderInfo : programInfo.shaders)
    {
        GLenum type        = shaderInfo.first;
        const char *source = shaderInfo.second;
        GLuint shader      = CompileShader(type, source);
        EXPECT_NE(0u, shader);
        glAttachShader(program, shader);
        glDeleteShader(shader);
    }

    glLinkProgram(program);

    return CheckLinkStatusAndReturnProgram(program, true);
}

void LinkProgramBenchmark::initializeBenchmark()
{
    if (GetParam().threadOption != ThreadOption::SingleThread &&
        !IsGLExtensionEnabled("GL_KHR_parallel_shader_compile"))
    {
        skipTest("non-single-thread but missing GL_KHR_parallel_shader_compile");
        return;
    }

    if (IsGLExtensionEnabled("GL_KHR_parallel_shader_compile") &&
        GetParam().threadOption == ThreadOption::SingleThread)
    {
        glMaxShaderCompilerThreadsKHR(0);
    }

    if (GetParam().driver == GLESDriverType::AngleEGL)
    {
        EGLDisplay display = static_cast<EGLWindow *>(getGLWindow())->getDisplay();
        if (IsEGLDisplayExtensionEnabled(display, "EGL_ANGLE_metal_program_cache_control"))
        {
            eglProgramCacheResizeANGLE(display, 9000, EGL_METAL_BINARY_ARCHIVE_CACHE_RESIZE_ANGLE);
        }
    }

    std::array<Vector3, 6> vertices = {{Vector3(-1.0f, 1.0f, 0.5f), Vector3(-1.0f, -1.0f, 0.5f),
                                        Vector3(1.0f, -1.0f, 0.5f), Vector3(-1.0f, 1.0f, 0.5f),
                                        Vector3(1.0f, -1.0f, 0.5f), Vector3(1.0f, 1.0f, 0.5f)}};

    glGenBuffers(1, &mVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vector3), vertices.data(),
                 GL_STATIC_DRAW);

    if (GetParam().taskOption == TaskOption::ProgramBinary)
    {
        GLuint program = compileAndLinkProgram();
        ASSERT_NE(0u, program);

        GLint binarySize = 0;
        glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &binarySize);
        mBinary.resize(binarySize, 0);
        glGetProgramBinary(program, binarySize, nullptr, &mBinaryFormat, mBinary.data());

        glDeleteProgram(program);
    }
}

void LinkProgramBenchmark::destroyBenchmark()
{
    glDeleteBuffers(1, &mVertexBuffer);
}

void LinkProgramBenchmark::drawBenchmark()
{
    if (GetParam().taskOption == TaskOption::CompileOnly)
    {
        const ProgramInfo &programInfo = kPrograms[GetParam().programIndex];
        for (const auto &shaderInfo : programInfo.shaders)
        {
            GLenum type        = shaderInfo.first;
            const char *source = shaderInfo.second;
            GLuint shader      = CompileShader(type, source);
            glDeleteShader(shader);
        }
    }
    else if (GetParam().taskOption == TaskOption::CompileAndLink)
    {
        GLuint program = compileAndLinkProgram();
        glUseProgram(program);

        // Draw with the program to ensure the shader gets compiled and used.
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDeleteProgram(program);
    }
    else if (GetParam().taskOption == TaskOption::ProgramBinary)
    {
        GLuint program = glCreateProgram();
        ASSERT_NE(0u, program);

        glProgramBinary(program, mBinaryFormat, mBinary.data(), mBinary.size());

        glUseProgram(program);

        // Draw with the program to ensure the shader gets compiled and used.
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDeleteProgram(program);
    }
}

using namespace egl_platform;

LinkProgramParams LinkProgramD3D11Params(TaskOption taskOption, ThreadOption threadOption)
{
    LinkProgramParams params(taskOption, threadOption);
    params.eglParameters = D3D11();
    return params;
}

LinkProgramParams LinkProgramMetalParams(
    TaskOption taskOption,
    ThreadOption threadOption,
    ProgramCacheOption programCacheOption = ProgramCacheOption::Unspecified)
{
    LinkProgramParams params(taskOption, threadOption, programCacheOption);
    params.eglParameters = METAL();
    return params;
}

LinkProgramParams LinkProgramOpenGLOrGLESParams(TaskOption taskOption, ThreadOption threadOption)
{
    LinkProgramParams params(taskOption, threadOption);
    params.eglParameters = OPENGL_OR_GLES();
    return params;
}

LinkProgramParams LinkProgramVulkanParams(TaskOption taskOption, ThreadOption threadOption)
{
    LinkProgramParams params(taskOption, threadOption);
    params.eglParameters = VULKAN();
    return params;
}

TEST_P(LinkProgramBenchmark, Run)
{
    run();
}

LinkProgramParams CombineParamsWithProgram(const LinkProgramParams &in, size_t programIndex)
{
    LinkProgramParams out = in;
    out.programIndex      = programIndex;
    return out;
}

std::vector<LinkProgramParams> gConfigurations = {
    LinkProgramD3D11Params(TaskOption::CompileOnly, ThreadOption::MultiThread),
    LinkProgramMetalParams(TaskOption::CompileOnly, ThreadOption::MultiThread),
    LinkProgramOpenGLOrGLESParams(TaskOption::CompileOnly, ThreadOption::MultiThread),
    LinkProgramVulkanParams(TaskOption::CompileOnly, ThreadOption::MultiThread),
    LinkProgramD3D11Params(TaskOption::CompileAndLink, ThreadOption::MultiThread),
    LinkProgramMetalParams(TaskOption::CompileAndLink, ThreadOption::MultiThread),
    LinkProgramOpenGLOrGLESParams(TaskOption::CompileAndLink, ThreadOption::MultiThread),
    LinkProgramVulkanParams(TaskOption::CompileAndLink, ThreadOption::MultiThread),
    LinkProgramD3D11Params(TaskOption::CompileOnly, ThreadOption::SingleThread),
    LinkProgramMetalParams(TaskOption::CompileOnly, ThreadOption::SingleThread),
    LinkProgramOpenGLOrGLESParams(TaskOption::CompileOnly, ThreadOption::SingleThread),
    LinkProgramVulkanParams(TaskOption::CompileOnly, ThreadOption::SingleThread),
    LinkProgramD3D11Params(TaskOption::CompileAndLink, ThreadOption::SingleThread),
    LinkProgramMetalParams(TaskOption::CompileAndLink, ThreadOption::SingleThread),
    LinkProgramMetalParams(TaskOption::ProgramBinary,
                           ThreadOption::SingleThread,
                           ProgramCacheOption::Unspecified),
    LinkProgramMetalParams(TaskOption::ProgramBinary,
                           ThreadOption::SingleThread,
                           ProgramCacheOption::MetalBinaryArchive),
    LinkProgramOpenGLOrGLESParams(TaskOption::CompileAndLink, ThreadOption::SingleThread),
    LinkProgramVulkanParams(TaskOption::CompileAndLink, ThreadOption::SingleThread)};

std::vector<LinkProgramParams> gConfigurationsWithEnumeratedPrograms =
    CombineWithIndicies(gConfigurations, 0u, ArraySize(kPrograms), CombineParamsWithProgram);

ANGLE_INSTANTIATE_TEST_ARRAY(LinkProgramBenchmark, gConfigurationsWithEnumeratedPrograms);

}  // anonymous namespace
