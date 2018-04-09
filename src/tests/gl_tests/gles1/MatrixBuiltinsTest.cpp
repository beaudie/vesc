//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// MatrixBuiltinsTest.cpp: Tests basic usage of glColor4(f|ub|x).

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "common/matrix_utils.h"
#include "random_utils.h"

#include <stdint.h>

using namespace angle;

class MatrixBuiltinsTest : public ANGLETest
{
  protected:
    MatrixBuiltinsTest()
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

TEST_P(MatrixBuiltinsTest, Rotate)
{
    float angle = 90.0f;
    float x     = 1.0f;
    float y     = 1.0f;
    float z     = 1.0f;

    angle::Mat4 testMatrix = angle::Mat4::Rotate(angle, angle::Vector3(x, y, z));

    glRotatef(angle, x, y, z);
    EXPECT_GL_NO_ERROR();

    angle::Mat4 outputMatrix;
    glGetFloatv(GL_MODELVIEW_MATRIX, outputMatrix.data());
    EXPECT_GL_NO_ERROR();

    EXPECT_TRUE(testMatrix.nearlyEqual(0.00001f, outputMatrix));
}

ANGLE_INSTANTIATE_TEST(MatrixBuiltinsTest, ES1_D3D11(), ES1_OPENGL(), ES1_OPENGLES());
