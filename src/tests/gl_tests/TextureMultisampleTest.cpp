//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TextureMultisampleTest: Tests of multisampled texture

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{
// Sample positions of d3d standard pattern. Some of the sample positions might not the same as
// opengl.
using SamplePositionsArray                                            = std::array<float, 32>;
static constexpr std::array<SamplePositionsArray, 5> kSamplePositions = {
    {{{0.5f, 0.5f}},
     {{0.75f, 0.75f, 0.25f, 0.25f}},
     {{0.375f, 0.125f, 0.875f, 0.375f, 0.125f, 0.625f, 0.625f, 0.875f}},
     {{0.5625f, 0.3125f, 0.4375f, 0.6875f, 0.8125f, 0.5625f, 0.3125f, 0.1875f, 0.1875f, 0.8125f,
       0.0625f, 0.4375f, 0.6875f, 0.9375f, 0.9375f, 0.0625f}},
     {{0.5625f, 0.5625f, 0.4375f, 0.3125f, 0.3125f, 0.625f,  0.75f,   0.4375f,
       0.1875f, 0.375f,  0.625f,  0.8125f, 0.8125f, 0.6875f, 0.6875f, 0.1875f,
       0.375f,  0.875f,  0.5f,    0.0625f, 0.25f,   0.125f,  0.125f,  0.75f,
       0.0f,    0.5f,    0.9375f, 0.25f,   0.875f,  0.9375f, 0.0625f, 0.0f}}}};

class TextureMultisampleTest : public ANGLETest
{
  protected:
    TextureMultisampleTest()
    {
        setWindowWidth(64);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void SetUp() override
    {
        ANGLETest::SetUp();

        glGenFramebuffers(1, &mFramebuffer);
        glGenTextures(1, &mTexture);

        ASSERT_GL_NO_ERROR();
    }

    void TearDown() override
    {
        glDeleteFramebuffers(1, &mFramebuffer);
        mFramebuffer = 0;
        glDeleteTextures(1, &mTexture);
        mTexture = 0;

        ANGLETest::TearDown();
    }

    void texStorageMultisample(GLenum target,
                               GLint samples,
                               GLenum format,
                               GLsizei width,
                               GLsizei height,
                               GLboolean fixedsamplelocations);

    GLuint mFramebuffer = 0;
    GLuint mTexture     = 0;
};

void TextureMultisampleTest::texStorageMultisample(GLenum target,
                                                   GLint samples,
                                                   GLenum internalformat,
                                                   GLsizei width,
                                                   GLsizei height,
                                                   GLboolean fixedsamplelocations)
{
    if ((getClientMajorVersion() < 3 || getClientMinorVersion() < 1) &&
        extensionEnabled("GL_ANGLE_texture_multisample"))
    {
        glTexStorage2DMultisampleANGLE(target, samples, internalformat, width, height,
                                       fixedsamplelocations);
    }
    else
    {
        glTexStorage2DMultisample(target, samples, internalformat, width, height,
                                  fixedsamplelocations);
    }
}

class TextureMultisampleTestES31 : public TextureMultisampleTest
{
  protected:
    TextureMultisampleTestES31() : TextureMultisampleTest() {}
};

// Tests that if es version < 3.1 and multisample extension is unsupported,
// GL_TEXTURE_2D_MULTISAMPLE_ANGLE is not supported in GetInternalformativ.
TEST_P(TextureMultisampleTest, MultisampleTargetGetInternalFormativBase)
{
    GLint maxSamples = 0;
    glGetInternalformativ(GL_TEXTURE_2D_MULTISAMPLE, GL_R8, GL_SAMPLES, 1, &maxSamples);
    if ((getClientMajorVersion() < 3 || getClientMinorVersion() < 1) &&
        !extensionEnabled("GL_ANGLE_texture_multisample"))
    {
        ASSERT_GL_ERROR(GL_INVALID_OPERATION);
    }
    else
    {
        ASSERT_GL_NO_ERROR();
    }
}

// Tests that if es version < 3.1 and multisample extension is unsupported,
// GL_TEXTURE_2D_MULTISAMPLE_ANGLE is not supported in FramebufferTexture2D.
TEST_P(TextureMultisampleTest, MultisampleTargetFramebufferTexture2D)
{
    GLint samples = 1;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mTexture);
    if ((getClientMajorVersion() < 3 || getClientMinorVersion() < 1) &&
        !extensionEnabled("GL_ANGLE_texture_multisample"))
    {
        ASSERT_GL_ERROR(GL_INVALID_ENUM);
    }
    else
    {
        texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA8, 64, 64, GL_FALSE);
        ASSERT_GL_NO_ERROR();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           mTexture, 0);
    if ((getClientMajorVersion() < 3 || getClientMinorVersion() < 1) &&
        !extensionEnabled("GL_ANGLE_texture_multisample"))
    {
        ASSERT_GL_ERROR(GL_INVALID_OPERATION);
    }
    else
    {
        ASSERT_GL_NO_ERROR();
    }
}

