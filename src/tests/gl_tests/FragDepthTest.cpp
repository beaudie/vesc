//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FragDepthTest:
//   Tests the correctness of gl_FragDepth usage.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

class FragDepthTest : public ANGLETest
{
  protected:
    void testSetUp() override
    {
        // Writes a fixed detph value and green.
        // Section 15.2.3 of the GL 4.5 specification says that conversion is not
        // done but clamping is so the output depth should be in [0.0, 1.0]
        constexpr char kFS[] =
            R"(#version 300 es
            precision highp float;
            layout(location = 0) out vec4 fragColor;
            uniform float u_depth;
            void main(){
                gl_FragDepth = u_depth;
                fragColor = vec4(0.0, 1.0, 0.0, 1.0);
            })";

        mProgram = CompileProgram(essl3_shaders::vs::Simple(), kFS);
        ASSERT_NE(0u, mProgram);

        mDepthLocation = glGetUniformLocation(mProgram, "u_depth");
        ASSERT_NE(-1, mDepthLocation);

        glBindTexture(GL_TEXTURE_2D, mColorTexture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, mDepthTexture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, 1, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorTexture,
                               0);

        ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override { glDeleteProgram(mProgram); }

    void CheckDepthWritten(float expectedDepth, float fsDepth, bool bindDepthBuffer)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
        if (bindDepthBuffer)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                                   mDepthTexture, 0);
            ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
        }
        else
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
            ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
        }
        glUseProgram(mProgram);

        // Clear to red, the FS will write green on success
        glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
        // Clear to the expected depth so it will be compared to the FS depth with
        // DepthFunc(GL_EQUAL)
        if (bindDepthBuffer)
        {
            glClearDepthf(expectedDepth);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniform1f(mDepthLocation, fsDepth);
        if (bindDepthBuffer)
        {
            glDepthFunc(GL_EQUAL);
            glEnable(GL_DEPTH_TEST);
        }

        drawQuad(mProgram, "a_position", 0.0f);
        EXPECT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    }

  private:
    GLuint mProgram;
    GLint mDepthLocation;

    GLTexture mColorTexture;
    GLTexture mDepthTexture;
    GLFramebuffer mFramebuffer;
};

// Test that writing to gl_FragDepth works
TEST_P(FragDepthTest, DepthBufferBound)
{
    CheckDepthWritten(0.5f, 0.5f, true);
}

// Test that writing to gl_FragDepth with no depth buffer works.
TEST_P(FragDepthTest, DepthBufferUnbound)
{
    // Depth test is disabled, so the expected depth should not matter.
    CheckDepthWritten(0.f, 0.5f, false);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(FragDepthTest);
ANGLE_INSTANTIATE_TEST(FragDepthTest, ES3_D3D11(), ES3_OPENGL(), ES3_OPENGLES(), ES3_METAL());
