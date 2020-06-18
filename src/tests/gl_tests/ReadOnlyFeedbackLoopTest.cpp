//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Test that invokes a usecase where there is a feedback loop but the framebuffer
// attachment is only read from

#include "test_utils/ANGLETest.h"

using namespace angle;

class ReadOnlyFeedbackLoopTest : public ANGLETest
{
  protected:
    ReadOnlyFeedbackLoopTest() : mTextureFillProgram(0), mColorFillProgram(0)
    {
        setWindowWidth(256);
        setWindowHeight(256);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testSetUp() override
    {
        mColorFillProgram   = getColorFillProgram();
        mTextureFillProgram = getTexturedFillProgram();

        if (mColorFillProgram == 0 || mTextureFillProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClearDepthf(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDepthRangef(-1.0f, 1.0f);

        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override { glDeleteProgram(mColorFillProgram); }

    GLuint getColorFillProgram()
    {
        if (mColorFillProgram)
        {
            return mColorFillProgram;
        }

        mColorFillProgram = CompileProgram(essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

        return mColorFillProgram;
    }

    GLuint getTexturedFillProgram()
    {
        if (mTextureFillProgram)
        {
            return mTextureFillProgram;
        }

        constexpr char kVS[] =
            "precision highp float;"
            "attribute vec4 a_position;\n"
            "varying mediump vec2 texCoord;\n"
            "void main()\n"
            "{\n"
            "    gl_Position = a_position;\n"
            "    texCoord = a_position.xy * 0.5 + vec2(0.5);\n"
            "}\n";

        constexpr char kFS[] =
            "varying mediump vec2 texCoord;\n"
            "uniform sampler2D tex;\n"
            "void main()\n"
            "{\n"
            "    gl_FragColor = texture2D(tex, texCoord);\n"
            "}\n";

        mTextureFillProgram = CompileProgram(kVS, kFS);
        return mTextureFillProgram;
    }

    GLuint mTextureFillProgram;
    GLuint mColorFillProgram;
};

// Render to various levels of a texture and check that they have the correct color data via
// ReadPixels
TEST_P(ReadOnlyFeedbackLoopTest, DepthFeedbackLoop)
{
    const GLuint width  = getWindowWidth();
    const GLuint height = getWindowHeight();

    GLuint colorTex = 0;

    glGenTextures(1, &colorTex);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    GLuint depthTex = 0;
    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT,
                 GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    GLuint gbufferFbo = 0;
    glGenFramebuffers(1, &gbufferFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, gbufferFbo);
    EXPECT_GL_NO_ERROR();

    // Render to the levels of the texture with different colors
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    EXPECT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
    EXPECT_GL_NO_ERROR();

    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_READ_FRAMEBUFFER));

    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Enable Depth test with passing always to write depth.
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_ALWAYS);

    const GLfloat depthValue = 0.0f;
    drawQuad(mColorFillProgram, essl1_shaders::PositionAttrib(), depthValue, 0.6f);

    EXPECT_GL_NO_ERROR();

    GLuint finalTex = 0;
    glGenTextures(1, &finalTex);
    glBindTexture(GL_TEXTURE_2D, finalTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    GLuint finalFbo = 0;
    glGenFramebuffers(1, &finalFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, finalFbo);
    EXPECT_GL_NO_ERROR();

    // Enable Depth test without depth write.
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_GREATER);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, finalTex, 0);
    EXPECT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
    EXPECT_GL_NO_ERROR();
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_READ_FRAMEBUFFER));

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, depthTex);
    drawQuad(mTextureFillProgram, essl1_shaders::PositionAttrib(), 0.7f, 1.0f);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glBindTexture(GL_TEXTURE_2D, finalTex);
    drawQuad(mTextureFillProgram, essl1_shaders::PositionAttrib(), 0.0f, 1.0f);
    EXPECT_GL_NO_ERROR();

    GLint depthColorValue = (depthValue)*128 + 128;
    EXPECT_PIXEL_EQ(width / 2, height / 2, depthColorValue, depthColorValue, depthColorValue, 255);
    EXPECT_PIXEL_EQ(0, 0, 0, 0, 255, 255);

    glDeleteFramebuffers(1, &gbufferFbo);
    glDeleteFramebuffers(1, &finalFbo);
    glDeleteTextures(1, &colorTex);
    glDeleteTextures(1, &depthTex);
    glDeleteTextures(1, &finalTex);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(ReadOnlyFeedbackLoopTest);
