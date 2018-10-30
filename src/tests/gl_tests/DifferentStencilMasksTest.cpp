//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DifferentStencilMasksTest:
//   Tests the equality between stencilWriteMask and stencilBackWriteMask.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

namespace
{
class DifferentStencilMasksTest : public ANGLETest
{
  protected:
    DifferentStencilMasksTest() : mProgram(0)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setConfigStencilBits(8);

        setWebGLCompatibilityEnabled(true);
    }

    void SetUp() override
    {
        ANGLETest::SetUp();

        mProgram = CompileProgram(essl1_shaders::vs::Zero(), essl1_shaders::fs::Blue());
        ASSERT_NE(0u, mProgram);

        glEnable(GL_STENCIL_TEST);
        ASSERT_GL_NO_ERROR();
    }

    void TearDown() override
    {
        glDisable(GL_STENCIL_TEST);
        if (mProgram != 0)
            glDeleteProgram(mProgram);

        ANGLETest::TearDown();
    }

    GLuint mProgram;
};

// Tests that effectively same front and back masks are legal.
TEST_P(DifferentStencilMasksTest, DrawWithSameEffectiveMask)
{
    // 0x00ff and 0x01ff are effectively 0x00ff by being masked by the current stencil bits, 8.
    glStencilMaskSeparate(GL_FRONT, 0x00ff);
    glStencilMaskSeparate(GL_BACK, 0x01ff);

    glUseProgram(mProgram);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    EXPECT_GL_NO_ERROR();
}

// Tests that effectively different front and back masks are illegal.
TEST_P(DifferentStencilMasksTest, DrawWithDifferentMask)
{
    glStencilMaskSeparate(GL_FRONT, 0x0001);
    glStencilMaskSeparate(GL_BACK, 0x0002);

    glUseProgram(mProgram);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

class DifferentStencilMasksWithoutStencilBufferTest : public DifferentStencilMasksTest
{
  protected:
    DifferentStencilMasksWithoutStencilBufferTest()
    {
        setConfigDepthBits(0);
        setConfigStencilBits(0);
    }
};

// Tests that effectively different front and back masks, without stencil bits, are legal.
TEST_P(DifferentStencilMasksWithoutStencilBufferTest, DrawWithDifferentMask)
{
    // The following platforms don't have an appropriate EGL config to create the window with.
    fprintf(stderr, "IsIntel()? %c, IsAMD()? %c, IsOSX()? %c, IsLinux()? %c\n",
            IsIntel() ? 'Y' : 'N',
            IsAMD() ? 'Y' : 'N',
            IsOSX() ? 'Y' : 'N',
            IsLinux() ? 'Y' : 'N');
    fprintf(stderr, "%d, %d\n", IsOSX() && (IsAMD() || IsIntel()), IsLinux() && (IsAMD() || IsIntel()));
    ANGLE_SKIP_TEST_IF(IsOSX() && (IsAMD() || IsIntel()));
    ANGLE_SKIP_TEST_IF(IsLinux() && (IsAMD() || IsIntel()));

    glStencilMaskSeparate(GL_FRONT, 0x0001);
    glStencilMaskSeparate(GL_BACK, 0x0002);

    glUseProgram(mProgram);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    EXPECT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(DifferentStencilMasksTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES3_D3D11(),
                       ES2_OPENGL(),
                       ES3_OPENGL(),
                       ES2_VULKAN());
ANGLE_INSTANTIATE_TEST(DifferentStencilMasksWithoutStencilBufferTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES3_D3D11(),
                       ES2_OPENGL(),
                       ES3_OPENGL(),
                       ES2_VULKAN());
} // anonymous namespace
