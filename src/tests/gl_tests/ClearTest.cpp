//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

#include "Vector.h"

using namespace angle;

namespace
{

float RandomFloat()
{
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

float RandomScaledFloat(float minValue, float maxValue)
{
    return minValue + (RandomFloat() * (maxValue - minValue));
}

Vector4 RandomVec4(int seed, float minValue, float maxValue)
{
    srand(seed);
    return Vector4(RandomScaledFloat(minValue, maxValue),
                   RandomScaledFloat(minValue, maxValue),
                   RandomScaledFloat(minValue, maxValue),
                   RandomScaledFloat(minValue, maxValue));
}

class ClearTestBase : public ANGLETest
{
  protected:
    ClearTestBase() : mProgram(0)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void SetUp() override
    {
        ANGLETest::SetUp();

        mFBOs.resize(2, 0);
        glGenFramebuffers(2, mFBOs.data());

        ASSERT_GL_NO_ERROR();
    }

    void TearDown() override
    {
        glDeleteProgram(mProgram);

        if (!mFBOs.empty())
        {
            glDeleteFramebuffers(static_cast<GLsizei>(mFBOs.size()), mFBOs.data());
        }

        if (!mTextures.empty())
        {
            glDeleteTextures(static_cast<GLsizei>(mTextures.size()), mTextures.data());
        }

        ANGLETest::TearDown();
    }

    void setupDefaultProgram()
    {
        const std::string vertexShaderSource = SHADER_SOURCE
        (
            precision highp float;
            attribute vec4 position;

            void main()
            {
                gl_Position = position;
            }
        );

        const std::string fragmentShaderSource = SHADER_SOURCE
        (
            precision highp float;

            void main()
            {
                gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
            }
        );

        mProgram = CompileProgram(vertexShaderSource, fragmentShaderSource);
        ASSERT_NE(0u, mProgram);
    }

    GLuint mProgram;
    std::vector<GLuint> mFBOs;
    std::vector<GLuint> mTextures;
};

class ClearTest : public ClearTestBase {};
class ClearTestES3 : public ClearTestBase {};

// Test clearing the default framebuffer
TEST_P(ClearTest, DefaultFramebuffer)
{
    glClearColor(0.25f, 0.5f, 0.5f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_NEAR(0, 0, 64, 128, 128, 128, 1.0);
}

// Test clearing a RGBA8 Framebuffer
TEST_P(ClearTest, RGBA8Framebuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);

    GLuint texture;
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);

    EXPECT_PIXEL_NEAR(0, 0, 128, 128, 128, 128, 1.0);
}

TEST_P(ClearTest, ClearIssue)
{
    // TODO(geofflang): Figure out why this is broken on Intel OpenGL
    if (isIntel() && getPlatformRenderer() == EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE)
    {
        std::cout << "Test skipped on Intel OpenGL." << std::endl;
        return;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glClearColor(0.0, 1.0, 0.0, 1.0);
    glClearDepthf(0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);

    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB565, 16, 16);

    EXPECT_GL_NO_ERROR();

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

    EXPECT_GL_NO_ERROR();

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    setupDefaultProgram();
    drawQuad(mProgram, "position", 0.5f);

    EXPECT_PIXEL_EQ(0, 0, 0, 255, 0, 255);
}

// Requires ES3
// This tests a bug where in a masked clear when calling "ClearBuffer", we would
// mistakenly clear every channel (including the masked-out ones)
TEST_P(ClearTestES3, MaskedClearBufferBug)
{
    unsigned char pixelData[] = { 255, 255, 255, 255 };

    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);

    GLuint textures[2];
    glGenTextures(2, &textures[0]);

    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0], 0);

    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, textures[1], 0);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(0, 0, 255, 255, 255, 255);

    float clearValue[] = { 0, 0.5f, 0.5f, 1.0f };
    GLenum drawBuffers[] = { GL_NONE, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, drawBuffers);
    glColorMask(GL_TRUE, GL_TRUE, GL_FALSE, GL_TRUE);
    glClearBufferfv(GL_COLOR, 1, clearValue);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(0, 0, 255, 255, 255, 255);

    // TODO: glReadBuffer support
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, 0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[1], 0);

    //TODO(jmadill): Robust handling of pixel test error ranges
    EXPECT_PIXEL_NEAR(0, 0, 0, 127, 255, 255, 1);

    glDeleteTextures(2, textures);
}

