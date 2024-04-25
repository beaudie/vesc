//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Tests for shader interpolation qualifiers
//

#include "common/mathutil.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

constexpr int kPixelColorThreshhold = 8;

class ShaderOpTest : public ANGLETest<>
{
  protected:
    ShaderOpTest() : ANGLETest()
    {
        setWindowWidth(256);
        setWindowHeight(256);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setConfigStencilBits(8);
        setMultisampleEnabled(0);
    }
};

// Test to reproduce a dEQP failure on QDC:
// Simplified test from dEQP-GLES2.functional.fragment_ops.interaction.basic_shader.22
TEST_P(ShaderOpTest, FragmentOps)
{
    const char *vertSrc = R"(
attribute vec4 pos;
bvec3 e = (bvec3(1, false, 1.0));
const bool f = int(-4) == int(true);
attribute float g;
uniform mediump float h;
float i = (float(true));
attribute vec3 j;

void main()
{
    gl_Position = pos;
    if (j == vec3(i, h, g))
        e = (bvec3(false, g, f));
    else
        12;
    e = (bvec3(1.0, f, 0.5));
}
)";
    const char *fragSrc = R"(
precision mediump float;
vec4 d = vec4(11.5, 8.5, -1.5, -8.5) - vec4(6.5, 7, 1, -6);
const vec4 c = vec4(-4.0, -1, 3, 3.25);
int a = int(float(5.75));
float b = float(5.0) * 1.5;

void main()
{
    gl_FragColor = d + c;
    if (float(b) <= float(a) + 0.001)
        gl_FragColor = (gl_FragColor);
}
)";
    ANGLE_GL_PROGRAM(program, vertSrc, fragSrc);
    glUseProgram(program);

    std::array<GLfloat, 12> attribJData = {0.5, 2.75, -1.5, 6, 2.75, -0.5, 0.5, 6, -0.5, 6, 6, 0.5};
    GLint attribJLoc                    = glGetAttribLocation(1, "j");
    ASSERT(attribJLoc >= 0);
    glEnableVertexAttribArray(attribJLoc);
    glVertexAttribPointer(attribJLoc, 3, GL_FLOAT, GL_FALSE, 0, attribJData.data());

    std::array<GLfloat, 4> attribGData = {0.5, 0, 0.5, 0};
    GLint attribgLoc                   = glGetAttribLocation(1, "g");
    ASSERT(attribgLoc >= 0);
    glEnableVertexAttribArray(attribgLoc);
    glVertexAttribPointer(attribgLoc, 1, GL_FLOAT, GL_FALSE, 0, attribGData.data());

    std::array<GLfloat, 16> attribPosData = {1, 1,  0.5, 1, -1, 1,  0.5, 1,
                                             1, -1, 0.5, 1, -1, -1, 0.5, 1};
    GLint attribPosLoc                    = glGetAttribLocation(1, "pos");
    ASSERT(attribPosLoc >= 0);
    glEnableVertexAttribArray(attribPosLoc);
    glVertexAttribPointer(attribPosLoc, 4, GL_FLOAT, GL_FALSE, 0, attribPosData.data());

    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glStencilMask(4294967295);
    glClearColor(0, 0.250122, 0.500244, 1);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    GLint uniformLoc                   = glGetUniformLocation(1, "h");
    std::array<GLfloat, 1> uniformData = {4.75};
    glUniform1fv(uniformLoc, 1, uniformData.data());
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    const uint16_t indices[] = {0, 1, 2, 2, 1, 3};
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, &indices[0]);
    EXPECT_PIXEL_COLOR_NEAR(64, 64, GLColor(255, 125, 125, 190), kPixelColorThreshhold);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ShaderOpTest);
ANGLE_INSTANTIATE_TEST_ES2(ShaderOpTest);