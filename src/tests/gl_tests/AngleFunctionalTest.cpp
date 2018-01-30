//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TextureMultisampleTest: Tests of multisampled texture

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class AngleFunctionalTest : public ANGLETest
{
  protected:
    AngleFunctionalTest()
    {
        setWindowWidth(64);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    std::vector<GLuint> mTextures;
};

// This test reproduce invalid heap bug when running deqp cases.
TEST_P(AngleFunctionalTest, InvalidHeapBug)
{
    int numTexture = 16;
    mTextures.resize(numTexture);
    glGenTextures(numTexture, &mTextures[0]);
    glDeleteTextures(numTexture, &mTextures[0]);

    numTexture = 14;
    mTextures.resize(numTexture);
    glGenTextures(numTexture, &mTextures[0]);
    glDeleteTextures(numTexture, &mTextures[0]);

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    ASSERT_GL_NO_ERROR();

    glGenTextures(1, &mTextures[0]);
    ASSERT_GL_NO_ERROR();

    glDeleteTextures(1, &mTextures[0]);
    ASSERT_GL_NO_ERROR();
}

ANGLE_INSTANTIATE_TEST(AngleFunctionalTest,
                       ES2_D3D9(),
                       ES3_D3D11(),
                       ES2_OPENGL(),
                       ES3_OPENGL(),
                       ES3_OPENGLES());
}