TEST_P(ClearTestES3, BadFBOSerialBug)
{
    // First make a simple framebuffer, and clear it to green
    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);

    GLuint textures[2];
    glGenTextures(2, &textures[0]);

    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0], 0);

    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);

    float clearValues1[] = { 0.0f, 1.0f, 0.0f, 1.0f };
    glClearBufferfv(GL_COLOR, 0, clearValues1);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(0, 0, 0, 255, 0, 255);

    // Next make a second framebuffer, and draw it to red
    // (Triggers bad applied render target serial)
    GLuint fbo2;
    glGenFramebuffers(1, &fbo2);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);

    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[1], 0);

    glDrawBuffers(1, drawBuffers);

    setupDefaultProgram();
    drawQuad(mProgram, "position", 0.5f);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);

    // Check that the first framebuffer is still green.
    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);
    EXPECT_PIXEL_EQ(0, 0, 0, 255, 0, 255);

    glDeleteTextures(2, textures);
    glDeleteFramebuffers(1, &fbo2);
}

// Test that SRGB framebuffers clear to the linearized clear color
TEST_P(ClearTestES3, SRGBClear)
{
    // TODO(jmadill): figure out why this fails
    if (isIntel() && GetParam() == ES3_OPENGL())
    {
        std::cout << "Test skipped on Intel due to failures." << std::endl;
        return;
    }

    // First make a simple framebuffer, and clear it
    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);

    GLuint texture;
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_SRGB8_ALPHA8, getWindowWidth(), getWindowHeight());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);

    EXPECT_PIXEL_NEAR(0, 0, 188, 188, 188, 128, 1.0);
}

// Test that framebuffers with mixed SRGB/Linear attachments clear to the correct color for each
// attachment
TEST_P(ClearTestES3, MixedSRGBClear)
{
    // TODO(cwallez) figure out why it is broken on Intel on Mac
#if defined(ANGLE_PLATFORM_APPLE)
    if (isIntel() && getPlatformRenderer() == EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE)
    {
        std::cout << "Test skipped on Intel on Mac." << std::endl;
        return;
    }
#endif

    // TODO(jmadill): figure out why this fails
    if (isIntel() && GetParam() == ES3_OPENGL())
    {
        std::cout << "Test skipped on Intel due to failures." << std::endl;
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);

    GLuint textures[2];
    glGenTextures(2, &textures[0]);

    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_SRGB8_ALPHA8, getWindowWidth(), getWindowHeight());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0], 0);

    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, textures[1], 0);

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, drawBuffers);

    // Clear both textures
    glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, 0, 0);

    // Check value of texture0
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0], 0);
    EXPECT_PIXEL_NEAR(0, 0, 188, 188, 188, 128, 1.0);

    // Check value of texture1
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[1], 0);
    EXPECT_PIXEL_NEAR(0, 0, 128, 128, 128, 128, 1.0);
}

