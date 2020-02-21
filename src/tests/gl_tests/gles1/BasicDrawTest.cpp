//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BasicDrawTest.cpp: Tests basic fullscreen quad draw with and without
// GL_TEXTURE_2D enabled.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include <vector>

using namespace angle;

class BasicDrawTest : public ANGLETest
{
  protected:
    BasicDrawTest()
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

    void drawRedQuad()
    {
        glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
        EXPECT_GL_NO_ERROR();
        glEnableClientState(GL_VERTEX_ARRAY);
        EXPECT_GL_NO_ERROR();
        glVertexPointer(2, GL_FLOAT, 0, mPositions.data());
        EXPECT_GL_NO_ERROR();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        EXPECT_GL_NO_ERROR();
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    }
};

// Draws a fullscreen quad with a certain color.
TEST_P(BasicDrawTest, DrawColor)
{
    drawRedQuad();
}

// Checks that textures can be enabled or disabled.
TEST_P(BasicDrawTest, EnableDisableTexture)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    // Green
    GLubyte texture[] = {
        0x00,
        0xff,
        0x00,
    };

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);

    // Texturing is disabled; still red;
    drawRedQuad();

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    // Texturing enabled; is green (provided modulate w/ white)
    glEnable(GL_TEXTURE_2D);
    EXPECT_GL_NO_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

TEST_P(BasicDrawTest, SanityCheckTest)
{
    // test glClearColorx, glClearDepthx, glLineWidthx, glPolygonOffsetx
    glClearColorx(0x4000, 0x8000, 0x8000, 0x8000);
    glClearDepthx(0x8000);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    EXPECT_PIXEL_NEAR(0, 0, 64, 128, 128, 128, 1.0);

    // Fail Depth Test and can't draw the red triangle
    std::vector<float> Positions = {-1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f};
    glEnable(GL_DEPTH_TEST);
    glLineWidthx(0x10000);
    glPolygonOffsetx(0, 0);
    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, Positions.data());
    glDrawArrays(GL_TRIANGLES, 0, 3);
    EXPECT_PIXEL_NEAR(0, 0, 64, 128, 128, 128, 1.0);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // test glDepthRangex and glGetFixedv
    glDepthRangex(0x8000, 0x10000);
    GLfixed depth_range[2];
    glGetFixedv(GL_DEPTH_RANGE, depth_range);
    EXPECT_EQ(0x8000, depth_range[0]);
    EXPECT_EQ(0x10000, depth_range[1]);

    // test glTexParameterx, glTexParameterxv and glGetTexParameterxv
    GLfixed params[4] = {};
    glTexParameterx(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glGetTexParameterxv(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, params);
    EXPECT_GL_TRUE(params[0]);

    GLfixed cropRect[4] = {0x10, 0x20, 0x10, 0x10};
    glTexParameterxv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, cropRect);
    glGetTexParameterxv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, params);
    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ(cropRect[i], params[i]);
    }

    // test glEnable, GL_MULTISAMPLE, GL_SAMPLE_COVERAGE
    GLfixed isMultisample;
    glEnable(GL_MULTISAMPLE);
    glGetFixedv(GL_MULTISAMPLE, &isMultisample);
    EXPECT_EQ(0x10000, isMultisample);
    glDisable(GL_MULTISAMPLE);
    glGetFixedv(GL_MULTISAMPLE, &isMultisample);
    EXPECT_EQ(0, isMultisample);

    GLfixed isSampleCoverage;
    GLfixed samplecoveragevalue;
    GLfixed samplecoverageinvert;
    glEnable(GL_SAMPLE_COVERAGE);
    glGetFixedv(GL_SAMPLE_COVERAGE, &isSampleCoverage);
    EXPECT_EQ(0x10000, isSampleCoverage);
    glSampleCoveragex(0x8000, true);
    glGetFixedv(GL_SAMPLE_COVERAGE_VALUE, &samplecoveragevalue);
    EXPECT_EQ(0x8000, samplecoveragevalue);
    glGetFixedv(GL_SAMPLE_COVERAGE_INVERT, &samplecoverageinvert);
    EXPECT_EQ(0x10000, samplecoverageinvert);
}

ANGLE_INSTANTIATE_TEST_ES1(BasicDrawTest);
