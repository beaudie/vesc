//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "end2end_tests/ANGLETest.h"

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

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        // Vertex Shader source
        const std::string vs = SHADER_SOURCE
        (
            #version 300 es\n
            in vec4 position;

            void main()
            {
                gl_Position = position;
            }
        );

        // Fragment Shader source
        const std::string SNormFS = SHADER_SOURCE
        (
            #version 300 es\n
            uniform mediump vec2 v;
            layout(location = 0) out mediump vec4 fragColor;

            void main()
            {
                uint u = webgl_packSnorm2x16_emu(v);
                r = webgl_unpackSnorm2x16_emu(u);
                if (r.x < 0)r.x = 1.0 + r.x;
                if (r.y < 0)r.y = 1.0 + r.y;
                fragColor = vec4(r, 0.0, 1.0);
            }
        );

        // Fragment Shader source
        const std::string HalfFS = SHADER_SOURCE
        (
             //precision mediump float;
             //uniform mediump vec2 v;

             void main()
             {
                 //uint u = webgl_packHalf2x16_emu(v);
                 //vec2 r = webgl_unpackHalf2x16_emu(u);
                 //if (r.x < 0)r.x = 1.0 + r.x;
                 //if (r.y < 0)r.y = 1.0 + r.y;
                 //gl_FragColor = vec4(r, 0.0, 1.0);
                 gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);
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

    virtual void TearDown()
    {
        glDeleteTextures(1, &mOffscreenTexture2D);
        glDeleteFramebuffers(1, &mOffscreenFramebuffer);
        glDeleteProgram(mSNormProgram);
        glDeleteProgram(mHalfProgram);

        ANGLETest::TearDown();
    }


    GLuint mSNormProgram;
    GLuint mHalfProgram;
    GLuint mOffscreenFramebuffer;
    GLuint mOffscreenTexture2D;
};


TEST_P(PackUnpackTest, pack_unpack_Snorm)
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

    GLint positionLocation = glGetAttribLocation(mSNormProgram, "position");
    GLint vec2Location = glGetUniformLocation(mSNormProgram, "v");
    glUseProgram(mSNormProgram);
    glUniform2f(vec2Location, 0.5f, -0.2f);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertexLocations);
    glEnableVertexAttribArray(positionLocation);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glUseProgram(0);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(8, 8, 127, 204, 0, 255);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
// Note: we run these tests against 9_3 on WARP due to hardware driver issues on Win7
ANGLE_INSTANTIATE_TEST(PackUnpackTest, ES3_OPENGL());