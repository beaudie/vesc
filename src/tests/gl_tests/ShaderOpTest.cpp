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

void main()
{
    gl_Position = pos;
}
)";
    const char *fragSrc = R"(
precision mediump float;
vec4 c = vec4(1.0, 0.5, 0.5, 0.75);

void main()
{
    gl_FragColor = c;
}
)";
    ANGLE_GL_PROGRAM(program, vertSrc, fragSrc);
    glUseProgram(program);

    std::array<GLfloat, 16> attribPosData = {1, 1,  0.5, 1, -1, 1,  0.5, 1,
                                             1, -1, 0.5, 1, -1, -1, 0.5, 1};
    GLint attribPosLoc                    = glGetAttribLocation(1, "pos");
    ASSERT(attribPosLoc >= 0);
    glEnableVertexAttribArray(attribPosLoc);
    glVertexAttribPointer(attribPosLoc, 4, GL_FLOAT, GL_FALSE, 0, attribPosData.data());

    const uint16_t indices[] = {0, 1, 2, 2, 1, 3};
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, &indices[0]);
    EXPECT_PIXEL_COLOR_NEAR(64, 64, GLColor(255, 125, 125, 190), kPixelColorThreshhold);

    GLColor color(0, 0, 0, 0);
    glReadPixels(64, 64, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &color);
    ASSERT(abs(color.R - 255) <= kPixelColorThreshhold);
    ASSERT(abs(color.G - 125) <= kPixelColorThreshhold);
    ASSERT(abs(color.B - 125) <= kPixelColorThreshhold);
    ASSERT(abs(color.A - 190) <= kPixelColorThreshhold);
}

// Test to reproduce a dEQP failure on QDC:
// Simplified test from dEQP-GLES2.functional.shaders.random.all_features.fragment.12
TEST_P(ShaderOpTest, FragmentAllFeaturesOps)
{
    const char *vertSrc = R"(
attribute vec4 pos;
attribute float a_u;
attribute float a_b;
attribute float a_m;
varying mediump float u;
varying mediump float b;
varying mediump float m;

void main()
{
    gl_Position = pos;
    u = a_u;
    b = a_b;
    m = a_m;
}
)";
    const char *fragSrc = R"(
precision mediump float;
int x = (ivec2(43.0, true) * ivec2(true, true) + ivec2(-0.25, -5.75) * ivec2(1.0, -8.25)).x;
uniform mediump float w;
int v = (ivec3(float(-15) * float(-1.75), acos(float(1.0)), float(false) != float(4.75))).x;
varying mediump float u;
const bool a = (ivec2(-5.0, 9.0) - ivec2(-7.0, -3)).s > 0;
varying mediump float b;
uniform mediump int c;
uniform mediump float d;
const float e = 6.0;
const bool f = exp2((-2.875) * -0.75) < sqrt(log(float(int(194.5))));
const bool g = bvec2(ivec4(3, e, 15, e).s <= ivec3(2.0, a, 6).x, ivec4(0.75, true, e, -13).xyw.s).s;
const float h = -0.5 * (vec2(e, int(-1)).y);
const vec3 i = (vec3(6.5, 5.5, -5.5) - vec3(3.5, 6.5, -3.5)) + vec4(float(3.0), int(e), 8, float(-5.5)).rab - vec4(-6.5, 7.0, 5.5, a).gabr.sqp.pts;
int j = ((int(9.0) * int(1.0) - int(8) * int(a)));
int k = (5) * (1);
float l = float(-12 * 1 - -5 + int(e));
varying mediump float m;
const ivec2 n = ivec2(inversesqrt(log2(2.0)), a).st;
uniform mediump int o;
const float p = -2.0;
const int q = (ivec2(int(14) * int(6), int(true) - int(2))).t;
const float r = -2.375 * exp2(acos(1.0)) - sqrt(sqrt(16.0));
const bool s = vec3(q, r, 10.5).b + vec3(-3, p, 8.75).s + float(vec2(1.5, -7.75).y) <= asin(sin(-0.125 * float(0))) + 0.001;
int t = (ivec4(s, r, s, 8.0)).ptqs.p - (ivec4(-6.75, 7, q, h).y * int(float(1)));

void main()
{
    gl_FragColor = log2(log((log2(vec4(x, w, v, u)))));
    if (((bvec4(d, false, t, int(0)).y)))
        if (s)
            false;
        else
            l = r - float(q) + -0.75 * p;
    else if ((int(bool(true)) + o) >= k * n.r + (int(2.0)))
        gl_FragColor = log(log((vec4(m, l, k, 6.625) + vec4(j, e, j, c))));
    else
        gl_FragColor = vec4(-0.5, int(i == vec3(h, g, -1)), int(f) * 1, float(-5.75) + e) * vec4(float(-1), bvec4(d, c, b, a).q, 0, 5 - int(6.0)).xzwy;
}
)";
    ANGLE_GL_PROGRAM(program, vertSrc, fragSrc);
    glUseProgram(program);
    EXPECT_GL_NO_ERROR();

    std::array<GLfloat, 16> attribPosData = {1, 1,  0.5, 1, -1, 1,  0.5, 1,
                                             1, -1, 0.5, 1, -1, -1, 0.5, 1};
    GLint attribPosLoc                    = glGetAttribLocation(1, "pos");
    ASSERT(attribPosLoc >= 0);
    glEnableVertexAttribArray(attribPosLoc);
    glVertexAttribPointer(attribPosLoc, 4, GL_FLOAT, GL_FALSE, 0, attribPosData.data());
    EXPECT_GL_NO_ERROR();

    std::array<GLfloat, 4> attribUData = {2.625, 2.625, 2.625, 2.625};
    GLint attribULoc                   = glGetAttribLocation(1, "a_u");
    ASSERT(attribULoc >= 0);
    glEnableVertexAttribArray(attribULoc);
    glVertexAttribPointer(attribULoc, 1, GL_FLOAT, GL_FALSE, 0, attribUData.data());
    EXPECT_GL_NO_ERROR();

    std::array<GLfloat, 4> attribBData = {0, 0, 0, 0};
    GLint attribBLoc                   = glGetAttribLocation(1, "a_b");
    ASSERT(attribBLoc >= 0);
    glEnableVertexAttribArray(attribBLoc);
    glVertexAttribPointer(attribBLoc, 1, GL_FLOAT, GL_FALSE, 0, attribBData.data());
    EXPECT_GL_NO_ERROR();

    std::array<GLfloat, 4> attribMData = {8.75, 8.75, 8.75, 8.75};
    GLint attribMLoc                   = glGetAttribLocation(1, "a_m");
    ASSERT(attribMLoc >= 0);
    glEnableVertexAttribArray(attribMLoc);
    glVertexAttribPointer(attribMLoc, 1, GL_FLOAT, GL_FALSE, 0, attribMData.data());
    EXPECT_GL_NO_ERROR();

    const uint16_t indices[] = {0, 1, 2, 2, 1, 3};
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, &indices[0]);
    EXPECT_GL_NO_ERROR();
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ShaderOpTest);
ANGLE_INSTANTIATE_TEST_ES2(ShaderOpTest);