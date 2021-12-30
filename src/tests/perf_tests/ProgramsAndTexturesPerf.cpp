//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramsAndTexturesPerf:
//   Performance tests using a combination of many Programs and Textures.
//

#include "ANGLEPerfTest.h"

#include "test_utils/draw_call_perf_utils.h"
#include "util/shader_utils.h"

using namespace angle;

struct ProgramsAndTexturesParams : public RenderTestParams
{
    ProgramsAndTexturesParams() {}

    GLsizei textureCount = 10;
    GLsizei programCount = 10;
    GLsizei texureSize   = 1;
};

std::ostream &operator<<(std::ostream &os, const ProgramsAndTexturesParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

class ProgramsAndTexturesPerf : public ANGLERenderTest,
                                public ::testing::WithParamInterface<ProgramsAndTexturesParams>
{
  public:
    ProgramsAndTexturesPerf() : ANGLERenderTest("ProgramsAndTexturesPerf", GetParam()) {}

    void initializeBenchmark() override
    {
        const ProgramsAndTexturesParams &p = GetParam();

        mTextures.resize(p.textureCount, 0);
        glGenTextures(p.textureCount, mTextures.data());

        constexpr std::array<GLubyte, 4> kRed = {255, 0, 0, 255};

        for (GLuint texture : mTextures)
        {
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, p.texureSize, p.texureSize, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, &kRed);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }

        for (GLsizei programIndex = 0; programIndex < p.programCount; ++programIndex)
        {
            GLuint newProg =
                CompileProgram(essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
            glUseProgram(newProg);
            glBindAttribLocation(newProg, 0, essl1_shaders::PositionAttrib());
            glLinkProgram(newProg);
            mPrograms.push_back(newProg);
        }

        mVertexBuffer = Create2DTriangleBuffer(1, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    void destroyBenchmark() override
    {
        glDeleteBuffers(1, &mVertexBuffer);

        const ProgramsAndTexturesParams &p = GetParam();
        glDeleteTextures(p.textureCount, mTextures.data());
        mTextures.clear();

        for (GLuint delProg : mPrograms)
        {
            glDeleteProgram(delProg);
        }
        mPrograms.clear();
    }

    void drawBenchmark() override
    {
        glClear(GL_COLOR_BUFFER_BIT);
        for (GLuint drawProg : mPrograms)
        {
            glUseProgram(drawProg);
            for (GLuint texture : mTextures)
            {
                glBindTexture(GL_TEXTURE_2D, texture);
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
        }
    }

  protected:
    std::vector<GLuint> mTextures;
    std::vector<GLuint> mPrograms;
    GLuint mVertexBuffer = 0;
};

TEST_P(ProgramsAndTexturesPerf, Run)
{
    run();
}

ProgramsAndTexturesParams ProgramsAndTexturesVulkanParams()
{
    ProgramsAndTexturesParams params;
    params.eglParameters     = egl_platform::VULKAN();
    params.iterationsPerStep = 1;
    return params;
}

ANGLE_INSTANTIATE_TEST(ProgramsAndTexturesPerf, ProgramsAndTexturesVulkanParams());
