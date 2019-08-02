//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DepthMaskTest.cpp: Test GLES functionality related to depth masks and other
// pipeline state as used by T-Rex benchmark

#include "test_utils/ANGLETest.h"

namespace angle
{
class DepthMaskTest : public ANGLETest
{
  protected:
    DepthMaskTest() : mProgram(0)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void testSetUp() override
    {
        mProgram = CompileProgram(essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        ASSERT_NE(0u, mProgram) << "shader compilation failed.";

        mColorUniform = glGetUniformLocation(mProgram, essl1_shaders::ColorUniform());
    }

    void testTearDown() override { glDeleteProgram(mProgram); }

    GLuint mProgram     = 0;
    GLint mColorUniform = -1;
};

TEST_P(DepthMaskTest, ChangeMaskBetweenDraws)
{
    int x = getWindowWidth() / 2;
    int y = getWindowHeight() / 2;

    // Clear to blue
    glClearDepthf(1.0f);
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    EXPECT_PIXEL_EQ(x, y, 0, 0, 255, 255);

    // Draw a quad with all colors masked and blending disabled, should remain blue
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glUseProgram(mProgram);
    glUniform4f(mColorUniform, 1.0f, 0.0f, 0.0f, 0.0f);
    EXPECT_GL_NO_ERROR();
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
    // EXPECT_PIXEL_EQ(x, y, 255, 0, 0, 0);

    glDepthMask(GL_FALSE);
    glUniform4f(mColorUniform, 0.0f, 1.0f, 0.0f, 0.0f);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.2f);
    // EXPECT_PIXEL_EQ(x, y, 0, 255, 0, 0);

    glDepthMask(GL_TRUE);
    glUniform4f(mColorUniform, 0.0f, 0.0f, 1.0f, 0.0f);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.4f);
    EXPECT_PIXEL_EQ(x, y, 0, 0, 255, 0);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against. D3D11 Feature Level 9_3 uses different D3D formats for vertex
// attribs compared to Feature Levels 10_0+, so we should test them separately.
ANGLE_INSTANTIATE_TEST(DepthMaskTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES2_OPENGL(),
                       ES3_OPENGL(),
                       ES2_OPENGLES(),
                       ES3_OPENGLES(),
                       ES2_VULKAN());

}  // namespace angle
