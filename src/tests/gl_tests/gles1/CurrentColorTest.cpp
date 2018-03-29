//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CurrentColorTest.cpp: Tests basic usage of glClientActiveTexture.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "random_utils.h"

#include <stdint.h>

using namespace angle;

class CurrentColorTest : public ANGLETest
{
  protected:
    CurrentColorTest()
    {
        setWindowWidth(32);
        setWindowHeight(32);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }
};

// State query: Checks the initial state is correct.
TEST_P(CurrentColorTest, InitialState)
{
    GLfloat color[4];
    glGetFloatv(GL_CURRENT_COLOR, color);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(1.0, color[0]);
    EXPECT_EQ(1.0, color[1]);
    EXPECT_EQ(1.0, color[2]);
    EXPECT_EQ(1.0, color[3]);
}

// Set test: Checks that the current color is properly set and retrieved.
TEST_P(CurrentColorTest, Set)
{
    float epsilon = 0.00001f;

    GLfloat color[4];
    glColor4f(0.1f, 0.2f, 0.3f, 0.4f);
    EXPECT_GL_NO_ERROR();

    glGetFloatv(GL_CURRENT_COLOR, color);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(0.1f, color[0]);
    EXPECT_EQ(0.2f, color[1]);
    EXPECT_EQ(0.3f, color[2]);
    EXPECT_EQ(0.4f, color[3]);

    glColor4ub(0xff, 0x0, 0x55, 0x33);

    glGetFloatv(GL_CURRENT_COLOR, color);
    EXPECT_GL_NO_ERROR();

    EXPECT_NEAR(1.0f, color[0], epsilon);
    EXPECT_NEAR(0.0f, color[1], epsilon);
    EXPECT_NEAR(1.0f / 3.0f, color[2], epsilon);
    EXPECT_NEAR(0.2f, color[3], epsilon);

    glColor4x(0x10000, 0x0, 0x3333, 0x5555);

    glGetFloatv(GL_CURRENT_COLOR, color);
    EXPECT_GL_NO_ERROR();

    EXPECT_NEAR(1.0f, color[0], epsilon);
    EXPECT_NEAR(0.0f, color[1], epsilon);
    EXPECT_NEAR(0.2f, color[2], epsilon);
    EXPECT_NEAR(1.0f / 3.0f, color[3], epsilon);
}

ANGLE_INSTANTIATE_TEST(CurrentColorTest, ES1_D3D11(), ES1_OPENGL(), ES1_OPENGLES());
