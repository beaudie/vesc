//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TextureUploadPerf:
//   Performance test for uploading texture data.
//

#include "ANGLEPerfTest.h"

#include <iostream>
#include <random>
#include <sstream>

#include "test_utils/gl_raii.h"
#include "util/shader_utils.h"

namespace angle
{
constexpr unsigned int kIterationsPerStep = 64;

struct TextureUploadParams final : public RenderTestParams
{
    TextureUploadParams()
    {
        iterationsPerStep = kIterationsPerStep;
        trackGpuTime      = true;

        baseSize = 2048;
        mipCount = 12;

        webgl = false;
    }

    std::string suffix() const override;

    GLsizei baseSize;
    GLint mipCount;

    bool webgl;
};

std::ostream &operator<<(std::ostream &os, const TextureUploadParams &params)
{
    os << params.suffix().substr(1);
    return os;
}

std::string TextureUploadParams::suffix() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::suffix();

    if (webgl)
    {
        strstr << "_webgl";
    }

    return strstr.str();
}

class TextureUploadBenchmark : public ANGLERenderTest,
                               public ::testing::WithParamInterface<TextureUploadParams>
{
  public:
    TextureUploadBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  private:
    void initShaders();

    std::vector<GLuint> mTextures;

    GLuint mProgram;
    GLuint mPositionLoc;
    GLuint mSamplerLoc;
};

TextureUploadBenchmark::TextureUploadBenchmark()
    : ANGLERenderTest("TextureUpload", GetParam()), mProgram(0u), mPositionLoc(-1), mSamplerLoc(-1)
{
    setWebGLCompatibilityEnabled(GetParam().webgl);
    setRobustResourceInit(GetParam().webgl);
}

void TextureUploadBenchmark::initializeBenchmark()
{
    const auto &params = GetParam();

    initShaders();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

    if (params.webgl)
    {
        glRequestExtensionANGLE("GL_EXT_disjoint_timer_query");
    }

    ASSERT_GL_NO_ERROR();
}

void TextureUploadBenchmark::initShaders()
{
    constexpr char kVS[] = R"(attribute vec4 a_position;
void main()
{
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(precision mediump float;
uniform sampler2D s_texture;
void main()
{
    gl_FragColor = texture2D(s_texture, vec2(0, 0));
})";

    mProgram = CompileProgram(kVS, kFS);
    ASSERT_NE(0u, mProgram);

    mPositionLoc = glGetAttribLocation(mProgram, "a_position");
    mSamplerLoc  = glGetUniformLocation(mProgram, "s_texture");
    glUseProgram(mProgram);

    glDisable(GL_DEPTH_TEST);

    ASSERT_GL_NO_ERROR();
}

void TextureUploadBenchmark::destroyBenchmark()
{
    glDeleteProgram(mProgram);
}

void TextureUploadBenchmark::drawBenchmark()
{
    const auto &params = GetParam();

    std::vector<float> textureData(params.baseSize * params.baseSize * 4, 0.5);

    startGpuTimer();
    for (size_t it = 0; it < params.iterationsPerStep; ++it)
    {
        GLTexture tex;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);

        // Stage data for all mips
        for (GLint mip = 0; mip < params.mipCount; mip++)
        {
            GLsizei levelSize = params.baseSize >> mip;
            glTexImage2D(GL_TEXTURE_2D, mip, GL_RGBA, levelSize, levelSize, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, textureData.data());
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glUniform1i(mSamplerLoc, 0);

        // Perform a draw just so the texture data is flushed.
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    stopGpuTimer();

    ASSERT_GL_NO_ERROR();
}

TextureUploadParams TextureUploadD3D11Params(bool webglCompat)
{
    TextureUploadParams params;
    params.eglParameters = egl_platform::D3D11();
    params.webgl         = webglCompat;
    return params;
}

TextureUploadParams TextureUploadOpenGLOrGLESParams(bool webglCompat)
{
    TextureUploadParams params;
    params.eglParameters = egl_platform::OPENGL_OR_GLES(false);
    params.webgl         = webglCompat;
    return params;
}

TextureUploadParams TextureUploadVulkanParams(bool webglCompat)
{
    TextureUploadParams params;
    params.eglParameters = egl_platform::VULKAN();
    params.webgl         = webglCompat;
    return params;
}

TEST_P(TextureUploadBenchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(TextureUploadBenchmark,
                       TextureUploadD3D11Params(false),
                       TextureUploadD3D11Params(true),
                       TextureUploadOpenGLOrGLESParams(false),
                       TextureUploadOpenGLOrGLESParams(true),
                       TextureUploadVulkanParams(false),
                       TextureUploadVulkanParams(true));
}  // namespace angle