// Tests basic functionality of glTexStorage2DMultisample.
TEST_P(TextureMultisampleTestES31, ValidateTextureStorageMultisampleParameters)
{
    ANGLE_SKIP_TEST_IF((getClientMajorVersion() < 3 || getClientMinorVersion() < 1) &&
                       !extensionEnabled("GL_ANGLE_texture_multisample"));

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mTexture);
    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, 1, GL_RGBA8, 1, 1, GL_FALSE);
    ASSERT_GL_NO_ERROR();

    GLint params = 0;
    glGetTexParameteriv(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_IMMUTABLE_FORMAT, &params);
    EXPECT_EQ(1, params);

    texStorageMultisample(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1, GL_FALSE);
    ASSERT_GL_ERROR(GL_INVALID_ENUM);

    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, 1, GL_RGBA8, 0, 0, GL_FALSE);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    GLint maxSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, 1, GL_RGBA8, maxSize + 1, 1, GL_FALSE);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    GLint maxSamples = 0;
    glGetInternalformativ(GL_TEXTURE_2D_MULTISAMPLE, GL_R8, GL_SAMPLES, 1, &maxSamples);
    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, maxSamples + 1, GL_RGBA8, 1, 1, GL_FALSE);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);

    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, 0, GL_RGBA8, 1, 1, GL_FALSE);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, 1, GL_RGBA, 0, 0, GL_FALSE);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, 1, GL_RGBA8, 1, 1, GL_FALSE);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);
}

// Tests the value of MAX_INTEGER_SAMPLES is no less than 1.
// [OpenGL ES 3.1 SPEC Table 20.40]
TEST_P(TextureMultisampleTestES31, MaxIntegerSamples)
{
    ANGLE_SKIP_TEST_IF((getClientMajorVersion() < 3 || getClientMinorVersion() < 1) &&
                       !extensionEnabled("GL_ANGLE_texture_multisample"));

    GLint maxIntegerSamples;
    glGetIntegerv(GL_MAX_INTEGER_SAMPLES, &maxIntegerSamples);
    EXPECT_GE(maxIntegerSamples, 1);
    EXPECT_NE(std::numeric_limits<GLint>::max(), maxIntegerSamples);
}

// Tests the value of MAX_COLOR_TEXTURE_SAMPLES is no less than 1.
// [OpenGL ES 3.1 SPEC Table 20.40]
TEST_P(TextureMultisampleTestES31, MaxColorTextureSamples)
{
    ANGLE_SKIP_TEST_IF((getClientMajorVersion() < 3 || getClientMinorVersion() < 1) &&
                       !extensionEnabled("GL_ANGLE_texture_multisample"));

    GLint maxColorTextureSamples;
    glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &maxColorTextureSamples);
    EXPECT_GE(maxColorTextureSamples, 1);
    EXPECT_NE(std::numeric_limits<GLint>::max(), maxColorTextureSamples);
}

// Tests the value of MAX_DEPTH_TEXTURE_SAMPLES is no less than 1.
// [OpenGL ES 3.1 SPEC Table 20.40]
TEST_P(TextureMultisampleTestES31, MaxDepthTextureSamples)
{
    ANGLE_SKIP_TEST_IF((getClientMajorVersion() < 3 || getClientMinorVersion() < 1) &&
                       !extensionEnabled("GL_ANGLE_texture_multisample"));

    GLint maxDepthTextureSamples;
    glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &maxDepthTextureSamples);
    EXPECT_GE(maxDepthTextureSamples, 1);
    EXPECT_NE(std::numeric_limits<GLint>::max(), maxDepthTextureSamples);
}

