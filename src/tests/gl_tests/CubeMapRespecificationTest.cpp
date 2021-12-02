//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

class CubeMapRespecificationTest : public ANGLETest
{
  protected:
    CubeMapRespecificationTest()
    {
        setWindowWidth(256);
        setWindowHeight(256);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testSetUp() override
    {
        mProgram = CompileProgram(essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mColorLocation = glGetUniformLocation(mProgram, essl1_shaders::ColorUniform());

        glUseProgram(mProgram);

        glClearColor(0, 0, 0, 0);
        glClearDepthf(0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override { glDeleteProgram(mProgram); }

    GLuint mProgram;
    GLint mColorLocation;
};

// Verify that re-uploading and respecifying storage doesn't crash.
TEST_P(CubeMapRespecificationTest, RedefineX)
{
    std::vector<uint8_t> imageData(26384);
    for (size_t i = 0; i < imageData.size(); ++i)
    {
        imageData[i] = 0x41;
    }

    GLuint texture;
    EXPECT_GL_NO_ERROR();
    glGenTextures(1, &texture);
    EXPECT_GL_NO_ERROR();
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    EXPECT_GL_NO_ERROR();
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB8, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 imageData.data());
    EXPECT_GL_NO_ERROR();
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB8, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 imageData.data());
    EXPECT_GL_NO_ERROR();
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB8, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 imageData.data());
    EXPECT_GL_NO_ERROR();
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB8, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 imageData.data());
    EXPECT_GL_NO_ERROR();
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB8, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 imageData.data());
    EXPECT_GL_NO_ERROR();
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB8, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 imageData.data());
    EXPECT_GL_NO_ERROR();
    glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 1, 5, 8, 8, GL_RGB, GL_UNSIGNED_BYTE,
                    imageData.data());
    EXPECT_GL_NO_ERROR();
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGB8, 16, 16);
    glFinish();
    EXPECT_GL_NO_ERROR();
    // The problematic upload
    glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 7, 6, 8, 8, GL_RGB, GL_UNSIGNED_BYTE,
                    imageData.data());
    printf("Finishing problem upload\n");
    glFinish();
    printf("Finishing problem upload (done)\n");
    EXPECT_GL_NO_ERROR();
    glFinish();
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glFinish();
    EXPECT_GL_NO_ERROR();

    glDeleteTextures(1, &texture);
    EXPECT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES3(CubeMapRespecificationTest);
