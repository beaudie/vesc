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

TEST_P(MatrixBuiltinsTest, Translate)
{
    float x = 1.0f;
    float y = 1.0f;
    float z = 1.0f;

    angle::Mat4 testMatrix = angle::Mat4::Translate(angle::Vector3(x, y, z));

    glTranslatef(1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, 1.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, 1.0f);
    EXPECT_GL_NO_ERROR();

    angle::Mat4 outputMatrix;
    glGetFloatv(GL_MODELVIEW_MATRIX, outputMatrix.data());
    EXPECT_GL_NO_ERROR();

    EXPECT_TRUE(testMatrix.nearlyEqual(0.00001f, outputMatrix));
}

TEST_P(MatrixBuiltinsTest, Scale)
{
    float x = 3.0f;
    float y = 9.0f;
    float z = 27.0f;

    angle::Mat4 testMatrix = angle::Mat4::Scale(angle::Vector3(x, y, z));

    glScalef(3.0f, 3.0f, 3.0f);
    glScalef(1.0f, 3.0f, 3.0f);
    glScalef(1.0f, 1.0f, 3.0f);
    EXPECT_GL_NO_ERROR();

    angle::Mat4 outputMatrix;
    glGetFloatv(GL_MODELVIEW_MATRIX, outputMatrix.data());
    EXPECT_GL_NO_ERROR();

    EXPECT_TRUE(testMatrix.nearlyEqual(0.00001f, outputMatrix));
}

TEST_P(MatrixBuiltinsTest, Frustum)
{

    float l = -1.0f;
    float r = 1.0f;
    float b = -1.0f;
    float t = 1.0f;
    float n = 0.1f;
    float f = 1.0f;

    angle::Mat4 testMatrix = angle::Mat4::Frustum(l, r, b, t, n, f);

    glFrustumf(l, r, b, t, n, f);
    EXPECT_GL_NO_ERROR();

    angle::Mat4 outputMatrix;
    glGetFloatv(GL_MODELVIEW_MATRIX, outputMatrix.data());
    EXPECT_GL_NO_ERROR();

    EXPECT_TRUE(testMatrix.nearlyEqual(0.00001f, outputMatrix));
}

TEST_P(MatrixBuiltinsTest, Ortho)
{
    float l = -1.0f;
    float r = 1.0f;
    float b = -1.0f;
    float t = 1.0f;
    float n = 0.1f;
    float f = 1.0f;

    angle::Mat4 testMatrix = angle::Mat4::Ortho(l, r, b, t, n, f);

    glOrthof(l, r, b, t, n, f);
    EXPECT_GL_NO_ERROR();

    angle::Mat4 outputMatrix;
    glGetFloatv(GL_MODELVIEW_MATRIX, outputMatrix.data());
    EXPECT_GL_NO_ERROR();

    EXPECT_TRUE(testMatrix.nearlyEqual(0.00001f, outputMatrix));
}

TEST_P(MatrixBuiltinsTest, FrustumNegative)
{
    glFrustumf(1.0f, 1.0f, 0.0f, 1.0f, 0.1f, 1.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glFrustumf(0.0f, 1.0f, 1.0f, 1.0f, 0.1f, 1.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glFrustumf(0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glFrustumf(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glFrustumf(0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glFrustumf(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

TEST_P(MatrixBuiltinsTest, OrthoNegative)
{
    glOrthof(1.0f, 1.0f, 0.0f, 1.0f, 0.1f, 1.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glOrthof(0.0f, 1.0f, 1.0f, 1.0f, 0.1f, 1.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glOrthof(0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glOrthof(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glOrthof(0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glOrthof(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

ANGLE_INSTANTIATE_TEST(MatrixBuiltinsTest, ES1_D3D11(), ES1_OPENGL(), ES1_OPENGLES());
