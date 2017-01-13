//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

class FloatingPointSurfaceTest : public ANGLETest
{
  protected:
    FloatingPointSurfaceTest()
    {
        setWindowWidth(512);
        setWindowHeight(512);
        setConfigRedBits(16);
        setConfigGreenBits(16);
        setConfigBlueBits(16);
        setConfigAlphaBits(16);
        setConfigComponentType(EGL_COLOR_COMPONENT_TYPE_FLOAT_EXT);
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        const std::string vsSource = SHADER_SOURCE(precision highp float; attribute vec4 position;

                                                   void main() { gl_Position = position; });

        const std::string fsSource =
            SHADER_SOURCE(precision highp float;

                          void main() { gl_FragColor = vec4(1.0, 2.0, 3.0, 4.0); });

        mProgram = CompileProgram(vsSource, fsSource);
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        ASSERT_GL_NO_ERROR();
    }

    virtual void TearDown()
    {
        glDeleteProgram(mProgram);

        ANGLETest::TearDown();
    }

    GLuint mProgram;
};

// Test clearing and checking the color is correct
TEST_P(FloatingPointSurfaceTest, Clearing)
{
    std::array<GLfloat, 4> clearColor = { {0.0f, 1.0f, 2.0f, 3.0f};
    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    std::array<GLfloat, 4> result = { {0.0f, 0.0f, 0.0f, 0.0f};
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_FLOAT, result.data());
    ASSERT_GL_NO_ERROR();

    ASSERT_EQ(clearColor, result);
}

// Test drawing and checking the color is correct
TEST_P(FloatingPointSurfaceTest, Drawing)
{
    glUseProgram(mProgram);
    drawQuad(mProgram, "position", 0.5f);
    std::array<GLfloat, 4> result = {{0.0f, 0.0f, 0.0f, 0.0f}};
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_FLOAT, result.data());
    ASSERT_GL_NO_ERROR();

    std::array<GLfloat, 4> expected = { {1.0f, 2.0f, 3.0f, 4.0f};
    ASSERT_EQ(expected, result);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(FloatingPointSurfaceTest,
                       ES2_D3D11(),
                       ES3_D3D11(),
                       ES2_D3D11(EGL_EXPERIMENTAL_PRESENT_PATH_FAST_ANGLE));
