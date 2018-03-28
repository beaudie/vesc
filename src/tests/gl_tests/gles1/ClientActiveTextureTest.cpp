//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ClientActiveTextureTest.cpp: Tests basic usage of glAlphaFunc.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "random_utils.h"

#include <stdint.h>

using namespace angle;

class ClientActiveTextureTest : public ANGLETest
{
  protected:
    ClientActiveTextureTest()
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
TEST_P(ClientActiveTextureTest, InitialState)
{
    GLenum clientActiveTexture = 0;
    glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE, (GLint *)&clientActiveTexture);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_TEXTURE0, (int)clientActiveTexture);
}

// Negative test: Checks against invalid use of glClientActiveTexture.
TEST_P(ClientActiveTextureTest, Negative)
{
    glClientActiveTexture(0);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glClientActiveTexture(GL_TEXTURE0 + 33);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Set test: Checks that GL_TEXTURE0..GL_TEXTURE[GL_MAX_TEXTURE_UNITS-1] can be set.
TEST_P(ClientActiveTextureTest, Set)
{
    GLint maxTextureUnits = 0;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTextureUnits);

    for (GLint i = 0; i < maxTextureUnits; i++)
    {
        glClientActiveTexture(GL_TEXTURE0 + i);
        EXPECT_GL_NO_ERROR();
        GLenum clientActiveTexture = 0;
        glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE, (GLint *)&clientActiveTexture);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(GL_TEXTURE0 + i, (int)clientActiveTexture);
    }
}

ANGLE_INSTANTIATE_TEST(ClientActiveTextureTest, ES1_D3D11(), ES1_OPENGL(), ES1_OPENGLES());