// The value of sample position should be equal to standard pattern on D3D.
TEST_P(TextureMultisampleTestES31, CheckSamplePositions)
{
    ANGLE_SKIP_TEST_IF(!IsD3D11());

    GLsizei maxSamples = 0;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);

    GLfloat samplePosition[2];

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFramebuffer);

    for (int sampleCount = 1; sampleCount <= maxSamples; sampleCount++)
    {
        GLTexture texture;
        size_t indexKey = static_cast<size_t>(ceil(log2(sampleCount)));
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
        glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, sampleCount, GL_RGBA8, 1, 1, GL_TRUE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                               texture, 0);
        EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
        ASSERT_GL_NO_ERROR();

        for (int sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
        {
            glGetMultisamplefv(GL_SAMPLE_POSITION, sampleIndex, samplePosition);
            EXPECT_EQ(samplePosition[0], kSamplePositions[indexKey][2 * sampleIndex]);
            EXPECT_EQ(samplePosition[1], kSamplePositions[indexKey][2 * sampleIndex + 1]);
        }
    }

    ASSERT_GL_NO_ERROR();
}

// Check pixels of multisampled texture on es 3.0 when ANGLE_texture_multisample is enabled
TEST_P(TextureMultisampleTestES31, CheckPixelColor)
{
    ANGLE_SKIP_TEST_IF(IsD3D11());
    ANGLE_SKIP_TEST_IF((getClientMajorVersion() < 3 || getClientMinorVersion() < 1) &&
                       !extensionEnabled("GL_ANGLE_texture_multisample"));

    const std::string vertexShaderSource1 =
        R"(#version 300 es
        in highp vec4 position;
        void main(void)
        {
            gl_Position = position;
        })";

    const std::string fragmentShaderSource1 =
        R"(#version 300 es
		layout(location = 0) out highp vec4 fragColor;
        void main(void)
        {
            fragColor = vec4(1.0, 0.0, 0.0, 1.0);
        })";

    const std::string vertexShaderSource2 =
        R"(#version 300 es
		in highp vec4 position;
		void main (void)
		{
			gl_Position = position;
		})";
    const std::string fragmentShaderSource2 =
        R"(#version 300 es
        #extension GL_ARB_texture_multisample : require
		layout(location = 0) out highp vec4 fragColor;
		uniform highp sampler2DMS u_sampler;
		void main (void)
		{
			fragColor = texelFetch(u_sampler, ivec2(int(floor(gl_FragCoord.x)), int(floor(gl_FragCoord.y))), 0);
		})";

    GLuint program1 = CompileProgram(vertexShaderSource1, fragmentShaderSource1);
    ASSERT_NE(0u, program1);

    GLuint program2 = CompileProgram(vertexShaderSource2, fragmentShaderSource2);
    ASSERT_NE(0u, program2);

    glUseProgram(program1);

    GLint positionLoc = glGetAttribLocation(program1, "position");
    ASSERT_NE(-1, positionLoc);

    setupQuadVertexBuffer(1.0f, 1.0f);
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    ASSERT_GL_NO_ERROR();

    // Create offscreen fbo and multisample texture as color attachment.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE_ANGLE, 2, GL_RGBA8, 1, 1, GL_TRUE);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE_ANGLE,
                           texture, 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);

    glUseProgram(program2);

    positionLoc = glGetAttribLocation(program2, "position");
    ASSERT_NE(-1, positionLoc);

    setupQuadVertexBuffer(1.0f, 1.0f);
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    ASSERT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    GLint m_sampler = glGetUniformLocation(program2, "u_sampler");
    glUniform1i(m_sampler, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    ASSERT_GL_NO_ERROR();

    if (program1 != 0)
        glDeleteProgram(program1);
    if (program2 != 0)
        glDeleteProgram(program2);

    ASSERT_GL_NO_ERROR();
}

ANGLE_INSTANTIATE_TEST(TextureMultisampleTest,
                       ES31_D3D11(),
                       ES3_OPENGL(),
                       ES3_OPENGLES(),
                       ES31_OPENGL(),
                       ES31_OPENGLES());
ANGLE_INSTANTIATE_TEST(TextureMultisampleTestES31,
                       ES3_OPENGL(),
                       ES3_OPENGLES(),
                       ES31_D3D11(),
                       ES31_OPENGL(),
                       ES31_OPENGLES());
}
