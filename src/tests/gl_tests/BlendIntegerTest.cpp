//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

class BlendIntegerTest : public ANGLETest
{
  protected:
    BlendIntegerTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);

        mProgram           = 0;
        mFramebuffer       = 0;
        mColorRenderbuffer = 0;
    }

    void compareValue(const uint32_t *value, const char *name)
    {
        uint32_t pixel[4];
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(0, 0, 1, 1, GL_RGBA_INTEGER, GL_UNSIGNED_INT, pixel);
        for (size_t componentIdx = 0; componentIdx < ArraySize(pixel); componentIdx++)
        {
            EXPECT_EQ(value[componentIdx], pixel[componentIdx])
                << " componentIdx=" << componentIdx << std::endl
                << " " << name << "[0]=" << value[0] << " pixel[0]=" << pixel[0] << std::endl
                << " " << name << "[1]=" << value[1] << " pixel[1]=" << pixel[1] << std::endl
                << " " << name << "[2]=" << value[2] << " pixel[2]=" << pixel[2] << std::endl
                << " " << name << "[3]=" << value[3] << " pixel[3]=" << pixel[3];
        }
    }

    void runTest()
    {
        glGenFramebuffers(1, &mFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

        glGenRenderbuffers(1, &mColorRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, mColorRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RG32UI, getWindowWidth(), getWindowHeight());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                  mColorRenderbuffer);

        const uint32_t clearValueui[4] = {5, 5, 0, 1};
        glClearBufferuiv(GL_COLOR, 0, clearValueui);

        ASSERT_GL_NO_ERROR();

        compareValue(clearValueui, "clearValueui");

        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE);

        drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);

        ASSERT_GL_NO_ERROR();

        // Enabled blending must be ignored for integer color attachment.
        const uint32_t colorValue[4] = {1, 1, 0, 1};
        compareValue(colorValue, "colorValue");
    }

    void testSetUp() override
    {
        const char *fs = R"(#version 300 es
out highp uvec4 o_drawBuffer0;
void main(void)
{
    o_drawBuffer0 = uvec4(1, 1, 1, 1);
})";

        mProgram = CompileProgram(essl3_shaders::vs::Simple(), fs);
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        glUseProgram(mProgram);
    }

    void testTearDown() override
    {
        glDeleteProgram(mProgram);
        glDeleteFramebuffers(1, &mFramebuffer);
        glDeleteRenderbuffers(1, &mColorRenderbuffer);
    }

    GLuint mProgram;

    GLuint mFramebuffer;
    GLuint mColorRenderbuffer;
};

TEST_P(BlendIntegerTest, UI32)
{
    runTest();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES3(BlendIntegerTest);
