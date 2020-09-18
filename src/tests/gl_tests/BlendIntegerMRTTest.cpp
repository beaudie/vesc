//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

class BlendIntegerMRTTest : public ANGLETest
{
  protected:
    BlendIntegerMRTTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void compareValue(const int32_t *value, const char *name, GLenum attachment)
    {
        int32_t pixel[4];
        glReadBuffer(attachment);
        glReadPixels(0, 0, 1, 1, GL_RGBA_INTEGER, GL_INT, pixel);
        for (size_t componentIdx = 0; componentIdx < 4; componentIdx++)
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
        constexpr char kFragment[] =
            "#version 300 es\n"
            "layout(location = 1) out highp vec4 o_drawBuffer1;\n"
            "layout(location = 2) out highp ivec4 o_drawBuffer2;\n"
            "layout(location = 3) out highp vec4 o_drawBuffer3;\n"
            "void main(void)\n"
            "{\n"
            "    o_drawBuffer1 = vec4(0, 0, 0, 0);\n"
            "    o_drawBuffer2 = ivec4(0, 0, 0, 0);\n"
            "    o_drawBuffer3 = vec4(0, 0, 0, 0);\n"
            "}\n";

        ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFragment);
        glUseProgram(program);

        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        GLRenderbuffer colorRenderbuffer1;
        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer1);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, getWindowWidth(), getWindowHeight());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER,
                                  colorRenderbuffer1);

        GLRenderbuffer colorRenderbuffer2;
        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer2);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA32I, getWindowWidth(), getWindowHeight());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER,
                                  colorRenderbuffer2);

        GLRenderbuffer colorRenderbuffer3;
        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer3);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, getWindowWidth(), getWindowHeight());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_RENDERBUFFER,
                                  colorRenderbuffer3);

        GLenum drawBuffers[] = {GL_NONE, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
                                GL_COLOR_ATTACHMENT3};
        glDrawBuffers(4, drawBuffers);

        const int32_t clearValue[4] = {-1, 2, -3, 4};
        glClearBufferiv(GL_COLOR, 2, clearValue);
        ASSERT_GL_NO_ERROR();
        compareValue(clearValue, "clearValue", GL_COLOR_ATTACHMENT2);

        glBlendEquation(GL_MAX);
        glEnable(GL_BLEND);

        drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
        ASSERT_GL_NO_ERROR();

        const int32_t drawValue[4] = {0, 0, 0, 0};
        compareValue(drawValue, "drawValue", GL_COLOR_ATTACHMENT2);
    }
};

// Test that blending does not cancel draws on integer attachments.
TEST_P(BlendIntegerMRTTest, Test)
{
    runTest();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES3(BlendIntegerMRTTest);
