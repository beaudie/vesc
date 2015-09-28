//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

class DifferentStencilMasksTest : public ANGLETest
{
  protected:
    DifferentStencilMasksTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setConfigStencilBits(8);
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        const std::string vertexShaderSource = SHADER_SOURCE
        (
            precision highp float;
            attribute vec4 position;

            void main()
            {
                gl_Position = position;
            }
        );

        const std::string fragmentShaderSource = SHADER_SOURCE
        (
            precision highp float;

            void main()
            {
                gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
            }
        );

        mProgram = CompileProgram(vertexShaderSource, fragmentShaderSource);
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        glEnable(GL_STENCIL_TEST);
        ASSERT_GL_NO_ERROR();
    }

    virtual void TearDown()
    {
        glDisable(GL_STENCIL_TEST);
        glDeleteProgram(mProgram);

        ANGLETest::TearDown();
    }

    GLuint mProgram;
};

TEST_P(DifferentStencilMasksTest, DrawWithSameEffectiveMask)
{
    // 0x00ff and 0x01ff are effectively 0x00ff by being masked by the current stencil bits, 8.
    glStencilMaskSeparate(GL_FRONT, 0x00ff);
    glStencilMaskSeparate(GL_BACK, 0x01ff);

    glUseProgram(mProgram);
    
    drawQuad(mProgram, "position", 0.5f);

    EXPECT_GL_NO_ERROR();
}

TEST_P(DifferentStencilMasksTest, DrawWithDifferenctMask)
{
    glStencilMaskSeparate(GL_FRONT, 0x0001);
    glStencilMaskSeparate(GL_BACK, 0x0002);

    glUseProgram(mProgram);

    drawQuad(mProgram, "position", 0.5f);

    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

class DifferentStencilMasksWithoutStencilBufferTest : public ANGLETest
{
protected:
    DifferentStencilMasksWithoutStencilBufferTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(0);
        setConfigStencilBits(0);
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        const std::string vertexShaderSource = SHADER_SOURCE
            (
            precision highp float;
        attribute vec4 position;

        void main()
        {
            gl_Position = position;
        }
        );

        const std::string fragmentShaderSource = SHADER_SOURCE
            (
            precision highp float;

        void main()
        {
            gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
        }
        );

        mProgram = CompileProgram(vertexShaderSource, fragmentShaderSource);
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        glEnable(GL_STENCIL_TEST);
        ASSERT_GL_NO_ERROR();
    }

    virtual void TearDown()
    {
        glDisable(GL_STENCIL_TEST);
        glDeleteProgram(mProgram);

        ANGLETest::TearDown();
    }

    GLuint mProgram;
};

TEST_P(DifferentStencilMasksWithoutStencilBufferTest, DrawWithDifferenctMask)
{
    glStencilMaskSeparate(GL_FRONT, 0x0001);
    glStencilMaskSeparate(GL_BACK, 0x0002);

    glUseProgram(mProgram);

    drawQuad(mProgram, "position", 0.5f);

    EXPECT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_INSTANTIATE_TEST(DifferentStencilMasksTest, ES2_D3D9(), ES2_D3D11(), ES3_D3D11(), ES2_OPENGL(), ES3_OPENGL());
ANGLE_INSTANTIATE_TEST(DifferentStencilMasksWithoutStencilBufferTest, ES2_D3D9(), ES2_D3D11(), ES3_D3D11(), ES2_OPENGL(), ES3_OPENGL());
