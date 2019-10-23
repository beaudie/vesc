//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FramebuffersPerf:
//   Performance test for setting framebuffer state.
//

#include "ANGLEPerfTest.h"

#include <iostream>
#include <random>
#include <sstream>

#include "util/shader_utils.h"

namespace angle
{
constexpr unsigned int kIterationsPerStep = 256;

struct FramebuffersParams final : public RenderTestParams
{
    FramebuffersParams()
    {
        iterationsPerStep = kIterationsPerStep;

        // Common default params
        majorVersion = 3;
        minorVersion = 0;
        windowWidth  = 720;
        windowHeight = 720;

        numTextures                = 4;
        numFramebuffers            = 3;
        textureRebindFrequency     = 5;
        framebufferRebindFrequency = 3;

        webgl = false;
    }

    std::string story() const override;
    size_t numTextures;
    size_t numFramebuffers;
    size_t textureRebindFrequency;
    size_t framebufferRebindFrequency;

    bool webgl;
};

std::ostream &operator<<(std::ostream &os, const FramebuffersParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

std::string FramebuffersParams::story() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::story();
    strstr << "_" << numTextures << "_textures";
    strstr << "_" << numFramebuffers << "_framebuffers";
    strstr << "_" << textureRebindFrequency << "_tex_rebind";
    strstr << "_" << framebufferRebindFrequency << "_fb_rebind";

    if (webgl)
    {
        strstr << "_webgl";
    }

    return strstr.str();
}

class FramebuffersBenchmark : public ANGLERenderTest,
                              public ::testing::WithParamInterface<FramebuffersParams>
{
  public:
    FramebuffersBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  private:
    void initShaders();
    void initTextures();
    void initFramebuffers();

    void rotateAndBindFramebuffers();
    void rotateAndBindTextures();

    std::vector<GLuint> mTextures;
    size_t texturesOffset;

    std::vector<GLuint> mFramebuffers;
    size_t framebuffersOffset;

    GLuint mProgram;
};

FramebuffersBenchmark::FramebuffersBenchmark()
    : ANGLERenderTest("Textures", GetParam()),
      mProgram(0u),
      texturesOffset(0u),
      framebuffersOffset(0u)
{
    setWebGLCompatibilityEnabled(GetParam().webgl);
    setRobustResourceInit(GetParam().webgl);
}

void FramebuffersBenchmark::initializeBenchmark()
{
    const auto &params = GetParam();

    // Verify the uniform counts are within the limits
    GLint maxTextureUnits;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
    if (params.numTextures > static_cast<size_t>(maxTextureUnits))
    {
        FAIL() << "Texture count (" << params.numTextures << ")"
               << " exceeds maximum texture unit count: " << maxTextureUnits << std::endl;
    }

    initShaders();
    initTextures();
    initFramebuffers();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

    rotateAndBindFramebuffers();
    rotateAndBindTextures();

    ASSERT_GL_NO_ERROR();
}

void FramebuffersBenchmark::initShaders()
{
    const auto &params = GetParam();

    std::string vs =
        "#version 300\n"
        "\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(0, 0, 0, 0);\n"
        "}\n";

    std::stringstream fstrstr;
    fstrstr << "#version 300\n\n";
    for (size_t i = 0; i < params.numTextures; i++)
    {
        fstrstr << "layout (location = " << i << ") out vec4 _out_" << i << ";\n";
    }
    fstrstr << "void main()\n"
               "{\n";
    for (size_t i = 0; i < params.numTextures; i++)
    {
        fstrstr << "_out_" << i << " = vec4(0, 0, 0, 0);\n";
    }
    fstrstr << "}\n";

    printf("---\n%s\n---\n", fstrstr.str().c_str());

    mProgram = CompileProgram(vs.c_str(), fstrstr.str().c_str());
    ASSERT_NE(0u, mProgram);

    // Use the program object
    glUseProgram(mProgram);

    std::vector<GLenum> drawBuffers;
    for (size_t i = 0; i < params.numTextures; i++)
    {
        drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
    }

    glDrawBuffers(params.numTextures, drawBuffers.data());
}

void FramebuffersBenchmark::initTextures()
{
    const auto &params = GetParam();

    size_t textureSize = static_cast<size_t>(1);

    for (size_t texIndex = 0; texIndex < params.numTextures * 2; texIndex++)
    {
        GLuint tex = 0;
        glGenTextures(1, &tex);

        glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + texIndex));
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureSize, textureSize, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        mTextures.push_back(tex);
    }
}

void FramebuffersBenchmark::initFramebuffers()
{
    const auto &params = GetParam();

    for (size_t fbIndex = 0; fbIndex < params.numFramebuffers * 2; fbIndex++)
    {
        GLuint fb = 0;
        glGenFramebuffers(1, &fb);

        mFramebuffers.push_back(fb);
    }
}

void FramebuffersBenchmark::rotateAndBindTextures()
{
    texturesOffset += 1;
    if (texturesOffset >= mTextures.size())
        texturesOffset = 0;

    for (size_t i = 0; i < mTextures.size(); i++)
    {
        GLuint texid = (i + texturesOffset) % mTextures.size();

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D,
                               mTextures[texid], 0);
    }
}

void FramebuffersBenchmark::rotateAndBindFramebuffers()
{
    framebuffersOffset += 1;
    if (framebuffersOffset >= mFramebuffers.size())
        framebuffersOffset = 0;

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFramebuffers[framebuffersOffset]);
}

void FramebuffersBenchmark::destroyBenchmark()
{
    glDeleteProgram(mProgram);
}

void FramebuffersBenchmark::drawBenchmark()
{
    const auto &params = GetParam();

    for (size_t it = 0; it < params.iterationsPerStep; ++it)
    {
        if (it % params.textureRebindFrequency == 0)
        {
            rotateAndBindTextures();
        }

        if (it % params.framebufferRebindFrequency == 0)
        {
            rotateAndBindFramebuffers();
        }

        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    ASSERT_GL_NO_ERROR();
}

FramebuffersParams D3D11Params(bool webglCompat)
{
    FramebuffersParams params;
    params.eglParameters = egl_platform::D3D11_NULL();
    params.webgl         = webglCompat;
    return params;
}

FramebuffersParams D3D9Params(bool webglCompat)
{
    FramebuffersParams params;
    params.eglParameters = egl_platform::D3D9_NULL();
    params.webgl         = webglCompat;
    return params;
}

FramebuffersParams OpenGLOrGLESParams(bool webglCompat)
{
    FramebuffersParams params;
    params.eglParameters = egl_platform::OPENGL_OR_GLES_NULL();
    params.webgl         = webglCompat;
    return params;
}

FramebuffersParams VulkanParams(bool webglCompat)
{
    FramebuffersParams params;
    params.eglParameters = egl_platform::VULKAN_NULL();
    params.webgl         = webglCompat;
    return params;
}

TEST_P(FramebuffersBenchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(FramebuffersBenchmark,
                       D3D11Params(false),
                       D3D11Params(true),
                       D3D9Params(true),
                       OpenGLOrGLESParams(false),
                       OpenGLOrGLESParams(true),
                       VulkanParams(false),
                       VulkanParams(true));
}  // namespace angle
