//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CurrentTextureCoordsTest.cpp: Tests basic usage of glMultiTexCoord4(f|x).

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "random_utils.h"

#include "common/vector_utils.h"
#include "libANGLE/GLES1State.h"

#include <stdint.h>

using namespace angle;

class CurrentTextureCoordsTest : public ANGLETest
{
  protected:
    CurrentTextureCoordsTest()
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
TEST_P(CurrentTextureCoordsTest, InitialState)
{
    GLint maxUnits = 0;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxUnits);
    EXPECT_GL_NO_ERROR();

    const gl::TextureCoordF kZero = {0.0f, 0.0f, 0.0f, 0.0f};
    gl::TextureCoordF actualTexCoord;

    for (GLint i = 0; i < maxUnits; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        EXPECT_GL_NO_ERROR();
        glGetFloatv(GL_CURRENT_TEXTURE_COORDS, (float *)&actualTexCoord);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(kZero, actualTexCoord);
    }
}

// Checks that errors are generated if the texture unit specified is invalid.
TEST_P(CurrentTextureCoordsTest, Negative)
{
    GLint maxUnits = 0;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxUnits);
    EXPECT_GL_NO_ERROR();

    glMultiTexCoord4f(GL_TEXTURE0 - 1, 1.0f, 0.0f, 0.0f, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glMultiTexCoord4f(GL_TEXTURE0 + maxUnits, 1.0f, 0.0f, 0.0f, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    const gl::TextureCoordF kZero = {0.0f, 0.0f, 0.0f, 0.0f};
    gl::TextureCoordF actualTexCoord;
    glGetFloatv(GL_CURRENT_TEXTURE_COORDS, (float *)&actualTexCoord);
    EXPECT_EQ(kZero, actualTexCoord);
}

TEST_P(CurrentTextureCoordsTest, Set)
{
    float epsilon  = 0.00001f;
    GLint maxUnits = 0;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxUnits);
    EXPECT_GL_NO_ERROR();

    gl::TextureCoordF actualTexCoord;

    for (int i = 0; i < maxUnits; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glMultiTexCoord4f(GL_TEXTURE0 + i, 0.1f, 0.2f, 0.3f, 0.4f);
        glGetFloatv(GL_CURRENT_TEXTURE_COORDS, (float *)&actualTexCoord);
        EXPECT_EQ(gl::TextureCoordF(0.1f, 0.2f, 0.3f, 0.4f), actualTexCoord);

        glMultiTexCoord4x(GL_TEXTURE0 + i, 0x10000, 0x0, 0x3333, 0x5555);
        glGetFloatv(GL_CURRENT_TEXTURE_COORDS, (float *)&actualTexCoord);
        EXPECT_NEAR(1.0f, actualTexCoord.s, epsilon);
        EXPECT_NEAR(0.0f, actualTexCoord.t, epsilon);
        EXPECT_NEAR(0.2f, actualTexCoord.r, epsilon);
        EXPECT_NEAR(1.0f / 3.0f, actualTexCoord.q, epsilon);
    }
}

ANGLE_INSTANTIATE_TEST(CurrentTextureCoordsTest, ES1_D3D11(), ES1_OPENGL(), ES1_OPENGLES());