TEST_P(ClearTestES3, RepeatedClear)
{
    SetWindowVisible(true);

    const std::string &vertexSource =
        "#version 300 es\n"
        "in highp vec2 position;\n"
        "out highp vec2 v_coord;\n"
        "void main(void)\n"
        "{\n"
        "	gl_Position = vec4(position, 0, 1);\n"
        "   vec2 texCoord = (position * 0.5) + 0.5;\n"
        "	v_coord = texCoord;\n"
        "}\n";

    const std::string &fragmentSource =
        "#version 300 es\n"
        "in highp vec2 v_coord;\n"
        "out highp vec4 color;\n"
        "uniform sampler2D tex;\n"
        "void main()\n"
        "{\n"
        "   color = texture(tex, v_coord);\n"
        "}\n";

    mProgram = CompileProgram(vertexSource, fragmentSource);
    ASSERT_NE(0u, mProgram);

    mTextures.resize(2, 0);
    glGenTextures(2, mTextures.data());

    struct RGBA8
    {
        uint8_t R, G, B, A;
    };

    auto Vec4ToColor = [](const Vector4 &vec)
    {
        RGBA8 color;
        color.R = static_cast<uint8_t>(vec.x * 255.0f);
        color.G = static_cast<uint8_t>(vec.y * 255.0f);
        color.B = static_cast<uint8_t>(vec.z * 255.0f);
        color.A = static_cast<uint8_t>(vec.w * 255.0f);
        return color;
    };

    GLenum format = GL_RGBA8;
    const int numRowsCols = 4;
    const int cellSize = 16;
    const int fboSizes[] = { cellSize, cellSize*numRowsCols };
    const float fmtValueMin = 0.0f;
    const float fmtValueMax = 1.0f;

    for (int fboNdx = 0; fboNdx < 2; fboNdx++)
    {
        glBindTexture(GL_TEXTURE_2D, mTextures[fboNdx]);
        glTexStorage2D(GL_TEXTURE_2D, 1, format, fboSizes[fboNdx], fboSizes[fboNdx]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[fboNdx]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[fboNdx], 0);
        ASSERT_GL_NO_ERROR();

        ASSERT_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }

    // larger fbo bound -- clear to transparent black
    Vector4 transparentBlack;
    glClearBufferfv(GL_COLOR, 0, transparentBlack.data());

    glUseProgram(mProgram);
    GLint uniLoc = glGetUniformLocation(mProgram, "tex");
    ASSERT_NE(-1, uniLoc);
    glUniform1i(uniLoc, 0);
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);

    for (int cellY = 0; cellY < numRowsCols; cellY++)
    {
        for (int cellX = 0; cellX < numRowsCols; cellX++)
        {
            int seed = cellX + cellY * numRowsCols;
            const Vector4 color = RandomVec4(seed, fmtValueMin, fmtValueMax);

            glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);
            glClearBufferfv(GL_COLOR, 0, color.data());

            glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[1]);
            glViewport(cellX*cellSize, cellY*cellSize, cellSize, cellSize);

            drawQuad(mProgram, "position", 0.5f);

            // Uncommenting the EXPECT call triggers glReadPixels, which seems to make the test pass.
            //RGBA8 pixelColor = Vec4ToColor(color);
            //EXPECT_PIXEL_NEAR(cellX*cellSize + 1, cellY*cellSize + 1, pixelColor.R, pixelColor.G, pixelColor.B, pixelColor.A, 1);
            //swapBuffers();
        }
    }

    std::vector<RGBA8> pixelData(fboSizes[1] * fboSizes[1]);
    glReadPixels(0, 0, fboSizes[1], fboSizes[1], GL_RGBA, GL_UNSIGNED_BYTE, pixelData.data());

    for (int cellY = 0; cellY < numRowsCols; cellY++)
    {
        for (int cellX = 0; cellX < numRowsCols; cellX++)
        {
            int seed = cellX + cellY * numRowsCols;
            const Vector4 color = RandomVec4(seed, fmtValueMin, fmtValueMax);
            RGBA8 pixelColor = Vec4ToColor(color);

            int testN = cellX * cellSize + cellY * cellSize * numRowsCols * cellSize + cellSize * numRowsCols + 1;
            EXPECT_NEAR(pixelColor.R, pixelData[testN].R, 1);
            EXPECT_NEAR(pixelColor.G, pixelData[testN].G, 1);
            EXPECT_NEAR(pixelColor.B, pixelData[testN].B, 1);
            EXPECT_NEAR(pixelColor.A, pixelData[testN].A, 1);
        }
    }

    glViewport(0, 0, getWindowWidth(), getWindowHeight());
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    drawQuad(mProgram, "position", 0.5f);
    swapBuffers();

    ASSERT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_INSTANTIATE_TEST(ClearTest, ES2_D3D9(), ES2_D3D11(), ES3_D3D11(), ES2_OPENGL(), ES3_OPENGL());
ANGLE_INSTANTIATE_TEST(ClearTestES3, ES3_D3D11(), ES3_OPENGL());

}  // anonymous namespace
