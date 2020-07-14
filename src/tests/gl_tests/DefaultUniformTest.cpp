//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include <array>
#include <cmath>
#include <sstream>

using namespace angle;

namespace
{

class DefaultUniformTest : public ANGLETest
{
  protected:
    DefaultUniformTest() : mProgram(0), mUniformU1Location(-1), mUniformU2Location(-1)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testSetUp() override
    {
        constexpr char kVS[] =
            "attribute vec4 position;\n"
            "uniform int u1;\n"
            "varying int v1;\n"
            "void main()\n"
            "{\n"
            "   gl_Position = position;\n"
            "}";
        constexpr char kFS[] =
            "uniform int u2;\n"
            "varying int v1;\n"
            "void main() {\n"
            "  gl_FragColor = u2<v1 ? vec4(1.0f, 0.0f, 0.0f, 1.0f) : vec4(0.0f, 1.0f, 0.0f, "
            "1.0f);\n"
            "}";

        mProgram = CompileProgram(kVS, kFS);
        ASSERT_NE(mProgram, 0u);

        mUniformU1Location = glGetUniformLocation(mProgram, "u1");
        ASSERT_NE(mUniformU1Location, -1);

        mUniformU2Location = glGetUniformLocation(mProgram, "u2");
        ASSERT_NE(mUniformU2Location, -1);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override { glDeleteProgram(mProgram); }

    GLuint mProgram;
    GLint mUniformU1Location;
    GLint mUniformU2Location;
};

TEST_P(DefaultUniformTest, DefaultUniformDifferentBuffers)
{
    glUseProgram(mProgram);
    glUniform1i(mUniformU1Location, 2);
    glUniform1i(mUniformU2Location, 1);
    glUseProgram(0);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(mProgram, "position", 0.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}
}  // namespace

ANGLE_INSTANTIATE_TEST_ES2(DefaultUniformTest);
