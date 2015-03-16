//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

class PackUnpackTest : public ANGLETest
{
  protected:
    PackUnpackTest()
    {
        setWindowWidth(16);
        setWindowHeight(16);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void SetUp() override
    {
        ANGLETest::SetUp();

        // Vertex Shader source
        const std::string vs = SHADER_SOURCE
        (   #version 300 es\n
            precision mediump float;
            in vec4 position;

            void main()
            {
                gl_Position = position;
            }
        );

        // Fragment Shader source
        const std::string SNormFS = SHADER_SOURCE
        (   #version 300 es\n
            precision mediump float;
            uniform mediump vec2 v;
            layout(location = 0) out mediump vec4 fragColor;

            void main()
            {
                uint u = packSnorm2x16(v);
                vec2 r = unpackSnorm2x16(u);
                if (r.x < 0.0)r.x = 1.0 + r.x;
                if (r.y < 0.0)r.y = 1.0 + r.y;
                fragColor = vec4(r, 0.0, 1.0);
            }
        );

        // Fragment Shader source
        const std::string HalfFS = SHADER_SOURCE
        (   #version 300 es\n
            precision mediump float;
            uniform mediump vec2 v;
            layout(location = 0) out mediump vec4 fragColor;

             void main()
             {
                 uint u = packHalf2x16(v);
                 vec2 r = unpackHalf2x16(u);
                 if (r.x < 0.0)r.x = 1.0 + r.x;
                 if (r.y < 0.0)r.y = 1.0 + r.y;
                 fragColor = vec4(r, 0.0, 1.0);
             }
        );

        mSNormProgram = CompileProgram(vs, SNormFS);
        mHalfProgram = CompileProgram(vs, HalfFS);
        if (mSNormProgram == 0 || mHalfProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        glGenFramebuffers(1, &mOffscreenFramebuffer);
        glGenTextures(1, &mOffscreenTexture2D);
    }

    void TearDown() override
    {
        glDeleteTextures(1, &mOffscreenTexture2D);
        glDeleteFramebuffers(1, &mOffscreenFramebuffer);
        glDeleteProgram(mSNormProgram);
        glDeleteProgram(mHalfProgram);

        ANGLETest::TearDown();
    }

    void CompareBeforeAfter(GLuint program, float input1, float input2, int output1, int output2)
    {
        glBindTexture(GL_TEXTURE_2D, mOffscreenTexture2D);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindFramebuffer(GL_FRAMEBUFFER, mOffscreenFramebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mOffscreenTexture2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, mOffscreenFramebuffer);
        glViewport(0, 0, 16, 16);
        const GLfloat color[] = { 1.0f, 1.0f, 0.0f, 1.0f };
        glClearBufferfv(GL_COLOR, 0, color);

        GLfloat vertexLocations[] =
        {
            -1.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
        };

        GLint positionLocation = glGetAttribLocation(program, "position");
        GLint vec2Location = glGetUniformLocation(program, "v");
        glUseProgram(program);
        glUniform2f(vec2Location, input1, input2);
        glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertexLocations);
        glEnableVertexAttribArray(positionLocation);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glUseProgram(0);

        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_EQ(8, 8, output1, output2, 0, 255);
     }
    GLuint mSNormProgram;
    GLuint mHalfProgram;
    GLuint mOffscreenFramebuffer;
    GLuint mOffscreenTexture2D;
};

TEST_P(PackUnpackTest, PackUnpackSnormNormal)
{
    CompareBeforeAfter(mSNormProgram, 0.5f, -0.2f, 127, 204);
    CompareBeforeAfter(mSNormProgram, -0.35f, 0.75f, 166, 191);
    CompareBeforeAfter(mSNormProgram, 0.00392f, -0.99215f, 1, 2);
    CompareBeforeAfter(mSNormProgram, 1.0f, -0.00392f, 255, 254);
}

TEST_P(PackUnpackTest, PackUnpackHalfNormal)
{
    CompareBeforeAfter(mHalfProgram, 0.5f, -0.2f, 127, 204);
    CompareBeforeAfter(mHalfProgram, -0.35f, 0.75f, 166, 191);
    CompareBeforeAfter(mHalfProgram, 0.00392f, -0.99215f, 1, 2);
    CompareBeforeAfter(mHalfProgram, 1.0f, -0.00392f, 255, 254);
}

TEST_P(PackUnpackTest, PackUnpackSnormSubnormal)
{
    CompareBeforeAfter(mSNormProgram, 0.00001f, -0.00001f, 0, 0);
}

TEST_P(PackUnpackTest, PackUnpackHalfSubnormal)
{
    CompareBeforeAfter(mHalfProgram, 0.00001f, -0.00001f, 0, 0);
}

TEST_P(PackUnpackTest, PackUnpackSnormZero)
{
    CompareBeforeAfter(mSNormProgram, 0.00000f, -0.00000f, 0, 0);
}

TEST_P(PackUnpackTest, PackUnpackHalfZero)
{
    CompareBeforeAfter(mHalfProgram, 0.00000f, -0.00000f, 0, 0);
}

TEST_P(PackUnpackTest, PackUnpackSnormOverflow)
{
    CompareBeforeAfter(mHalfProgram, 67000.0f, -67000.0f, 255, 0);
}

TEST_P(PackUnpackTest, PackUnpackHalfOverflow)
{
    CompareBeforeAfter(mHalfProgram, 67000.0f, -67000.0f, 255, 0);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
// Note: we run these tests against 9_3 on WARP due to hardware driver issues on Win7
ANGLE_INSTANTIATE_TEST(PackUnpackTest, ES3_OPENGL());