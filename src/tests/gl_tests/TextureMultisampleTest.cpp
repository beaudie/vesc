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
                               GLboolean fixedsamplelocations,
                               int majorversion,
                               int clientversion);

    GLuint mFramebuffer = 0;
    GLuint mTexture     = 0;
};

void TextureMultisampleTest::texStorageMultisample(GLenum target,
                                                   GLint samples,
                                                   GLenum internalformat,
                                                   GLsizei width,
                                                   GLsizei height,
                                                   GLboolean fixedsamplelocations,
                                                   int majorversion,
                                                   int clientversion)
{
    if (majorversion < 3 || clientversion < 1)
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
        texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA8, 64, 64, GL_FALSE,
                              getClientMajorVersion(), getClientMinorVersion());
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
    if ((getClientMajorVersion() < 3 || getClientMinorVersion() < 1) &&
        !extensionEnabled("GL_ANGLE_texture_multisample"))
    {
        return;
    }

    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 ||
                       getClientMinorVersion() < 1 &&
                           (IsNVIDIA() && IsWindows() && IsOpenGLES() || IsAndroid()));

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mTexture);
    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, 1, GL_RGBA8, 1, 1, GL_FALSE,
                          getClientMajorVersion(), getClientMinorVersion());
    ASSERT_GL_NO_ERROR();

    GLint params = 0;
    glGetTexParameteriv(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_IMMUTABLE_FORMAT, &params);
    EXPECT_EQ(1, params);

    texStorageMultisample(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1, GL_FALSE, getClientMajorVersion(),
                          getClientMinorVersion());
    ASSERT_GL_ERROR(GL_INVALID_ENUM);

    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, 1, GL_RGBA8, 0, 0, GL_FALSE,
                          getClientMajorVersion(), getClientMinorVersion());
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    GLint maxSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, 1, GL_RGBA8, maxSize + 1, 1, GL_FALSE,
                          getClientMajorVersion(), getClientMinorVersion());
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    GLint maxSamples = 0;
    glGetInternalformativ(GL_TEXTURE_2D_MULTISAMPLE, GL_R8, GL_SAMPLES, 1, &maxSamples);
    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, maxSamples + 1, GL_RGBA8, 1, 1, GL_FALSE,
                          getClientMajorVersion(), getClientMinorVersion());
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);

    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, 0, GL_RGBA8, 1, 1, GL_FALSE,
                          getClientMajorVersion(), getClientMinorVersion());
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, 1, GL_RGBA, 0, 0, GL_FALSE,
                          getClientMajorVersion(), getClientMinorVersion());
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, 1, GL_RGBA8, 1, 1, GL_FALSE,
                          getClientMajorVersion(), getClientMinorVersion());
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);
}

// Tests the value of MAX_INTEGER_SAMPLES is no less than 1.
// [OpenGL ES 3.1 SPEC Table 20.40]
TEST_P(TextureMultisampleTestES31, MaxIntegerSamples)
{
    if ((getClientMajorVersion() < 3 || getClientMinorVersion() < 1) &&
        !extensionEnabled("GL_ANGLE_texture_multisample"))
    {
        return;
    }

    GLint maxIntegerSamples;
    glGetIntegerv(GL_MAX_INTEGER_SAMPLES, &maxIntegerSamples);
    EXPECT_GE(maxIntegerSamples, 1);
    EXPECT_NE(std::numeric_limits<GLint>::max(), maxIntegerSamples);
}

// Tests the value of MAX_COLOR_TEXTURE_SAMPLES is no less than 1.
// [OpenGL ES 3.1 SPEC Table 20.40]
TEST_P(TextureMultisampleTestES31, MaxColorTextureSamples)
{
    if ((getClientMajorVersion() < 3 || getClientMinorVersion() < 1) &&
        !extensionEnabled("GL_ANGLE_texture_multisample"))
    {
        return;
    }

    GLint maxColorTextureSamples;
    glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &maxColorTextureSamples);
    EXPECT_GE(maxColorTextureSamples, 1);
    EXPECT_NE(std::numeric_limits<GLint>::max(), maxColorTextureSamples);
}

// Tests the value of MAX_DEPTH_TEXTURE_SAMPLES is no less than 1.
// [OpenGL ES 3.1 SPEC Table 20.40]
TEST_P(TextureMultisampleTestES31, MaxDepthTextureSamples)
{
    if ((getClientMajorVersion() < 3 || getClientMinorVersion() < 1) &&
        !extensionEnabled("GL_ANGLE_texture_multisample"))
    {
        return;
    }

    GLint maxDepthTextureSamples;
    glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &maxDepthTextureSamples);
    EXPECT_GE(maxDepthTextureSamples, 1);
    EXPECT_NE(std::numeric_limits<GLint>::max(), maxDepthTextureSamples);
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
