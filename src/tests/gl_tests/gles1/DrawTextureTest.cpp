//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DrawTextureTest.cpp: Tests basic usage of glDrawTex*.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include <vector>

using namespace angle;

class DrawTextureTest : public ANGLETest
{
  protected:
    DrawTextureTest()
    {
        setWindowWidth(32);
        setWindowHeight(32);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    std::vector<float> mPositions = {
        -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
    };
};

// Negative test for invalid width/height values.
TEST_P(DrawTextureTest, NegativeValue)
{
    glDrawTexiOES(0, 0, 0, -1, 0);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glDrawTexiOES(0, 0, 0, 0, -1);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glDrawTexiOES(0, 0, 0, -1, -1);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Basic draw.
TEST_P(DrawTextureTest, Basic)
{
    GLuint tex;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glEnable(GL_TEXTURE_2D);

    // Green
    GLubyte texture[] = {
        0x00, 0xff, 0x00,
    };

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);

    GLint cropRect[] = {0, 0, 1, 1};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, cropRect);
    EXPECT_GL_NO_ERROR();

    GLint viewport[4] = {};
    glGetIntegerv(GL_VIEWPORT, viewport);
    EXPECT_GL_NO_ERROR();

    glDrawTexiOES(0, 0, 0, viewport[2], viewport[3]);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

ANGLE_INSTANTIATE_TEST(DrawTextureTest, ES1_D3D11(), ES1_OPENGL(), ES1_OPENGLES());
